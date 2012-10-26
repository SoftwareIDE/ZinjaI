#include "DebugManager.h"
#include "mxCompiler.h"
#include "ConfigManager.h"
#include "mxMainWindow.h"
#include <wx/process.h>
#include <wx/stream.h>
#include "mxMessageDialog.h"
#include "mxUtils.h"
#include "mxSource.h"
#include "ProjectManager.h"
#include "ids.h"
#include "mxInspectionGrid.h"
#include "mxBacktraceGrid.h"
#include <wx/grid.h>
#include "Parser.h"
#include "mxInspectionExplorer.h"
#include "mxBreakList.h"
#include "mxInspectionMatrix.h"
#include "mxArgumentsDialog.h"
#include <wx/msgdlg.h>
#include "mxApplication.h"
#include <iostream>
#include "Language.h"
#include "mxOSD.h"
#include "winStuff.h"
#include "mxExternInspection.h"
#include <algorithm>
using namespace std;

#ifdef DEBUG
// #define DEBUG_MANAGER_LOG_TALK
#define DEBUG_LOG_FILE "debug.log"
#endif

#define BACKTRACE_MACRO "define zframeaddress\nset $fi=0\nwhile $fi<$arg0\nprintf \"*zframe-%u={\",$fi\ninfo frame $fi\nprintf \"}\\n\"\nset $fi=$fi+1\nend\nend"

DebugManager *debug;

DebugManager::DebugManager() {
	pause_breakpoint = NULL;
	current_handle = -1;
	last_backtrace_size = 0;
	inspections_count = 0;
//	backtrace_visible = false;
	threadlist_visible = false;
	waiting = debugging = running = really_running = false;
	process = NULL;
	input = NULL;
	output = NULL;
	pid = 0;
	notitle_source = current_source = NULL;
#if !defined(_WIN32) && !defined(__WIN32__)
	tty_pid = 0;
	tty_process = NULL;
#endif
}

DebugManager::~DebugManager() {
}

