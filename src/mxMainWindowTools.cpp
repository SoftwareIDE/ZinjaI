#include <wx/menu.h>
#include <wx/string.h>
#include <wx/ffile.h>
#include "mxMainWindow.h"
#include "ProjectManager.h"
#include "mxCustomTools.h"
#include "mxHelpWindow.h"
#include "ConfigManager.h"
#include "ids.h"
#include "Language.h"
#include "mxMessageDialog.h"
#include "mxSource.h"
#include "Parser.h"
#include "parserData.h"
#include "mxOutputView.h"
#include "mxProjectStatistics.h"
#include "mxCompiler.h"
#include "mxTextDialog.h"
#include "mxStatusBar.h"
#include "mxWxfbInheriter.h"
#include "mxDiffWindow.h"
#include "mxDiffSideBar.h"
#include "mxDoxyDialog.h"
#include "mxOSD.h"
#include "mxShareSourceWindow.h"
#include "mxOpenSharedWindow.h"
#include "mxDrawClasses.h"
#include "mxFlowWindow.h"
#include "mxExeInfo.h"
#include "mxColoursEditor.h"
#include "mxListSharedWindow.h"
#include "mxComplementInstallerWindow.h"
#include "mxCppCheckConfigDialog.h"
#include "mxValgrindConfigDialog.h"
#include "Toolchain.h"
#include "mxNewWizard.h"
#include <fstream>
#include "mxArt.h"
#include "mxMakefileDialog.h"
#include <wx/file.h>
#include <wx/textfile.h>
#include "mxGprofOutput.h"
#include "execution_workaround.h"
#include "mxWxfbConfigDialog.h"
#include "MenusAndToolsConfig.h"
using namespace std;

/// @brief Muestra el cuadro de configuración de cppcheck (mxCppCheckConfigDialog)
void mxMainWindow::OnToolsCppCheckConfig(wxCommandEvent &event) {
	if (project) new mxCppCheckConfigDialog(this);
}

/// @brief Lanza cppcheck sobre los fuentes del proyecto en una mxOutputWindow
void mxMainWindow::OnToolsCppCheckRun(wxCommandEvent &event) {
	if (!config->CheckCppCheckPresent()) return;
	if (!project && !notebook_sources->GetPageCount()) return;
	mxOutputView *cppcheck = new mxOutputView("CppCheck",mxOV_EXTRA_NULL,"","",mxVO_CPPCHECK,DIR_PLUS_FILE(config->temp_dir,"cppcheck.out"));
	
	wxString file_args, cppargs, toargs, extra_args, path;
	
	if (project) {
		
		// files
		project->SaveAll(false);
		if (!project->cppcheck) project->cppcheck=new cppcheck_configuration;
		wxArrayString files,exclude_list;
		mxUT::Split(project->cppcheck->exclude_list,exclude_list,true,false);
		project->GetFileList(files,FT_SOURCE,true);
		wxString list(DIR_PLUS_FILE(config->temp_dir,"cppcheck.lst"));
		wxFile flist(list,wxFile::write);
		char el='\n';
		for (unsigned int i=0;i<files.GetCount();i++) {
			if (exclude_list.Index(files[i])==wxNOT_FOUND) {
				flist.Write(/*mxUT::Quotize(*/DIR_PLUS_FILE(project->path,files[i])/*)*/);
				flist.Write((void*)&el,1);
			}
		}
		flist.Close();
		file_args=wxString("--file-list=")<<mxUT::Quotize(list);

		project->AnalizeConfig(project->path,true,current_toolchain.mingw_dir,true);
		toargs=project->cpp_compiling_options;
		
		// extra_args
		if (project->cppcheck->copy_from_config)
			extra_args<<mxUT::Split(project->cppcheck->config_d,"-D")<<" "<<mxUT::Split(project->cppcheck->config_u,"-U");
		
		// cppargs
		cppargs<<mxUT::Split(project->cppcheck->style,"--enable=")<<" ";
		cppargs<<mxUT::Split(project->cppcheck->platform,"--platform=")<<" ";
		cppargs<<mxUT::Split(project->cppcheck->standard,"--std=")<<" ";
		cppargs<<mxUT::Split(project->cppcheck->suppress_ids,"--suppress=")<<" ";
		if (project->cppcheck->suppress_file.Len()) cppargs<<"--suppressions_list="<<mxUT::Quotize(DIR_PLUS_FILE(project->path,project->cppcheck->suppress_file))<<" ";
		if (project->cppcheck->inline_suppr) cppargs<<"--inline-suppr ";
		
		// path
		path=project->path;
		
	} else {
		
		mxSource *src = CURRENT_SOURCE;
		
		//files
		file_args = mxUT::Quotize(src->SaveSourceForSomeTool());
		
		// toargs
		toargs=src->GetCompilerOptions(true);
		
		// cppargs
		cppargs="--enable=all --inline-suppr";
		
		// path
		path=src->working_folder.GetFullPath();
		
	}
	
	// args, lo que sale de las opciones de compilacion
	wxString args; // -D..., -I...., -U....
	wxArrayString array;
	mxUT::Split(toargs,array,false,true);
	for (unsigned int i=0;i<array.GetCount();i++) {
		if ((!project || project->cppcheck->copy_from_config) && array[i].StartsWith("-D")) {
			if (array[i].Len()==2) {
				args<<" -D "<<array[++i];
			} else {
				args<<" -D "<<array[i].Mid(2);
			}
		} else if ((!project || project->cppcheck->copy_from_config) && array[i].StartsWith("\"-D")) {
			if (array[i]=="\"-D\"")
				args<<" -D "<<array[++i];
			else
				args<<" -D \""<<array[i].Mid(3);
		} else if (array[i].StartsWith("-I")) {
			if (array[i].Len()==2) {
				args<<" -I "<<array[++i];
			} else {
				args<<" -I "<<array[i].Mid(2);
			}
		} else if (array[i].StartsWith("\"-I")) {
			if (array[i]=="\"-I\"")
				args<<" -I "<<array[++i];
			else
				args<<" -I \""<<array[i].Mid(3);
		}
	}
	if (extra_args.Len()) args<<" "<<extra_args;
	
	wxString command = mxUT::Quotize(config->Files.cppcheck_command)<<" "<<cppargs<<" --template \"[{file}:{line}] ({severity},{id}) {message}\" "<<args<<" "<<file_args;
	cppcheck->Launch(path,command);
	
}

