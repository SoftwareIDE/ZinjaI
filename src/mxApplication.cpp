#include <wx/thread.h>
#include <wx/log.h>
#include <wx/dir.h>
#include <wx/image.h>
#include <wx/filename.h>
#include <wx/utils.h>
#include "mxApplication.h"
#include "mxSource.h"
#include "ShareManager.h"
#include "Language.h"
#include "mxSingleton.h"
#include "mxBeginnerPanel.h"
#include "mxIconInstaller.h"
#include "MenusAndToolsConfig.h"
#include "ConfigManager.h"
#include "HelpManager.h"
#include "Parser.h"
#include "CodeHelper.h"
#include "ProjectManager.h"
#include "DebugManager.h"
#include "mxMainWindow.h"
#include "mxHelpWindow.h"
#include "mxTipsWindow.h"
#include "mxUtils.h"
#include "mxArt.h"
#include "mxMessageDialog.h"
#include "version.h"
#include "mxUpdatesChecker.h"
#include "mxSizers.h"
#include "mxErrorRecovering.h"
#include "mxSplashScreen.h"
using namespace std;

IMPLEMENT_APP(mxApplication)
	
mxApplication *app = nullptr;

#ifdef _ZINJAI_DEBUG
wxLongLong start_time = wxGetLocalTimeMillis();
wxLongLong aux_start_time;
#endif

bool mxApplication::OnInit() {
	
	if (argc==2 && wxString(argv[1])=="--version") {
		cout<<"ZinjaI "<<VERSION<<endl;
		return false;
	}
	
#ifndef _ZINJAI_DEBUG
	wxLog::SetActiveTarget(new wxLogStderr());
#endif
	
	app=this;
	srand(time(nullptr));
	
	sizers = new mxSizers();
	
	wxFileName f_path = wxGetCwd(); 
	f_path.MakeAbsolute();
	wxString cmd_path = f_path.GetFullPath();
	wxFileName f_cmd(argv[0]);

	wxFileName f_zpath = f_cmd.GetPathWithSep();
	f_zpath.MakeAbsolute();
	wxString zpath(f_zpath.GetPathWithSep());
	bool flag=false;
	if (f_zpath!=f_path) {
		if ( (flag=(wxFileName::FileExists(DIR_PLUS_FILE(zpath,"zinjai.dir")) ||wxFileName::FileExists(DIR_PLUS_FILE(zpath,"ZinjaI.dir"))) ) )
			wxSetWorkingDirectory(zpath);
#ifdef __APPLE__
		else if ( (flag=(wxFileName::FileExists(DIR_PLUS_FILE(zpath,"../Resources/zinjai.dir")) ||wxFileName::FileExists(DIR_PLUS_FILE(zpath,"../Resources/ZinjaI.dir"))) ) ) {
			zpath = DIR_PLUS_FILE(zpath,"../Resources");
			wxSetWorkingDirectory(zpath);
		}
#endif
		else
			zpath = cmd_path;
	}

	if (!flag && !wxFileName::FileExists("zinjai.dir") && !wxFileName::FileExists("ZinjaI.dir")) {
 		wxMessageBox("ZinjaI no pudo determinar el directorio donde fue instalado.\n"
			            "Compruebe que el directorio de trabajo actual sea el correcto.\n"
						"ZinjaI cannot determinate installation path. Please verify that\n"
						"working path is the right one.","Error");
	}
	
	// inicialize mxUtils and ConfigManager
	bool first_run = ConfigManager::Initialize(zpath);
	
	// cargar archivo de internacionalizacion
	if (first_run) SelectLanguage();
	
	// si delega la carga a otra instancia termina inmediatamente
	if (InitSingleton(cmd_path)) return false;

	// init image handlers and show splash screen
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxXPMHandler);
	ShowSplash();
	
	// load art and help files
	mxArt::Initialize();
	// inicialize HelpManager
	HelpManager::Initialize();
	
	// inicialize CodeHelper
	CodeHelper::Initialize();

	// inicialize ProjectManager
	project = nullptr;
	
	// verifica si hay compilador, terminal, etc instalado y configurado...
	// ...y carga el resto de la configuracion (toolchains, colores, atajos, toolbars, etc)
	config->FinishiLoading(); 
	
	// create main window
	if (config->Init.size_x==0 || config->Init.size_y==0)
		main_window = new mxMainWindow(nullptr, wxID_ANY, "ZinjaI ", wxDefaultPosition, wxSize(800, 600));
	else
		main_window = new mxMainWindow(nullptr, wxID_ANY, "ZinjaI", wxPoint(config->Init.pos_x,config->Init.pos_y), wxSize(config->Init.size_x,config->Init.size_y));

	// inicialize debug manager
	DebugManager::Initialize();
	
	// set top window and let the magic do the rest
	SetTopWindow(main_window);
	
#if !defined(__WIN32__) && !defined(__APPLE__)
	if (first_run) new mxIconInstaller(true);