bool DebugManager::Start(bool update) {
#ifdef DEBUG_MANAGER_LOG_TALK
	debug_log_file.Open(DEBUG_LOG_FILE,"w+");
#endif
	if (update && project->PrepareForBuilding()) { // ver si hay que recompilar antes
		compiler->BuildOrRunProject(true,true,true);
		return false;
	}
	
	if (parser->working) {
		mxMessageDialog(main_window,LANG(DEBUG_MUST_WAIT_FOR_PARSER,"Debe esperar a que se termine de actualizar el arbol de simbolos para comenzar la depuracion."),LANG(GENERAL_WARNING,"Aviso"),mxMD_INFO|mxMD_OK).ShowModal();
		return true;
	}
	SetStateText(LANG(DEBUG_STATUS_STARTING,"Iniciando depuracion..."),true);
	project->Debug(); // lanzar el depurador con los parametros del proyecto
	project->SetBreakpoints(); // setear los puntos de interrupcion del proyecto
	if (!Run() && has_symbols) {
#ifdef __WIN32__
		if (wxFileName(DIR_PLUS_FILE(project->path,project->executable_name)).GetPath().Contains(' '))
			mxMessageDialog(main_window,LANG(DEBUG_ERROR_STARTING_SPACES,"Error al iniciar el proceso. Puede intentar mover el ejecutable o el proyecto a una ruta sin espacios."),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
		else
#endif
			mxMessageDialog(main_window,LANG(DEBUG_ERROR_STARTING,"Error al iniciar el proceso."),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
		return false;
	}
	return true;
}

bool DebugManager::Start(bool update, mxSource *source) {
#ifdef DEBUG_MANAGER_LOG_TALK
	debug_log_file.Open(DEBUG_LOG_FILE,_T("w+"));
#endif
	if (source) {
		// ver si hay que compilar antes
		if (update) {
			if (source->sin_titulo) { // si no esta guardado, siempre compilar
				source->SaveTemp();
				compiler->CompileSource(source,true,true);
				return false;
			} else if (source->GetModify() || !wxFileName::FileExists(source->binary_filename.GetFullPath()) || source->binary_filename.GetModificationTime()<source->source_filename.GetModificationTime()) {
				source->SaveSource();
				compiler->CompileSource(source,true,true);
				return false;
			} else if (config->Running.check_includes && utils->AreIncludesUpdated(source->binary_filename.GetModificationTime(),source->source_filename)) {
				compiler->CompileSource(source,true,true);
				return false;
			}
		}

		if (parser->working) {
			mxMessageDialog(main_window,LANG(DEBUG_MUST_WAIT_FOR_PARSER,"Debe esperar a que se termine de actualizar el arbol de simbolos para comenzar la depuracion."),LANG(GENERAL_WARNING,"Aviso"),mxMD_INFO|mxMD_OK).ShowModal();
			return true;
		}
		SetStateText(LANG(DEBUG_STATUS_STARTING,"Iniciando depuracion..."),true);
		wxString args;
		if (source->config_running.always_ask_args) {
			int res = (new mxArgumentsDialog(main_window,source->exec_args))->ShowModal();
			if (res&AD_CANCEL) return 0;
			if (res&AD_ARGS) {
				source->exec_args  = mxArgumentsDialog::last_arguments;;
				args = source->exec_args ;
			}
			if (res&AD_REMEMBER) {
				source->config_running.always_ask_args=false;
				if (res&AD_EMPTY) source->exec_args =_T("");
			}
		} else if (source->exec_args[0]!='\0')
			args = source->exec_args;	

		compiler->last_caption = source->page_text;
		compiler->last_runned = source;
		if (Start(source->working_folder.GetFullPath(),source->binary_filename.GetFullPath(),args,true,source->config_running.wait_for_key)) {
			int cuantos_sources = main_window->notebook_sources->GetPageCount();
			for (int i=0;i<cuantos_sources;i++) {
				mxSource *src = (mxSource*)(main_window->notebook_sources->GetPage(i));
				if (!src->sin_titulo && source!=src)
					SetBreakPoints(src);
			}
			SetBreakPoints(source);
			if (source->sin_titulo) notitle_source=source;
			if (!Run()) {
#ifdef __WIN32__
				if (source->binary_filename.GetPath().Contains(' '))
					mxMessageDialog(main_window,LANG(DEBUG_ERROR_STARTING_SPACES,"Error al iniciar el proceso. Puede intentar mover el ejecutable o el proyecto a una ruta sin espacios."),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
				else
#endif
					mxMessageDialog(main_window,LANG(DEBUG_ERROR_STARTING,"Error al iniciar el proceso."),LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal();
				return false;
			}
			return true;
		}
	}
	return false;
}

bool DebugManager::Start(wxString workdir, wxString exe, wxString args, bool show_console, bool wait_for_key) {
	mxOSD osd(main_window,project?LANG(OSD_STARTING_DEBUGGER,"Iniciando depuracion..."):"");
	ResetDebuggingStuff();
#if !defined(_WIN32) && !defined(__WIN32__)
	wxString tty_cmd, tty_file(DIR_PLUS_FILE(config->temp_dir,_T("tty.id")));
	if (show_console) {
		if (wxFileName::FileExists(tty_file))
			wxRemoveFile(tty_file);
		tty_cmd<<config->Files.terminal_command<<_T(" ")<<config->Files.runner_command<<_T(" -tty ")<<tty_file;
		tty_cmd.Replace("${TITLE}",LANG(GENERA_CONSOLE_CAPTION,"ZinjaI - Consola de Ejecucion"));
		if (wait_for_key) tty_cmd<<_T(" -waitkey");
	//	utils->ParameterReplace(tty_cmd,_T("${ZINJAI_DIR}"),wxGetCwd());
		tty_process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_DEBUG);
		tty_pid = wxExecute(tty_cmd,wxEXEC_ASYNC,tty_process);
		tty_running = tty_pid!=0;
	} else {
		tty_pid=0; tty_running=false; tty_process=NULL;
	}
#endif
	wxString command(config->Files.debugger_command);
	command<<_T(" -quiet -nx -interpreter=mi");
	if (config->Debug.readnow)
		command<<_T(" --readnow");
	if (project && project->macros_file.Len() && wxFileName(DIR_PLUS_FILE(project->path,project->macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(project->path,project->macros_file)<<_T("\"");
	else if (wxFileName(DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)<<_T("\"");
#if !defined(__WIN32__)
	if (show_console) {
		pid=0;
		wxDateTime t0=wxDateTime::Now(); // algunas terminales no esperan a que lo de adentro se ejecute para devolver el control (mac por ejemplo)
		while ((tty_running || (wxDateTime::Now()-t0).GetSeconds()<10) && !wxFileName::FileExists(tty_file))
			{ wxYield(); wxMilliSleep(100); }
		if (!tty_running && !wxFileName::FileExists(tty_file)) {
			debugging = false;
			mxMessageDialog(main_window,LANG(DEBUG_ERROR_WITH_TERMINAL,"Ha ocurrido un error al iniciar la terminal para la ejecucion.\n"
										   "Compruebe que el campo \"Comando del terminal\" de la pestaña\n"
										   "\"Rutas 2\" del cuadro de \"Preferencias\" sea correcto."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			main_window->compiler_tree.treeCtrl->SetItemText(main_window->compiler_tree.state,_T("Error al iniciar depuracion"));
			return false;
		}
		while (true) {
			wxTextFile ftty(tty_file);
			ftty.Open();
			wxString tty_path = ftty.GetFirstLine();
			if (tty_path.Len()) {
				command<<_T(" -tty=")<<tty_path;
				ftty.Close();
				break;
			}
			ftty.Close();
			{ wxYield(); wxMilliSleep(100); }
		}
	} else
		command<<_T(" -tty=/dev/null");
#endif
	if (args.Len())
		command<<_T(" --args \"")<<exe<<_T("\" ")<</*utils->EscapeString(*/args/*)*/;	
	else
		command<<_T(" \"")<<exe<<_T("\"");	
	process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_DEBUG);
	process->Redirect();
	pid = wxExecute(command,wxEXEC_ASYNC,process);
	if (pid>0) {
		input = process->GetInputStream();
		output = process->GetOutputStream();
		wxString hello = WaitAnswer();
		if (hello.Find(_T("no debugging symbols found"))!=wxNOT_FOUND) {
			mxMessageDialog(main_window,LANG(DEBUG_NO_SYMBOLS,"El ejecutable que se intenta depurar no contiene informacion de depuracion.\nCompruebe que en las opciones de depurarcion este activada la informacion de depuracion,\nverifique que no este seleccionada la opcion \"stripear el ejecutable\" en las opciones de enlazado,\n y recompile el proyecto si es necesario (Ejecucion->Limpiar y luego Ejecucion->Compilar)."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			SendCommand(_T("-gdb-exit"));
			debugging = false; has_symbols=false;
			main_window->compiler_tree.treeCtrl->SetItemText(main_window->compiler_tree.state,_T("Error al iniciar depuracion"));
			return false;
		}
		// configure debugger
#if defined(_WIN32) || defined(__WIN32__)
		SendCommand(_T("-gdb-set new-console on"));
#endif
		if (!config->Debug.auto_solibs) SendCommand(_T("set auto-solib-add off"));
		SetFullOutput(false);
//		SendCommand(_T(BACKTRACE_MACRO));
		SendCommand(wxString(_T("-environment-cd "))<<utils->EscapeString(workdir,true));
		main_window->PrepareGuiForDebugging(gui_is_prepared=true);
		return true;
	} else  {
			mxMessageDialog(main_window,wxString(LANG(DEBUG_ERROR_STATING_GDB,"Ha ocurrido un error al ejecutar el depurador."))
#if !defined(_WIN32) && !defined(__WIN32__)
							<<_T("\n")<<LANG(DEBUG_ERROR_STARTING_GDB_LINUX,"Si el depurador (gdb) no se encuentra instalado\n"
							"en su systema debe instalarlo con el gestor de\n"
							"paquetes que corresponda a su distribucion\n"
							"(apt-get, yum, yast, installpkg, etc.)")
#endif
				,_T("Error al iniciar depurador"),mxMD_OK|mxMD_ERROR).ShowModal();
			main_window->PrepareGuiForDebugging(false);
	}
	pid=0;
	debugging = false;
	main_window->compiler_tree.treeCtrl->SetItemText(main_window->compiler_tree.state,_T("Error al iniciar depuracion"));
	return false;
}


/**
* Resetea los atributos utilizados durante la depuración (como lista de inspecciones,
* banderas de estado, buffer para comunicación con el proceso de gdb, etc).
* Es llamada por Start y LoadCoreDump.
**/

void DebugManager::ResetDebuggingStuff() {
#if !defined(_WIN32) && !defined(__WIN32__)
	tty_running = false;
#endif
	black_list.Clear(); stepping_in=false;
	utils->Split(config->Debug.blacklist,black_list,true,false);
	stopping=false;
	gui_is_prepared = false;
	pause_breakpoint=NULL;
	
	list<mxExternInspection*>::iterator i1=extern_list.begin();
	while (i1!=extern_list.end()) if ((*i1)->SetOutdated()) i1=extern_list.erase(i1); else ++i1;
	
	// setear en -1 todos los ids de los pts de todos interrupcion, para evitar confusiones con depuraciones anteriores
	BreakPointInfo *bpi=NULL;
	while ((bpi=BreakPointInfo::GetGlobalNext(bpi))) bpi->gdb_id=-1;
	
	for (int i=0;i<inspections_count;i++)
		if (inspections[i].is_vo) {
			SetFramelessInspection(i);
		} else if (inspections[i].expr.StartsWith(">")) {
			inspections[i].name=inspections[i].expr.Mid(1);
			inspections[i].on_scope=true;
			inspections[i].frameless=false;
		}
	
	buffer[0]=buffer[1]=buffer[2]=buffer[3]=buffer[4]=buffer[5]=' ';
	buffer[6]='\0';
	debugging = true;
	pid = 0;
	input = NULL;
	output = NULL;
	recording_for_reverse=inverse_exec=false;
	main_window->ClearDebugLog();
	has_symbols=true;
}


bool DebugManager::Attach(long apid, mxSource *source) {
#ifdef DEBUG_MANAGER_LOG_TALK
	debug_log_file.Open(DEBUG_LOG_FILE,"w+");
#endif
	mxOSD osd(main_window,LANG(OSD_STARTING_DEBUGGER,"Iniciando depuracion..."));
	ResetDebuggingStuff();
	wxString exe = source?source->binary_filename.GetFullPath():DIR_PLUS_FILE(project->path,project->active_configuration->output_file);
	wxString command(config->Files.debugger_command);
	command<<_T(" -quiet -nx -interpreter=mi");
	if (config->Debug.readnow)
		command<<_T(" --readnow");
	if (project && project->macros_file.Len() && wxFileName(DIR_PLUS_FILE(project->path,project->macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(project->path,project->macros_file)<<_T("\"");
	else if (wxFileName(DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)<<_T("\"");
	process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_DEBUG);
	process->Redirect();
	pid = wxExecute(command,wxEXEC_ASYNC,process);
	if (pid>0) {
		input = process->GetInputStream();
		output = process->GetOutputStream();
		wxString hello = WaitAnswer();
		if (hello.Find(_T("no debugging symbols found"))!=wxNOT_FOUND) {
			mxMessageDialog(main_window,LANG(DEBUG_NO_SYMBOLS,"El ejecutable que se intenta depurar no contiene informacion de depuracion.\nCompruebe que en las opciones de depurarcion este activada la informacion de depuracion,\nverifique que no este seleccionada la opcion \"stripear el ejecutable\" en las opciones de enlazado,\n y recompile el proyecto si es necesario (Ejecucion->Limpiar y luego Ejecucion->Compilar)."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			SendCommand(_T("-gdb-exit"));
			debugging = false;
			return false;
		}
		// configure debugger
		SetFullOutput(false);
//		SendCommand(_T(BACKTRACE_MACRO));
		main_window->PrepareGuiForDebugging(gui_is_prepared=true);
		// mostrar el backtrace y marcar el punto donde corto
		SendCommand(wxString(_T("attach "))<<apid);
		SetStateText(wxString(LANG(DEBUG_STATUS_ATTACHING_TO,"Depurador adjuntado al proceso "))<<apid);
		Backtrace(true);
		long line;
		main_window->backtrace_ctrl->GetCellValue(0,BG_COL_LINE).ToLong(&line);
		wxString file = main_window->backtrace_ctrl->GetCellValue(0,BG_COL_FILE);
		if (file.Len()) {
			debug->MarkCurrentPoint(file,line,mxSTC_MARK_STOP);
			debug->SelectFrame(main_window->backtrace_ctrl->GetCellValue(0,BG_COL_LEVEL),0);
		}
		if (project) 
			project->SetBreakpoints();
		else {
			int cuantos_sources = main_window->notebook_sources->GetPageCount();
			for (int i=0;i<cuantos_sources;i++) {
				mxSource *src = (mxSource*)(main_window->notebook_sources->GetPage(i));
				if (!src->sin_titulo && source!=src)
					SetBreakPoints(src);
			}
			SetBreakPoints(source);
		}
		return true;
	}
	debugging = false;
	pid = 0;
	return false;	
}
	

/**
* Inicia el depurador cargando un volcado de memoria desde un archivo en lugar
* de ejecutar el programa. Para generar estos dumps en GNU/Linux, ejecutar
* "ulimit -c unlimited" en el shell; luego, al ejecutar un programa, si este
* revienta por un segfault, genera un archivo core*. En esta función, si se
* recibe un mxSource se asume que es un programa simple, sino (source==NULL)
* que se trata de un proyecto. Esta opción no está disponible en la versión
* para Windows.
* @param core_file la ruta completa al archivo core
* @param source puntero al mxSource si es un programa simple, NULL si es un proyecto
* @retval true si se cargo correctamente
* @retval false si hay error
**/

bool DebugManager::LoadCoreDump(wxString core_file, mxSource *source) {
#ifdef DEBUG_MANAGER_LOG_TALK
	debug_log_file.Open(DEBUG_LOG_FILE,"w+");
#endif
	
	mxOSD osd(main_window,project?LANG(OSD_LOADING_CORE_DUMP,"Cargando volcado de memoria..."):"");
	
	ResetDebuggingStuff();
	wxString exe = source?source->binary_filename.GetFullPath():DIR_PLUS_FILE(project->path,project->active_configuration->output_file);
	wxString command(config->Files.debugger_command);
	command<<_T(" -quiet -nx -interpreter=mi");
	if (config->Debug.readnow)
		command<<_T(" --readnow");
	if (project && project->macros_file.Len() && wxFileName(DIR_PLUS_FILE(project->path,project->macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(project->path,project->macros_file)<<_T("\"");
	else if (wxFileName(DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)<<_T("\"");
	command<<_T(" -c \"")<<core_file<<_T("\" \"")<<exe<<_T("\"");	
	process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_DEBUG);
	process->Redirect();
	pid = wxExecute(command,wxEXEC_ASYNC,process);
	if (pid>0) {
		input = process->GetInputStream();
		output = process->GetOutputStream();
		wxString hello = WaitAnswer();
		/// @todo: el mensaje puede aparecer por las bibliotecas, ver como diferenciar
//		if (hello.Find(_T("no debugging symbols found"))!=wxNOT_FOUND) {
//			mxMessageDialog(main_window,LANG(DEBUG_NO_SYMBOLS,"El ejecutable que se intenta depurar no contiene informacion de depuracion.\nCompruebe que en las opciones de depurarcion que este activada la informacion de depuracion,\nverifique que no este seleccionada la opcion \"stripear el ejecutable\" en las opciones de enlazado,\n y recompile el proyecto si es necesario (Ejecucion->Limpiar y luego Ejecucion->Compilar)."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
//			SendCommand(_T("-gdb-exit"));
//			debugging = false;
//			return false;
//		}
		// configure debugger
		SetFullOutput(false);
//		SendCommand(_T(BACKTRACE_MACRO));
		main_window->PrepareGuiForDebugging(gui_is_prepared=true);
		// mostrar el backtrace y marcar el punto donde corto
		Backtrace(true);
		UpdateInspection();
		long line;
		main_window->backtrace_ctrl->GetCellValue(0,BG_COL_LINE).ToLong(&line);
		wxString file = main_window->backtrace_ctrl->GetCellValue(0,BG_COL_FILE);
		if (file.Len()) {
			debug->MarkCurrentPoint(file,line,mxSTC_MARK_STOP);
			debug->SelectFrame(main_window->backtrace_ctrl->GetCellValue(0,BG_COL_LEVEL),0);
		}
		return true;
	}
	pid=0;
	debugging = false;
	return false;
}

bool DebugManager::Stop() {
	if (stopping) return false; else stopping = true;
	if (waiting || !debugging) {
#if defined(__APPLE__) || defined(__WIN32__)
		process->Kill(pid,wxSIGKILL);
		return false;
#else
		Pause();
#endif
	}
	waiting=true;
	if (pid) {
		last_command=_T("-gdb-exit\n");
		output->Write(_T("-gdb-exit\n"),10);
	}
	return true;
}

bool DebugManager::Run() {
	if (waiting || !debugging) return false;
	running = true;
	wxString ans = SendCommand(_T("-exec-run"));
	if (ans.Contains(_T("^running"))) {
		SetStateText(LANG(DEBUG_STATUS_RUNNING,"Ejecutando..."));
		HowDoesItRuns();
		if (config->Debug.raise_main_window)
			main_window->Raise();
		running = false;
		return true;
	}
	running = false;
	return false;
}

void DebugManager::HowDoesItRuns() {
#if defined(_WIN32) || defined(__WIN32__)
	static wxString sep=_T("\\"),wrong_sep=_T("/");
#else
	static wxString sep=_T("/"),wrong_sep=_T("\\");
#endif
	SetStateText(LANG(DEBUG_STATUS_RUNNING,"Ejecutando..."));
	MarkCurrentPoint();
	really_running = true;
	wxString ans = WaitAnswer(), state_text=LANG(DEBUG_STATUS_UNKNOWN,"Estado desconocido");

	really_running = false;
	if (!process || stopping) return;
	int st_pos = ans.Find(_T("*stopped"));
	if (st_pos==wxNOT_FOUND) {
		SetStateText(state_text);
#ifdef DEBUG_MANAGER_LOG_TALK
		wxString debug_log_string; debug_log_string<<"ERROR RUNNING: "<<ans;
		debug_log_file.Write(debug_log_string);
		debug_log_file.Flush();
#endif
		return;
	}
	ans=ans.Mid(st_pos);
	wxString how = GetValueFromAns(ans,_T("reason"),true);
	int mark = 0;
	int disable = -1;
	if (how==_T("breakpoint-hit")) {
		wxString sbn = GetValueFromAns(ans,_T("bkptno"),true);
		if (sbn.Len()) {
			long bn;
			sbn.ToLong(&bn);
			disable=bn;
		}
		mark = mxSTC_MARK_EXECPOINT;
		state_text=LANG(DEBUG_STATUS_BREAK,"El programa alcanzo un punto de interrupcion");
	} else if (how==_T("watchpoint-trigger") || how==_T("access-watchpoint-trigger") || how==_T("read-watchpoint-trigger")) {
		mark = mxSTC_MARK_EXECPOINT;
		state_text=LANG(DEBUG_STATUS_WATCH,"El programa se interrumpio por un WatchPoint: ");
		long l;
		GetValueFromAns(ans.AfterFirst('{'),_T("number"),true).ToLong(&l);
		for (int i=0;i<inspections_count;i++) {
			if (inspections[i].watch_read && inspections[i].watch_write && inspections[i].watch_num==l) {
				state_text<<inspections[i].expr;
				inspection_grid->SelectRow(i);
				break;
			}
		}
	} else if (how==_T("watchpoint-scope")) {
		mark = mxSTC_MARK_EXECPOINT;
		state_text=LANG(DEBUG_STATUS_WATCH_OUT,"El WatchPoint ha dejado de ser valido: ");
		long l;
		GetValueFromAns(ans,_T("wpnum"),true).ToLong(&l);
		for (int i=0;i<inspections_count;i++) {
			if (inspections[i].watch_read && inspections[i].watch_write && inspections[i].watch_num==l) {
				state_text<<inspections[i].expr;
				inspections[i].watch_read = inspections[i].watch_write = false;
				inspection_grid->SetCellValue(i,IG_COL_WATCH,_T("no"));
				inspection_grid->SelectRow(i);
				break;
			}
		}
	} else if (how==_T("location-reached")) {
		mark = mxSTC_MARK_EXECPOINT;
		state_text=LANG(DEBUG_STATUS_LOCATION_REACHED,"El programa alcanzo la ubicacion seleccionada");
	} else if (how==_T("function-finished")) {
		wxString retval = GetValueFromAns(ans,_T("return-value"),true);
		mark = mxSTC_MARK_EXECPOINT;
		if (retval.Len()) {
			state_text=LANG(DEBUG_STATUS_FUNCTION_ENDED_RETVAL,"Valor de retorno: ");
			state_text<<retval;
		} else 
			state_text=LANG(DEBUG_STATUS_FUNCTION_ENDED,"La funcion ha finalizado.");
	} else if (how==_T("end-stepping-range")) {
		mark = mxSTC_MARK_EXECPOINT;
		state_text=LANG(DEBUG_STATUS_STEP_DONE,"Paso avanzado");
	} else if (how==_T("exited-normally") || how==_T("exited")) {
		if (how==_T("exited-normally"))
			state_text=LANG(DEBUG_STATUS_ENDED_NORMALLY,"El programa finalizo con normalidad");
		else
			state_text=wxString(LANG(DEBUG_STATUS_EXIT_WITH_CODE,"El programa finalizo con el codigo de salida "))<<GetValueFromAns(ans,_T("exit-code"),true);
	} else if (how==_T("signal-received")) {
		if (pause_breakpoint) {
			if (pause_breakpoint->gdb_status==BPS_PENDING)
				SetBreakPoint(pause_breakpoint);
			else
				DeleteBreakPoint(pause_breakpoint);
			pause_breakpoint=NULL;
			Continue();
			return;
		}
		if (GetValueFromAns(ans,_T("signal-meaning"),true)==_T("Trace/breakpoint trap")) {
			mark = mxSTC_MARK_EXECPOINT;
			state_text=LANG(DEBUG_STATUS_TRAP,"El programa se interrumpio para ceder el control al depurador");
#ifdef __WIN32__
			SendCommand(_T("-thread-select 1"));
#endif
		} else {
			mark = mxSTC_MARK_STOP;
			state_text=wxString(LANG(DEBUG_STATUS_ABNORMAL_INTERRUPTION,"El programa se interrumpio anormalmente ( "))<<GetValueFromAns(ans,_T("signal-name"),true)<<_T(" = ")<<GetValueFromAns(ans,_T("signal-meaning"),false)<<_T(" )");
		}
	} else if (how==_T("exited-signaled") || how==_T("exited")) {
		mark = mxSTC_MARK_STOP;
		state_text=LANG(DEBUG_STATUS_TERMINAL_CLOSED,"La terminal del programa ha sido cerrada");
	} 
#ifdef DEBUG_MANAGER_LOG_TALK
 	else{ 
		wxString debug_log_string; debug_log_string<<"NEW REASON: "<<ans;
		debug_log_file.Write(debug_log_string);
		debug_log_file.Flush();
	}
#endif
	if (mark) {
		wxString fname = GetSubValueFromAns(ans,_T("frame"),_T("fullname"),true,true);
		if (!fname.Len())
			fname = GetSubValueFromAns(ans,_T("frame"),_T("file"),true,true);
		fname.Replace(_T("//"),sep);
		fname.Replace(_T("\\\\"),sep);
		fname.Replace(wrong_sep,sep);
		wxString line =  GetSubValueFromAns(ans,_T("frame"),_T("line"),true);
		long fline = -1;
		//if (backtrace_visible && config->Debug.autoupdate_backtrace)
		if (stepping_in && mark==mxSTC_MARK_EXECPOINT && black_list.Index(fname)!=wxNOT_FOUND)
			StepIn();
		else {
			stepping_in=false;
			if (line.ToLong(&fline)) {
				MarkCurrentPoint(fname,fline,mark);
				if (threadlist_visible) ListThreads();
				Backtrace(true);
			} else {
				if (threadlist_visible) ListThreads();
				Backtrace(false);
			}
			UpdateInspection();
		}
	} else {
//		if (config->Debug.autoupdate_backtrace)
			BacktraceClean();
			ThreadListClean();
		running = false;
		Stop();
	}
	if (disable>=0) {
		BreakPointInfo *bpi=BreakPointInfo::FindFromNumber(disable,true);
		if (bpi && bpi->only_once) bpi->SetStatus(BPS_DISABLED_ONLY_ONCE);
	}
	SetStateText(state_text);
}

/**
* @brief Removes a breakpoint from gdb and from ZinjaI's list
*
* If !debug->running it deletes the brekapoint inmediatly, but if its running
* it marks the breakpoint to be deleted with pause_* and Pause the execution.
* Next time debug see the execution paused will invoke this method again and 
* resume the execution, so user can delete breakpoint without directly pausing
* the program being debugged.
**/
void DebugManager::DeleteBreakPoint(BreakPointInfo *_bpi) {
	if (!debugging) return;
	if (waiting) { // si esta ejecutando, anotar para sacar y mandar a pausar
		pause_breakpoint=_bpi;
		Pause();
	} else {
		// decirle a gdb que lo saque
		SendCommand(_T("-break-delete "),_bpi->gdb_id);
		// sacarlo de la memoria y las listas de zinjai
		delete _bpi;
	}
}

int DebugManager::SetLiveBreakPoint(BreakPointInfo *_bpi) {
	if (debugging && waiting) { 
		pause_breakpoint=_bpi;
		_bpi->SetStatus(BPS_PENDING);
		Pause();
		return -1;
	} else {
		return SetBreakPoint(_bpi);
	}
}

/**
* Coloca un pto de interrupción en gdb
*
* Le pide al depurador agregar un pto de interrupción. Esta función se usa tanto 
* para proyectos como programas simples, y se llama desde mxSource::OnMarginClick,
* DebugManager::HowDoesItRuns y ProjectManager::SetBreakpoints y 
* DebugManager::SetBreakPoints. Si logra colocar el bp retorna su id, sino -1.
*
* Setea además las propiedades adicionales si están definidas en el BreakPointInfo.
*
* Antes de colocar el breakpoint se fija si es una direccion valida, porque
* sino gdb lo coloca más adelante sin avisar.
**/
int DebugManager::SetBreakPoint(BreakPointInfo *_bpi) {
	if (waiting || !debugging) return 0;
	wxString adr = GetAddress(_bpi->fname,_bpi->line_number);
	if (!adr.Len()) { _bpi->SetStatus(BPS_ERROR_SETTING); ShowBreakPointLocationErrorMessage(_bpi); return -1;  }
	wxString ans = SendCommand(wxString(_T("-break-insert \"\\\""))<<_bpi->fname<<_T(":")<<_bpi->line_number+1<<_T("\\\"\""));
	wxString num = GetSubValueFromAns(ans,_T("bkpt"),_T("number"),true);
	if (!num.Len()) { // a veces hay que poner dos barras (//) antes del nombre del archivo en vez de una (en los .h? ¿por que?)
		wxString file=_bpi->fname;
		int p = file.Find('/',true);
		if (p!=wxNOT_FOUND) {
			wxString file2 = file.Mid(0,p);
			file2<<'/'<<file.Mid(p);
			ans = SendCommand(wxString(_T("-break-insert \""))<<file2<<_T(":")<<_bpi->line_number+1<<_T("\""));
			num = GetSubValueFromAns(ans,_T("bkpt"),_T("number"),true);
		}
	}
	int id=-1;
	BREAK_POINT_STATUS status=BPS_SETTED;
	if (num.Len()) {
		long l; num.ToLong(&l); id=l;
		// setear las opciones adicionales
		if (_bpi->ignore_count) SetBreakPointOptions(id,_bpi->ignore_count);
		if (_bpi->only_once||!_bpi->enabled) SetBreakPointEnable(id,_bpi->enabled,_bpi->only_once);
		if (!_bpi->enabled) status=BPS_USER_DISABLED;
		if (_bpi->cond.Len()) if (!SetBreakPointOptions(id,_bpi->cond)) { status=BPS_ERROR_SETTING; ShowBreakPointConditionErrorMessage(_bpi); }
	} else { // si no se pudo colocar correctamente
		status=BPS_ERROR_SETTING; 
		ShowBreakPointLocationErrorMessage(_bpi);
	}
	_bpi->SetStatus(status,id);
	return id;
}

wxString DebugManager::InspectExpression(wxString var, bool pretty) {
	if (waiting || !debugging) return _T("");
	if (!pretty) {
		return GetValueFromAns( SendCommand(_T("-data-evaluate-expression "),utils->EscapeString(var,true)),_T("value") ,true,true);
	} else {
		SendCommand("-gdb-set print pretty on");
		wxString ret = GetValueFromAns( SendCommand(_T("-data-evaluate-expression "),utils->EscapeString(var,true)),_T("value") ,true,true);
		SendCommand("-gdb-set print pretty off");
		return ret;
	}
//	wxString ret = GetValueFromAns( SendCommand(wxString(_T("-data-evaluate-expression "))<<var),_T("value") ,true);
//	if (ret.Mid(0,2)==_T("\\\"") && ret.Mid(ret.Len()-2,2)==_T("\\\"")) {
//		ret[ret.Len()-2]='\"';
//		return ret.Mid(1,ret.Len()-2);
//	}
//	return ret;
}


bool DebugManager::Backtrace(bool dont_select_if_first) {
#if defined(_WIN32) || defined(__WIN32__)
	static wxString sep=_T("\\"),wrong_sep=_T("/");
#else
	static wxString sep=_T("/"),wrong_sep=_T("\\");
#endif
	if (waiting || !debugging) 
		return false;
	main_window->backtrace_ctrl->BeginBatch();
	
	// averiguar las direcciones de cada frame, para saber donde esta cada inspeccion V2
	long fdepth=0;
	if (GetValueFromAns(SendCommand(_T("-stack-info-depth")),_T("depth"),true).ToLong(&fdepth)) {
		if (fdepth>BACKTRACE_SIZE) fdepth=BACKTRACE_SIZE;
		for (int i=0;i<fdepth;i++) frames_addrs[i]=wxString()<<fdepth+1-i;
	} else {
		for (int i=0;i<BACKTRACE_SIZE;i++) frames_addrs[i]="error"; 
	}
	
	wxString frames = fdepth?SendCommand("-stack-list-frames 0 ",fdepth-1):"";
	int i=0,p,c,n=0,sll=frames.Len();
	const wxChar * chfr = frames.c_str();
	// to_select* es para marcar el primer frame que tenga info de depuracion
	wxString line_str,to_select, to_select_file; 
	bool to_select_done=false;
	int to_select_line, to_select_row=-1;
	i=frames.Find(_T("stack="));
	if (i==wxNOT_FOUND) {
		for (int c=0;c<fdepth;c++) {
			main_window->backtrace_ctrl->SetCellValue(c,BG_COL_FILE,_T("<<Imposible determinar ubicacion>>"));
			main_window->backtrace_ctrl->SetCellValue(c,BG_COL_FUNCTION,_T("<<Imposible determinar ubicacion>>"));
		}
		main_window->backtrace_ctrl->EndBatch();
	} else {
		i+=7;
		while (true) {
			while (i<sll && chfr[i]!='{') 
				i++; 
			if (i<sll) c=n++; else break;
			if (c==BACKTRACE_SIZE) break; // ver que consecuecias tiene
			p=i+1;
			while (chfr[i]!='}') 
				i++; 
			wxString s(frames.SubString(p,i-1));
			main_window->backtrace_ctrl->SetCellValue(c,BG_COL_LEVEL,frames_nums[c]=GetValueFromAns(s,_T("level"),true));
			wxString func = GetValueFromAns(s,_T("func"),true);
			if (func[0]=='?') {
				main_window->backtrace_ctrl->SetCellValue(c,BG_COL_FUNCTION,_T("<<informacion no disponible>>"));
				main_window->backtrace_ctrl->SetCellValue(c,BG_COL_FILE,_T(""));
				main_window->backtrace_ctrl->SetCellValue(c,BG_COL_LINE,_T(""));
				main_window->backtrace_ctrl->SetCellValue(c,BG_COL_ARGS,_T(""));
			} else {
				main_window->backtrace_ctrl->SetCellValue(c,BG_COL_FUNCTION,func);
	//			frames_addrs[c]=SendCommand(_T("info frame "),frames_nums[c]).BeforeFirst(':').AfterLast(' ');
				wxString fname = GetValueFromAns(s,_T("fullname"),true,true);
				if (!fname.Len())
					fname = GetValueFromAns(s,_T("file"),true,true);
				fname.Replace(_T("\\\\"),sep,true);
				fname.Replace(_T("//"),sep,true);
				fname.Replace(wrong_sep,sep,true);
				main_window->backtrace_ctrl->SetCellValue(c,BG_COL_FILE,fname);
				// seleccionar el frame de mas arriba que tenga info de depuracion (que en line diga algo)
				line_str=GetValueFromAns(s,_T("line"),true);
				if (!to_select_done && line_str.Len() && wxFileName::FileExists(fname)) {
					to_select_done=true; 
					to_select=GetValueFromAns(s,_T("level"),true);
					long l=0;
					line_str.ToLong(&l);
					to_select_line=l;
					to_select_file=fname;

					to_select_row=c;
				}
				main_window->backtrace_ctrl->SetCellValue(c,BG_COL_LINE,line_str);
			}
		}
	}
//	// averiguar las direcciones de cada frame, para saber donde esta cada inspeccion
//	wxString frame_address = SendCommand(_T("zframeaddress "),n);
//	for (i=0;i<n;i++) {
//		int p = frame_address.Find(_T("*zframe-"));
//		if (p==wxNOT_FOUND) { 
//			frames_addrs[i]="error"; 
//		} else {
//			frame_address=frame_address.Mid(p+8);
//			frames_addrs[i]=frame_address.BeforeFirst(':').AfterLast(' ');
//		}
//	}
		
	wxString args ,args_list = n?SendCommand("-stack-list-arguments 1 0 ",n-1):"";
	int arglev;
	const wxChar * chag = args_list.c_str();
//cerr<<"CHAG="<<endl<<chag<<endl<<endl;
	bool comillas = false, cm_dtype=false; //cm_dtype indica el tipo de comillas en que estamos, inicializar en false es solo para evitar el warning
	i=args_list.Find(_T("stack-args="));
	if (i==wxNOT_FOUND) {
		for (int c=0;c<fdepth;c++)
			main_window->backtrace_ctrl->SetCellValue(c,BG_COL_ARGS,_T("<<Imposible determinar argumentos>>"));
		main_window->backtrace_ctrl->EndBatch();
	} else {
		i+=12;
		for (int c=0;c<n;c++) {
			// chag+i = frame={level="0",args={{name="...
			while (chag[i]!='[' && chag[i]!='{') 
				i++; 
			p=++i;
			arglev=0;
			// chag+i = level="0",args={{name="...
			while ((chag[i]!=']' && chag[i]!='}') || comillas || arglev>0) {
				if (comillas) {
					if (cm_dtype && chag[i]=='\"') 
						comillas=false;
					else if (!cm_dtype && chag[i]=='\'') 
						comillas=false;
					else if (chag[i]=='\\') i++;
				} else {
					if (chag[i]=='\"' || chag[i]=='\'')
						{ comillas=true; cm_dtype=(chag[i]=='\"'); }
					else if (!comillas && (chag[i]=='{' || chag[i]=='['))
						arglev++;
					else if (!comillas && (chag[i]==']' || chag[i]=='}'))
						arglev--;
					else if (chag[i]=='\\') i++;
				}
				i++;
			}
			wxString s(args_list.SubString(p,i-1));
			wxString args, sub;
			int j=0, l=s.Len();
			const wxChar * choa = s.c_str();
			// choa+j = level="0",args={{name="...
			while (j<l && choa[j]!='{' && choa[j]!='[')	j++; j++;
			// choa+j = {name="...
			while (j<l) {
				while (j<l && choa[j]!='{' && choa[j]!='[')
					j++;
				if (j==l)
					break;
				p=++j;
				arglev=0; comillas=false;
				while (choa[j]!='}' || comillas || arglev>0) {
					if (comillas) {
						if (cm_dtype && choa[j]=='\"') 
							comillas=false;
						else if (!cm_dtype && choa[j]=='\'') 
							comillas=false;
						else if (choa[j]=='\\') j++;
					} else {
						if (choa[j]=='\"' || choa[j]=='\'')
						{ comillas=true; cm_dtype=(choa[j]=='\"'); }
						else if (choa[j]=='{' || choa[j]=='[')
							arglev++;
						else if (choa[j]==']' || choa[j]=='}')
							arglev--;
						else if (choa[j]=='\\') j++;
					}
					j++;
				}
				sub=s.SubString(p,j-1);
				if (args.Len())
					args<<_T(", ");
				args<<GetValueFromAns(sub,_T("name"),true)<<_T("=")<<GetValueFromAns(sub,_T("value"),true,true);
			}
			main_window->backtrace_ctrl->SetCellValue(c,BG_COL_ARGS,args);
		}	
	}
	c=n;
	while (c<last_backtrace_size) { // borrar los renglones que sobran
		for (int i=0;i<BG_COLS_COUNT;i++)
			main_window->backtrace_ctrl->SetCellValue(c,i,_T(""));
		c++;
	}
	main_window->backtrace_ctrl->SelectRow(0);
	last_backtrace_size = n;
	if (!dont_select_if_first || to_select!=frames_nums[0]) {
		if (to_select_row==-1) {
			current_frame_num=frames_nums[0];
			current_frame=frames_addrs[0];
			debug->MarkCurrentPoint(to_select_file,to_select_line);
		} else {
			main_window->backtrace_ctrl->SelectRow(to_select_row);
			SelectFrame(to_select,to_select_row);
			debug->MarkCurrentPoint(to_select_file,to_select_line,mxSTC_MARK_FUNCCALL);
		}
	}
	else if (last_backtrace_size>0) {
		current_frame_num=frames_nums[0];
		current_frame=frames_addrs[0];
	} else 
		current_frame=current_frame_num=_T("");
	main_window->backtrace_ctrl->EndBatch();
	return true;
}

void DebugManager::StepIn() {
	if (waiting || !debugging) return;
	running = true;
	stepping_in=true; really_running=true;
	wxString ans = SendCommand(_T("-exec-step"));
	if (ans.Mid(1,7)=_T("running"))
		HowDoesItRuns();
	running = false;
}

void DebugManager::StepOut() {
	if (waiting || !debugging) return;
	running = true; really_running=true;
	wxString ans = SendCommand(_T("-exec-finish"));
	if (ans.Mid(1,7)=_T("running"))
		HowDoesItRuns();
	running = false;
}

void DebugManager::StepOver() {
	if (waiting || !debugging) return;
	running = true; really_running=true;
	wxString ans = SendCommand(_T("-exec-next"));
	if (ans.Mid(1,7)=_T("running"))
		HowDoesItRuns();
	running = false;
}


void DebugManager::Pause() {
	if (!waiting && !debugging) return;
#if defined(_WIN32) || defined(__WIN32__)
	if (!winLoadDBP()) {
		mxMessageDialog(main_window,_T("Esta caracteristica no se encuentra presente en versiones de Windows previas a XP-SP2"),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();		
		return;
	}
	long child_pid = winGetChildPid(pid);
	if (child_pid) winDebugBreak(child_pid);
	
#else
	process->Kill(pid,wxSIGINT);
#endif
}

void DebugManager::Continue() {
	if (waiting || !debugging) return;
	running = true;
	MarkCurrentPoint();
	wxString ans = SendCommand(_T("-exec-continue"));
	if (!stopping && ans.Mid(1,7)==_T("running")) {
		HowDoesItRuns();
		if (config->Debug.raise_main_window)
			main_window->Raise();
	}
	running = false;
}

wxString DebugManager::WaitAnswer() {
	waiting = true;
	static wxString ret,warn;
	ret.Clear(); warn.Clear();
	int c=6,i,iwarn;
	bool on_warn=false, first=true;
	while (process) {
		// cortar warnings y otros mensajes del depurador para el usuario
		while (buffer[c]!='\0') {
			if (first || buffer[c]=='\r' || buffer[c]=='\n') {
				if (on_warn) {
					// si estabamos en un warning (mensaje para el debug_log_panel, o interesante para analizar por el DebugManager)
					buffer[c]='\0';
					warn<<buffer+iwarn;
					if (warn.StartsWith("&\"Error in re-setting breakpoint ")) {
						long bn=-1;	if (warn.Mid(33).BeforeFirst(':').ToLong(&bn)) {
							BreakPointInfo *bpi=BreakPointInfo::FindFromNumber(bn,true);
							if (bpi) bpi->SetStatus(BPS_ERROR_SETTING);
							if (bpi) ShowBreakPointLocationErrorMessage(bpi);
						}
					} else if (warn[0]=='=' || warn.StartsWith("&\"warning:") ) {
						if (warn[0]=='&') warn=warn.Mid(2,warn.Len()-3);
						main_window->AddToDebugLog(warn);
					} 
					// eliminar el warning del buffer
					warn.clear(); i=iwarn; c++;
					while (buffer[c]!='\0')
						buffer[i++]=buffer[c++];
					buffer[i]='\0'; c=iwarn;
				} else {
					if (!first) c++; else first=false;
				}
				if ((on_warn = (buffer[c]=='=' || buffer[c]=='&'))) iwarn=c;
			} else c++;
		}
		if (on_warn) {
			warn<<buffer+iwarn;
			buffer[iwarn]='\0';
		}
		c=i=6;
		while (buffer[c]!='\0' && ! ((buffer[c]=='\n' || buffer[c]=='\r') && buffer[c-1]==' ' && buffer[c-2]==')' && buffer[c-3]=='b' && buffer[c-4]=='d' && buffer[c-5]=='g' && buffer[c-6]=='('))
			c++;
		if (buffer[c]!='\0') {
			buffer[c-6]='\0';
			ret<<buffer+6;
			while (buffer[c]=='\r' || buffer[c]=='\n')
				c++;
			while (buffer[c]!='\0')
				buffer[i++]=buffer[c++];
			buffer[i]='\0';

#ifdef DEBUG_MANAGER_LOG_TALK
		wxString debug_log_string; debug_log_string<<"\n<<< "<<ret;
		debug_log_file.Write(debug_log_string);
		debug_log_file.Flush();
#endif
			
			waiting = false;
			last_answer = ret;
			return ret;
		}
		if(c!=6) {
			ret<<buffer+6;
			buffer[5]=buffer[c-1];
			buffer[4]=buffer[c-2];
			buffer[3]=buffer[c-3];
			buffer[2]=buffer[c-4];
			buffer[1]=buffer[c-5];
			buffer[0]=buffer[c-6];
			buffer[6]='\0';
		}
		wxDateTime t1=wxDateTime::Now();
		while ( process && ! input->CanRead() && input->IsOk()) {
			if (really_running)  {
				app->Yield(true);
				wxMilliSleep(50);
			} else {
				wxMilliSleep(10);
				wxTimeSpan t2=t1-wxDateTime::Now();
				if (t2.GetSeconds()>30 && mxMD_YES==mxMessageDialog(main_window,_T("Alguna operación en el depurador está tomando demasiado tiempo, desea interrumpirla?."),_T("UPS!"),(mxMD_YES_NO|mxMD_WARNING)).ShowModal())
						return _T("");
				else t1=wxDateTime::Now();
			}
		}
		if (process && input->IsOk()) 
			c = input->Read(buffer+6,249).LastRead();
		else break;
		buffer[c+6]='\0'; iwarn=c=6;
		
	}
	last_answer = _T("");
	return _T("");
}

wxString DebugManager::SendCommand(wxString command) {
#ifdef DEBUG_MANAGER_LOG_TALK
		wxString debug_log_string; debug_log_string<<"\n>>> "<<command;
		debug_log_file.Write(debug_log_string);
		debug_log_file.Flush();
#endif
	if (!process) return _T("");
	last_command=command;
	output->Write(command.c_str(),command.Len());
	output->Write("\n",1);
	return WaitAnswer();
}

wxString DebugManager::SendCommand(wxString command, int i) {
#ifdef DEBUG_MANAGER_LOG_TALK
		wxString debug_log_string; debug_log_string<<"\n>>> "<<command<<i;
		debug_log_file.Write(debug_log_string);
		debug_log_file.Flush();
#endif
	if (!process) return _T("");
	command<<i<<_T("\n");
	last_command=command;
	output->Write(command.c_str(),command.Len());
	return WaitAnswer();
}

wxString DebugManager::SendCommand(wxString cmd1, wxString cmd2) {
#ifdef DEBUG_MANAGER_LOG_TALK
		wxString debug_log_string; debug_log_string<<"\n>>> "<<cmd1<<cmd2;
		debug_log_file.Write(debug_log_string);
		debug_log_file.Flush();
#endif
	if (!process) return _T("");
	cmd1<<cmd2<<_T("\n");
	last_command=cmd1;
	output->Write(cmd1.c_str(),cmd1.Len());
	return WaitAnswer();
}

/// @brief Sets all breakpoints from an untitled or out of project mxSource
void DebugManager::SetBreakPoints(mxSource *source) {
	if (waiting || !debugging) return;
	BreakPointInfo *bpi=*source->breaklist;
	while (bpi) {
		bpi->UpdateLineNumber();
		debug->SetBreakPoint(bpi);
		bpi=bpi->Next();
	}
}

/**
* @brief Marca el punto actual del código donde se encuentra el depurador
*
* Marca el punto donde se encuentra en el frame actual, borrando la marca anterior
* si es que había. Si se invoca sin argumentos borra la marca anterior y nada mas.
* La marca es la flecha en el margen, verde para ejecucion normal y en el frame
* interior, amarillo para otro frame, rojo cuando se detuvo por un problema
* irrecuperable, como un segfault.
*
* @param cfile path completo del archivo
* @param cline numero de line en Base 1
* @param cmark tipo de marcador: -1 (ninguna), mxSTC_MARK_EXECPOINT (verde), mxSTC_MARK_FUNCCALL (amarillo), mxSTC_MARK_STOP (rojo)
**/
bool DebugManager::MarkCurrentPoint(wxString cfile, int cline, int cmark) {
	if (cmark!=-1) {
		if (current_source) {
			if (current_handle!=-1) 
				current_source->MarkerDeleteHandle(current_handle);
			if ( (current_source->GetFullPath()) !=cfile ) {
				current_source = main_window->IsOpen(cfile);
				if (!current_source) {
					if (notitle_source && notitle_source->temp_filename==cfile)
						current_source = notitle_source;
					else
						current_source = main_window->OpenFile(cfile,!project);
				}
			}
		} else {
			if (notitle_source && notitle_source->temp_filename==cfile)
				current_source = notitle_source;
			else {
				current_source = main_window->IsOpen(cfile);
				current_source = main_window->OpenFile(cfile,!project);
			}
		}
		if (current_source==EXTERNAL_SOURCE) current_source=NULL;
		if (current_source) {
			int x=main_window->notebook_sources->GetPageIndex(current_source);
			if (x!=main_window->notebook_sources->GetSelection())
				main_window->notebook_sources->SetSelection(x);
			current_handle = current_source->MarkerAdd(cline-1,cmark);
			current_source->MarkError(cline-1);
		}
	} else {
		if (current_source) {
			current_source->MarkerDeleteHandle(current_handle);
			current_handle=-1;
		}
	}
	return current_source;
}

void DebugManager::CloseSource(mxSource *source) {
	if (source==current_source)
		current_source=NULL;
	if (source==notitle_source)
		notitle_source=NULL;
}

/**
* @brief Parsea la salida de un comando enviado a gdb para extraer un campo
*
* @param ans        cadena con el resultado arrejado por gdb en su interfase mi, usualmente obtenida mediante SendCommand
* @param key        nombre del campo cuyo valor se quiere extraer (por ejemplo, si la cadena dice 'name="booga"' y se le pasa "name" se obtiene "booga")
* @param crop       indica si hay que suprimir las comillas que rodean al valor
* @param fix_slash  si el valor es texto puede contener secuencias de escape, esto indica si hay que reemplazarlas por sus valores reales
**/
wxString DebugManager::GetValueFromAns(wxString ans, wxString key, bool crop, bool fix_slash) {
	if (ans.Len()<key.Len()+2)
		return _T("");
	unsigned int sc=0, t = ans.Len()-key.Len()-2;
	key<<_T("=");
	char stack[25], c;
//	bool not_ignore=true;
	bool is_the_first = (ans.Left(key.Len())==key) ;
	for (unsigned int i=0;i<t;i++) {
		c=ans[i];
		if (sc && !is_the_first) {
			if (stack[sc]=='[' && c==']')
				sc--;
			else if (stack[sc]=='(' && c==')')
				sc--;
			else if (stack[sc]=='{' && c=='}')
				sc--;
			else if (stack[sc]=='\"' && c=='\"')
				sc--;
			else if (stack[sc]=='\'' && c=='\'')
				sc--;
			else if (stack[sc]!='\"' && stack[sc]!='\'' /*&& not_ignore*/) {
				if (c=='[' || c=='\"' || c=='{' || c=='(')
					stack[++sc]=c;
//			} else if (stack[sc]=='\"' && c=='\\') {
//				not_ignore=false;
//			} else 
//				not_ignore=true;
			} else if (c=='\\') {
				i++; 
				if (ans[i]=='n') 
					ans[i]='\n';
				continue;
			}
		}  else {
			if (c=='[' || c=='\"' || c=='\'' || c=='{')
				stack[++sc]=c;
			else if ( (c==',' && ans.Mid(i+1,key.Len())==key) || is_the_first ) {
				if (is_the_first)
					i--;
				i+=key.Len()+1;
				sc=0;
				int p1=i;
				c=ans[i];
				while ( i<ans.Len() && !( sc==0 && (c==',' || c=='}' || c=='\r' || c=='\n') ) ) {
					if (sc) {
						if (stack[sc]=='[' && c==']')
							sc--;
						else if (stack[sc]=='(' && c==')')
							sc--;
						else if (stack[sc]=='{' && c=='}')
							sc--;
						else if (stack[sc]=='\"' && c=='\"') {
							sc--;
						}
						else if (stack[sc]=='\'' && c=='\'')
							sc--;
						else if (stack[sc]!='\"' && stack[sc]!='\'' /*&& not_ignore*/) {
							if (c=='[' || c=='\"' || c=='{' || c=='(')
								stack[++sc]=c;
//						} else if (stack[sc]=='\"' && c=='\\')
//							not_ignore=false;
//						else 
//							not_ignore=true;
						} else if (c=='\\') {
							i++; 
							if (ans[i]=='n') 
								ans[i]='\n';
						}
					} else if (c=='[' || c=='\"' || c=='\'' || c=='{') {
						stack[++sc]=c;
					}
					c=ans[++i];
				}
				if (!fix_slash)
					return ans.Mid(p1+(crop?1:0),i-p1-(crop?2:0));
				wxString ret = ans.Mid(p1+(crop?1:0),i-p1-(crop?2:0));
				
				int l=ret.Len(),i=0,d=0;
				while (i<l) {
					if (ret[i+d]=='\\') { 
						if (ret[i+d+1]=='n') 
							ret[i+d+1]='\n';
						else if (ret[i+d+1]<='9' && ret[i+d+1]>='0' && i+3<l && ret[i+d+2]<='9' && ret[i+d+2]>='0' && ret[i+d+3]<='9' && ret[i+d+3]>='0')
							{ ret[i+d+3]=(ret[i+d+1]-'0')*8*8+(ret[i+d+2]-'0')*8+(ret[i+d+3]-'0'); d+=2; l-=2; }
						d++; l--;
						ret[i]=ret[i+d];
						i++;
					} else {
						if (d) 
							ret[i]=ret[i+d];
						i++;
					}
				}
				return ret.Mid(0,l);
			}
		}
	}
	return _T("");
}

wxString DebugManager::GetSubValueFromAns(wxString ans, wxString key1, wxString key2, bool crop, bool fix_slash) {
	wxString value = GetValueFromAns(ans,key1);
	if (value.Len()) {
		value[0]=value[value.Len()-1]=',';
		return GetValueFromAns(value,key2,crop,fix_slash);
	}
	return _T("");
}

void DebugManager::SetStateText(wxString text, bool refresh) {
	main_window->compiler_tree.treeCtrl->SetItemText(main_window->compiler_tree.state,text);
	main_window->SetStatusText(text);
	main_window->toolbar_status_text->SetLabel(wxString(LANG(DEBUG_STATUS_PREFIX,"  Depuracion: "))+text);
	if (refresh)
		wxYield();
}


void DebugManager::BacktraceClean() {
	int c=0;
	while (c<last_backtrace_size) { // borrar los renglones que sobran
		for (int i=0;i<BG_COLS_COUNT;i++)
			main_window->backtrace_ctrl->SetCellValue(c,i,_T(""));
		c++;
	}
	last_backtrace_size = 0;
}

/**
* @brief Busca la dirección de memoria donde empiezan las instrucciones de una linea particular del codigo fuente
*
* Si gdb no reconoce la ubicacion devuelve una cadena vacia. Esta funcion sirve 
* entre otras cosas para saber si es una ubicación válida, por ejemplo para
* verificar antes de colocar los puntos de interrupción.
*
* @param fname ruta del archivo, con cualquier barra (si es windows corrige)
* @param line número de linea en base 0
**/
wxString DebugManager::GetAddress(wxString fname, int line) {
	if (waiting || !debugging) return _T("");
#if defined(_WIN32) || defined(__WIN32__)
	for (unsigned int i=0;i<fname.Len();i++) // corregir las barras en windows para que no sean caracter de escape
		if (fname[i]=='\\') 
			fname[i]='/';
#endif
	wxString ans = SendCommand(wxString(_T("info line \""))<<fname<<_T(":")<<line+1<<_T("\""));
	int r=ans.Find(_T("starts at"));
	if (r!=wxNOT_FOUND) {
		ans=ans.Mid(r);
		r=ans.Find(_T("0x"));
		if (r!=wxNOT_FOUND)
			return ans.Mid(r).BeforeFirst(' ');
	}
	return _T("");
}

/**
* @brief Hace un salto sin miramientos a alguna linea de código
*
* @param fname ruta del archivo, con cualquier barra (si es windows corrige)
* @param line número de linea en base 0
**/
bool DebugManager::Jump(wxString fname, int line) {
	if (waiting || !debugging) return false;
	running = true;
	wxString adr = GetAddress(fname,line);
	if (adr.Len()) {
		wxString ans=SendCommand(_T("-gdb-set $pc="),adr);
		if (ans.SubString(1,5)!=_T("error")) {
			MarkCurrentPoint(fname,line+1,mxSTC_MARK_EXECPOINT);
			Backtrace(true);
			running = false;
			return true;
		}
	}
	running = false;
	return false;
}

/**
* @brief Busca la dirección de memoria donde empiezan las instrucciones de una linea particular del codigo fuente
*
* Si gdb no reconoce la ubicacion devuelve una cadena vacia. Esta funcion sirve 
* entre otras cosas para saber si es una ubicación válida, por ejemplo para
* verificar antes de colocar los puntos de interrupción.
*
* @param fname ruta del archivo, con cualquier barra (si es windows corrige)
* @param line número de linea en base 0
**/
bool DebugManager::RunUntil(wxString fname, int line) {
	if (waiting || !debugging) return false;
	running = true;
	wxString adr = GetAddress(fname,line);
	if (adr.Len()) {
		wxString ans = SendCommand(_T("advance *"),adr); // aca estaba exec-until pero until solo funciona en un mismo frame, advance se los salta sin problemas
		if (ans.SubString(1,5)==_T("error"))
			return false;
		HowDoesItRuns(); /// @todo: guardar adr para comparar en HowDoesItRuns y saber si realmente llego o no a ese punto (siempre dice que si, aunque termine el programa sin pasar por ahi)
		running = false;
		return true;
	}
	return false;
}

bool DebugManager::Return(wxString what) {
	if (waiting || !debugging) return false;
	wxString ans = SendCommand(wxString(_T("-exec-return "))<<what);
	if (ans.SubString(1,4)!=_T("done")) 
		return false;
	
	wxString fname = GetSubValueFromAns(ans,_T("frame"),_T("fullname"),true);
	if (!fname.Len())
		fname = GetSubValueFromAns(ans,_T("frame"),_T("file"),true);
	wxString line =  GetSubValueFromAns(ans,_T("frame"),_T("line"),true);
	long fline = -1;
	line.ToLong(&fline);
	MarkCurrentPoint(fname,fline,mxSTC_MARK_EXECPOINT);
	//if (backtrace_visible && config->Debug.autoupdate_backtrace)
		Backtrace(true);
	UpdateInspection();
	return true;
}

void DebugManager::ProcessKilled() {
#ifdef DEBUG_MANAGER_LOG_TALK
	debug_log_file.Close();
#endif
	MarkCurrentPoint();
	notitle_source = NULL;
	debugging=false;
#if !defined(_WIN32) && !defined(__WIN32__)
	if (tty_running)
		tty_process->Kill(tty_pid,wxSIGKILL);
#endif
	delete process;
	process=NULL;
	pid=0;
	running = debugging = waiting = false;
	wxCommandEvent evt;
	if (gui_is_prepared) {
		main_window->PrepareGuiForDebugging(false);
	}
}

#if !defined(_WIN32) && !defined(__WIN32__)
void DebugManager::TtyProcessKilled() {
	if (pid && debugging && !stopping) Stop();
	delete tty_process;
	tty_process = NULL;
	tty_pid = 0;
	tty_running = false;
}
#endif

/**
* @brief expr is input and ouput argument
* @brief type is output argument
**/
wxString DebugManager::CreateVO(wxString &expr, wxString &type) {
	wxString ans = SendCommand("-var-create - * ",utils->EscapeString(expr,true));
	type = GetValueFromAns(ans,"type",true);
	wxArrayString &from=config->Debug.inspection_improving_template_from;
	if (config->Debug.improve_inspections_by_type) {
		wxString mtype=type; if (mtype.EndsWith(" &")) { mtype.RemoveLast(); mtype.RemoveLast(); }
		for(unsigned int i=0, n=from.GetCount(); i<n; i++) {
			if (from[i]==mtype) {
				wxString e=config->Debug.inspection_improving_template_to[i];
				e.Replace("${EXP}",expr,true); expr=e;
				SendCommand("-var-delete ",GetValueFromAns(ans,"name",true));
				ans = SendCommand("-var-create - * ",utils->EscapeString(expr,true));
				break;
			}
		}
	}
	return ans;
}

bool DebugManager::ModifyInspection(int num, wxString expr, bool force_new) {
	if (waiting || !debugging) return false;
	last_error.Clear();
	if (!expr.Len()) {
		if (num!=inspections_count) {
			inspection_grid->SetCellValue(num,IG_COL_EXPR,inspections[num].expr);
		}
		return false;
	}
	bool is_vo=expr[0]!='>';
	if (config->Debug.select_modified_inspections)
		inspection_grid->HightlightChange(num);
	if (num==inspections_count || force_new) {
		wxString ans,value,vo_type;
		if (is_vo) ans = CreateVO(expr,vo_type);
		inspection_grid->SetCellValue(num,IG_COL_EXPR,expr);
		inspectinfo ii(
			is_vo?GetValueFromAns(ans,"name",true):expr.Mid(1),
			is_vo?vo_type:wxString("<cmd>"),
			expr);
		ii.frame=current_frame;
		ii.is_vo=is_vo;
		if (ii.name.Len()) {
			inspection_grid->SetReadOnly(num,IG_COL_LEVEL,true);
			inspection_grid->SetCellValue(num,IG_COL_LEVEL,current_frame_num);
			inspection_grid->SetCellValue(num,IG_COL_TYPE,ii.type);
			if (is_vo) {
				value = GetValueFromAns( SendCommand(_T("-var-evaluate-expression "),ii.name) , _T("value") ,true ,true);
			} else {
				value = GetMacroOutput(ii.name);
			}
			if (value[0]=='@') {
				wxString value2=value.Mid(value.Find(':')+2);
				ii.is_class = value2[0]=='{';
				ii.is_array = value2[0]=='[';
				ii.always_evaluate = true;
			} else {
//				ii.use_data_evaluate = true;
				ii.is_class = value[0]=='{';
				ii.is_array = value[0]=='[';
				ii.use_data_evaluate = ii.always_evaluate = ii.is_array||ii.is_class;
			}
			if (( ii.is_class || ii.is_array ) && ii.is_vo) {
				inspection_grid->SetCellRenderer(num,IG_COL_VALUE,new mxPlusCellRenderer());
				if (ii.use_data_evaluate)
					inspection_grid->SetCellValue(num,IG_COL_VALUE, InspectExpression(ii.expr));
				else
					inspection_grid->SetCellValue(num,IG_COL_VALUE,value);
				inspection_grid->SetReadOnly(num,IG_COL_VALUE,true);
//				inspection_grid->SetReadOnly(num,IG_COL_WATCH,true);
//				inspection_grid->SetReadOnly(num,IG_COL_FORMAT,true);
				if (ii.is_class) {
//					inspection_grid->SetReadOnly(num,IG_COL_FORMAT,true);
					inspection_grid->SetCellValue(num,IG_COL_FORMAT,LANG(INSPECTGRID_TYPE_STRUCT,"estructura"));
				} else {
//					inspection_grid->SetReadOnly(num,IG_COL_FORMAT,false);
					inspection_grid->SetCellValue(num,IG_COL_FORMAT,LANG(INSPECTGRID_TYPE_ARRAY,"arreglo"));
				}
			} else {
				inspection_grid->SetCellRenderer(num,IG_COL_VALUE,new wxGridCellStringRenderer());
				inspection_grid->SetReadOnly(num,IG_COL_VALUE,!is_vo);
				inspection_grid->SetCellValue(num,IG_COL_VALUE, value );
//				inspection_grid->SetReadOnly(num,IG_COL_WATCH,false);
//				inspection_grid->SetReadOnly(num,IG_COL_FORMAT,false);
				inspection_grid->SetCellValue(num,IG_COL_FORMAT,is_vo?LANG(INSPECTGRID_FORMAT_NATURAL,"natural"):_T("macro"));
			}
			inspection_grid->SetCellValue(num,IG_COL_WATCH,_T("no"));
			if (num==inspections_count) {
				inspection_grid->AddRow();
				inspections_count++;
				inspections.push_back(ii);
			} else 
				inspections[num]=ii;
			return true;
		} else {
			if (!ans.Len()) last_error=_T("<killed>"); else
				last_error = GetValueFromAns(ans,_T("msg"),true,true);
			inspection_grid->SetCellValue(num,IG_COL_EXPR,_T(""));
		}
	} else /*if (inspections[num].expr!=expr)*/ {
		if (!inspections[num].frameless && inspections[num].is_vo && inspections[num].on_scope && !SelectFrameForInspeccion(inspections[num].frame))
			inspection_grid->SetCellValue(num,IG_COL_LEVEL,_T("Error"));	
		else 
			inspection_grid->SetCellValue(num,IG_COL_LEVEL,current_frame_num);
		ModifyInspectionWatch(num,false,false);
		// para modificar una expresion se crea una nueva y destruye la anterior
		wxString ans,value,vo_type;
		if (is_vo) ans = CreateVO(expr,vo_type); // SendCommand(_T("-var-create - * "),utils->EscapeString(expr,true));
		inspection_grid->SetCellValue(num,IG_COL_EXPR,expr);
		inspectinfo ii(
			is_vo?GetValueFromAns(ans,_T("name"),true):expr.Mid(1),
			is_vo?vo_type:wxString(_T("<cmd>")),
			expr);
		ii.is_vo=is_vo;
		ii.frameless=false;
		if (!inspections[num].frameless && inspections[num].is_vo && inspections[num].on_scope) {
			ii.frame_num = inspections[num].frame_num;
			ii.frame = inspections[num].frame;
		} else {
			ii.frame_num = current_frame_num;
			ii.frame = current_frame;
		}
		if (ii.name.Len()) { // si se pudo crea el nuevo VO
			inspection_grid->SetCellValue(num,IG_COL_TYPE,ii.type);
			if (is_vo) {
				if (inspection_grid->GetCellValue(num,IG_COL_FORMAT)==LANG(INSPECTGRID_TYPE_STRUCT,"estructura") || SendCommand(_T("-var-set-format "),ii.name+_T(" ")+inspection_grid->GetCellValue(num,IG_COL_FORMAT)).Mid(1,5)==_T("error")) 
					inspection_grid->SetCellValue(num,IG_COL_FORMAT,LANG(INSPECTGRID_FORMAT_NATURAL,"natural"));
				value = GetValueFromAns(SendCommand(_T("-var-evaluate-expression "),ii.name),_T("value"),true,true);
			} else {
				inspection_grid->SetCellValue(num,IG_COL_FORMAT,_T("macro"));
				value = GetMacroOutput(ii.name);
			}
			if (value[0]=='@') {
				wxString value2 = value.Mid(value.Find(':')+2);
				ii.is_class = value2[0]=='{';
				ii.is_array = value2[0]=='[';
				ii.always_evaluate = true;
			} else {
				ii.use_data_evaluate = true;
				ii.is_class = value[0]=='{';
				ii.is_array = value[0]=='[';
				ii.always_evaluate = ii.is_array||ii.is_class;
			}
			if ( (ii.is_class || ii.is_array) && is_vo ) {
				inspection_grid->SetReadOnly(num,IG_COL_VALUE,true);
				inspection_grid->SetCellRenderer(num,IG_COL_VALUE,new mxPlusCellRenderer());
				if (ii.use_data_evaluate)
					inspection_grid->SetCellValue(num,IG_COL_VALUE, InspectExpression(ii.expr));
				else
					inspection_grid->SetCellValue(num,IG_COL_VALUE,value);
//				inspection_grid->SetReadOnly(num,IG_COL_WATCH,true);
//				inspection_grid->SetReadOnly(num,IG_COL_FORMAT,true);
				if (ii.is_class) {
					inspection_grid->SetCellValue(num,IG_COL_FORMAT,LANG(INSPECTGRID_TYPE_STRUCT,"estructura"));
//					inspection_grid->SetReadOnly(num,IG_COL_FORMAT,true);
				} else {
					inspection_grid->SetCellValue(num,IG_COL_FORMAT,LANG(INSPECTGRID_TYPE_ARRAY,"arreglo"));
//					inspection_grid->SetReadOnly(inspections_count,IG_COL_FORMAT,false);
				}
			} else {
				inspection_grid->SetReadOnly(num,IG_COL_VALUE,!is_vo);
				inspection_grid->SetCellRenderer(num,IG_COL_VALUE,new wxGridCellStringRenderer());
				inspection_grid->SetCellValue(num,IG_COL_VALUE, value );
//				inspection_grid->SetReadOnly(num,IG_COL_FORMAT,false);
//				inspection_grid->SetReadOnly(num,IG_COL_WATCH,false);
			}
			inspection_grid->SetCellValue(num,IG_COL_WATCH,_T("no"));
			// reemplazar las datos de la inspeccion y borrar el VO viejo
			if (inspections[num].is_vo) SendCommand(_T("-var-delete "),inspections[num].name);
			inspections[num]=ii;
			SelectFrameForInspeccion(current_frame);
			return true;
		} else { // si no se pudo crear el VO
			if (!ans.Len()) last_error=_T("<killed>"); else
				last_error = GetValueFromAns(ans,_T("msg"),true,true);
			inspection_grid->SetCellValue(num,IG_COL_EXPR,inspections[num].expr);
		}
	}
	return false;
}

bool DebugManager::DuplicateInspection(int num) {
	if (waiting || !debugging) return false;
	last_error.Clear();
	wxString expr=inspection_grid->GetCellValue(num,IG_COL_EXPR);
	inspection_grid->InsertRows(num+1,1);
	inspections.insert(inspections.begin()+num,inspections[num]);
	inspection_grid->SetCellValue(num+1,IG_COL_EXPR,expr);
	inspections_count++;
	if (!inspections[num+1].frameless) SelectFrameForInspeccion(inspections[num].frame);
	bool retval=ModifyInspection(num+1,expr,true);
	if (!inspections[num+1].frameless) {
		inspections[num+1].frame=inspections[num].frame; 
		inspection_grid->SetCellValue(num+1,IG_COL_LEVEL,inspection_grid->GetCellValue(num,IG_COL_LEVEL));
	} else {
		inspection_grid->HightlightDisable(num+1);
		inspection_grid->SetCellValue(num+1,IG_COL_VALUE,LANG(INSPECTGRID_OUT_OF_SCOPE,"<<< Fuera de Scope >>>"));
	}
	SelectFrameForInspeccion(current_frame);
	return retval;
}

bool DebugManager::DeleteInspection(int num) {
	if (!debugging || waiting) return false;
	if (inspections[num].on_scope && inspections[num].is_vo) {
		wxString ans = SendCommand(_T("-var-delete "),inspections[num].name);
		if (ans.SubString(1,5)==_T("error"))
			return false;
	}
	inspections.erase(inspections.begin()+num);
	inspections_count--;
	return true;
}

bool DebugManager::ModifyInspectionValue(int num, wxString value) {
	if (!debugging || waiting) return false;
	wxString ans = SendCommand(_T("-var-assign "),inspections[num].name+_T(" ")+utils->EscapeString(value,true));
	if (ans.Mid(1,4)==_T("done"))
		UpdateInspection();
	return ans.Mid(1,4)==_T("done");
}

bool DebugManager::ModifyInspectionFormat(int num, wxString format) {
	if (!debugging || waiting) return false;
	wxString ans = SendCommand(_T("-var-set-format "),inspections[num].name+_T(" ")+format);
	if (ans.SubString(1,5)==_T("error"))
		return false;
	inspection_grid->SetCellValue(num,IG_COL_FORMAT,format);
	inspection_grid->SetCellValue(num,IG_COL_VALUE,
		GetValueFromAns( SendCommand(_T("-var-evaluate-expression "), inspections[num].name) , _T("value") ,true, true ) );
	return false;
}

void DebugManager::UpdateFramelessInspection() {
	if (!debugging || waiting || inspections_count==0) return;
	for (unsigned int i=0;i<inspections.size();i++) {
		if (!inspections[i].frameless || inspections[i].freezed) continue;
		wxString vnew = InspectExpression(inspections[i].expr);
		if (!vnew.Len()) {
			vnew = LANG(DEBUG_INSPECTION_NOT_AVAILABLE,"<<<No Disponible>>>");
			if (config->Debug.select_modified_inspections) 
				inspection_grid->HightlightDisable(i);
		} else if (config->Debug.select_modified_inspections) {
			wxString vold = inspection_grid->GetCellValue(i,IG_COL_VALUE);
			if (vold!=vnew) inspection_grid->HightlightChange(i);
			else inspection_grid->HightlightNone(i);
		}
		inspection_grid->SetCellValue(i,IG_COL_VALUE,vnew);
	}
}

bool DebugManager::UpdateInspection() {
	if (!debugging || waiting || inspections_count==0) return false;
	inspection_grid->BeginBatch();
	if (config->Debug.select_modified_inspections)
		for (int i=0;i<inspections_count;i++)
			if (inspections[i].is_vo && inspections[i].on_scope && !inspections[i].always_evaluate && !inspections[i].freezed)
				inspection_grid->HightlightNone(i);
	wxString my_frame=current_frame, my_frame_num=current_frame_num;
//	if (config->Debug.select_modified_inspections) 
//		inspection_grid->ResetChangeHightlights();
	UpdateFramelessInspection();
	// actualizar los numeros de frame segun el backtrace
//	if (config->Debug.inspection_frame_address) {
		for (int i=0;i<inspections_count;i++) {
			inspectinfo &ii = inspections[i];
			if (ii.frameless || !ii.on_scope) continue;
			if (ii.frame!=my_frame) {
				bool found=false;
				for (int j=0;j<last_backtrace_size;j++) {
					if (frames_addrs[j]==ii.frame) {
						found=true;
						my_frame=frames_addrs[j];
						my_frame_num=frames_nums[j];
						ii.frame_num=my_frame_num;
						inspection_grid->SetCellValue(i,IG_COL_LEVEL,my_frame_num);
						break;
					}
				}
				if (!found) {
					ii.frame_num=_T("Error");
					inspection_grid->SetCellValue(i,IG_COL_LEVEL,_T("Error"));
				}
			} else {
				ii.frame_num=my_frame_num;
				inspection_grid->SetCellValue(i,IG_COL_LEVEL,my_frame_num);
			}
		}
//	} else {
//		for (int i=0;i<inspections_count;i++)
//			inspections[i].frame_num=_T("<ND>");
//	}
	
	my_frame=current_frame; my_frame_num=current_frame_num;
	
	wxString ans = SendCommand(_T("-var-update *"));
	int p = ans.Find(_T("name="));
	while (p!=wxNOT_FOUND) {
		int p2 = p+6;
		while (ans[p2]!='\"')
			p2++;
		wxString name = ans.SubString(p+6,p2-1);
		ans.Remove(0,p2);
		p = ans.Find(_T("in_scope="));
		if (p!=wxNOT_FOUND && ans[p+10]=='f') {
			for (unsigned int i=0;i<inspections.size();i++)
				if (inspections[i].on_scope && inspections[i].name==name && inspections[i].is_vo) {
					if (!inspections[i].freezed) {
						inspection_grid->SetCellValue(i,IG_COL_VALUE,LANG(INSPECTGRID_OUT_OF_SCOPE,"<<<Fuera de Ambito>>>"));
						inspection_grid->SetCellRenderer(i,IG_COL_VALUE,new wxGridCellStringRenderer());
						if (config->Debug.select_modified_inspections) inspection_grid->HightlightDisable(i);
					}
					inspection_grid->SetCellValue(i,IG_COL_LEVEL,_T(""));
					inspection_grid->SetReadOnly(i,IG_COL_VALUE,true);
					inspection_grid->SetReadOnly(i,IG_COL_FORMAT,true);
					inspection_grid->SetReadOnly(i,IG_COL_WATCH,true);
					inspections[i].on_scope=false;
					if (inspections[i].is_vo) SendCommand(_T("-var-delete "),inspections[i].name);
					break;
				}
		} else {
			for (unsigned int i=0;i<inspections.size();i++) {
				if (!inspections[i].freezed && inspections[i].on_scope && inspections[i].is_vo && inspections[i].name==name) {
//					bool do_inspect=false; // ver si esta el scope en el backtrace y seleccionar el frame
//					if (inspections[i].frame!=my_frame) {
//						for (int j=0;j<last_backtrace_size;j++)
//							if (frames_addrs[j]==inspections[i].frame) {
//								do_inspect=true;
//								my_frame=frames_addrs[j];
//								inspection_grid->SetCellValue(i,IG_COL_LEVEL,frames_nums[j]);
//								break;
//							}
//					} else 
//						do_inspect=true;
//					if (!do_inspect) 
//						inspection_grid->SetCellValue(i,IG_COL_LEVEL,_T("Error"));
//					else {
					if (inspections[i].frame_num[0]!='E') {
						inspection_grid->SetCellValue(i,IG_COL_VALUE,
							GetValueFromAns( SendCommand(_T("-var-evaluate-expression "), name) , _T("value") ,true,true ) );
						if (config->Debug.select_modified_inspections)
							inspection_grid->HightlightChange(i);
					}
					break;
				}
			}
		}
		p = ans.Find(_T("name="));
	}

	// actualizar los tipos compuestos (abria que buscar algo mas eficiente)
	for (unsigned int i=0;i<inspections.size();i++) {
		if (!inspections[i].on_scope || inspections[i].frameless || inspections[i].freezed) continue;
		if (inspections[i].is_vo && inspections[i].always_evaluate) {
//			bool do_inspect=false; // ver si esta el scope en el backtrace y seleccionar el frame
//			if (inspections[i].frame!=my_frame) {
//				for (int j=0;j<last_backtrace_size;j++) {
//					if (frames_addrs[j]==inspections[i].frame) {
//						do_inspect=true;
//						my_frame=frames_addrs[j];
//						SendCommand(_T("-stack-select-frame "),my_frame_num=frames_nums[j]);
//						break;
//					}
//				}
//			} else do_inspect=true;
//			if (do_inspect) {
			if (inspections[i].frame_num[0]!='E') {
//				SendCommand(_T("-stack-select-frame "),my_frame_num=frames_nums[j]);
				if (my_frame_num!=inspections[i].frame_num) {
					SendCommand(_T("-stack-select-frame "),my_frame_num=inspections[i].frame_num);
					my_frame = frames_addrs[i];
				}
				if (inspections[i].use_data_evaluate) {
					inspection_grid->SetCellValue(i,IG_COL_LEVEL,my_frame_num);
					if (config->Debug.select_modified_inspections) {
						wxString vold = inspection_grid->GetCellValue(i,IG_COL_VALUE);
						wxString vnew = InspectExpression(inspections[i].expr);
						if (vold!=vnew) {
							inspection_grid->SetCellValue(i,IG_COL_VALUE,vnew);
							inspection_grid->HightlightChange(i);
						} else
							inspection_grid->HightlightNone(i);
					} else
						inspection_grid->SetCellValue(i,IG_COL_VALUE,InspectExpression(inspections[i].expr));
				} else {
					if (config->Debug.select_modified_inspections) {
						wxString vold = inspection_grid->GetCellValue(i,IG_COL_VALUE);
						wxString vnew = GetValueFromAns(SendCommand(_T("-var-evaluate-expression "),inspections[i].name),_T("value"),true,true);
						if (vold!=vnew) {
							inspection_grid->SetCellValue(i,IG_COL_VALUE,vnew);
							inspection_grid->HightlightChange(i);
						} else
							inspection_grid->HightlightNone(i);
					} else
						inspection_grid->SetCellValue(i,IG_COL_VALUE,GetValueFromAns(SendCommand(_T("-var-evaluate-expression "),inspections[i].name),_T("value"),true,true));
				}
//			} else {
//				inspection_grid->SetCellValue(i,IG_COL_LEVEL,_T("Error"));
			}
		} else if (!inspections[i].is_vo) { // no es vo
			if (config->Debug.select_modified_inspections) {
				wxString vold = inspection_grid->GetCellValue(i,IG_COL_VALUE);
				wxString vnew = GetMacroOutput(inspections[i].name);
				if (vold!=vnew) {
					inspection_grid->SetCellValue(i,IG_COL_VALUE,vnew);
					inspection_grid->HightlightChange(i);
				} else
					inspection_grid->HightlightNone(i);
			} else
				inspection_grid->SetCellValue(i,IG_COL_VALUE,GetMacroOutput(inspections[i].name));
		}
	}
//	// opacar las que no estan en scope
//	if (config->Debug.select_modified_inspections) {
//		for (unsigned int i=0;i<inspections.size();i++) {
//			if (!inspections[i].on_scope && inspections[i].is_vo)
//				inspection_grid->HightlightDisable(i);
//		}
//	}
	inspection_grid->EndBatch();
	
	list<mxExternInspection*>::iterator i1=extern_list.begin(), i2=extern_list.end();
	if (i1!=i2) {
		// el print-repeats se desactiva para que las otras ventanas puedan mostrar todo, 
		// y despues se activa ya que en la tabla de inspecciones no entran muchos datos seguro
		SetFullOutput(true);
		while (i1!=i2) {
			bool do_inspect=false;
			if ((*i1)->frame.Len()) {
				if ((*i1)->frame!=my_frame) {
					for (int j=0;j<last_backtrace_size;j++) {
						if (frames_addrs[j]==(*i1)->frame) {
							do_inspect=true;
							my_frame=frames_addrs[j];
							SendCommand(_T("-stack-select-frame "),my_frame_num=frames_nums[j]);
							break;
						}
					}
				} else do_inspect=true;
			} else do_inspect=true;
			if (do_inspect) (*i1)->Update(); ++i1;
		}
		SetFullOutput(false);
	}
	
//	list<mxInspectionMatrix*>::iterator i1=matrix_list.begin(), i2=matrix_list.end();
//	if (i1!=i2) {
//		SendCommand(_T("-gdb-set print elements 0"));
//		while (i1!=i2) { 
//			bool do_inspect=false;
//			if ((*i1)->frame!=my_frame) {
//				for (int j=0;j<last_backtrace_size;j++) {
//					if (frames_addrs[j]==(*i1)->frame) {
//						do_inspect=true;
//						my_frame=frames_addrs[j];
//						SendCommand(_T("-stack-select-frame "),my_frame_num=frames_nums[j]);
//						break;
//					}
//				}
//			} else do_inspect=true;
//			if (do_inspect)
//				(*i1)->UpdateTable(); i1++;
//		}
//		SendCommand(_T("-gdb-set print elements 10"));
//	}
	if (my_frame!=current_frame) SendCommand(_T("-stack-select-frame "),current_frame_num);
	
//	list<mxInspectionExplorer*>::iterator i3=explorer_list.begin(), i4=explorer_list.end();
//	while (i3!=i4) { 
//		(*i3)->Grow();
//		i3++;
//	}
	
	return true;
}

wxString DebugManager::GetNextItem(wxString &ans, int &from) {
	char stack[50];
	int st_size=0;
	bool comillas = false;
	int p1=from,p=from, l=ans.Len();
	const wxChar * ch = ans.c_str();
	while ( !(( st_size==0 && !comillas && (ch[p]==',' || ch[p]==']')) || p>=l ) ) {
		switch (ch[p]) {
		case '\\':
			p++;
			break;
		case '{':
			stack[st_size++]='{';
			break;
		case '[':		
			stack[st_size++]='[';
			break;
		case ']':
			if (st_size && stack[st_size-1]=='[')
				st_size--;
			break;
		case '}':
			if (st_size &&  stack[st_size-1]=='{')
				st_size--;
			break;
		case '\"':
			comillas=!comillas;
			break;
		}
		p++;
	}
	from = p+1;
	return ans.SubString(p1,p-1);
}

bool DebugManager::SelectFrame(wxString strnum, int idx) {
	wxString ans = SendCommand(_T("-stack-select-frame "),strnum);
	current_frame = frames_addrs[idx];
	current_frame_num = strnum;
	return ans.Mid(1,4)==_T("done");
}

bool DebugManager::DoThat(wxString what) {
	wxMessageBox(SendCommand(what),what,wxOK,main_window);
	return true;
}

/// @brief replace one inspection for a class/array with several inspection with its members (or a pointer with the pointed data)
bool DebugManager::BreakCompoundInspection(int n) {
	if (!debugging || waiting) return false;
	if (inspections_count<=n || (!inspections[n].is_array && !inspections[n].is_class && inspections[n].type[inspections[n].type.Len()-1]!='*') || !inspections[n].on_scope )
		return false;
	int p=0;
	if (!inspections[n].is_array && !inspections[n].is_class) {
		inspection_grid->SetCellValue(n,IG_COL_EXPR,wxString(_T("*"))<<RewriteExpressionForBreaking(inspections[n].expr));
		ModifyInspection(n,wxString(_T("*"))<<RewriteExpressionForBreaking(inspections[n].expr),false);
		return true;
	}
	bool first = true, breaking_class=inspections[n].is_class;
	wxString fans, ans, main_name=inspections[n].name; // main expr es la expresion "original", a la que se le agregar\Uffffffffn subindice, o un punto el nombre de atributo para armar las expresiones hijas
	wxString main_expr = RewriteExpressionForBreaking(inspections[n].expr);
	wxString frame = inspections[n].frame, level = inspection_grid->GetCellValue(n,IG_COL_LEVEL);
	bool is_cpp = GetValueFromAns(SendCommand(_T("-var-info-expression "),inspections[n].name),"lang",true,false)=="C++";
	if (inspections[n].is_class) {
		fans = SendCommand(_T("-var-list-children "),inspections[n].name);
		p = fans.Find(_T("name=\""));
	} else {
		fans<<_T("name=\"")<<main_name<<_T("\"");
		// ver si era un "arreglo" artificial (sintaxis gdb)
		if (main_expr.Find('@')!=wxNOT_FOUND) { 
			if (main_expr[0]=='*')
				main_expr = main_expr.Mid(1,main_expr.Find('@',true)-1);
			else
				main_expr = wxString(_T("(&"))+main_expr.Mid(0,main_expr.Find('@',true))+_T(")");
		}
	}
	wxArrayInt to_break;
	while (p!=wxNOT_FOUND) {
		fans.Remove(0,p+6);
		wxString name = fans.Mid(0,fans.Find('\"'));
		bool is_parent = breaking_class && name!=main_name+_T(".private") && name!=main_name+_T(".public") && name!=main_name+_T(".protected");
		long cant=1;
		if (!is_cpp) { // los structs de C no tienen todo junto en el mismo nivel
			name=inspections[n].name;
			is_parent=false;
		}
		if (!is_parent) {
			ans = SendCommand(_T("-var-list-children "),name);
			GetValueFromAns(ans,_T("numchild"),true).ToLong(&cant);
		}
		if (cant) {
			if (first) {
				if (cant>1) {
					MakeRoomForInspections(n,cant-1);
				}
				first=false;
			} else {
				MakeRoomForInspections(n,cant);
			}
			if (is_parent) {
				inspections[n].frame = frame;
				inspections[n].name = name;
				inspections[n].expr = main_expr;
				inspections[n].is_class = true;
				inspections[n].frame = frame;
				inspections[n].on_scope = true;
				to_break.Add(n);
				n++;
			} else {
				p = ans.Find('[');
				if (p!=wxNOT_FOUND) {
					p++;
					wxString item = GetNextItem(ans,p);
					wxString pre_expr=main_expr+_T(".");
					if (breaking_class && main_expr.StartsWith("(*") && main_expr.Last()==')') {
						wxString aux=main_expr.Mid(2); aux.RemoveLast();
						if (aux==RewriteExpressionForBreaking(aux))
						pre_expr=aux+"->";
					}
					while (item.Left(6)==_T("child=")) {
						item.Remove(0,7);
						inspections[n].frame = frame;
						inspections[n].is_vo = true;
						inspections[n].on_scope = true;
						inspections[n].name = GetValueFromAns(item,_T("name"),true);
						inspections[n].type = GetValueFromAns(item,_T("type"),true);
						wxString new_exp = GetValueFromAns(item,_T("exp"),true);
						if (breaking_class)
							inspections[n].expr = pre_expr+new_exp;
						else
							inspections[n].expr = main_expr+"["+new_exp+"]";
						inspection_grid->SetCellValue(n,IG_COL_LEVEL, level);
						inspection_grid->SetCellValue(n,IG_COL_TYPE, inspections[n].type);
						inspection_grid->SetCellValue(n,IG_COL_EXPR, inspections[n].expr);
						
						wxString value = GetValueFromAns( SendCommand(_T("-var-evaluate-expression "),inspections[n].name) , _T("value") ,true, true );
						if (value[0]=='@') {
							wxString value2=value.Mid(value.Find(':')+2);
							inspections[n].is_class = value2[0]=='{';
							inspections[n].is_array = value2[0]=='[';
							inspections[n].always_evaluate = true;
						} else {
							inspections[n].use_data_evaluate = true;
							inspections[n].is_class = value[0]=='{';
							inspections[n].is_array = value[0]=='[';
							inspections[n].always_evaluate = inspections[n].is_class||inspections[n].is_array;
						}
						if (inspections[n].is_class || inspections[n].is_array) {
							inspection_grid->SetCellRenderer(n,IG_COL_VALUE,new mxPlusCellRenderer());
							if (inspections[n].use_data_evaluate)
								inspection_grid->SetCellValue(n,IG_COL_VALUE, InspectExpression(inspections[n].expr));
							else
								inspection_grid->SetCellValue(n,IG_COL_VALUE,value);
							inspection_grid->SetReadOnly(n,IG_COL_VALUE,true);
//							inspection_grid->SetReadOnly(n,IG_COL_FORMAT,true);
//							inspection_grid->SetReadOnly(n,IG_COL_WATCH,true);
							if (inspections[n].is_array)
								inspection_grid->SetCellValue(n,IG_COL_FORMAT,LANG(INSPECTGRID_TYPE_ARRAY,"arreglo"));
							else
								inspection_grid->SetCellValue(n,IG_COL_FORMAT,LANG(INSPECTGRID_TYPE_STRUCT,"estructura"));
						} else {
							inspection_grid->SetCellRenderer(n,IG_COL_VALUE,new wxGridCellStringRenderer());
							inspection_grid->SetReadOnly(n,IG_COL_VALUE,false);
							inspection_grid->SetCellValue(n,IG_COL_VALUE, value );
//							inspection_grid->SetReadOnly(n,IG_COL_FORMAT,false);
//							inspection_grid->SetReadOnly(n,IG_COL_WATCH,false);
							inspection_grid->SetCellValue(n,IG_COL_FORMAT,LANG(INSPECTGRID_FORMAT_NATURAL,"natural"));
						}
						inspection_grid->SetReadOnly(n,IG_COL_TYPE,true);
						inspection_grid->SetCellValue(n,IG_COL_WATCH,_T("no"));
						n++;
						item = GetNextItem(ans,p);
					}
				}
			}
		}
		if (!is_cpp) break;
		p = fans.Find(_T("name=\""));
	}
	for (int i=to_break.Count()-1;i>=0;i--)
		BreakCompoundInspection(to_break[i]);
	return true;
}

//wxString DebugManager::utils->EscapeString(wxString str, bool add_comillas) {
//	int i=0, l=str.Len();
//	if (add_comillas) {
//		i=1;
//		str.Prepend(wxChar('\"'));
//		l++;
//	}
//	while(i!=l) {
//		if (str[i]=='\\' || str[i]=='\"') {
//			str=str.Mid(0,i)+wxChar('\\')+str.Mid(i);
//			i++; l++;
//		}
//		i++;
//	}
//	if (add_comillas)
//		str.Append('\"');
//	return str;
//}


bool DebugManager::CreateVO(wxString expr, wxString &name, wxString &type, int &children) {
	wxString ans = SendCommand(_T("-var-create - * "),utils->EscapeString(expr,true));
//	if (ans.Left(5)!=_T("^done")) DEBUG_INFO("ans");
	if (ans.Left(5)!=_T("^done")) return false;
	name = GetValueFromAns(ans,_T("name"),true);
	type = GetValueFromAns(ans,_T("type"),true);
	long c=0; 
	GetValueFromAns(ans,_T("numchild"),true).ToLong(&c);
	children = c;
	return false;
}

wxString DebugManager::GetVOValue(wxString name) {
	if (!debugging || waiting) return _T("");
	return GetValueFromAns(SendCommand(_T("-var-evaluate-expression "),name),_T("value"),true,true);
}

bool DebugManager::DeleteVO(wxString name) {
	if (!debugging || waiting) return false;
	return SendCommand(_T("-var-delete "),name).Left(5)==_T("^done");
}

int DebugManager::GetVOChildrenData(mxIEItemData **data, wxString name, wxString main_expr, wxString main_frame, int what_is) {
	if (!debugging || waiting) return -1;
	wxString ans = SendCommand(_T("-var-list-children "),name);
	int n=0, p=ans.Find('[');
	if (p!=wxNOT_FOUND) {
		p++;
		wxString item = GetNextItem(ans,p);
		while (item.Left(6)==_T("child=")) {
			item.Remove(0,7);
			wxString new_expr = GetValueFromAns(item,_T("exp"),true);
			wxString expr;
			if (what_is==DI_IN_FATHER_POINTER || what_is==DI_IN_FATHER_CLASS)
				expr = main_expr+_T("::")+new_expr;
			else if (what_is==DI_IN_CLASS)
				expr = RewriteExpressionForBreaking(main_expr)+_T(".")+new_expr;
			else if (what_is==DI_IN_CLASS_POINTER)
				expr = RewriteExpressionForBreaking(main_expr)+_T("->")+new_expr;
			else if (what_is==DI_POINTER)
				expr = wxString(_T("*"))<<main_expr;
			else if (what_is==DI_ARRAY)
				expr = main_expr+_T("[")+new_expr+_T("]");
			else
				expr = main_expr;
			long l;
			GetValueFromAns(item,_T("numchild"),true).ToLong(&l);
			data[n] = new mxIEItemData(
				GetValueFromAns(item,_T("name"),true),
				expr, new_expr, main_frame,
				GetValueFromAns(item,_T("type"),true),
				l);		
			if (what_is==DI_CLASS_POINTER) {
				if (data[n]->expr==_("public")||data[n]->expr==_("protected")||data[n]->expr==_("private"))
					data[n]->what_is = DI_IN_CLASS_POINTER;
				else {
					data[n]->real_expr = RewriteExpressionForBreaking(main_expr)+_T("->")+data[n]->expr;
					data[n]->what_is = DI_FATHER_POINTER;
				}
			} else if (what_is==DI_FATHER_POINTER) {
				if (data[n]->expr==_("public")||data[n]->expr==_("protected")||data[n]->expr==_("private"))
					data[n]->what_is = DI_IN_FATHER_POINTER;
				else {
					data[n]->real_expr = RewriteExpressionForBreaking(main_expr)+_T("->")+data[n]->expr;
					data[n]->what_is = DI_FATHER_POINTER;
				}
			} else if (what_is==DI_CLASS) {
				if (data[n]->expr==_("public")||data[n]->expr==_("protected")||data[n]->expr==_("private"))
					data[n]->what_is = DI_IN_CLASS;
				else {
					data[n]->real_expr = RewriteExpressionForBreaking(main_expr)+_T(".")+data[n]->expr;
					data[n]->what_is = DI_FATHER_CLASS;
				}
			} else if (what_is==DI_FATHER_CLASS) {
				if (data[n]->expr==_("public")||data[n]->expr==_("protected")||data[n]->expr==_("private"))
					data[n]->what_is = DI_IN_FATHER_CLASS;
				else {
					data[n]->real_expr = RewriteExpressionForBreaking(main_expr)+_T(".")+data[n]->expr;
					data[n]->what_is = DI_FATHER_CLASS;
				}
			} else {
				if (data[n]->type.Last()=='*' || data[n]->type.Last()==']') {
					if (InspectExpression(wxString(_T("*("))<<data[n]->real_expr<<_T(")"))[0]=='{')
						data[n]->what_is = DI_CLASS_POINTER;
					else
						data[n]->what_is = DI_POINTER;
				}
			}
			item = GetNextItem(ans,p);
			n++;
		}
	}
	return n;
}

wxString DebugManager::RewriteExpressionForBreaking(wxString main_expr) {
	int i=0,l=main_expr.Len();
	char c; bool comillas=false;
	int parentesis=0; bool first_level0_parentesis=true;
	while (i<l) { 	// agregar parentesis si la expresion no es simple
		c=main_expr[i];
		if (c=='\'') { 
			if (main_expr[++i]=='\\') i++;
		} else if (c=='\"') {
			comillas=!comillas;
		} else if (!comillas) {
			if (first_level0_parentesis && c=='(') {
				first_level0_parentesis=false;
				parentesis++;
			} else if (parentesis) {
				if (c==')') parentesis--;
			} else {
				if (c=='[') {
					int l=1; i++;
					while (l) {
						c=main_expr[i];
						if (c=='\'') { if (main_expr[++i]=='\\') i++; }
						else if (c=='\"') comillas=!comillas;
						else if (c=='[') l++;
						else if (c==']') l--;
						i++;
					}
				} else if (c=='@') {
					break;
				} else if ( ! ( (c>='a'&&c<='z') || (c>='A'&&c<='Z') || (c>='0'&&c<='9') || c=='_' || c=='.') ) {
					main_expr.Prepend(wxChar('('));
					main_expr.Append(wxChar(')'));
					break;
				}
			}
		}
		i++;
	}
	return main_expr;
}

void DebugManager::ClearInspections() {
	if (!debugging || waiting) return;
	for (int i=0;i<inspections_count;i++)
		if (inspections[i].on_scope && inspections[i].is_vo)
			SendCommand(_T("-var-delete "),inspections[i].name);
	inspections.clear();
	inspection_grid->DeleteRows(0,inspections_count);
	inspections_count=0;
}

bool DebugManager::GetArgs (wxArrayString &array, wxString level) {
	if (!debugging || waiting) return false;
	wxString args_list = SendCommand(_T("-stack-list-arguments 0 "),level+_T(" ")+level);
	const wxChar * chag = args_list.c_str();
	bool comillas = false;
	int i=args_list.Find('[')+1;
	while (chag[i]!='[') 
		i++; 
	int p=++i;
	while (chag[i]!=']' && !comillas) {
		if (chag[i]=='\"' && !chag[i-1]=='\\')
			comillas=!comillas;
		i++;
	}
	wxString s(args_list.SubString(p,i-1));
	wxString args, sub;
	int j=0, l=s.Len();
	const wxChar * choa = s.c_str();
	while (true) {
		while ( j<l && !(choa[j]=='n' && choa[j+1]=='a' && choa[j+2]=='m' && choa[j+3]=='e' && choa[j+4]=='=') )
			j++;
		if (j==l) break;
		j+=6;
		p=j;
		while (choa[j]!='\"')
			j++;
		array.Add(s.SubString(p,j-1));
	}
	return true;
}

bool DebugManager::GetLocals (wxArrayString &array, wxString level) {
	if (!debugging || waiting) return false;
	wxString args_list = SendCommand(_T("-stack-list-locals 0"));
	const wxChar * choa = args_list.c_str();
	int p,j=args_list.Find('[')+1, l= args_list.Len();
	while (true) {
		while ( j<l && !(choa[j]=='n' && choa[j+1]=='a' && choa[j+2]=='m' && choa[j+3]=='e' && choa[j+4]=='=') )
			j++;
		if (j==l) break;
		j+=6;
		p=j;
		while (choa[j]!='\"')
			j++;
		array.Add(args_list.SubString(p,j-1));
	}
	return true;
}

void DebugManager::MakeRoomForInspections(int pos, int cant) {
	if (pos==inspections_count)
		inspection_grid->AppendRows(cant);
	else
		inspection_grid->InsertRows(pos,cant);
	inspections.resize(inspections_count+cant);
	inspections_count+=cant;
	for (int i=0;i<cant;i++)
		inspections[i].frame=current_frame;
	for (int i=inspections_count-1;i>=pos+cant;i--)
		inspections[i]=inspections[i-cant];
}

bool DebugManager::ModifyInspectionWatch(int num, bool read, bool write) {
	if (waiting || !debugging) return false;
	if (num>=inspections_count) return false;
	inspectinfo &ii = inspections[num];
	if (read!=ii.watch_read || write!=ii.watch_write) { // si cambio
		// borrar el anterior
		if (ii.watch_read || ii.watch_write)
			SendCommand(_T("-break-delete "),ii.watch_num);
		ii.watch_read=ii.watch_write=false;
		// hacer el nuevo si es necesario
		if (read || write) {
			wxString cmd(_T("-break-watch "));
			if (read && write) cmd<<_T("-a ");
			else if (read) cmd<<_T("-r ");
			wxString ans = GetValueFromAns(SendCommand(cmd,utils->EscapeString(ii.expr,true)).AfterFirst('{'),_T("number"),true);
			if (ans.Len()) {
				long l;
				ans.ToLong(&l);
				ii.watch_num=l;
				ii.watch_read=read;
				ii.watch_write=write;
			} else
				return false;
		}
	}
	return true;
}

/**
* @brief Agrega los breakpoints y watchpoints a la tabla de puntos de interrupcion
**/
void DebugManager::PopulateBreakpointsList(mxBreakList *break_list, bool also_watchpoints) {
	wxGrid *grid=break_list->grid;
	if (debug->running) return;
	grid->ClearGrid();
	wxString ans=SendCommand(_T("-break-list"));
	int p=ans.Find(_T("body=["))+6;
	wxString item=GetNextItem(ans,p);
	while (item.Mid(0,5)==_T("bkpt=")) {
		item=item.Mid(6); 
		wxString type=GetValueFromAns(item,_T("type"),true);
		if (type=="breakpoint") {
			long id=-1; GetValueFromAns(item,_T("number"),true).ToLong(&id); 
			BreakPointInfo *bpi=BreakPointInfo::FindFromNumber(id,true);
			int cont=break_list->AppendRow(bpi?bpi->zinjai_id:-1);
			grid->SetCellValue(cont,BL_COL_TYPE,_T("bkpt"));
			grid->SetCellValue(cont,BL_COL_HIT,GetValueFromAns(item,_T("times"),true));
			if (GetValueFromAns(item,_T("enabled"),true)==_T("y")) {
				if (GetValueFromAns(item,_T("disp"),true)==_T("dis"))
					grid->SetCellValue(cont,BL_COL_ENABLE,_T("una vez"));
				else
					grid->SetCellValue(cont,BL_COL_ENABLE,_T("habilitado"));
			} else
				grid->SetCellValue(cont,BL_COL_ENABLE,_T("deshabilitado"));
			wxString fname = GetValueFromAns(item,_T("fullname"),true);
			if (!fname.Len()) fname = GetValueFromAns(item,_T("file"),true);
			grid->SetCellValue(cont,BL_COL_WHY,fname + _T(": linea ") +GetValueFromAns(item,_T("line"),true));
			grid->SetCellValue(cont,BL_COL_COND,GetValueFromAns(item,_T("cond"),true));
		} else if (also_watchpoints && type.Contains("watchpoint")) {
			int cont=break_list->AppendRow(-1);
			if (type.Mid(0,3)==_("rea")) {
				grid->SetCellValue(cont,BL_COL_TYPE,_T("w(l)"));
			} else if (type.Mid(0,3)==_("acc")) {
				grid->SetCellValue(cont,BL_COL_TYPE,_T("w(e)"));
			} else /*if (type.Mid(0,3)==_("acc"))*/ {
				grid->SetCellValue(cont,BL_COL_TYPE,_T("w(l/e)"));
			}
			grid->SetCellValue(cont,BL_COL_HIT,GetValueFromAns(item,_T("times"),true));
			grid->SetCellValue(cont,BL_COL_ENABLE,GetValueFromAns(item,_T("enabled"),true)==_T("y")?_T("Si"):_T("No"));
			long l;
			GetValueFromAns(item,_T("number"),true).ToLong(&l);
			for (int i=0, n=l;i<inspections_count;i++) {
				if ( (inspections[i].watch_read || inspections[i].watch_write) && inspections[i].watch_num==n)
					grid->SetCellValue(cont,BL_COL_WHY,inspections[i].expr);
			}
		}
		item=GetNextItem(ans,p);
	}
}

/// @brief Define the number of time the breakpoint should be ignored before actually stopping the execution
void DebugManager::SetBreakPointOptions(int num, int ignore_count) {
	wxString cmd(_T("-break-after "));
	cmd<<num<<_T(" ")<<ignore_count;
	SendCommand(cmd);
}

/// @brief Define the condition for a conditional breakpoint and returns true if the condition was correctly setted
bool DebugManager::SetBreakPointOptions(int num, wxString condition) {
	wxString cmd(_T("-break-condition "));
	cmd<<num<<_T(" ")<<utils->EscapeString(condition);
	wxString ans = SendCommand(cmd);
	return ans.Len()>4 && ans.Mid(1,4)==_T("done");
}

void DebugManager::SetBreakPointEnable(int num, bool enable, bool once) {
	wxString cmd(_T("-break-"));
	if (enable) {
		cmd<<(once?_T("enable once "):_T("enable "));
	} else {
		cmd<<_T("disable ");
	}
	cmd<<num;
	wxString ans = SendCommand(cmd);
}

// //**
//* @brief find a BreakPointInfo that matches a gdb_id or zinjai_id
//*
//* If use_gdb finds a breakpoint that matches gdb_id, else 
//* finds a breakpoint that matches zinjai_id. If there's no match
//* returns NULL.
//*
//* This method searchs first in opened files (main_window->notebook_sources)
//* starting by the current source, then searchs in project's files if there
//* is a project.
//**/
//BreakPointInfo *DebugManager::FindBreakInfoFromNumber(int _id, bool use_gdb_id) {
//	int sc = main_window->notebook_sources->GetPageCount();
//	if (sc) {
//		// buscar primero en el fuente acutal (siempre deberia estar ahi)
//		mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
//		BreakPointInfo *bpi= *source->breaklist;
//		while (bpi && ((use_gdb_id&&bpi->gdb_id!=_id)||(!use_gdb_id&&bpi->zinjai_id!=_id))) bpi=bpi->Next();
//		if (bpi) return bpi;
//		// si no lo encontro buscar en el resto de los fuentes abiertos
//		for (int i=0;i<sc;i++) {
//			source = (mxSource*)(main_window->notebook_sources->GetPage(i));
//			bpi = *source->breaklist;
//			while (bpi && bpi->gdb_id!=_id) bpi=bpi->Next();
//			if (bpi) return bpi;
//		}
//	}
//	if (project) {
//		for(int i=0;i<2;i++) { 
//			file_item *file =i==0?project->first_source:project->first_header;
//			ML_ITERATE(file) {
//				BreakPointInfo *bpi = file->breaklist;
//				while (bpi && bpi->gdb_id!=_id)
//					bpi = bpi->Next();
//				if (bpi) return bpi;
//				file = file->next;
//			}
//		}
//	}
//	return NULL;
//}

int DebugManager::GetBreakHitCount(int num) {
	long l=0;
	wxString ans = SendCommand(_T("-break-info "),num);
	int p = ans.Find(_T("times="));
	if (p==wxNOT_FOUND) return -1;
	GetValueFromAns(ans.Mid(p),_T("times"),true).ToLong(&l);
	return l;
}

//void DebugManager::RegisterMatrix(mxInspectionMatrix *matrix) {
//	matrix_list.push_back(matrix);
// }
//
//void DebugManager::UnRegisterMatrix(mxInspectionMatrix *matrix) {
//	list<mxInspectionMatrix*>::iterator i1=matrix_list.begin(), i2=matrix_list.end();
//	while (i1!=i2) {
//		if (*i1==matrix) {
//			matrix_list.erase(i1);
//			return;
//		} else
//			i1++;
//	}
// }

void DebugManager::RegisterExternInspection(mxExternInspection *ei) {
	extern_list.push_back(ei);
}

void DebugManager::UnRegisterExternInspection(mxExternInspection *ei) {
	list<mxExternInspection*>::iterator it=find(extern_list.begin(),extern_list.end(),ei);
	if (it!=extern_list.end()) extern_list.erase(it);
}

//void DebugManager::RegisterExplorer(mxInspectionExplorer *explorer) {
//	explorer_list.push_back(explorer);
// }
//
//void DebugManager::UnRegisterExplorer(mxInspectionExplorer *explorer) {
//	list<mxInspectionExplorer*>::iterator i1=explorer_list.begin(), i2=explorer_list.end();
//	while (i1!=i2) {
//		if (*i1==explorer) {
//			explorer_list.erase(i1);
//			return;
//		} else
//			i1++;
//	}
// }

bool DebugManager::SelectFrameForInspeccion(wxString addr) {
	for (int i=0;i<last_backtrace_size;i++) {
		if (frames_addrs[i]==addr) {
			wxString ans = SendCommand(_T("-stack-select-frame "),frames_nums[i]);
			return ans.Mid(1,4)==_T("done");
		}
	}
	return false;
}

bool DebugManager::SaveCoreDump(wxString core_file) {
	if (!debugging || waiting) return false;
	SendCommand(wxString(_T("gcore "))<<core_file);
	return true;
}

wxString DebugManager::GetMacroOutput(wxString cmd, bool keep_endl) {
	wxString ans = SendCommand(cmd), ret;
	int p=0, l=ans.Len(), d;
	bool comillas=false;
	while (p<l) {
		if (ans[p]==126) {
			p+=2;
			int s=p;
			bool ignore=false;
			while (p<l && (ignore||ans[p]!='\"') ) {
				if (ignore && ans[p]=='n') {
					ret<<ans.Mid(s,p-s-1);
					s=p+1;
				} else if (!ignore && !comillas && ans[p]=='$') {
					ret<<ans.Mid(s,p-s);
					while (p<l && ans[p]!=' ') p++;
					if (p<l) p++;
					while (p<l && ans[p]!=' ') p++;
					s=p+1;
					continue;
				}
				if (!ignore && ans[p]=='\\') ignore=true; 
				else ignore=false;
				p++;
			}
			ret<<ans.Mid(s,p-s);
		} else if (ans[p]==94 && ans.Mid(p+1,5)==_T("error")) {
			int s=p+1;
			while (p<l && ans[p]!='\n' && ans[p]!='\r') p++;
			if (ans[p]=='\n' || ans[p]=='\r') p++;
			return ans.Mid(s,p-s-1);
		}
		while (p<l && ans[p]!='\n' && ans[p]!='\r') p++;
		if (ans[p]=='\n' || ans[p]=='\r')  { if (keep_endl&&ans[p]=='\n') ret<<"\n"; p++; }
	}
	p=0; l=ret.Len(); d=0;
	while (p+d<l) {
		if (ret[p+d]=='\\') {
			d++;
			if (ret[p+d]=='t') ret[p]='\t';
			else ret[p]=ret[p+d];
		} else 
			if (d) ret[p]=ret[p+d];
		p++;
	}
	return d?ret.Mid(0,l-d):ret;
}

bool DebugManager::EnableInverseExec() {
	if (recording_for_reverse) {
		if (inverse_exec) ToggleInverseExec();
		wxString ans=SendCommand("record stop");
		recording_for_reverse=false;
	} else {
		wxString ans=SendCommand("record");
		if (!ans.Contains(_T("error"))) {
			recording_for_reverse=true;
			return true;
		} else 
			mxMessageDialog(main_window,LANG(DEBUG_ERROR_REVERSE,"Ha ocurrido un error al intentar activar esta caracteristica.\nPara utilizarla debe instalar gdb version 7.0 o superior.\nAdema, no todas las plataformas soportan este tipo de ejecucion."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
	}
	return false;
}

bool DebugManager::ToggleInverseExec() {
	if (!debugging || waiting) return false;
	if (recording_for_reverse) {
		wxString ans=inverse_exec?SendCommand("-gdb-set exec-direction forward"):SendCommand("-gdb-set exec-direction reverse");
		if (!ans.Contains(_T("error"))) inverse_exec=!inverse_exec;
	} else {
		mxMessageDialog(main_window,LANG(DEBUG_REVERSE_DISABLED,"Solo se puede retroceder la ejecucion hasta el punto en donde la ejecucion hacia atras fue habilitada.\n"
			                           "Actualmente esta caracteristica no esta habilitada. Utilice el comando \"Habilitar Ejecucion Hacia Atras\"\n"
										"del menu de Depuracion para habilitarla."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
	}
	return inverse_exec;
}

void DebugManager::SaveInspectionsTable(wxString name) {
	SaveInspectionsTable(GetInspectionsTable(name,true));
}

void DebugManager::SaveInspectionsTable(inspectlist *il) {
	il->vars.Clear();
	for (int i=0;i<inspections_count;i++)
		il->vars.Add(inspections[i].expr);
}

inspectlist *DebugManager::GetInspectionsTable(wxString name, bool create_if_not_exists) {
	name.MakeLower();
	for (unsigned int i=0;i<inspections_tables.size();i++)
		if (inspections_tables[i]->name.Lower()==name)
			return inspections_tables[i];
	if (create_if_not_exists) {
		inspections_tables.push_back(new inspectlist(name));
		return inspections_tables[inspections_tables.size()-1];
	}
	return NULL;
}

void DebugManager::LoadInspectionsTable(wxString name) {
	inspectlist *il = GetInspectionsTable(name);
	if (!il) return;
	LoadInspectionsTable(il);
}

void DebugManager::LoadInspectionsTable(inspectlist *il) {
	unsigned int cant = inspection_grid->GetNumberRows()-1;
	if (cant<il->vars.size()) inspection_grid->AddRow(il->vars.size()-cant);
	if (cant>il->vars.size()) inspection_grid->DeleteRows(il->vars.size(),cant-il->vars.size());
	inspections.resize(inspections_count=il->vars.size());
	for (unsigned int i=0;i<il->vars.size();i++) {
		inspection_grid->SetCellValue(i,IG_COL_VALUE,il->vars[i]);
		if (debugging && !running && !waiting) {
			wxString vnew = InspectExpression(inspections[i].expr);
			if (!vnew.Len()) {
				vnew = LANG(DEBUG_INSPECTION_NOT_AVAILABLE,"<<<No Disponible>>>");
				if (config->Debug.select_modified_inspections) inspection_grid->HightlightDisable(i);
			} else
				if (config->Debug.select_modified_inspections) inspection_grid->HightlightChange(i);
			inspection_grid->SetCellValue(i,IG_COL_VALUE,vnew);
		} else {
			inspection_grid->SetCellValue(i,IG_COL_VALUE,LANG(DEBUG_INSPECTION_NOT_AVAILABLE,"<<<No Disponible>>>"));
			if (config->Debug.select_modified_inspections) inspection_grid->HightlightDisable(i);
		}
		inspection_grid->SetCellValue(i,IG_COL_EXPR,il->vars[i]);
		inspection_grid->SetCellValue(i,IG_COL_LEVEL,_T("*"));
		inspection_grid->SetCellValue(i,IG_COL_TYPE,_T("<<?>>"));
		inspection_grid->SetCellValue(i,IG_COL_FORMAT,_T("<<?>>"));
		inspection_grid->SetCellValue(i,IG_COL_WATCH,_T("no"));
		inspections[i].expr=il->vars[i];
		inspections[i].frameless=true;
		inspections[i].is_vo=false;
		inspections[i].use_data_evaluate=true;
	}
	UpdateFramelessInspection();
}

void DebugManager::ClearSavedInspectionTables() {
	for (unsigned int i=0;i<inspections_tables.size();i++)
		delete inspections_tables[i];
	inspections_tables.clear();
}

void DebugManager::SetFramelessInspection(int i) {
	if (inspections[i].is_vo) DeleteVO(inspections[i].name);
	inspections[i].is_vo=false;
	inspections[i].frameless=true;
	wxString vnew = InspectExpression(inspections[i].expr);
	if (!vnew.Len()) {
		if (config->Debug.select_modified_inspections && !inspections[i].freezed)
			inspection_grid->HightlightDisable(i);
		vnew = LANG(DEBUG_INSPECTION_NOT_AVAILABLE,"<<<No Disponible>>>");
	}
	if (!inspections[i].freezed) inspection_grid->SetCellValue(i,IG_COL_VALUE,vnew);
	inspection_grid->SetCellValue(i,IG_COL_LEVEL,_T("*"));
	inspection_grid->SetCellValue(i,IG_COL_TYPE,_T("<<?>>"));
	inspection_grid->SetCellValue(i,IG_COL_FORMAT,_T("<<?>>"));
	inspection_grid->SetCellValue(i,IG_COL_WATCH,_T("no"));
	inspection_grid->SetCellRenderer(i,IG_COL_VALUE,new wxGridCellStringRenderer());
}

void DebugManager::DeleteInspectionsTable(wxString name) {
	for (unsigned int i=0;i<inspections_tables.size();i++)
		if (inspections_tables[i]->name.Lower()==name) {
			inspections_tables.erase(inspections_tables.begin()+i);
			return;
		}
}

bool DebugManager::ToggleInspectionFreeze(int n) {
	if ((inspections[n].freezed=!inspections[n].freezed)) {
		if (config->Debug.select_modified_inspections)
			inspection_grid->HightlightFreeze(n);
	} else if (!inspections[n].on_scope && inspections[n].is_vo) {
		inspection_grid->SetCellValue(n,IG_COL_VALUE,LANG(INSPECTGRID_OUT_OF_SCOPE,"<<<Fuera de Ambito>>>"));
		inspection_grid->SetCellRenderer(n,IG_COL_VALUE,new wxGridCellStringRenderer());
		if (config->Debug.select_modified_inspections) inspection_grid->HightlightDisable(n);
	} else if (inspections[n].is_vo && !inspections[n].always_evaluate) {
		wxString vold = inspection_grid->GetCellValue(n,IG_COL_VALUE);
		wxString vnew = GetValueFromAns(SendCommand(_T("-var-evaluate-expression "),inspections[n].name),_T("value"),true,true);
		inspection_grid->SetCellValue(n,IG_COL_VALUE,vnew);
		if (config->Debug.select_modified_inspections) {
			if (vold==vnew)
				inspection_grid->HightlightNone(n);
			else
				inspection_grid->HightlightChange(n);
		}
	} else {
		UpdateInspection();
	}
	return inspections[n].freezed;
}

void DebugManager::UnregisterSource(mxSource *src) {
	if (current_source==src) current_source=NULL;
}

bool DebugManager::OffLineInspectionModify(int i, wxString value) {
	if (!value.Len()) return false;
	if (i>=0 && i<inspections_count) {
		inspections[i].expr=value;
		inspection_grid->SetCellValue(i,IG_COL_VALUE,LANG(INSPECTGRID_OUT_OF_SCOPE,"<<<Fuera de Ambito>>>"));
		inspection_grid->SetCellRenderer(i,IG_COL_VALUE,new wxGridCellStringRenderer());
		if (config->Debug.select_modified_inspections) inspection_grid->HightlightDisable(i);
		return true;
	} else if (i==inspections_count) {
		inspections.push_back(inspectinfo(value,_T(""),value));
		inspection_grid->AddRow(); inspections_count++;
		inspection_grid->SetCellValue(i,IG_COL_VALUE,LANG(INSPECTGRID_OUT_OF_SCOPE,"<<<Fuera de Ambito>>>"));
		inspection_grid->SetCellRenderer(i,IG_COL_VALUE,new wxGridCellStringRenderer());
		if (config->Debug.select_modified_inspections) inspection_grid->HightlightDisable(i);
		return true;
	} else
		return false;
}

bool DebugManager::OffLineInspectionDelete(int i) {
	if (i==-1) {
		inspections.clear();
		inspections_count=0;
		return true;
	} else if (i>=0 && i<inspections_count) {
		inspections.erase(inspections.begin()+i);
		return true;
	} else
		return false;
}

void DebugManager::ListThreads() {
	wxString ans=SendCommand("-thread-list-ids");
	if (ans.Contains("number-of-threads=\"0\"")) {
		main_window->threadlist_ctrl->SetData(0,"-",LANG(THREADS_NO_THREADS,"<<No hay hilos>>"),"--","--");
		main_window->threadlist_ctrl->SetNumber(1);
	} else {
		wxString cur=ans.Mid(ans.Find("current-thread-id=")+16).BeforeFirst('\"');
		ans=GetValueFromAns(ans,"thread-ids");
		int c=0;
		wxString id,det,file,line,func;
		int i=ans.Find("thread-id=");
		while (i!=wxNOT_FOUND) {
			ans=ans.Mid(i+11);
			id=ans.BeforeFirst('\"');
			det = SendCommand("-thread-info ",id);
//			cerr<<det<<endl;
			i = det.Find("threads=");
			func.Clear();
			if (i!=wxNOT_FOUND) {
				det=GetValueFromAns(det.Mid(i+10),"frame");
				if (det.Len()) {
					det=det.Mid(1);
					func=GetValueFromAns(det,"func");
					file=GetValueFromAns(det,"file");
					if (!file.Len()) file=GetValueFromAns(det,"fullname");
					line=GetValueFromAns(det,"line");
				}
			}
			if (!func.Len()) {
				func=LANG(THREADS_NO_INFO,"<<Informacion no disponible>>");
				file.Clear(); line.Clear();
			}
			main_window->threadlist_ctrl->SetData(c++,id,func,file,line);
			if (id==cur) main_window->threadlist_ctrl->SelectRow(c-1);
			i=ans.Find("thread-id=");
//			cerr<<c<<"   "<<func<<endl;
		}
//		cerr<<endl;
//		cerr<<endl;
		main_window->threadlist_ctrl->SetNumber(c);
	}
}

void DebugManager::ThreadListClean() {
	main_window->threadlist_ctrl->SetData(0,"","","","");
	main_window->threadlist_ctrl->SetNumber(1);
}

void DebugManager::SelectThread(wxString id) {
	SendCommand("-thread-select ",id);
	Backtrace();
//	UpdateFramelessInspection();
}

void DebugManager::SetFullOutput (bool on) {
	if (on) {
		SendCommand(_T("set print repeats 0"));
		SendCommand(_T("set print elements 0"));
	} else {
		SendCommand(_T("set print repeats 100"));
		SendCommand(_T("set print elements 100"));
	}
}

void DebugManager::ShowBreakPointLocationErrorMessage (BreakPointInfo *_bpi) {
	static bool show_breakpoint_error=true;
	if (!show_breakpoint_error) return;
	int res=mxMessageDialog(main_window,
		wxString(LANG(DEBUG_BAD_BREAKPOINT_WARNING,"El depurador no pudo colocar un punto de interrupcion en:"))<<
		"\n"<<_bpi->fname<<": "<<_bpi->line_number+1<<"\n"<<
		LANG(DEBUG_BAD_BREAKPOINT_WARNING_LOCATION,"Las posibles causas son:\n"
		"* Fue colocado en un archivo que no se compila en el proyecto/programa.\n"
		"* Fue colocado en una linea que no genera codigo ejecutable (ej: comentario).\n"
		"* Información de depuración desactualizada o inexistente. Intente recompilar\n"
		"   completamente el programa/proyecto, utilizando el item Limpiar del menu Ejecucion\n"
		"   antes de depurar.\n"
		"* Espacios o acentos en las rutas de los archivos fuente. Si sus directorios contienen\n"
		"   espacios o acentos en sus nombres pruebe renombrarlos o mover el proyecto.")
		,LANG(GENERAL_WARNING,"Aviso"),mxMD_WARNING|mxMD_OK,"No volver a mostrar este mensaje",false).ShowModal();
	if (res&mxMD_CHECKED) show_breakpoint_error=false;
}

void DebugManager::ShowBreakPointConditionErrorMessage (BreakPointInfo *_bpi) {
	static bool show_breakpoint_error=true;
	if (!show_breakpoint_error) return;
	int res=mxMessageDialog(main_window,
		wxString(LANG(DEBUG_BAD_BREAKPOINT_WARNING,"El depurador no pudo colocar un punto de interrupcion en:"))<<
		"\n"<<_bpi->fname<<": "<<_bpi->line_number+1<<"\n"<<
		LANG(DEBUG_BAD_BREAKPOINT_WARNING_CONDITION,"La condición ingresada no es válida.")
		,LANG(GENERAL_WARNING,"Aviso"),mxMD_WARNING|mxMD_OK,"No volver a mostrar este mensaje",false).ShowModal();
	if (res&mxMD_CHECKED) show_breakpoint_error=false;
}

