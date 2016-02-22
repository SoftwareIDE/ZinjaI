#include "CustomTools.h"
#include "mxUtils.h"
#include "mxOutputView.h"
#include "mxMainWindow.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "Parser.h"
#include "mxCompiler.h"
#include "raii.h"

CustomToolsPack::CustomToolsPack(int _cant) :tools(new OneCustomTool[_cant]),cant(_cant) {
	
}

CustomToolsPack::CustomToolsPack(const CustomToolsPack &other):tools(new OneCustomTool[other.cant]),cant(other.cant) {
	for(int i=0;i<cant;i++) tools[i]=other.tools[i];
}

CustomToolsPack::~CustomToolsPack() {
	delete [] tools;
}

void CustomToolsPack::ParseConfigLine (const wxString & key, const wxString & value) {
	int p=key.Len()-1; while(p>0 && key[p-1]>='0'&&key[p-1]<='9') p--;
	if (p<=0) return;
	long l=0;
	if (!key.Mid(p).ToLong(&l)) return;
	if (l<0||l>=cant) return;
	wxString mkey=key.Mid(0,p-1);
	if (mkey=="name") tools[l].name=value;
	else if (mkey=="command") tools[l].command=value;
	else if (mkey=="workdir") tools[l].workdir=value;
	else if (mkey=="async_exec") tools[l].async_exec=mxUT::IsTrue(value);
	else if (mkey=="pre_action") mxUT::ToInt(value,tools[l].pre_action);
	else if (mkey=="post_action") mxUT::ToInt(value,tools[l].post_action);
	else if (mkey=="output_mode") mxUT::ToInt(value,tools[l].output_mode);
	else if (mkey=="on_toolbar") tools[l].on_toolbar=mxUT::IsTrue(value);
}

void CustomToolsPack::WriteConfig (wxTextFile & file) {
	for(int i=0;i<cant;i++) { 
		if (tools[i].command.IsEmpty()) continue;
		file.AddLine(wxString("name_")<<i<<"="<<tools[i].name);
		file.AddLine(wxString("command_")<<i<<"="<<tools[i].command);
		file.AddLine(wxString("workdir_")<<i<<"="<<tools[i].workdir);
		file.AddLine(wxString("pre_action_")<<i<<"="<<tools[i].pre_action);
		file.AddLine(wxString("post_action_")<<i<<"="<<tools[i].post_action);
		file.AddLine(wxString("output_mode_")<<i<<"="<<tools[i].output_mode);
		file.AddLine(wxString("async_exec_")<<i<<"="<<(tools[i].async_exec?"1":"0"));
		file.AddLine(wxString("on_toolbar_")<<i<<"="<<(tools[i].on_toolbar?"1":"0"));
	}
}