#endif
	
	// show startup tip
	if ( (config->Init.show_tip_on_startup && !config->Init.show_welcome) 
#ifndef _ZINJAI_DEBUG
		|| config->Init.version!=VERSION
#endif
	) {
		main_window->Refresh();
		tips_window = new mxTipsWindow(main_window, wxID_ANY);
		if (splash) splash->ShouldClose();
	}
	
	LoadFilesOrWelcomePanel(cmd_path);
	
	// si estaba abriendo un proyecto el usuario puede haber cerrado la ventana antes de que el parser termine
	if (!main_window) return false;
	
	// puede haber llegado algo para el singleton mientras cargabamos
	if (singleton) singleton->ProcessToOpenQueue();
	
	// devolver el foco a la ventana de sugerencias si existe, para que se pueda cerrar con Esc
	if (tips_window) tips_window->Raise();
	
	// recuperarse de un segfault y/o buscar actualizaciones
	if (!mxErrorRecovering::RecoverSomething()) {
		if (config->Init.check_for_updates) 
			mxUpdatesChecker::BackgroundCheck();
	}
	
	return true;
}

void mxApplication::SelectLanguage ( ) {
	wxDir dir("lang");
	wxString spec="*.pre", filename;
	bool cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
	wxArrayString langs;
	while ( cont ) {
		langs.Add(filename.BeforeLast('.'));
		cont = dir.GetNext(&filename);
	}
	if (langs.Index("spanish")==wxNOT_FOUND) langs.Add("spanish");
	if (langs.GetCount()>1) {
		wxString newlang = wxGetSingleChoice("Select a Language:\nSeleccione el idioma:","ZinjaI",langs);
		if (newlang.Len()) config->Init.language_file=newlang;
	}
}

bool mxApplication::InitSingleton(const wxString &cmd_path) {
	// inicialize singleton manager
	singleton = new mxSingleton; // siempre debe existir la instancia...
	if (!config->Init.singleton) return false; // ...aunque no se use
	singleton->Start();
	if (singleton->IsRunning() || argc==1) return false; // si no hay otra instancia o no hay argumentos
	// intentar cargar todo en la otra instancia
	bool done=true;
	for (int i=1; i<argc;i++) {
		wxString name = argv[i];
		if (name!="--last-source" && name!="--last-project" && name!="--no-splash" && name.AfterLast('.').Lower()!=PROJECT_EXT) {
			bool opened = singleton->RemoteOpen(DIR_PLUS_FILE(cmd_path,name));
			int ret=0;
			while (!opened && ret<2) { // dos reintentos, por si estaba muy ocupado
				wxMilliSleep(10*rand()%50); // delay aleatorio tip ethernet
				opened = singleton->RemoteOpen(DIR_PLUS_FILE(cmd_path,name));
				ret++;
			}
			if (opened)
				argv[i][0]='\0';
			else
				done=false;
		} else {
			done=false;
		}
	}
	return done;
}

void mxApplication::ShowSplash ( ) {
	bool no_splash=false;
	for (int i=1;i<argc;i++) 
		if (wxString(argv[i])=="--no-splash") {
			argv[i][0]='\0';
			no_splash=true;
		}
	if (!no_splash && wxFileName(DIR_PLUS_FILE("imgs",SPLASH_FILE)).FileExists()) 
			splash = new mxSplashScreen(DIR_PLUS_FILE("imgs",_T(SPLASH_FILE)));
}

void mxApplication::LoadFilesOrWelcomePanel(const wxString &cmd_path) {
	// load files or create and empty one
	if (argc==1 || (argc==2&&argv[1][0]=='\0')) {
		if (config->Init.show_welcome) {
			main_window->ShowWelcome(true);
		} else {
			if (config->Init.new_file==0)
				main_window->NewFileFromText("");
			else
				main_window->NewFileFromTemplate(mxUT::WichOne(config->Files.default_template,"templates",true));
			main_window->Refresh();
		} 
		if (splash) splash->ShouldClose();
	} else {
		main_window->Refresh();
		wxYield();
		if (splash) splash->ShouldClose();
		for (int i=1; i<argc;i++) {
			if (wxString(argv[i])=="--last-source" && config->Files.last_source[0].Len())
				main_window->OpenFileFromGui(wxString(config->Files.last_source[0]));
			else if (wxString(argv[i])=="--last-project" && config->Files.last_project[0].Len())
				main_window->OpenFileFromGui(wxString(config->Files.last_project[0]));
//			else if (wxString(argv[i])==".")
//				main_window->ShowExplorerTreePanel();
			else if (argv[i][0]!='\0')
				main_window->OpenFileFromGui(wxFileName(DIR_PLUS_FILE(cmd_path,argv[i])).GetLongPath());
		}
		if (!project && main_window->notebook_sources->GetPageCount()==0 && config->Init.show_welcome) {
			main_window->ShowWelcome(true);
		}
	}
	
	
	if ( (project || main_window->notebook_sources->GetPageCount()>0) && config->Init.show_beginner_panel && !config->Init.autohide_panels) {
		if (config->Init.show_beginner_panel && !beginner_panel) {
			beginner_panel = new mxBeginnerPanel(main_window);
			main_window->aui_manager.InsertPane(beginner_panel, wxAuiPaneInfo().Name("beginner_panel").Caption("Panel de Asistencias").Right().Layer(0).Show(), wxAUI_INSERT_ROW);
			_menu_item(mxID_VIEW_BEGINNER_PANEL)->Check(true);
			main_window->aui_manager.Update();
		}
		
	}
}
