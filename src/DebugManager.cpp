#include <wx/process.h>
#include <wx/stream.h>
#include <wx/msgdlg.h>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iostream>
#include "DebugManager.h"
#include "mxCompiler.h"
#include "ConfigManager.h"
#include "mxMainWindow.h"
#include "mxMessageDialog.h"
#include "mxUtils.h"
#include "mxSource.h"
#include "ProjectManager.h"
#include "ids.h"
#include "mxBacktraceGrid.h"
#include "Parser.h"
#include "mxBreakList.h"
#include "mxInspectionMatrix.h"
#include "mxArgumentsDialog.h"
#include "mxApplication.h"
#include "Language.h"
#include "mxOSD.h"
#include "winStuff.h"
#include "mxThreadGrid.h"
#include "Inspection.h"
#include "DebugPatcher.h"
#include "MenusAndToolsConfig.h"
#include "lnxStuff.h"
using namespace std;

#ifdef _DEBUG_LOG
	wxFFile debug_log_file;
#endif


//#define BACKTRACE_MACRO "define zframeaddress\nset $fi=0\nwhile $fi<$arg0\nprintf \"*zframe-%u={\",$fi\ninfo frame $fi\nprintf \"}\\n\"\nset $fi=$fi+1\nend\nend"

DebugManager *debug = NULL;

DebugManager::DebugManager() {
	on_pause_action = NULL;
	signal_handlers_state=NULL;
	backtrace_shows_args=true;
	status = DBGST_NULL;
	current_handle = -1;
	stack_depth = -1;
//	inspections_count = 0;
//	backtrace_visible = false;
	threadlist_visible = false;
	waiting = debugging = running = false;
	process = NULL;
	input = NULL;
	output = NULL;
	pid = 0;
	notitle_source = current_source = NULL;
#ifndef __WIN32__
	tty_pid = 0;
	tty_process = NULL;
#endif
}

DebugManager::~DebugManager() {
}