void CustomToolsPack::Run (int i) {
	if (tools[i].command.IsEmpty()) {
		mxMessageDialog(main_window,LANG(MAINW_CUSTOM_TOOL_UNDEFINED,"Esta herramienta no esta correctamente configurada.\nUtilice el comando \"Configuracion\" del submenu \"Herramientas Personalizadas\" del menu \"Herramientas\"."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		return;
	}
	if (tools[i].pre_action==CT_PRE_COMPILE) {
		_LAMBDA_1( lmbRunTool, OneCustomTool *,tool, { new mxCustomToolProcess(*tool); } );
		if (project) compiler->BuildOrRunProject(false, new lmbRunTool(&(tools[i])) );
		else main_window->CompileSource( false, new lmbRunTool(&(tools[i])) );
	} else 
		new mxCustomToolProcess(tools[i]);
}

mxCustomToolProcess::mxCustomToolProcess(const OneCustomTool &_tool) : tool(_tool), output_view(nullptr) {
	
	wxString cmd=tool.command;
	
	switch (tool.pre_action) {
		case CT_PRE_SAVE_PROJECT: 
			if (project) project->Save();
			// salteo el break adrede
		case CT_PRE_SAVE_ALL: {
			for (int i=0,j=main_window->notebook_sources->GetPageCount();i<j;i++) {
				mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(i));
				if (source->GetModify() && !source->sin_titulo) {
					source->SaveSource();
					parser->ParseSource(source,true);
				}
			}
			mxSource *src=main_window->GetCurrentSource(); if (!src||!src->sin_titulo) break; // else salteo el break adrede
		}
		case CT_PRE_SAVE_ONE: {
			mxSource *src=main_window->GetCurrentSource();
			if (src) src->SaveSourceForSomeTool();
		}
	}
	
	wxString name=tool.name; 
	if (!name.Len()) name=" "; name.Replace("\"","\\\"");
	
	wxString project_path, project_bin, bin_workdir, current_source, current_dir, temp_dir, args;
	mxSource *src=main_window->GetCurrentSource();
	if (src) {
		current_source=src->GetFullPath();
		current_dir=src->GetPath();
		if (current_dir.EndsWith("\\")||current_dir.EndsWith("/")) current_dir.RemoveLast();
		bin_workdir=src->working_folder.GetFullPath();
		project_bin=src->GetBinaryFileName().GetFullPath();
		project_path=current_dir;
		temp_dir=src->temp_filename.GetPath();
		args=src->exec_args;
	}
	if (project) {
		temp_dir=DIR_PLUS_FILE(project->path,project->active_configuration->temp_folder);
		project_path = project->path;
		project_bin = project->active_configuration->output_file;
		project_bin.Replace("${TEMP_DIR}",temp_dir);
		project_bin = DIR_PLUS_FILE(project->path,project_bin);
		bin_workdir=DIR_PLUS_FILE(project->path,project->active_configuration->working_folder);
		args=project->active_configuration->args;
	}
	if (bin_workdir.EndsWith("\\")||bin_workdir.EndsWith("/")) bin_workdir.RemoveLast();
	if (project_path.EndsWith("\\")||project_path.EndsWith("/")) project_path.RemoveLast();
	if (temp_dir.EndsWith("\\")||temp_dir.EndsWith("/")) temp_dir.RemoveLast();
	
	cmd.Replace("${ARGS}",args);
	cmd.Replace("${BIN_WORKDIR}",bin_workdir);
	cmd.Replace("${CURRENT_SOURCE}",current_source);
	cmd.Replace("${CURRENT_DIR}",current_dir);
	cmd.Replace("${PROJECT_PATH}",project_path);
	cmd.Replace("${PROJECT_BIN}",project_bin);
	cmd.Replace("${TEMP_DIR}",temp_dir);
	cmd.Replace("${MINGW_DIR}",current_toolchain.mingw_dir);
	if (config->Files.browser_command.Len())
		cmd.Replace("${BROWSER}",config->Files.browser_command);
	else {
#ifdef __WIN32__
		mxUT::ParameterReplace(cmd,"${BROWSER}",DIR_PLUS_FILE(config->zinjai_bin_dir,"shellexecute.exe"));
#else
		cmd.Replace("${BROWSER}","xdg-open");
#endif
	}
#ifdef __WIN32__
	mxUT::ParameterReplace(cmd,"${OPEN}",DIR_PLUS_FILE(config->zinjai_bin_dir,"shellexecute.exe"));
#else
	cmd.Replace("${OPEN}","xdg-open");
#endif
	cmd.Replace("${ZINJAI_DIR}",config->zinjai_dir);
	
	wxString workdir = tool.workdir;
	if (workdir.Len()) {
		workdir.Replace("${TEMP_DIR}",temp_dir);
		workdir.Replace("${BIN_WORKDIR}",bin_workdir);
		workdir.Replace("${CURRENT_DIR}",current_dir);
		workdir.Replace("${PROJECT_PATH}",project_path);
		workdir.Replace("${MINGW_DIR}",current_toolchain.mingw_dir);
		workdir.Replace("${ZINJAI_DIR}",config->zinjai_dir);
	} else 
		workdir=project?project_path:current_dir;
	
#ifdef __WIN32__
	workdir.Replace("/","\\");
#else
	workdir.Replace("\\","/");
#endif
	
	int exec_flags = wxEXEC_MAKE_GROUP_LEADER | (tool.async_exec?wxEXEC_ASYNC:wxEXEC_SYNC);
	
	if (tool.output_mode==CT_OUTPUT_DIALOG) {
		exec_flags = wxEXEC_MAKE_GROUP_LEADER|wxEXEC_ASYNC;
		SetOutputView(new mxOutputView(name));
#ifdef __WIN32__
	} else if (tool.output_mode==CT_OUTPUT_HIDDEN) {
		Redirect();
#endif
	} else if (tool.output_mode==CT_OUTPUT_TERMINAL || tool.output_mode==CT_OUTPUT_TERMINAL_WAIT) {
		exec_flags |= wxEXEC_NOHIDE;
		if (tool.output_mode==CT_OUTPUT_TERMINAL_WAIT) cmd = mxUT::GetRunnerBaseCommand(WKEY_ALWAYS) << ". " << cmd;
#ifndef __WIN32__
		wxString term_cmd = config->Files.terminal_command;
		term_cmd.Replace("${TITLE}",name,true);
		cmd = term_cmd+" "+cmd;
		
#endif
	}
	
	_IF_DEBUGMODE(cmd);
	RaiiWorkDirChanger cwd_guard(workdir); // set temp cwd
	int pid = wxExecute(cmd,exec_flags,this);
	cwd_guard.RestoreNow();
	
	if (tool.output_mode==CT_OUTPUT_DIALOG) output_view->Launched(this,pid);
	else if (!tool.async_exec) OnTerminate(0,pid);
}

void mxCustomToolProcess::OnTerminate (int pid, int status) {
	switch (tool.post_action) {
		case CT_POST_RELOAD_ALL: {
			for (int i=0,j=main_window->notebook_sources->GetPageCount();i<j;i++) {
				mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(i));
				if (!source->sin_titulo) source->UserReload();
			}
			mxSource *src=main_window->GetCurrentSource(); 
			if (!src||!src->sin_titulo) break; 
			// else salteo el break adrede
		}
		case CT_POST_RELOAD_ONE: {
			mxSource *src=main_window->GetCurrentSource();
			if (src) src->UserReload();
			break;
		}
		case CT_POST_RUN: {
			wxCommandEvent evt;
			main_window->OnRunRun(evt);
			break;
		}
		case CT_POST_DEBUG: {
			wxCommandEvent evt;
			main_window->OnDebugRun(evt);
			break;
		}
	}
	if (output_view) output_view->OnProcessTerminateCommon(status);
	wxProcess::OnTerminate(pid,status); // so this will be self-deleted
}

void mxCustomToolProcess::SetOutputView (mxOutputView * _output_view) {
	Redirect(); output_view=_output_view;
}

int CustomToolsPack::GetFreeSpot ( ) {
	for(int i=0;i<cant;i++)
		if (tools[i].command.IsEmpty()) return i;
	return -1;
}