/// @brief Despliega el panel de valgrind para mostrar los resultados de CppCheck
void mxMainWindow::OnToolsCppCheckView(wxCommandEvent &event) {
	ShowValgrindPanel(mxVO_CPPCHECK,DIR_PLUS_FILE(config->temp_dir,"cppcheck.out"));
}

void mxMainWindow::OnToolsCppCheckHelp(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("cppcheck.html");	
}

void mxMainWindow::OnToolsProjectStatistics(wxCommandEvent &evt) {
	if (project) new mxProjectStatistics(this);
}

#if !defined(_WIN32) && !defined(__WIN32__)
/**
* @brief Prepara una corrida para ejecutarse con Valgrind
*
* Coloca el comando de valgrind en compiler->valgrind_cmd, que es lo que se va a 
* usar al preparar la ejecucion. Despues llama a Ejecutar del menu Ejecutar, 
* y al limpia limpia compiler->valgrind_cmd para que la proxima ejecucion
* vuelva a ser normal
**/
void mxMainWindow::OnToolsValgrindRun(wxCommandEvent &event) {
	if (!config->Init.valgrind_seen && !mxUT::GetOutput(wxString("\"")<<config->Files.valgrind_command<<"\" --version").Len()) {
		mxMessageDialog(main_window,LANG(MAINW_VALGRIND_MISSING,"Valgrind no se ecuentra correctamente instalado/configurado\n"
			"en su pc. Para descargar e instalar Doxygen dirijase a\n"
			"http://www.valgrind.org. Si ya se encuentra instalado,\n"
			"configure su ubiciacion en la pestania \"Rutas 2\" del\n"
			"dialog de \"Preferencias\" (menu \"Archivo\")."),
			LANG(GENERAL_WARNING,"Advertencia"),mxMD_OK|mxMD_WARNING).ShowModal();
		return;
	} else config->Init.valgrind_seen=true;
	
	if (!valgrind_config) valgrind_config=new mxValgrindConfigDialog(this);
	if (valgrind_config->ShowModal()==0) return;
	
	wxString val_file = DIR_PLUS_FILE(config->temp_dir,"valgrind.out");
	val_file.Replace(" ","\\ ");
	compiler->valgrind_cmd = config->Files.valgrind_command+" "<<valgrind_config->GetArgs()<<" --log-file="+val_file;
	OnRunRun(event);
	compiler->valgrind_cmd="";
}

/// @brief Despliega el panel de Valgrind para mostrar los resultados del analisis dinamico
void mxMainWindow::OnToolsValgrindView(wxCommandEvent &event) {
	ShowValgrindPanel(mxVO_VALGRIND,DIR_PLUS_FILE(config->temp_dir,"valgrind.out"));
}

void mxMainWindow::OnToolsValgrindHelp(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("valgrind.html");	
}

#endif

void mxMainWindow::OnToolsConsole(wxCommandEvent &evt) {
	wxString path=config->zinjai_dir;
	if (project) 
		path=project->path;
	else IF_THERE_IS_SOURCE 
		path=CURRENT_SOURCE->GetPath(true);
#if defined(__APPLE__)
	mxUT::Execute(path,"/Applications/Utilities/Terminal.app/Contents/MacOS/Terminal",wxEXEC_NOHIDE);
#elif defined(__WIN32__)
	mxUT::Execute(path,"cmd",wxEXEC_NOHIDE);
#else
	mxUT::Execute(path,config->Files.terminal_command.BeforeFirst(' '),wxEXEC_NOHIDE);
#endif
}

void mxMainWindow::OnToolsWxfbConfig(wxCommandEvent &event) {
	if (project) new mxWxfbConfigDialog();
}