bool DebugManager::Start(bool update) {
#ifdef _DEBUG_LOG
	debug_log_file.Open(_DEBUG_LOG,"w+");
#endif
	if (update && project->PrepareForBuilding()) { // ver si hay que recompilar antes
		compiler->BuildOrRunProject(true,true,true);
		return false;
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
#ifdef _DEBUG_LOG
	debug_log_file.Open(_DEBUG_LOG,_T("w+"));
#endif
	if (source) {
		// ver si hay que compilar antes
		if (update) {
			if (source->sin_titulo) { // si no esta guardado, siempre compilar
				source->SaveTemp();
				compiler->CompileSource(source,true,true);
				return false;
			} else if (source->GetModify() || !source->GetBinaryFileName().FileExists() || source->GetBinaryFileName().GetModificationTime()<source->source_filename.GetModificationTime()) {
				source->SaveSource();
				compiler->CompileSource(source,true,true);
				return false;
			} else if (config->Running.check_includes && mxUT::AreIncludesUpdated(source->GetBinaryFileName().GetModificationTime(),source->source_filename)) {
				compiler->CompileSource(source,true,true);
				return false;
			}
		}

		SetStateText(LANG(DEBUG_STATUS_STARTING,"Iniciando depuracion..."),true);
		wxString args;
		if (source->config_running.always_ask_args) {
			int res = mxArgumentsDialog(main_window,source->exec_args,source->working_folder.GetFullPath()).ShowModal();
			if (res&AD_CANCEL) return 0;
			if (res&AD_ARGS) {
				source->exec_args = mxArgumentsDialog::last_arguments;
				source->working_folder = mxArgumentsDialog::last_workdir;
				args = source->exec_args ;
			}
			if (res&AD_REMEMBER) {
				source->config_running.always_ask_args=false;
				if (res&AD_EMPTY) source->exec_args ="";
			}
		} else if (source->exec_args[0]!='\0')
			args = source->exec_args;	

		compiler->last_caption = source->page_text;
		compiler->last_runned = current_source = source;
		if (Start(source->working_folder.GetFullPath(),source->GetBinaryFileName().GetFullPath(),args,true,source->config_running.wait_for_key)) {
			int cuantos_sources = main_window->notebook_sources->GetPageCount();
			for (int i=0;i<cuantos_sources;i++) {
				mxSource *src = (mxSource*)(main_window->notebook_sources->GetPage(i));
				if (!src->sin_titulo && source!=src)
					SetBreakPoints(src,true);
			}
			SetBreakPoints(source);
			if (source->sin_titulo) notitle_source=source;
			if (!Run()) {
#ifdef __WIN32__
				if (source->GetBinaryFileName().GetFullPath().Contains(' '))
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
	debug_patcher->Init(exe);
#ifndef __WIN32__
	wxString tty_cmd, tty_file(DIR_PLUS_FILE(config->temp_dir,_T("tty.id")));
	if (show_console) {
		if (wxFileName::FileExists(tty_file))
			wxRemoveFile(tty_file);
		tty_cmd<<config->Files.terminal_command<<" "<<config->Files.runner_command<<_T(" -tty ")<<tty_file;
		tty_cmd.Replace("${TITLE}",LANG(GENERA_CONSOLE_CAPTION,"ZinjaI - Consola de Ejecucion"));
		if (wait_for_key) tty_cmd<<_T(" -waitkey");
	//	mxUT::ParameterReplace(tty_cmd,_T("${ZINJAI_DIR}"),wxGetCwd());
		tty_process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_DEBUG);
		tty_pid = wxExecute(tty_cmd,wxEXEC_ASYNC,tty_process);
	} else {
		tty_pid=0; tty_process=NULL;
	}
#endif
	wxString command(config->Files.debugger_command);
	command<<_T(" -quiet -nx -interpreter=mi");
	if (config->Debug.readnow)
		command<<_T(" --readnow");
	if (wxFileName(DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)<<"\"";
	if (project && project->macros_file.Len() && wxFileName(DIR_PLUS_FILE(project->path,project->macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(project->path,project->macros_file)<<"\"";
#ifndef __WIN32__
	if (show_console) {
		pid=0;
		wxDateTime t0=wxDateTime::Now(); // algunas terminales no esperan a que lo de adentro se ejecute para devolver el control (mac por ejemplo)
		while ((tty_pid || (wxDateTime::Now()-t0).GetSeconds()<10) && !wxFileName::FileExists(tty_file))
			{ wxYield(); wxMilliSleep(100); }
		if (!tty_pid && !wxFileName::FileExists(tty_file)) {
			debugging = false;
			mxMessageDialog(main_window,LANG(DEBUG_ERROR_WITH_TERMINAL,"Ha ocurrido un error al iniciar la terminal para la ejecucion.\n"
										   "Compruebe que el campo \"Comando del terminal\" de la pestaña\n"
										   "\"Rutas 2\" del cuadro de \"Preferencias\" sea correcto."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			main_window->SetCompilingStatus(LANG(DEBUG_STATUS_INIT_ERROR,"Error al iniciar depuracion"));
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
//	if (args.Len())
//		command<<_T(" --args \"")<<exe<<"\" "<</*mxUT::EscapeString(*/args/*)*/;	
//	else
		command<<" \""<<exe<<"\"";	
	process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_DEBUG);
	process->Redirect();
	
#ifndef __WIN32__
	if (project && project->active_configuration->exec_method==EMETHOD_INIT) {
		command=wxString()<<"/bin/sh -c "<<mxUT::SingleQuotes(wxString()
			<<". "<<DIR_PLUS_FILE(project->path,project->active_configuration->exec_script)<<" &>/dev/null; "<<command);
	}
#endif
	
	pid = wxExecute(command,wxEXEC_ASYNC,process);
	if (pid>0) {
		input = process->GetInputStream();
		output = process->GetOutputStream();
		wxString hello = WaitAnswer();
		if (hello.Find(_T("no debugging symbols found"))!=wxNOT_FOUND) {
			mxMessageDialog(main_window,LANG(DEBUG_NO_SYMBOLS,"El ejecutable que se intenta depurar no contiene informacion de depuracion.\nCompruebe que en las opciones de depurarcion este activada la informacion de depuracion,\nverifique que no este seleccionada la opcion \"stripear el ejecutable\" en las opciones de enlazado,\n y recompile el proyecto si es necesario (Ejecucion->Limpiar y luego Ejecucion->Compilar)."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			SendCommand(_T("-gdb-exit"));
			debugging = false; has_symbols=false;
			main_window->SetCompilingStatus(LANG(DEBUG_STATUS_INIT_ERROR,"Error al iniciar depuracion"));
			return false;
		}
		Start_ConfigureGdb();
		// configure debugger
#ifdef __WIN32__
		SendCommand(_T("-gdb-set new-console on"));
#endif
		if (args.Len()) cerr<<SendCommand("set args ",args);
//		SendCommand(_T(BACKTRACE_MACRO));
		SendCommand(wxString(_T("-environment-cd "))<<mxUT::EscapeString(workdir,true));
		main_window->PrepareGuiForDebugging(gui_is_prepared=true);
		return true;
	} else  {
			mxMessageDialog(main_window,wxString(LANG(DEBUG_ERROR_STATING_GDB,"Ha ocurrido un error al ejecutar el depurador."))
#ifndef __WIN32__
							<<"\n"<<LANG(DEBUG_ERROR_STARTING_GDB_LINUX,"Si el depurador (gdb) no se encuentra instalado\n"
							"en su systema debe instalarlo con el gestor de\n"
							"paquetes que corresponda a su distribucion\n"
							"(apt-get, yum, yast, installpkg, etc.)")
#endif
				,"Error al iniciar depurador",mxMD_OK|mxMD_ERROR).ShowModal();
			main_window->PrepareGuiForDebugging(false);
	}
	pid=0;
	debugging = false;
	main_window->SetCompilingStatus(LANG(DEBUG_STATUS_INIT_ERROR,"Error al iniciar depuracion"));
	return false;
}


/**
* Resetea los atributos utilizados durante la depuración (como lista de inspecciones,
* banderas de estado, buffer para comunicación con el proceso de gdb, etc).
* Es llamada por Start y LoadCoreDump.
**/

void DebugManager::ResetDebuggingStuff() {
	status=DBGST_STARTING;
#ifndef __WIN32__
	tty_pid = 0; tty_process=NULL;
#endif
	black_list.Clear(); stepping_in=false;
	mxUT::Split(config->Debug.blacklist,black_list,true,false);
	gui_is_prepared = false;
	
	// setear en -1 todos los ids de los pts de todos interrupcion, para evitar confusiones con depuraciones anteriores
	GlobalListIterator<BreakPointInfo*> bpi=BreakPointInfo::GetGlobalIterator();
	while (bpi.IsValid()) { bpi->gdb_id=-1; bpi.Next(); }
	
	buffer[0]=buffer[1]=buffer[2]=buffer[3]=buffer[4]=buffer[5]=' ';
	buffer[6]='\0';
	debugging = true;
	child_pid = pid = 0;
	input = NULL;
	output = NULL;
	recording_for_reverse=inverse_exec=false;
	main_window->ClearDebugLog();
	has_symbols=true;
	should_pause=false;
	debug_patcher = new DebugPatcher();
	current_thread_id = -1; current_frame_id = -1;
	if (on_pause_action) delete on_pause_action;
	on_pause_action = NULL;
	watchpoints.clear();
}


bool DebugManager::SpecialStart(mxSource *source, const wxString &gdb_command, const wxString &status_message, bool should_continue) {
#ifdef _DEBUG_LOG
	debug_log_file.Open(_DEBUG_LOG,"w+");
#endif
	mxOSD osd(main_window,LANG(OSD_STARTING_DEBUGGER,"Iniciando depuracion..."));
	ResetDebuggingStuff();
	wxString exe = source?source->GetBinaryFileName().GetFullPath():DIR_PLUS_FILE(project->path,project->active_configuration->output_file);
	wxString command(config->Files.debugger_command);
	command<<_T(" -quiet -nx -interpreter=mi");
	if (config->Debug.readnow)
		command<<_T(" --readnow");
	if (wxFileName(DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)<<"\"";
	if (project && project->macros_file.Len() && wxFileName(DIR_PLUS_FILE(project->path,project->macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(project->path,project->macros_file)<<"\"";
	command<<" "<<mxUT::Quotize(exe);	
	process = new wxProcess(main_window->GetEventHandler(),mxPROCESS_DEBUG);
	process->Redirect();
	pid = wxExecute(command,wxEXEC_ASYNC,process);
	if (pid>0) {
		input = process->GetInputStream();
		output = process->GetOutputStream();
		wxString hello = WaitAnswer();
		if (hello.Find("no debugging symbols found")!=wxNOT_FOUND) {
			mxMessageDialog(main_window,LANG(DEBUG_NO_SYMBOLS,"El ejecutable que se intenta depurar no contiene informacion de depuracion.\nCompruebe que en las opciones de depurarcion este activada la informacion de depuracion,\nverifique que no este seleccionada la opcion \"stripear el ejecutable\" en las opciones de enlazado,\n y recompile el proyecto si es necesario (Ejecucion->Limpiar y luego Ejecucion->Compilar)."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			SendCommand("-gdb-exit");
			debugging = false;
			return false;
		}
		// configure debugger
		Start_ConfigureGdb();
//		SendCommand(_T(BACKTRACE_MACRO));
		main_window->PrepareGuiForDebugging(gui_is_prepared=true);
		// mostrar el backtrace y marcar el punto donde corto
		wxString ans = SendCommand(gdb_command);
		if (ans.StartsWith("^error")) {
			mxMessageDialog(main_window,wxString(LANG(DEBUG_SPECIAL_START_FAILED,"Ha ocurrido un error al iniciar la depuración:"))+debug->GetValueFromAns(ans,"msg",true,true),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
			main_window->SetCompilingStatus(LANG(DEBUG_STATUS_INIT_ERROR,"Error al iniciar depuracion"));
			Stop(); return false;
		}
		SetStateText(status_message);
		UpdateBacktrace();
//		long line;
//		main_window->backtrace_ctrl->GetCellValue(0,BG_COL_LINE).ToLong(&line);
//		wxString file = main_window->backtrace_ctrl->GetCellValue(0,BG_COL_FILE);
//		if (file.Len()) {
//			debug->MarkCurrentPoint(file,line,mxSTC_MARK_STOP);
//			debug->SelectFrame(-1,1);
//		}
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
		if (should_continue) { osd.Hide(); Continue(); }
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
#ifdef _DEBUG_LOG
	debug_log_file.Open(_DEBUG_LOG,"w+");
#endif
	
	mxOSD osd(main_window,project?LANG(OSD_LOADING_CORE_DUMP,"Cargando volcado de memoria..."):"");
	
	ResetDebuggingStuff();
	wxString exe = source?source->GetBinaryFileName().GetFullPath():DIR_PLUS_FILE(project->path,project->active_configuration->output_file);
	wxString command(config->Files.debugger_command);
	command<<_T(" -quiet -nx -interpreter=mi");
	if (config->Debug.readnow)
		command<<_T(" --readnow");
	if (wxFileName(DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(config->zinjai_dir,config->Debug.macros_file)<<"\"";
	if (project && project->macros_file.Len() && wxFileName(DIR_PLUS_FILE(project->path,project->macros_file)).FileExists())
		command<<_T(" -x \"")<<DIR_PLUS_FILE(project->path,project->macros_file)<<"\"";
	command<<" -c "<<mxUT::Quotize(core_file)<<" "<<mxUT::Quotize(exe);	
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
		Start_ConfigureGdb();
//		SendCommand(_T(BACKTRACE_MACRO));
		main_window->PrepareGuiForDebugging(gui_is_prepared=true);
		// mostrar el backtrace y marcar el punto donde corto
		UpdateBacktrace();
		DebuggerInspection::OnDebugStart();
//		long line;
//		main_window->backtrace_ctrl->GetCellValue(0,BG_COL_LINE).ToLong(&line);
//		wxString file = main_window->backtrace_ctrl->GetCellValue(0,BG_COL_FILE);
//		if (file.Len()) {
//			debug->MarkCurrentPoint(file,line,mxSTC_MARK_STOP);
//			debug->SelectFrame(-1,0);
//		}
		return true;
	}
	pid=0;
	debugging = false;
	return false;
}

bool DebugManager::Stop() {
	if (status==DBGST_STOPPING) return false; else status=DBGST_STOPPING;
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
		output->Write("-exec-abort\n",12);
		output->Write("-gdb-exit\n",10);
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
		return true;
	} else {
		running = false;
		return false;
	}
}

void DebugManager::HowDoesItRuns() {
#ifdef __WIN32__
	static wxString sep="\\",wrong_sep="/";
#else
	static wxString sep="/",wrong_sep="\\";
#endif
	SetStateText(LANG(DEBUG_STATUS_RUNNING,"Ejecutando..."));
	MarkCurrentPoint();
	
	while (true) { // para que vuelva a este punto cuando llega a un break point que no debe detener la ejecucion
		running = true; 
		wxString ans = WaitAnswer();
		running = false;
		wxString state_text=LANG(DEBUG_STATUS_UNKNOWN,"Estado desconocido"); 
		if (!process || status==DBGST_STOPPING) return;
		int st_pos = ans.Find(_T("*stopped"));
		if (st_pos==wxNOT_FOUND) {
			_IF_DEBUGMODE(wxMessageBox(wxString("HowDoesItRuns answer: ")<<ans));
			SetStateText(state_text);
#ifdef _DEBUG_LOG
			wxString debug_log_string; debug_log_string<<"ERROR RUNNING: "<<ans;
			debug_log_file.Write(debug_log_string);
			debug_log_file.Flush();
	#endif
			return;
		}
		ans=ans.Mid(st_pos);
		wxString how = GetValueFromAns(ans,"reason",true);
		wxString thread_id = GetValueFromAns(ans,"thread-id",true);
		if (!thread_id.ToLong(&current_thread_id)) current_thread_id=-1;
		
#define _aux_continue SendCommand(_T("-exec-continue")); waiting=true; wxYield(); waiting=false; continue;
		bool should_continue=false; // cuando se pauso solo para colocar un brekapoint y seguir, esto indica que siga sin analizar la salida... puede ser how diga signal-received (lo normal) o que se haya pausado justo por un bp de los que solo actualizan la tabla de inspecciones
		
		if (on_pause_action) {// si se pauso solo para colocar un brekapoint o algo asi, hacerlo y setear banderas para que siga ejecutando
			on_pause_action->Do(); 
			delete on_pause_action; 
			on_pause_action=NULL;
			should_pause=false; // la pausa no la generó el usuario, sino que era solo para colocar el brakpoint
			should_continue=true;
		}
		
		BreakPointInfo *bpi=NULL; int mark = 0;
		if (how=="breakpoint-hit") {
			wxString sbn = GetValueFromAns(ans,"bkptno",true);
			if (sbn.Len()) {
				long bn;
				if (sbn.ToLong(&bn)) {
					bpi=BreakPointInfo::FindFromNumber(bn,true);
					if (!should_pause && bpi && bpi->action==BPA_INSPECTIONS) {
						UpdateInspections();
						_aux_continue;
					}
				}
			}
			mark = mxSTC_MARK_EXECPOINT;
			state_text=LANG(DEBUG_STATUS_BREAK,"El programa alcanzo un punto de interrupcion");
		} else if (how==_T("watchpoint-trigger") || how==_T("access-watchpoint-trigger") || how==_T("read-watchpoint-trigger")) {
			mark = mxSTC_MARK_EXECPOINT;
			state_text=LANG(DEBUG_STATUS_WATCH,"El programa se interrumpio por un WatchPoint: ");
			wxString num = GetValueFromAns(ans.AfterFirst('{'),"wpnum",true);
			state_text << num << ": " << watchpoints[num];
		} else if (how==_T("watchpoint-scope")) {
			mark = mxSTC_MARK_EXECPOINT;
			state_text=LANG(DEBUG_STATUS_WATCH_OUT,"El WatchPoint ha dejado de ser valido: ");
			wxString num = GetValueFromAns(ans.AfterFirst('{'),"wpnum",true);
			state_text << num << ": " << watchpoints[num];
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
			if (should_continue) { _aux_continue; }
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
#ifdef _DEBUG_LOG
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
			if (stepping_in && mark==mxSTC_MARK_EXECPOINT && black_list.Index(fname)!=wxNOT_FOUND)
				StepIn();
			else {
				stepping_in=false;
				if (line.ToLong(&fline)) {
//					MarkCurrentPoint(fname,fline,mark);  // lo hace UpdateBacktrace
					if (threadlist_visible) UpdateThreads();
				} else {
					if (threadlist_visible) UpdateThreads();
				}
				UpdateBacktrace();
				UpdateInspections();
			}
		} else {
			BacktraceClean();
			ThreadListClean();
			running = false; // es necesario esto?
			Stop();
		}
		if (bpi && bpi->action==BPA_STOP_ONCE) bpi->SetStatus(BPS_DISABLED_ONLY_ONCE);
		SetStateText(state_text); should_pause=false;
		if (bpi && bpi->annotation.Len() && current_source) {
			wxYield();
			current_source->ShowBaloon(bpi->annotation,current_source->PositionFromLine(current_source->GetCurrentLine()));
		}
		return;
	}
}

/**
* @brief Removes a breakpoint from gdb and from ZinjaI's list
*
* If !debug->waiting it deletes the brekapoint inmediatly, but if its running
* it marks the breakpoint to be deleted with pause_* and Pause the execution.
* Next time debug see the execution paused will invoke this method again and 
* resume the execution, so user can delete breakpoint without directly pausing
* the program being debugged.
**/
bool DebugManager::DeleteBreakPoint(BreakPointInfo *_bpi) {
	if (!debugging) return false;
	if (_bpi->gdb_id==-1) { // only invalid breakpoints can be safetly deleted from zinjai without interacting with gdb (the ones that were not correctly setted in gdb (for instance, with a wrong line number))
		delete _bpi; 
		return true;
	}
	if (waiting) { // si esta ejecutando, anotar para sacar y mandar a pausar
		class OnPauseRemoveBreakpoint : public OnPauseAction {
			BreakPointInfo *p;
		public:
			OnPauseRemoveBreakpoint(BreakPointInfo *bp) :p(bp) {}
			void Do() /*override*/ { debug->DeleteBreakPoint(p); }
			bool Invalidate(void *ptr) /*override*/ { return p==ptr; }
		};
		PauseFor(new OnPauseRemoveBreakpoint(_bpi));
		return false;
	} else {
		// decirle a gdb que lo saque
		SendCommand("-break-delete ",_bpi->gdb_id);
		// sacarlo de la memoria y las listas de zinjai
		delete _bpi;
	}
	return true;
}

int DebugManager::SetLiveBreakPoint(BreakPointInfo *_bpi) {
	if (debugging && waiting) {
		class OnPauseAddBreakpoint : public OnPauseAction {
			BreakPointInfo *p;
		public:
			OnPauseAddBreakpoint(BreakPointInfo *bp) :p(bp) {}
			void Do() /*override*/ { debug->SetBreakPoint(p); }
			bool Invalidate(void *ptr) /*override*/ { return p==ptr; }
		};
		PauseFor(new OnPauseAddBreakpoint(_bpi));
		_bpi->SetStatus(BPS_PENDING);
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
int DebugManager::SetBreakPoint(BreakPointInfo *_bpi, bool quiet) {
	if (waiting || !debugging) return 0;
	wxString adr = GetAddress(_bpi->fname,_bpi->line_number);
	if (!adr.Len()) { _bpi->SetStatus(BPS_ERROR_SETTING); if (!quiet) ShowBreakPointLocationErrorMessage(_bpi); return -1;  }
	wxString ans = SendCommand(wxString("-break-insert \"\\\"")<<_bpi->fname<<":"<<_bpi->line_number+1<<"\\\"\"");
	wxString num = GetSubValueFromAns(ans,"bkpt","number",true);
	if (!num.Len()) { // a veces hay que poner dos barras (//) antes del nombre del archivo en vez de una (en los .h? ¿por que?)
		wxString file=_bpi->fname;
		int p = file.Find('/',true);
		if (p!=wxNOT_FOUND) {
			wxString file2 = file.Mid(0,p);
			file2<<'/'<<file.Mid(p);
			ans = SendCommand(wxString("-break-insert \"")<<file2<<":"<<_bpi->line_number+1<<"\"");
			num = GetSubValueFromAns(ans,"bkpt","number",true);
		}
	}
	int id=-1;
	BREAK_POINT_STATUS status=BPS_SETTED;
	if (num.Len()) {
		long l; num.ToLong(&l); id=l;
		// setear las opciones adicionales
		if (_bpi->ignore_count) SetBreakPointOptions(id,_bpi->ignore_count);
		if (_bpi->action==BPA_STOP_ONCE ||!_bpi->enabled) SetBreakPointEnable(id,_bpi->enabled,_bpi->action==BPA_STOP_ONCE);
		if (!_bpi->enabled) status=BPS_USER_DISABLED;
		if (_bpi->cond.Len()) if (!SetBreakPointOptions(id,_bpi->cond)) { status=BPS_ERROR_SETTING; ShowBreakPointConditionErrorMessage(_bpi); }
	} else { // si no se pudo colocar correctamente
		status=BPS_ERROR_SETTING; 
		if (!quiet) ShowBreakPointLocationErrorMessage(_bpi);
	}
	_bpi->SetStatus(status,id);
	return id;
}

wxString DebugManager::InspectExpression(wxString var, bool full) {
	if (waiting || !debugging) return "";
	SetFullOutput(full);
	if (var.StartsWith(">")) return GetMacroOutput(var.Mid(1));
	return GetValueFromAns( SendCommand(_T("-data-evaluate-expression "),mxUT::EscapeString(var,true)),_T("value") ,true,true);
}

void DebugManager::SetBacktraceShowsArgs(bool show) {
	backtrace_shows_args=show;
	UpdateBacktrace();
}

bool DebugManager::UpdateBacktrace(bool set_frame) {
#ifdef __WIN32__
	static wxString sep="\\",wrong_sep="/";
#else
	static wxString sep="/",wrong_sep="\\";
#endif
	if (waiting || !debugging) return false;
	main_window->backtrace_ctrl->BeginBatch();
	
	int last_stack_depth = stack_depth>BACKTRACE_SIZE?BACKTRACE_SIZE:stack_depth; 
	
	// averiguar las direcciones de cada frame, para saber donde esta cada inspeccion V2 
	// ya no se usan "direcciones", sino que simplemente se numeran desde el punto de entrada para "arriba"
	if (!GetValueFromAns(SendCommand("-stack-info-depth ",BACKTRACE_SIZE),"depth",true).ToLong(&stack_depth)) stack_depth=0;
	
	wxString frames = stack_depth>0?SendCommand("-stack-list-frames 0 ",stack_depth-1):"";
	
	const wxChar * chfr = frames.c_str();
	// to_select* es para marcar el primer frame que tenga info de depuracion
	int i=frames.Find("stack=");
	if (i==wxNOT_FOUND) {
		current_frame_id=-1;
		for (int c=0;c<last_stack_depth;c++) {
			main_window->backtrace_ctrl->SetCellValue(c,BG_COL_FILE,LANG(BACKTRACE_NO_INFO,"<<Imposible determinar ubicacion>>"));
			main_window->backtrace_ctrl->SetCellValue(c,BG_COL_FUNCTION,LANG(BACKTRACE_NO_INFO,"<<Imposible determinar ubicacion>>"));
		}
		main_window->backtrace_ctrl->EndBatch();
		return false;
	} else i+=7; 
	int cant_levels=0, to_select_level=-1, sll=frames.Len();
	while (cant_levels<BACKTRACE_SIZE) {
		while (i<sll && chfr[i]!='{') i++; 
		if (i==sll) break;
		int p=i+1;
		while (chfr[i]!='}') i++; 
		wxString s(frames.SubString(p,i-1));
#ifdef _ZINJAI_DEBUG
		if (GetValueFromAns(s,"level",true)!=(wxString()<<(cant_levels)))
			cerr<<"ERROR: DebugManager::Backtrace  wrong frame level!!!"<<endl;
#endif
		main_window->backtrace_ctrl->SetCellValue(cant_levels,BG_COL_LEVEL,wxString()<<(cant_levels+1));
		wxString func = GetValueFromAns(s,"func",true);
		if (func[0]=='?') {
			main_window->backtrace_ctrl->SetCellValue(cant_levels,BG_COL_FUNCTION,LANG(BACKTRACE_NOT_AVAILABLE,"<<informacion no disponible>>"));
			main_window->backtrace_ctrl->SetCellValue(cant_levels,BG_COL_FILE,"");
			main_window->backtrace_ctrl->SetCellValue(cant_levels,BG_COL_LINE,"");
			main_window->backtrace_ctrl->SetCellValue(cant_levels,BG_COL_ARGS,"");
		} else {
			main_window->backtrace_ctrl->SetCellValue(cant_levels,BG_COL_FUNCTION,func);
			wxString fname = GetValueFromAns(s,"fullname",true,true);
			if (!fname.Len()) fname = GetValueFromAns(s,"file",true,true);
			fname.Replace("\\\\",sep,true);
			fname.Replace("//",sep,true);
			fname.Replace(wrong_sep,sep,true);
			main_window->backtrace_ctrl->SetCellValue(cant_levels,BG_COL_FILE,fname);
			// seleccionar el frame de mas arriba que tenga info de depuracion (que en line diga algo)
			wxString line_str=GetValueFromAns(s,"line",true);
			main_window->backtrace_ctrl->SetCellValue(cant_levels,BG_COL_LINE,line_str);
			if (to_select_level==-1 && line_str.Len() && wxFileName::FileExists(fname))
				to_select_level=cant_levels;
		}
		cant_levels++;
	}
	
	// completar la columna de argumentos si es que está visible
	if (backtrace_shows_args) { 
		debug->SetFullOutput(false);
		wxString args ,args_list = cant_levels?SendCommand("-stack-list-arguments 1 0 ",cant_levels-1):"";
		const wxChar * chag = args_list.c_str();
	//cerr<<"CHAG="<<endl<<chag<<endl<<endl;
		i=args_list.Find("stack-args=");
		if (i==wxNOT_FOUND) {
			for (int c=0;c<last_stack_depth;c++)
				main_window->backtrace_ctrl->SetCellValue(c,BG_COL_ARGS,LANG(BACKTRACE_NO_ARGUMENTS,"<<Imposible determinar argumentos>>"));
			main_window->backtrace_ctrl->EndBatch();
		} else {
			bool comillas = false, cm_dtype=false; //cm_dtype indica el tipo de comillas en que estamos, inicializar en false es solo para evitar el warning
			i+=12;
			for (int c=0;c<cant_levels;c++) {
				// chag+i = frame={level="0",args={{name="...
				while (chag[i]!='[' && chag[i]!='{') 
					i++; 
				int p=++i, arglev=0;
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
					if (args.Len()) args<<", ";
					args<<GetValueFromAns(sub,"name",true)<<"="<<GetValueFromAns(sub,"value",true,true);
				}
				main_window->backtrace_ctrl->SetCellValue(c,BG_COL_ARGS,args);
			}	
		}
	}
	
	// "limpiar" los renglones que sobran
	for (int c=stack_depth; c<last_stack_depth; c++) {
		for (int i=0;i<BG_COLS_COUNT;i++)
			main_window->backtrace_ctrl->SetCellValue(c,i,"");
	}
	
	// seleccionar el frame actual, o el más cercano que tenga info de depuración
	if (to_select_level>=0) {
		main_window->backtrace_ctrl->SelectRow(to_select_level);
		SelectFrame(-1,to_select_level);
		wxString file=main_window->backtrace_ctrl->GetCellValue(to_select_level,BG_COL_FILE);
		wxString sline=main_window->backtrace_ctrl->GetCellValue(to_select_level,BG_COL_LINE);
		long line=0; if (sline.ToLong(&line)) debug->MarkCurrentPoint(file,line,mxSTC_MARK_FUNCCALL);
	} else {
		main_window->backtrace_ctrl->SelectRow(0);
		current_frame_id = GetFrameID(0);
	}
	
	main_window->backtrace_ctrl->EndBatch();
	return true;
}

void DebugManager::StepIn() {
	if (waiting || !debugging) return;
	stepping_in=true; running=true;
	wxString ans = SendCommand("-exec-step");
	if (ans.Mid(1,7)="running") HowDoesItRuns();
	else running = false;
}

void DebugManager::StepOut() {
	if (waiting || !debugging) return;
	running = true;
	wxString ans = SendCommand("-exec-finish");
	if (ans.Mid(1,7)="running") HowDoesItRuns();
	else running = false;
}

void DebugManager::StepOver() {
	if (waiting || !debugging) return;
	running = true; 
	wxString ans = SendCommand("-exec-next");
	if (ans.Mid(1,7)="running")	HowDoesItRuns();
	else running = false;
}


void DebugManager::Pause() {
	should_pause=true;
	if (!waiting && !debugging) return;
#ifdef __WIN32__
	if (!winLoadDBP()) {
		mxMessageDialog(main_window,"Esta caracteristica no se encuentra presente en versiones de Windows previas a XP-SP2",LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
		return;
	}
	if (FindOutChildPid()) winDebugBreak(child_pid);
#else
	process->Kill(pid,wxSIGINT);
#endif
}

/**
* Si ya se conocia (child_pid!=0) retorna true sin hacer nada, sino retorna true 
* si logra determinarlo, en cuyo caso lo coloca en child_pid.
* En linux usa el comando gdb "info proc status", en windows busca un proceso 
* hijo del proceso de gdb.
**/
bool DebugManager::FindOutChildPid() {
	if (child_pid!=0) return true; // si ya se sabe cual es, no hace nada
#ifdef __WIN32__
	child_pid = winGetChildPid(pid);
#else
	wxString val= mxUT::UnEscapeString(SendCommand("info proc status"));
	int pos = val.Find("Pid:"); 
	if (pos==wxNOT_FOUND) return false;
	pos+=4; while (val[pos]==' '||val[pos]=='\t') pos++;
	child_pid=0; 
	while (val[pos]>='0'&&val[pos]<='9') { 
		child_pid*=10; child_pid+=val[pos]-'0'; pos++;
	}
#endif
	return child_pid!=0;
}

void DebugManager::Continue() {
	if (waiting || !debugging || status==DBGST_STOPPING) return;
	
#ifndef __WIN32__
	if (config->Debug.return_focus_on_continue && FindOutChildPid()) {
		// intentar darle el foco a alguna ventana de la aplicacion, o a la terminal si no hay ventana
		if (!setFocus(child_pid)) setFocus(tty_pid);
	}
#endif
	
	running = true;
	MarkCurrentPoint();
	wxString ans = SendCommand("-exec-continue");
	if (ans.Mid(1,7)=="running") {
		HowDoesItRuns();
		if (config->Debug.raise_main_window)
			main_window->Raise();
	} else 
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

#ifdef _DEBUG_LOG
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
			if (running) {
				app->Yield(true);
				wxMilliSleep(50);
			} else {
				wxMilliSleep(10);
				wxTimeSpan t2=t1-wxDateTime::Now();
				if (t2.GetSeconds()>30 && mxMD_YES==mxMessageDialog(main_window,_T("Alguna operación en el depurador está tomando demasiado tiempo, desea interrumpirla?."),_T("UPS!"),(mxMD_YES_NO|mxMD_WARNING)).ShowModal())
						return "";
				else t1=wxDateTime::Now();
			}
		}
		if (process && input->IsOk()) 
			c = input->Read(buffer+6,249).LastRead();
		else break;
		buffer[c+6]='\0'; iwarn=c=6;
		
	}
	last_answer = "";
	return "";
}

wxString DebugManager::SendCommand(wxString command) {
	waiting = true;
#ifdef _DEBUG_LOG
		wxString debug_log_string; debug_log_string<<"\n>>> "<<command;
		debug_log_file.Write(debug_log_string);
		debug_log_file.Flush();
#endif
	if (!process) return "";
	last_command=command;
	output->Write(command.c_str(),command.Len());
	output->Write("\n",1);
	return WaitAnswer();
}

wxString DebugManager::SendCommand(wxString command, int i) {
	waiting = true;
#ifdef _DEBUG_LOG
		wxString debug_log_string; debug_log_string<<"\n>>> "<<command<<i;
		debug_log_file.Write(debug_log_string);
		debug_log_file.Flush();
#endif
	if (!process) return "";
	command<<i<<"\n";
	last_command=command;
	output->Write(command.c_str(),command.Len());
	return WaitAnswer();
}

wxString DebugManager::SendCommand(wxString cmd1, wxString cmd2) {
	waiting = true;
#ifdef _DEBUG_LOG
		wxString debug_log_string; debug_log_string<<"\n>>> "<<cmd1<<cmd2;
		debug_log_file.Write(debug_log_string);
		debug_log_file.Flush();
#endif
	if (!process) return "";
	cmd1<<cmd2<<"\n";
	last_command=cmd1;
	output->Write(cmd1.c_str(),cmd1.Len());
	return WaitAnswer();
}

/// @brief Sets all breakpoints from an untitled or out of project mxSource
void DebugManager::SetBreakPoints(mxSource *source, bool quiet) {
	if (waiting || !debugging) return;
	const LocalList<BreakPointInfo*> &breakpoints=source->m_extras->GetBreakpoints();
	for(int i=0;i<breakpoints.GetSize();i++) { 
		breakpoints[i]->UpdateLineNumber();
		debug->SetBreakPoint(breakpoints[i],quiet);
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
						current_source = main_window->OpenFile(cfile);
				}
			}
		} else {
			if (notitle_source && notitle_source->temp_filename==cfile)
				current_source = notitle_source;
			else {
				current_source = main_window->IsOpen(cfile);
				if (!current_source) 
					current_source = main_window->OpenFile(cfile);
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
		return "";
	unsigned int sc=0, t = ans.Len()-key.Len()-2;
	key<<_T("=");
	char stack[25];
//	bool not_ignore=true;
	bool is_the_first = (ans.Left(key.Len())==key) ;
	for (unsigned int i=0;i<t;i++) {
		char c=ans[i];
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
	return "";
}

wxString DebugManager::GetSubValueFromAns(wxString ans, wxString key1, wxString key2, bool crop, bool fix_slash) {
	wxString value = GetValueFromAns(ans,key1);
	if (value.Len()) {
		value[0]=value[value.Len()-1]=',';
		return GetValueFromAns(value,key2,crop,fix_slash);
	}
	return "";
}

void DebugManager::SetStateText(wxString text, bool refresh) {
	main_window->SetCompilingStatus(text);
	menu_data->toolbar_status_text->SetLabel(wxString(LANG(DEBUG_STATUS_PREFIX,"  Depuracion: "))+text);
	if (refresh) wxYield();
}


void DebugManager::BacktraceClean() {
	if (stack_depth>BACKTRACE_SIZE) stack_depth=BACKTRACE_SIZE;
	for(int c=0;c<stack_depth;c++) {
		for (int i=0;i<BG_COLS_COUNT;i++)
			main_window->backtrace_ctrl->SetCellValue(c,i,"");
	}
	stack_depth = 0;
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
	if (waiting || !debugging) return "";
#ifdef __WIN32__
	for (unsigned int i=0;i<fname.Len();i++) // corregir las barras en windows para que no sean caracter de escape
		if (fname[i]=='\\') 
			fname[i]='/';
#endif
	wxString ans = SendCommand(wxString("info line \"")<<fname<<":"<<line+1<<"\"");
	int r=ans.Find("starts at");
	if (r!=wxNOT_FOUND) {
		ans=ans.Mid(r);
		r=ans.Find("0x");
		if (r!=wxNOT_FOUND)
			return ans.Mid(r).BeforeFirst(' ');
	}
	return "";
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
		wxString ans=SendCommand("-gdb-set $pc=",adr);
		if (ans.SubString(1,5)!="error") {
			MarkCurrentPoint(fname,line+1,mxSTC_MARK_EXECPOINT);
			UpdateBacktrace();
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
*
* @return true si la direccion dada en fname y line es valida y el depurador efectivamente
*         se puso a ejecutar, false si la direccion no era valida o hubo algun error
**/
bool DebugManager::RunUntil(wxString fname, int line) {
	if (waiting || !debugging) return false;
	running = true;
	wxString adr = GetAddress(fname,line);
	if (adr.Len()) {
		wxString ans = SendCommand("advance *",adr); // aca estaba exec-until pero until solo funciona en un mismo frame, advance se los salta sin problemas
		if (ans.SubString(1,5)=="error")
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
	if (ans.SubString(1,4)!="done") 
		return false;
	
	wxString fname = GetSubValueFromAns(ans,_T("frame"),_T("fullname"),true);
	if (!fname.Len())
		fname = GetSubValueFromAns(ans,_T("frame"),_T("file"),true);
	wxString line =  GetSubValueFromAns(ans,_T("frame"),_T("line"),true);
	long fline = -1;
	line.ToLong(&fline);
	MarkCurrentPoint(fname,fline,mxSTC_MARK_EXECPOINT);
	UpdateBacktrace();
	UpdateInspections();
	return true;
}

void DebugManager::ProcessKilled() {
	delete debug_patcher;
#ifdef _DEBUG_LOG
	debug_log_file.Close();
#endif
	MarkCurrentPoint();
	notitle_source = NULL;
	debugging=false;
#ifndef __WIN32__
	if (tty_process)
		tty_process->Kill(tty_pid,wxSIGKILL);
#endif
	delete process;
	process=NULL;
	pid=0;
	running = debugging = waiting = false;
	status=DBGST_NULL;
	wxCommandEvent evt;
	if (gui_is_prepared) main_window->PrepareGuiForDebugging(false);
	DebuggerInspection::OnDebugStop();
}

#ifndef __WIN32__
void DebugManager::TtyProcessKilled() {
	if (pid && debugging && status!=DBGST_STOPPING) Stop();
	delete tty_process;
	tty_process = NULL;
	tty_pid = 0;
}
#endif

void DebugManager::UpdateInspections() {
	DebuggerInspection::OnDebugPause();
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

bool DebugManager::SelectFrame(long frame_id, long frame_level) {
	if (frame_level==-1) frame_level = GetFrameLevel(frame_id); 
	else if (frame_id==-1) frame_id = GetFrameID(frame_level); 
	wxString ans = SendCommand("-stack-select-frame ",frame_level);
	if (ans.Mid(1,4)=="done") { current_frame_id = frame_id; return true; }
	else return false;
}

bool DebugManager::DoThat(wxString what) {
	wxMessageBox(SendCommand(what),what,wxOK,main_window);
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
					grid->SetCellValue(cont,BL_COL_ENABLE,_T("once"));
				else
					grid->SetCellValue(cont,BL_COL_ENABLE,_T("enabled"));
			} else
				grid->SetCellValue(cont,BL_COL_ENABLE,_T("diabled"));
			wxString fname = GetValueFromAns(item,_T("fullname"),true);
			if (!fname.Len()) fname = GetValueFromAns(item,_T("file"),true);
			grid->SetCellValue(cont,BL_COL_WHY,fname + _T(": line ") +GetValueFromAns(item,_T("line"),true));
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
			grid->SetCellValue(cont,BL_COL_ENABLE,GetValueFromAns(item,_T("enabled"),true)==_T("y")?_T("enabled"):_T("disabled"));
			grid->SetCellValue(cont,BL_COL_WHY,
				mxUT::UnEscapeString(GetValueFromAns(item,_T("number"),true)) + ": "
				+GetValueFromAns(item,_T("what"),true)
				);
		}
		item=GetNextItem(ans,p);
	}
}

/// @brief Define the number of time the breakpoint should be ignored before actually stopping the execution
void DebugManager::SetBreakPointOptions(int num, int ignore_count) {
	wxString cmd(_T("-break-after "));
	cmd<<num<<" "<<ignore_count;
	SendCommand(cmd);
}

/// @brief Define the condition for a conditional breakpoint and returns true if the condition was correctly setted
bool DebugManager::SetBreakPointOptions(int num, wxString condition) {
	wxString cmd(_T("-break-condition "));
	cmd<<num<<" "<<mxUT::EscapeString(condition);
	wxString ans = SendCommand(cmd);
	return ans.Len()>4 && ans.Mid(1,4)=="done";
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
// }

int DebugManager::GetBreakHitCount(int num) {
	long l=0;
	wxString ans = SendCommand(_T("-break-info "),num);
	int p = ans.Find(_T("times="));
	if (p==wxNOT_FOUND) return -1;
	GetValueFromAns(ans.Mid(p),_T("times"),true).ToLong(&l);
	return l;
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
		} else if (ans[p]==94 && ans.Mid(p+1,5)=="error") {
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
		if (!ans.Contains("error")) {
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
		if (!ans.Contains("error")) inverse_exec=!inverse_exec;
	} else {
		mxMessageDialog(main_window,LANG(DEBUG_REVERSE_DISABLED,"Solo se puede retroceder la ejecucion hasta el punto en donde la ejecucion hacia atras fue habilitada.\n"
			                           "Actualmente esta caracteristica no esta habilitada. Utilice el comando \"Habilitar Ejecucion Hacia Atras\"\n"
										"del menu de Depuracion para habilitarla."),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
	}
	return inverse_exec;
}

void DebugManager::UnregisterSource(mxSource *src) {
	if (current_source==src) current_source=NULL;
	if (notitle_source==src) notitle_source=NULL;
}

void DebugManager::UpdateThreads() {
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
		}
		main_window->threadlist_ctrl->SetNumber(c);
	}
}

void DebugManager::ThreadListClean() {
	main_window->threadlist_ctrl->SetData(0,"","","","");
	main_window->threadlist_ctrl->SetNumber(1);
}

bool DebugManager::SelectThread(long thread_id) {
	wxString ans = SendCommand("-thread-select ",thread_id);
	if (!ans.StartsWith("^done")) return false;
	current_thread_id=thread_id; 
	current_frame_id = GetFrameID(0);
	return true;
}

void DebugManager::SetFullOutput (bool on, bool force) {
	static bool is_full=false;
	if (!force&&is_full==on) return;
	if (on) SendCommand("set print elements 0");
	else SendCommand("set print elements 100"); /// @todo: put this number in preferences
	is_full=on;
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

void DebugManager::SendSignal (const wxString & signame) {
	if (waiting || !debugging || status==DBGST_STOPPING) return;
	running = true; MarkCurrentPoint();
	wxString ans = SendCommand("signal ",signame);
	if (ans.Contains("^running")) {
		HowDoesItRuns();
		if (config->Debug.raise_main_window)
			main_window->Raise();
	}
	running = false;
}

bool DebugManager::GetSignals(vector<SignalHandlingInfo> & v) {
	wxString ans; v.clear();
	if (debugging) {
		if (waiting) {
			mxMessageDialog(main_window,"Debe pausar o detener la ejecución para modificar el comportamiento ante señales.",LANG(GENERAL_ERROR,"Error"),mxMD_INFO|mxMD_OK).ShowModal(); return false;
		} else {
			ans = SendCommand("info signals");
			if (!ans.Contains("^done")) return false;
		}
	} else {
		ans = mxUT::GetOutput("gdb --interpreter=mi --quiet --batch -ex \"info signal\"",true);
	}
	while (ans.Contains('\n')) {
		wxString line=ans.BeforeFirst('\n');
		ans=ans.AfterFirst('\n');
		// para saber si esta linea es una señal o no, vemos si empieza con ~"XXX, con XXX mayúsculas 
		// otras lineas son por ej la cabecera de la tabla (~"Signal...), lineas en blanco /~"\n"), o de ayuda (~"Use...)
		if (! (line.Len()>4 && line[0]=='~' && line[1]=='\"' && (line[2]>='A'&&line[2]<='Z') && (line[3]>='A'&&line[3]<='Z') && (line[4]>='A'&&line[4]<='Z') ) ) continue;
		line = mxUT::UnEscapeString(line.Mid(1));
		SignalHandlingInfo si;
		// la primer palabra es el nombre
		int i=0,i0=0, l=line.Len();
		while (i<l && (line[i]!=' '&&line[i]!='\t')) i++;
		si.name=line.Mid(i0,i-i0);
		// la segunda es un Yes/No que corresponde a Stop
		while (i<l && (line[i]==' '||line[i]=='\t')) i++;
		i0=i;
		while (i<l && (line[i]!=' '&&line[i]!='\t')) i++;
		si.stop=line.Mid(i0,i-i0)=="Yes";
		// la segunda es un Yes/No que corresponde a Print
		while (i<l && (line[i]==' '||line[i]=='\t')) i++;
		i0=i;
		while (i<l && (line[i]!=' '&&line[i]!='\t')) i++;
		si.print=line.Mid(i0,i-i0)=="Yes";
		// la segunda es un Yes/No que corresponde a Pass
		while (i<l && (line[i]==' '||line[i]=='\t')) i++;
		i0=i;
		while (i<l && (line[i]!=' '&&line[i]!='\t')) i++;
		si.pass=line.Mid(i0,i-i0)=="Yes";
		// lo que queda es la descripcion
		while (i<l && (line[i]==' '||line[i]=='\t')) i++;
		si.description=line.Mid(i);
		if (si.description.EndsWith("\n")) si.description.RemoveLast();
		v.push_back(si);
	}
	if (!signal_handlers_state) {
		signal_handlers_state = new vector<SignalHandlingInfo>[2];
		signal_handlers_state[0]=signal_handlers_state[1]=v;
	}
	return true;
}

bool DebugManager::SetSignalHandling (SignalHandlingInfo & si, int i) {
	if (debugging) {
		if (waiting) return false;
		wxString cmd("handle "); cmd<<si.name;
		cmd<<" "<<(si.print?"print":"noprint");
		cmd<<" "<<(si.stop?"stop":"nostop");
		cmd<<" "<<(si.pass?"pass":"nopass");
		wxString ans = SendCommand(cmd);
		if (!ans.Contains("^done")) return false;
	}
	if (i!=-1 && signal_handlers_state) signal_handlers_state[1][i]=si;
	return true;
}

void DebugManager::Start_ConfigureGdb ( ) {
	// averiguar la version de gdb
	gdb_version = 0;
	wxString ver=SendCommand("-gdb-version");
	unsigned int i=0; 
	bool have_main_version=false;
	while (i<ver.Len() && !have_main_version) {
		if (i+2<ver.Len() && ver[i]==' ' && (ver[i+1]>='0'&&ver[i+1]<='9') && ver[i+2]=='.') { // 1-digit main version
			gdb_version=(ver[i+1]-'0')*1000; have_main_version=true; i+=2;
		} else if (i+3<ver.Len() && ver[i]!=' ' && (ver[i+1]>='0'&&ver[i+1]<='9') && (ver[i+2]>='0'&&ver[i+2]<='9') && ver[i+3]=='.') { // 2-digits main version
			gdb_version=(ver[i+1]-'0')*10000+(ver[i+2]-'0')*1000; have_main_version=true; i+=3;
		}
		if (have_main_version) {
			if (i+1<ver.Len() && (ver[i+1]>='0'&&ver[i+1]<='9')) {
				if (i+2<ver.Len() && (ver[i+2]>='0'&&ver[i+2]<='9')) // 2-digits sub version
					gdb_version+=(ver[i+1]-'0')*10+(ver[i+2]-'0');
				else // 1-digit sub version
					gdb_version+=(ver[i+1]-'0');
			}	
		}
		i++;
	}
	// configurar comportamiento ante señales
	if (signal_handlers_state) {
		for(unsigned int i=0;i<signal_handlers_state[0].size();i++) { 
			if (signal_handlers_state[0][i]!=signal_handlers_state[1][i])
				debug->SetSignalHandling(signal_handlers_state[1][i]);
		}
	}
	// otras configuraciones varias
	if (!config->Debug.auto_solibs) SendCommand("set auto-solib-add off");
//	SendCommand("set print addr off"); // necesito las direcciones para los helpers de los arreglos
	SendCommand(_T("set print repeats 0"));
	SetFullOutput(false,true);
	// reiniciar sistema de inspecciones
	DebuggerInspection::OnDebugStart();
}

void DebugManager::Initialize() {
	debug = new DebugManager();
}

void DebugManager::TemporaryScopeChange::ChangeIfNeeded(DebuggerInspection *di) {
	if (!di->IsFrameless()) ChangeTo(di->GetFrameID(),di->GetThreadID());
}

bool DebugManager::PauseFor (OnPauseAction * action) {
	if (!debugging) { delete action; return false; } // si no estamos depurando, no hacer nada (no deberia pasar)
	if (!waiting) { action->Do(); delete action; return true; } // si esta en pausa ejecuta en el momento (no deberia pasar)
	if (on_pause_action) return false; // por ahora no se puede encolar mas de una accion
	on_pause_action = action; // encolar la accion
	Pause(); // pausar para que se ejecute
	return true;
}

void DebugManager::InvalidatePauseEvent(void *ptr) {
	if (!on_pause_action) return;
	if (!on_pause_action->Invalidate(ptr)) return;
	delete on_pause_action; on_pause_action=NULL;
}

/// @retval el numero id en gdb si lo agrego, "" si no pudo
wxString DebugManager::AddWatchPoint (const wxString &expression, bool read, bool write) {
	if (expression.IsEmpty()) return "";
	wxString ans = SendCommand("-break-watch ",mxUT::EscapeString(expression));
	if (!ans.StartsWith("^done")) return "";
	wxString num = GetSubValueFromAns(ans,"wpt","number",true);
	watchpoints[num] = expression;
	return num;
}

bool DebugManager::DeleteWatchPoint (const wxString & num) {
	wxString ans = SendCommand("-break-delete ",num);
	return ans.StartsWith("^done");
}