void mxMainWindow::OnToolsWxfbNewRes(wxCommandEvent &event) {
	if (project) {
		
		if (!project->GetWxfbActivated()) project->ActivateWxfb(true);
		
		wxString name=mxGetTextFromUser("Nombre del archivo:","Nuevo Proyecto wxFormBuilder","",this);
		if (name.Len()==0) return;
		
		wxString fname=DIR_PLUS_FILE(project->path,name);
		wxString fbase=fname;
		if (mxUT::EndsWithNC(fbase,".FBP"))
			fbase=fbase.Mid(0,fbase.Len()-4);
		else
			fname = fname+".fbp";
		
		wxString folder=project->path;
		int pos1=fname.Find('\\',true);
		int pos2=fname.Find('/',true);
		if (pos1!=wxNOT_FOUND && pos2!=wxNOT_FOUND) {
			int pos = pos1>pos2?pos1:pos2;
			folder=fname.Mid(0,pos);
			name=fname.Mid(pos+1);
		} else if (pos1!=wxNOT_FOUND) {
			folder=fname.Mid(0,pos1);
			name=fname.Mid(pos1+1);
		} else if (pos2!=wxNOT_FOUND) {
			folder=fname.Mid(0,pos2);
			name=fname.Mid(pos2+1);
		}
		
		if (wxNOT_FOUND!=name.Find('-') 
			|| wxNOT_FOUND!=name.Find('<') || wxNOT_FOUND!=name.Find('?') 
			|| wxNOT_FOUND!=name.Find('>') || wxNOT_FOUND!=name.Find('*') ) {
				mxMessageDialog(this,"El nombre ingresado no es valido",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
				return;
			}
		
		// controlar que no exista
		wxString cpp_name = fbase+".cpp";
		wxString h_name = fbase+".h";
		if (wxFileName::FileExists(fname)) {
			if (mxMD_NO==mxMessageDialog(this,"Ya existe un archivo con ese nombre. Desea conservarlo?\nSi elige No se borrara su contenido.",name,(mxMD_YES_NO|mxMD_WARNING)).ShowModal())
				wxRemoveFile(fname);
		} 
		
		// crear el cpp
		if (!wxFileName::FileExists(cpp_name)) {
			wxTextFile cpp_file(cpp_name);
			cpp_file.Create();
			cpp_file.Write();
			cpp_file.Close();
		}
		
		// crear el h
		if (!wxFileName::FileExists(h_name)) {
			wxTextFile h_file(h_name);
			h_file.Create();
			h_file.Write();
			h_file.Close();
		}
		
		if (mxUT::EndsWithNC(name,".FBP")) name=name.Mid(0,name.Len()-4);
		if (!wxFileName::FileExists(fname)) {
			// crear el fbp
			wxTextFile fbp_file(fname);
			fbp_file.Create();
			fbp_file.AddLine("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>");
			fbp_file.AddLine("<wxFormBuilder_Project>");
			fbp_file.AddLine("<FileVersion major=\"1\" minor=\"10\" />");
			fbp_file.AddLine("<object class=\"Project\" expanded=\"1\">");
			fbp_file.AddLine("<property name=\"class_decoration\"></property>");
			fbp_file.AddLine("<property name=\"code_generation\">C++</property>");
			fbp_file.AddLine("<property name=\"disconnect_events\">1</property>");
			fbp_file.AddLine("<property name=\"encoding\">ANSI</property>");
			fbp_file.AddLine("<property name=\"event_generation\">connect</property>");
			fbp_file.AddLine(wxString("<property name=\"file\">")<<name<<"</property>");
			fbp_file.AddLine("<property name=\"first_id\">1000</property>");
			fbp_file.AddLine("<property name=\"help_provider\">none</property>");
			fbp_file.AddLine("<property name=\"internationalize\">0</property>");
			fbp_file.AddLine(wxString("<property name=\"name\">")<<name<<"</property>");
			fbp_file.AddLine("<property name=\"namespace\"></property>");
			fbp_file.AddLine("<property name=\"path\">.</property>");
			fbp_file.AddLine("<property name=\"precompiled_header\"></property>");
			fbp_file.AddLine("<property name=\"relative_path\">1</property>");
			fbp_file.AddLine("<property name=\"use_enum\">0</property>");
			fbp_file.AddLine("<property name=\"use_microsoft_bom\">0</property>");
			fbp_file.AddLine("</object>");
			fbp_file.AddLine("</wxFormBuilder_Project>");
			fbp_file.Write();
			fbp_file.Close();
		}
		
		// asociar al proyecto y abrir
		project_file_item *item;
		
		wxfb_configuration *wxfb = project->GetWxfbConfiguration();
		
		item=project->HasFile(cpp_name);
		if (!item) item=project->AddFile(FT_SOURCE,cpp_name);
		if (wxfb->set_wxfb_sources_as_readonly) project->SetFileReadOnly(item,true);
		if (wxfb->dont_show_base_classes_in_goto) project->SetFileHideSymbols(item,true);
		
		item=project->HasFile(h_name);
		if (!item) item=project->AddFile(FT_HEADER,h_name);
		if (wxfb->set_wxfb_sources_as_readonly) project->SetFileReadOnly(item,true);
		if (wxfb->dont_show_base_classes_in_goto) project->SetFileHideSymbols(item,true);
		
		item=project->HasFile(fname);
		if (!item) item=project->AddFile(FT_OTHER,fname);
		
		main_window->OpenFile(fname,false);
		return;
	}
}

void mxMainWindow::OnToolsWxfbLoadRes(wxCommandEvent &event) {
	if (project) {
		
		wxFileDialog dlg (this, "Abrir Archivo", project?project->last_dir:config->Files.last_dir, " ", "Any file (*)|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST );
		dlg.SetWildcard("wxFormBuilder projects|"WILDCARD_WXFB);
		if (dlg.ShowModal() != wxID_OK) return;
		
		project->last_dir=dlg.GetDirectory();
		
		wxString fbp_name = dlg.GetPath();
		
		wxString fbase = project->WxfbGetSourceFile(fbp_name);
		
		wxFileName fn_header(fbase+".h");
		fn_header.Normalize();
		wxString h_name=fn_header.GetFullPath();
		
		wxFileName fn_source(fbase+".cpp");
		fn_source.Normalize();
		wxString cpp_name=fn_source.GetFullPath();
		
		if (!project->GetWxfbActivated()) project->ActivateWxfb(true);
		
		if (!project->HasFile(cpp_name))
			project->AddFile(FT_SOURCE,cpp_name);
		if (!project->HasFile(h_name))
			project->AddFile(FT_HEADER,h_name);
		if (!project->HasFile(fbp_name))
			project->AddFile(FT_OTHER,fbp_name);
//		main_window->OpenFile(fbp_name,false);
		project->WxfbGenerate(true,project->HasFile(fbp_name));
		parser->Parse();
		
	}
}

void mxMainWindow::OnToolsWxfbRegen(wxCommandEvent &event) {
	if (project && project->GetWxfbActivated()) {
		status_bar->SetStatusText("Regenerando proyectos wxFormBuilder...");
		project->WxfbGenerate();
//		parser->Parse();
//		status_bar->SetStatusText(LANG(GENERAL_READY,"Listo"));
	}
}

void mxMainWindow::OnToolsWxfbHelp(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("wxformbuilder.html");
}

void mxMainWindow::OnToolsWxfbHelpWx(wxCommandEvent &event) {
	if (wxFileName::FileExists(DIR_PLUS_FILE(config->zinjai_dir,config->Help.wxhelp_index)))
		mxUT::OpenInBrowser(DIR_PLUS_FILE(config->zinjai_dir,config->Help.wxhelp_index));
	else if (mxMD_OK==mxMessageDialog(this,"ZinjaI no pudo encontrar la ayuda de wxWidgets. A continuacion le permitira buscarla\nmanualmente y luego recordara esta seleccion (en cualquier momento se puede modificar\ndesde el cuadro de Preferencias). Usualmente, el archivo indice es \"wx_contents.html\".","Ayuda wxWidgets",(mxMD_INFO|mxMD_OK_CANCEL)).ShowModal()) {
		wxFileDialog dlg(this,"Indice de ayuda wxWidgets:",config->Help.wxhelp_index);
		if (wxID_OK==dlg.ShowModal()) {
			config->Help.wxhelp_index=dlg.GetPath();
			mxUT::OpenInBrowser(DIR_PLUS_FILE(config->zinjai_dir,config->Help.wxhelp_index));
		}
	}
}


class ToolsWxfbInheriterAction : public Parser::OnEndAction {
	bool update;
public:
	ToolsWxfbInheriterAction(bool _update):update(_update){}
	void Do() { new mxWxfbInheriter(main_window,"",update); }
};

void mxMainWindow::OnToolsWxfbInheritClass(wxCommandEvent &event) {
	OnToolsWxfbRegen(event);
	parser->OnEnd(new ToolsWxfbInheriterAction(false),true);
}

void mxMainWindow::OnToolsWxfbUpdateInherit(wxCommandEvent &event) {
	OnToolsWxfbRegen(event);
	parser->OnEnd(new ToolsWxfbInheriterAction(true),true);
}

void mxMainWindow::OnToolsRemoveComments (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE CURRENT_SOURCE->RemoveComments();
}

void mxMainWindow::OnToolsAlignComments (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		long l=-1;
		wxString input=mxGetTextFromUser("Alinear Comentarios","Nro de Columna:",wxString()<<config->Source.alignComments,this);
		if (input.Len() && input.ToLong(&l) && l>=0) 
			src->AlignComments(config->Source.alignComments=l);
	}
}

void mxMainWindow::OnToolsDiffPrevMark(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->GotoDiffChange(false);
}

void mxMainWindow::OnToolsDiffNextMark(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->GotoDiffChange(true);
}

void mxMainWindow::OnToolsDiffTwoSources(wxCommandEvent &event) {
	if (notebook_sources->GetPageCount()>1)
		new mxDiffWindow(NULL);
	else
		mxMessageDialog(main_window,"Debe tener al menos dos archivos abiertos para poder compararlos",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();		
}

void mxMainWindow::OnToolsDiffToHimself(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src = CURRENT_SOURCE;
		if (src->sin_titulo)
			mxMessageDialog(main_window,"El archivo actual no ha sido guardado",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();		
		else {
			if (src->GetModify())
				new mxDiffWindow(CURRENT_SOURCE,src->source_filename.GetFullPath());
			else
				mxMessageDialog(main_window,"El archivo actual no ha sido modificado",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();		
		}
	} else
		mxMessageDialog(main_window,"Debe tener al menos un archivo abierto para poder compararlo",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();		
}

void mxMainWindow::OnToolsDiffToDiskFile(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		wxFileDialog dlg (this, "Abrir Archivo", project?project->last_dir:config->Files.last_dir, " ", "Any file (*)|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST );
		dlg.SetWildcard("Archivos de C/C++|"WILDCARD_CPP"|Fuentes|"WILDCARD_SOURCE"|Cabeceras|"WILDCARD_HEADER"|Todos los archivos|*");
		if (dlg.ShowModal() == wxID_OK) {
			if (project)
				project->last_dir=dlg.GetDirectory();
			else
				config->Files.last_dir=dlg.GetDirectory();
			new mxDiffWindow(CURRENT_SOURCE,dlg.GetPath());
		}
	} else
		mxMessageDialog(main_window,"Debe tener al menos un archivo abierto para poder compararlo",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();				
}

void mxMainWindow::OnToolsDiffClear(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *src = CURRENT_SOURCE;
		src->MarkDiffs(0,src->GetLineCount()-1,mxSTC_MARK_DIFF_NONE);
		src->Refresh();
		if (diff_sidebar) diff_sidebar->Refresh();
	}
}

void mxMainWindow::OnToolsDoxyConfig(wxCommandEvent &event) {
	if (project) new mxDoxyDialog();
}

void mxMainWindow::OnToolsDoxyGenerate(wxCommandEvent &event) {
	if (project) {
		if (config->CheckDoxygenPresent()) {
			mxOutputView *doxy = new mxOutputView("Doxygen",mxOV_EXTRA_URL,"Ver HTMLs",
				DIR_PLUS_FILE(project->path,DIR_PLUS_FILE(project->GetDoxygenConfiguration()->destdir,DIR_PLUS_FILE("html","index.html"))),
				mxVO_DOXYGEN,DIR_PLUS_FILE(config->temp_dir,"doxygen.out"));
			project->SaveAll(false);
			project->GenerateDoxyfile(DIR_PLUS_FILE(project->path,"Doxyfile"));
			doxy->Launch(project->path,wxString("\"")<<config->Files.doxygen_command<<"\" Doxyfile");
		}
	}
}

void mxMainWindow::OnToolsDoxyView(wxCommandEvent &event) {
	if (project) {
		mxUT::OpenInBrowser(DIR_PLUS_FILE(project->path,DIR_PLUS_FILE(project->GetDoxygenConfiguration()->destdir,DIR_PLUS_FILE("html","index.html"))));
	}
}


void mxMainWindow::OnToolsDoxyHelp(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("doxygen.html");
}

void mxMainWindow::OnToolsDiffHelp(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("diff.html");
}

void mxMainWindow::OnToolsShareHelp(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("share.html");
}

void mxMainWindow::OnToolsGprofHelp (wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("gprof.html");
}

bool mxMainWindow::OnToolsGprofGcovSetAux(wxCommandEvent &event,wxString tool, wxString arg) {
	if (project) {
		bool present = mxUT::IsArgumentPresent(project->active_configuration->compiling_extra,arg);
		mxUT::SetArgument(project->active_configuration->compiling_extra,arg,!present);
		mxUT::SetArgument(project->active_configuration->linking_extra,arg,!present);
		if (mxMD_YES==mxMessageDialog(this,wxString(!present?
			(LANG1(MAINW_GPROF_GPROF_ENABLED,"Se agregaron los parámetros de compilación necesarios para utilizar <{1}>.",tool)):
			(LANG1(MAINW_GPROF_GPROF_DISABLED,"Se quitaron los parámetros de compilación necesarios para utilizar <{1}>.",tool))
			)+"\n"+
			LANG(MAINW_GCOV_GPROF_ENABLE_DISABLE_QUESTION,"Para que esta modificación tenga efecto probablemente deba recompilar todo su proyecto.\n"
			"¿Desea hacerlo ahora? (si elige no, deberá limpiar el proyecto manualmente más tarde para hacerlo)."),
			tool,(mxMD_YES_NO|mxMD_INFO)).ShowModal()) {
				project->Clean();
				OnRunCompile(event);
			}
		return !present;
	} else IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE; 
		wxString comp_opts=src->GetCompilerOptions(false);
		bool present = mxUT::IsArgumentPresent(comp_opts,arg);
		mxUT::SetArgument(comp_opts,arg,!present);
		src->SetCompilerOptions(comp_opts);
		mxMessageDialog(this,wxString(!present?
			(LANG1(MAINW_GPROF_GPROF_ENABLED,"Se agregaron los parámetros de compilación necesarios para utilizar <{1}>.",tool)):
			(LANG1(MAINW_GPROF_GPROF_DISABLED,"Se quitaron los parámetros de compilación necesarios para utilizar <{1}>.",tool))
			),tool,(mxMD_OK|mxMD_INFO)).ShowModal();
		OnRunClean(event);
		return !present;
	} else
		return false;
}
void mxMainWindow::OnToolsGprofSet (wxCommandEvent &event) {
	OnToolsGprofGcovSetAux(event,"gprof","-pg");
}

void mxMainWindow::OnToolsGprofDot (wxCommandEvent &event) {
	_menu_item(mxID_TOOLS_GPROF_DOT)->Check(true);
	_menu_item(mxID_TOOLS_GPROF_FDP)->Check(false);
	config->Init.graphviz_dot=true;
}

void mxMainWindow::OnToolsGprofFdp (wxCommandEvent &event) {
	_menu_item(mxID_TOOLS_GPROF_DOT)->Check(false);
	_menu_item(mxID_TOOLS_GPROF_FDP)->Check(true);
	config->Init.graphviz_dot=false;	
}

/**
* Verifica que exita información de profiling para el fuente o proyecto actual,
* y ejecuta gprof para extraerla un archivo de texto. Retorna el path del archivo
* generado. Si hay error retorna una cadena vacia.
**/
wxString mxMainWindow::OnToolsGprofShowListAux(bool include_command) {
	if (!project && notebook_sources->GetPageCount()==0) return "";
	
	// ver si hay informacion de profiling
	wxString gmon = project ? (DIR_PLUS_FILE(project->active_configuration->working_folder.Len()?DIR_PLUS_FILE(project->path,project->active_configuration->working_folder):project->path,"gmon.out")) : (DIR_PLUS_FILE(CURRENT_SOURCE->working_folder.GetFullPath(),"gmon.out"));
	if (!wxFile::Exists(gmon)) {
		mxMessageDialog(this,LANG(MAINW_GPROF_OUTPUT_MISSING,"No se encontro informacion de profiling.\nPara saber como generarla consulte la ayuda."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		return "";
	}
	
	// ejecutar gprof...
	status_bar->SetStatusText(LANG(MAINW_GPROF_STATUS_ANALIZING,"Analizando informacion de perfilado..."));
	mxOSD osd(this,LANG(OSD_GENERATING_GRAPH,"Generando grafo..."));
	wxString command("gprof -b ");
	if (project)
		command<<mxUT::Quotize(project->GetExePath());
	else IF_THERE_IS_SOURCE
		command<<mxUT::Quotize(CURRENT_SOURCE->GetBinaryFileName().GetFullPath());
	command<<" "<<mxUT::Quotize(gmon);
	// ...y capturar salida en un archivo de texto
	wxString gout = DIR_PLUS_FILE(config->temp_dir,"gprof.txt");
	wxRemoveFile(gout); // eliminar resultados previos para evitar problemas
	wxFFile fgout(gout,"w+");
	wxArrayString output;
	if (include_command) fgout.Write(wxString("> ")<<command+"\n\n");
	int retval=mxExecute(command, output, wxEXEC_NODISABLE|wxEXEC_SYNC);
	if (retval||!output.GetCount()) { // si hay algun error al ejecutar gprof
		osd.Hide(); 
		mxMessageDialog(this,wxString(LANG(MAINW_GPROF_ERROR,"Ha ocurrido un error al intentar procesar la información de perfilado"))
							 +(retval?" (error 1).":" (error 2)."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal(); 
		return "";
	}
	for (unsigned int i=0;i<output.GetCount();i++)
		fgout.Write(output[i]+"\n");
	fgout.Close();
	
	return gout;
}

void mxMainWindow::OnToolsGprofShow (wxCommandEvent &event) {
	
	// procesar binario de gprof para obtener la salida como texto
	wxString gout=OnToolsGprofShowListAux(); 
	if (!gout.Len()) return;
	
	mxOSD osd(this,LANG(OSD_GENERATING_GRAPH,"Generando grafo..."));
	
	// pedir al usuario los limites para el grafo (arcos y nodos con poco peso se descartan)
	static double edge_tres=0.1,node_tres=0.1;
	wxString edgt = mxGetTextFromUser(LANG(MAINW_GPROF_EDGE_THERSHOLD,"Eliminar arcos bajo este umbral (%):"),LANG(MAINW_GPROF_ASK_TITLE,"Perfil de Ejecucion"),wxString()<<edge_tres,this);
	if (!edgt.Len()) return;
	edgt.Replace(",",".");
	wxString nodt = mxGetTextFromUser(LANG(MAINW_GPROF_NODE_THERSHOLD,"Eliminar nodos bajo este umbral (%):"),LANG(MAINW_GPROF_ASK_TITLE,"Perfil de Ejecucion"),wxString()<<node_tres,this);
	if (!nodt.Len()) return;
	nodt.Replace(",",".");
	
	// procesar salida con gprof2dot para obtener el grafo
	status_bar->SetStatusText(LANG(MAINW_GPROF_DRAWING,"Dibujando grafo..."));
	wxString pout = DIR_PLUS_FILE(config->temp_dir,"gprof.dot");
#if defined(__WIN32__)	
	wxString command="graphviz/gprof2dot/gprof2dot.exe ";
#else
	wxString command="python graphviz/gprof2dot/gprof2dot.py ";
#endif
	nodt.ToDouble(&edge_tres); edgt.ToDouble(&node_tres);
	command<<mxUT::Quotize(gout)<<" -e "<<edge_tres<<" -n "<<node_tres<<" -o "<<mxUT::Quotize(pout);
	int retval=mxExecute(command,wxEXEC_NODISABLE|wxEXEC_SYNC);
	if (retval) { osd.Hide(); mxMessageDialog(this,wxString(LANG(MAINW_GPROF_ERROR,"Ha ocurrido un error al intentar procesar la información de perfilado"))+" (error 2).",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal(); return; }
	
	// mostrar el grafo
	status_bar->SetStatusText(LANG(MAINW_GPROF_SHOWING,"Mostrando resultados..."));
	retval = mxUT::ProcessGraph(pout,!config->Init.graphviz_dot,"",LANG(MAINW_GPROF_GRAPH_TITLE,"Informacion de Profiling"));
	if (retval) { osd.Hide(); mxMessageDialog(this,wxString(LANG(MAINW_GPROF_ERROR,"Ha ocurrido un error al intentar procesar la información de perfilado"))+" (error 3).",LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal(); return; }
	status_bar->SetStatusText(LANG(GENERAL_READY,"Listo"));
	
}

void mxMainWindow::OnToolsGprofList (wxCommandEvent &event) {
	
	// procesar binario de gprof para obtener la salida como texto
	wxString gout=OnToolsGprofShowListAux(); 
	if (!gout.Len()) return;
	
	// analizar la salida del archivo de texto y mostrar en nueva ventana
	new mxGprofOutput(this,gout);
//	mxSource *src=OpenFile(gout,false);
//	if (src && src!=EXTERNAL_SOURCE) src->MakeUntitled("<profiling>");
	status_bar->SetStatusText(LANG(GENERAL_READY,"Listo"));
	
}

void mxMainWindow::OnToolsDiffApply(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		CURRENT_SOURCE->ApplyDiffChange();
		if (diff_sidebar) diff_sidebar->Refresh();
	}
}

void mxMainWindow::OnToolsDiffDiscard(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		CURRENT_SOURCE->DiscardDiffChange();
		if (diff_sidebar) diff_sidebar->Refresh();
	}
}

void mxMainWindow::OnToolsDiffShow(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE
		CURRENT_SOURCE->ShowDiffChange();
}

void mxMainWindow::OnToolsShareShare (wxCommandEvent &event) {
	IF_THERE_IS_SOURCE 
		new mxShareSourceWindow(CURRENT_SOURCE,CURRENT_SOURCE->page_text,this);
}

void mxMainWindow::OnToolsShareOpen (wxCommandEvent &event) {
	if (!open_shared) open_shared = new mxOpenSharedWindow(this);
	open_shared->Show();
}

void mxMainWindow::OnToolsShareList(wxCommandEvent &event) {
	new mxListSharedWindow(this);
}

void mxMainWindow::OnToolsDrawProject(wxCommandEvent &event) {
	if (project) project->DrawGraph();
}

void mxMainWindow::OnToolsDrawClasses(wxCommandEvent &event) {
	if (project) {
		project->UpdateSymbols();
	} else {
		IF_THERE_IS_SOURCE
			parser->ParseSource(CURRENT_SOURCE);
	}
	new mxDrawClasses();
}

/// @brief manda a dibujar un diagrama de flujo (de eso se encargan mxFlowWindow y mxFlowCanvas)
void mxMainWindow::OnToolsDrawFlow(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		new mxFlowWindow(source, source->page_text);
	}
}

void mxMainWindow::OnToolsExeProps (wxCommandEvent &event) {
	if (project)
		new mxExeInfo(this,NULL);
	else IF_THERE_IS_SOURCE
		new mxExeInfo(this,CURRENT_SOURCE);
}

void mxMainWindow::OnToolsCustomToolsSettings(wxCommandEvent &evt) {
	new mxCustomTools(false,-1);
}

void mxMainWindow::OnToolsProjectToolsSettings(wxCommandEvent &evt) {
	new mxCustomTools(true,-1);
}

void mxMainWindow::OnToolsCustomHelp(wxCommandEvent &evt) {
	mxHelpWindow::ShowHelp("custom_tools.html");	
}

void mxMainWindow::OnToolsCustomTool(wxCommandEvent &event) {
	config->custom_tools.Run(event.GetId()-mxID_CUSTOM_TOOL_0);
}

void mxMainWindow::OnToolsCustomProjectTool(wxCommandEvent &event) {
	project->custom_tools.Run(event.GetId()-mxID_CUSTOM_PROJECT_TOOL_0);
}

class ToolsCodeCopyFromHAction : public Parser::OnEndAction {
	mxSource *source;
	wxString the_one;
public:
	ToolsCodeCopyFromHAction(mxSource *_source,wxString _the_one):source(_source),the_one(_the_one){}
	void Do() { main_window->ToolsCodeCopyFromH(source,the_one); }
};

void mxMainWindow::OnToolsCodeCopyFromH(wxCommandEvent &event) {
	IF_THERE_IS_SOURCE {
		mxSource *source = CURRENT_SOURCE;
		wxString the_one;
		if (source->sin_titulo || !(the_one=mxUT::GetComplementaryFile(source->source_filename,FT_SOURCE)).Len()) {
			mxMessageDialog(this,LANG(MAINW_CODETOOLS_NO_HEADER_FOUND,"No se pudo determinar el archivo de cabecera asociado."),LANG(GENERAL_WARNING,"Advertencia"),mxMD_WARNING|mxMD_OK).ShowModal();
			return;
		}
		mxSource *other=IsOpen(the_one);
		parser->ParseSource(source);
		if (other) parser->ParseSource(other);
		else parser->ParseFile(the_one);
		parser->OnEnd(new ToolsCodeCopyFromHAction(source,the_one),true);
	}
}
		
void mxMainWindow::ToolsCodeCopyFromH(mxSource *source, wxString the_one) {
	pd_file *file=parser->GetFile(the_one);
	if (!file) return;
	source->BeginUndoAction();
	pd_ref *func;
	func=file->first_func_dec;
	wxArrayString choices;
		while (func->next) { func=func->next; } // para atras, para que queden ordenadas como en el h
		while (func->prev) {
			if (!PD_UNREF(pd_func,func)->file_def && !(PD_UNREF(pd_func,func)->properties&PD_CONST_VIRTUAL_PURE)) {
				choices.Add(PD_UNREF(pd_func,func)->full_proto+(PD_UNREF(pd_func,func)->properties&PD_CONST_CONST?" const":"")); // const no deberia estar en el full proto?
			}
			func=func->prev;
		}
	source->EndUndoAction();
	if (!choices.GetCount()) {
		mxMessageDialog(this,LANG(MAINW_CODETOOLS_NO_NEW_METHOD_FUNCTION,"No se encontraron funciones/metodos sin implementar."),LANG(GENERAL_WARNING,"Advertencia"),mxMD_WARNING|mxMD_OK).ShowModal();
		return;
	}
	wxArrayInt sels;
	int line=-1;
	for (unsigned int i=0;i<choices.GetCount();i++) sels.Add(i);
	int nsels=wxGetMultipleChoices(sels,LANG(MAINW_CODETOOLS_CHOOSE_FUNCTIONS_METHODS,"Seleccione las funciones/metodos a implementar:"),LANG(MENUITEM_TOOLS_CODE_COPY_FROM_H,"Implementar Metodos/Funciones faltantes"),choices,this);
	for (int i=0;i<nsels;i++) {
		while (source->GetLineCount() && source->GetLine(source->GetLineCount()-1).Len()>1) {
			wxString aux=source->GetLine(source->GetLineCount()-1);
			source->AppendText("\n");
		}
		while (source->GetLineCount()>1 && source->GetLine(source->GetLineCount()-2).Len()>1) {
			wxString aux=source->GetLine(source->GetLineCount()-2);
			source->AppendText("\n");
		}
		if (line==-1) line=source->GetLineCount();
		source->AppendText(choices[sels[i]]+" {\n\t\n}\n\n");
	}
	if (line>=0) {
		source->EnsureVisibleEnforcePolicy(source->GetLineCount()-1);
		source->MarkError(line,false);
	}
}

static void GetValidLines(const char *fin, int *v, int n) {
	memset(v,0,sizeof(int)*n);
	ifstream f(fin);
	string l; int ln=1;
	getline(f,l);
	string fname=l.substr(l.find('\"'),l.rfind('\"')-l.find('\"')+1);
	bool counting=false; int old_last,last=0;
	while (getline(f,l)) {
		if (l.size()>2 && l[0]=='#' && l[1]==' ') {
			int n=l.size(), i=2;
			while (i<n&&l[i]!=' ') i++;
			old_last=last; last=atoi(l.substr(2,i).c_str());
			while (i<n&&l[i]==' ') i++;
			if (i!=n) {
				if (counting) v[old_last-1]=ln+1;
				counting=(l.substr(i,l.rfind('\"')-i+1)==fname);
			}
		} else if (counting) {
			v[(last++)-1]=l.length()?ln:0;
		}
		ln++;
	}
	f.close();
}

void mxMainWindow::OnToolsPreprocMarkValid ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE ToolsPreproc(1);
}

void mxMainWindow::OnToolsPreprocUnMarkAll ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		src->IndicatorSetStyle(2,wxSTC_INDIC_STRIKE);
		int lse = src->GetEndStyled();
		src->StartStyling(0,wxSTC_INDICS_MASK);
		src->SetStyling(src->GetLength(),0);
		src->StartStyling(lse,0x1F);
	}
}

void mxMainWindow::OnToolsPreprocReplaceMacros ( wxCommandEvent &event ) {
	IF_THERE_IS_SOURCE ToolsPreproc(2);
}

void mxMainWindow::OnToolsPreprocHelp ( wxCommandEvent &event ) {
	mxHelpWindow::ShowHelp("menu_herramientas.html#expandir_macros");	
}

/**
* @brief Realiza algunas de las acciones del submenú Preprocesador del menú Herramientas
*
* @param id_commnad 1=marcar lineas validas, 2=reemplazar macros
**/
void mxMainWindow::ToolsPreproc( int id_command ) {
	
	mxOSD osd(this,LANG(MAINW_PREPROC_OSD,"Preprocesando..."));
	mxSource *src=CURRENT_SOURCE;
	wxString bin_name;
	bin_name = DIR_PLUS_FILE(config->home_dir,"preprocessed.tmp");
	if (project) {
		if (src->GetModify()) src->SaveSource();
//		file_item *item = project->FindFromName(src->source_filename.GetFullPath());
		wxString fname = src->source_filename.GetFullPath();
		project->AnalizeConfig(project->path,true,current_toolchain.mingw_dir,true);
		bool cpp = fname.Last()!='c';
		wxString command = wxString(cpp?current_toolchain.cpp_compiler:current_toolchain.c_compiler)+
			(cpp?project->cpp_compiling_options:project->c_compiling_options)+" "+mxUT::Quotize(fname)+" -c -E -o "+mxUT::Quotize(bin_name);
		if (id_command==1) command<<" -fdirectives-only -C";
		_IF_DEBUGMODE(command);
		int x =mxUT::Execute(project->path,command, wxEXEC_SYNC/*|wxEXEC_HIDE*/);	
		if (x!=0) { 
			_IF_DEBUGMODE(x);
			osd.Hide();
			mxMessageDialog(this,LANG(MAINW_PREPROC_ERROR,"No se pudo preprocesar correctamente el fuente."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			return;
		}
	} else {
		wxString fname=src->SaveSourceForSomeTool();
		wxString z_opts(wxString(" "));
		// prepare command line
		bool cpp = src->IsCppOrJustC();
		if (config->Debug.format.Len()) z_opts<<config->Debug.format<<" ";
		z_opts<<(cpp?current_toolchain.cpp_compiling_options:current_toolchain.c_compiling_options)<<" ";
//		z_opts<<current_toolchain.linker_options<<" ";
		wxString ext=src->source_filename.GetExt();
		if (!src->sin_titulo && (!ext.Len()||(ext[0]>='0'&&ext[0]<='9'))) z_opts<<"-x c++ "; 
		z_opts<<"-E "; if (id_command==1) z_opts<<"-fdirectives-only -C ";
		wxString comp_opts = src->GetCompilerOptions();
		wxString command = wxString(cpp?current_toolchain.cpp_compiler:current_toolchain.c_compiler)+z_opts+"\""+fname+"\" "+comp_opts+" -o \""+bin_name<<"\"";
		_IF_DEBUGMODE(command);
		int x =mxUT::Execute(src->source_filename.GetPath(),command, wxEXEC_SYNC/*|wxEXEC_HIDE*/);	
		if (x!=0) { 
			_IF_DEBUGMODE(x);
			osd.Hide();
			mxMessageDialog(this,LANG(MAINW_PREPROC_ERROR,"No se pudo preprocesar correctamente el fuente."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			return;
		}
	}
	int n=src->GetLineCount();
	int *v=new int[n];
	GetValidLines(bin_name.c_str(),v,n);
	
	
	if (id_command==1) {
		src->IndicatorSetStyle(2,wxSTC_INDIC_STRIKE);
		if (ctheme->DEFAULT_BACK.Green()+ctheme->DEFAULT_BACK.Blue()+ctheme->DEFAULT_BACK.Red()<256*3/2)
			src->IndicatorSetForeground (2, wxColour(255,255,255));
		else
			src->IndicatorSetForeground (2, wxColour(0,0,0));
		int lse = src->GetEndStyled();
		src->StartStyling(0,wxSTC_INDICS_MASK);
		src->SetStyling(src->GetLength(),0);
		for(int i=0;i<n;i++) { 
			if (i&&src->GetCharAt(src->GetLineEndPosition(i-1)-1)=='\\') v[i]=v[i-1];
			if (!v[i]) {
				int i1=src->GetLineIndentPosition(i);
				int i2=src->GetLineEndPosition(i);
				// descomentar para que no tache el #if... que da verdadero
//				if (
//					(i1+3<i2 && src->GetTextRange(i1,i1+3)=="#if")||
//					(i1+3<i2 && src->GetTextRange(i1,i1+5)=="#else")||
//					(i1+3<i2 && src->GetTextRange(i1,i1+5)=="#elif")
//					) {
//						int j=i+1;
//						while(j<n && src->GetLineIndentPosition(j)==src->GetLineEndPosition(j)) j++;
//						if (j<n && v[j]) v[i]=true;
//					}
//				if (!v[i]) {
					src->StartStyling(i1,wxSTC_INDICS_MASK);
					src->SetStyling(i2-i1,wxSTC_INDIC2_MASK);
//				}
			}
		}
		delete []v;
		src->StartStyling(lse,0x1F);
	} else if (id_command==2) {
		int l0=src->LineFromPosition(src->GetSelectionStart());
		int l1=src->LineFromPosition(src->GetSelectionEnd());
		if (l1<l0) { int x=l1; l1=l0; l0=x; }
		wxTextFile fil(bin_name);  fil.Open();
		wxString ret;
		for(int i=l0;i<=l1;i++) {
			ret<<(v[i]?fil.GetLine(v[i]):"")<<"\n";
		}
		ret.RemoveLast();
		src->ShowBaloon(ret,src->PositionFromLine(l0?l0-1:0));
	}
//	src->SetFocus();
}

void mxMainWindow::OnToolsInstallComplements(wxCommandEvent &evt) {
	new mxComplementInstallerWindow(this);
}

void mxMainWindow::OnToolsCreateTemplate(wxCommandEvent &evt) {
	wxString user_templates_dir=DIR_PLUS_FILE(config->home_dir,"templates");
	if (!wxFileName::DirExists(user_templates_dir)) 
		wxFileName::Mkdir(user_templates_dir);
	if (project) {
		wxString description=wxGetTextFromUser(LANG(MAINW_ENTER_FRIENDLY_NAME_FOR_NEW_PROJECT_TEMPLATE,"Ingrese el nombre para mostrar del nuevo template de proyecto"),LANG(MAINW_GENERATING_TEMPLATE,"Generando plantilla..."),project->project_name);
		if (!description.Len()) return; 
		wxString filename=wxGetTextFromUser(LANG(MAINW_ENTER_FILE_NAME_FOR_NEW_PROJECT_TEMPLATE,"Ingrese el nombre del archivo del nuevo template de proyecto"),LANG(MAINW_GENERATING_TEMPLATE,"Generando plantilla..."),wxFileName(project->filename).GetName());
		if (!filename.Len()) return; 
		mxOSD osd(this,LANG(MAINW_GENERATING_TEMPLATE,"Generando plantilla..."));
		project->Clean(); wxYield(); // remove temporals
		wxString project_name=project->project_name; project->project_name=description; // replace project name with the new description
		project->Save(true); // save in place as template
		project->project_name=project_name; // restore original project name
		project->CleanAll(); // remove temp files
		wxString dest_dir=DIR_PLUS_FILE(user_templates_dir,filename);
		wxFileName::Mkdir(dest_dir); mxUT::XCopy(project->path,dest_dir,true); // copy all project files
		wxString zpr=wxFileName(project->filename).GetFullName(); 
		wxString orig_zpr=DIR_PLUS_FILE(dest_dir,zpr);
		wxString dest_zpr=DIR_PLUS_FILE(dest_dir,filename+DOT_PROJECT_EXT);
		if (dest_zpr!=orig_zpr) wxRenameFile(orig_zpr,dest_zpr); // rename .zpr
		project->Save(); // restore real project file to non-template status
	} else IF_THERE_IS_SOURCE {
		wxString description = wxGetTextFromUser(LANG(MAINW_ENTER_FRIENDLY_NAME_FOR_NEW_SIMPLE_PROGRAM_TEMPLATE,"Ingrese el nombre para mostrar del nuevo template de programa simple"));
		if (!description.Len()) return; 
		wxString filename = wxGetTextFromUser(LANG(MAINW_ENTER_FILE_NAME_FOR_NEW_SIMPLE_PROGRAM_TEMPLATE,"Ingrese el nombre del archivo del nuevo template de programa simple"));
		if (!filename.Len()) return; 
		mxSource *src=CURRENT_SOURCE;
		wxString dest_file=DIR_PLUS_FILE(user_templates_dir,filename+".tpl");
		// create the template file
		wxTextFile template_file(dest_file);
		template_file.Create();
		template_file.AddLine(wxString("// !Z! Name: ")+description);
		int cur_pos=src->GetCurrentPos(),delta=0;
		for(int i=0;i<cur_pos;i++) { if (src->GetCharAt(i)=='\r') delta++; }
		template_file.AddLine(wxString("// !Z! Caret: ")<<(cur_pos-delta));
		template_file.AddLine(wxString("// !Z! Options: ")+src->GetCompilerOptions(false));
		if (!src->cpp_or_just_c) template_file.AddLine(wxString("!Z! Type: C"));
		for(int i=0;i<src->GetLineCount();i++) {
			wxString str=src->GetLine(i);
			while (str.EndsWith("\r")||str.EndsWith("\n")) str.RemoveLast();
			template_file.AddLine(str);
		}
		template_file.Write(); template_file.Close();
	}
	mxMessageDialog(this,LANG(MAINW_TEMPLATE_GENERATED,"Plantilla generada"),LANG(MENUITEM_TOOLS_CREATE_TEMPLATE,"Guardar como nueva plantilla..."),mxMD_OK).Show();
	delete wizard; wizard=NULL;
}

void mxMainWindow::OnToolsExportMakefile (wxCommandEvent &event) {
	new mxMakefileDialog(this);
}


void mxMainWindow::OnToolsGcovSet (wxCommandEvent & event) {
	if (OnToolsGprofGcovSetAux(event,"gcov","--coverage"))
		ShowGCovSideBar();
}

void mxMainWindow::OnToolsGcovReset (wxCommandEvent & event) {
	if (project) {
		wxString path=project->GetTempFolder();
		if (mxMD_NO==mxMessageDialog(this,wxString("Se eliminarán todos los archivos con extensión .gcov, .gcno y .gcda del\ndirectorio de temporales (")<<path<<")\n¿Desea Continuar?","gcov",(mxMD_YES_NO|mxMD_WARNING)).ShowModal()) return;
		wxArrayString array;
		mxUT::GetFilesFromDir(array,path,true);
		for(unsigned int i=0;i<array.GetCount();i++) { 
			if (array[i].EndsWith(".gcov")||array[i].EndsWith(".gcda")||array[i].EndsWith(".gcno"))
				wxRemoveFile(DIR_PLUS_FILE(path,array[i]));
		}
	} else IF_THERE_IS_SOURCE {
		mxSource *src=CURRENT_SOURCE;
		wxRemoveFile(DIR_PLUS_FILE(src->GetPath(false),src->GetFileName(true))+".gcov");
		wxRemoveFile(DIR_PLUS_FILE(src->GetPath(false),src->GetFileName(false))+".gcda");
		wxRemoveFile(DIR_PLUS_FILE(src->GetPath(false),src->GetFileName(false))+".gcno");
	}
}

void mxMainWindow::OnToolsGcovShow (wxCommandEvent & event) {
	ShowGCovSideBar();
}

void mxMainWindow::OnToolsGcovHelp (wxCommandEvent & event) {
	mxHelpWindow::ShowHelp("gcov.html");
}
