#include "mxApplication.h"
#include "mxSource.h"
#include "ShareManager.h"
#include <iostream>
#include "Language.h"
#include "mxSingleton.h"
#include "mxBeginnerPanel.h"
#include <wx/thread.h>
#include "mxIconInstaller.h"

IMPLEMENT_APP(mxApplication)

#include <wx/image.h>
#include <wx/filename.h>

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
#include <wx/utils.h>
using namespace std;

mxApplication *app;

#ifdef DEBUG
wxLongLong start_time = wxGetLocalTimeMillis();
wxLongLong aux_start_time;
#endif

bool mxApplication::OnInit() {
	
	SHOW_MILLIS("Entering OnIni...");
	
	if (argc==2 && wxString(argv[1])==_T("--version")) {
		cout<<"ZinjaI "<<VERSION<<endl;
		return false;
	}
	
	app=this;
	srand(time(0));
	
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
		if ( (flag=(wxFileName::FileExists(DIR_PLUS_FILE(zpath,_T("zinjai.dir"))) ||wxFileName::FileExists(DIR_PLUS_FILE(zpath,_T("ZinjaI.dir")))) ) )
			wxSetWorkingDirectory(zpath);
#ifdef __APPLE__
		else if ( (flag=(wxFileName::FileExists(DIR_PLUS_FILE(zpath,_T("../Resources/zinjai.dir"))) ||wxFileName::FileExists(DIR_PLUS_FILE(zpath,_T("../Resources/ZinjaI.dir")))) ) ) {
			zpath = DIR_PLUS_FILE(zpath,_T("../Resources"));
			wxSetWorkingDirectory(zpath);
		}
#endif
		else
			zpath = cmd_path;
	}

	if (!flag && !wxFileName::FileExists(_T("zinjai.dir")) && !wxFileName::FileExists(_T("ZinjaI.dir"))) {
 		wxMessageBox(_T("ZinjaI no pudo determinar el directorio donde fue instalado.\n"
			            "Compruebe que el directorio de trabajo actual sea el correcto.\n"
						"ZinjaI cannot determinate installation path. Please verify that\n"
						"working path is the right one."),_T("Error"));
	}
	
	// agregar colores personalizados a la base de datos

	// inicialize mxUtils and ConfigManager
	utils = new mxUtils;
	SHOW_MILLIS("Loading config...");
	config = new ConfigManager(zpath);

	SHOW_MILLIS("Initializing singleton manager...");
	// inicialize singleton manager
	singleton = new mxSingleton;
	if (config->Init.singleton) {
		singleton->Start();
		if (!singleton->IsRunning() && argc!=1) {
			bool done=true;
			for (int i=1; i<argc;i++) {
				wxString name = argv[i];
				if (name!=_T("--last-source") && name!=_T("--last-project") && name.AfterLast('.').Lower()!=PROJECT_EXT) {
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
			if (done) return false;
		}
	}

	// show splash screen
	SHOW_MILLIS("Loading image handlers...");
	splash = NULL;
	wxImage::AddHandler(new wxPNGHandler);
	wxImage::AddHandler(new wxXPMHandler);
	wxBitmap bitmap;
	bool no_splash=false;
	for (int i=1;i<argc;i++) 
		if (wxString(argv[i])=="--no-splash") {
			argv[i][0]='\0';
			no_splash=true;
		}
	SHOW_MILLIS("Loading splash...");
	if (!no_splash && config->Init.show_splash && wxFileName(DIR_PLUS_FILE(_T("imgs"),_T(SPLASH_FILE))).FileExists()) 
		splash = new mxSplashScreen(DIR_PLUS_FILE(_T("imgs"),_T(SPLASH_FILE)));
//	wxYield();
	
	SHOW_MILLIS("Initializing art stuff...");
	
	bitmaps = new mxArt(config->Files.skin_dir);

	// poner el idioma del compilador en castellano
	if (config->Init.lang_es) 
		wxSetEnv(_T("LANG"),_T("es_ES"));
	else
		wxSetEnv(_T("LANG"),_T("en_US"));
	
//	wxSetEnv(_T("LD_LIBRARY_PATH"),_T("/home/santiago/OpenFOAM/ThirdParty/ParaView3.3-cvs/platforms/linuxGcc/bin:/home/santiago/OpenFOAM/OpenFOAM-1.5/lib/linuxGccDPDebug/openmpi-1.2.6:/home/santiago/OpenFOAM/ThirdParty/openmpi-1.2.6/platforms/linuxGccDPDebug/lib:/home/santiago/OpenFOAM/ThirdParty/gcc-4.3.1/platforms/linux/lib:/home/santiago/OpenFOAM/santiago-1.5/lib/linuxGccDPDebug:/home/santiago/OpenFOAM/OpenFOAM-1.5/lib/linuxGccDPDebug"));
	
	SHOW_MILLIS("Creating helpers...");
	// inicialize HelpManager
	help = new HelpManager;
	
	// inicialize CodeHelper
	code_helper = new CodeHelper(config->Help.min_len_for_completion);

	// inicialize ProjectManager
	project = NULL;
	helpw = NULL;
	
	// create main window
	if (config->Init.size_x==0 || config->Init.size_y==0)
		main_window = new mxMainWindow(NULL, wxID_ANY, _T("ZinjaI "), wxDefaultPosition, wxSize(800, 600));
	else
		main_window = new mxMainWindow(NULL, wxID_ANY, _T("ZinjaI"), wxPoint(config->Init.pos_x,config->Init.pos_y), wxSize(config->Init.size_x,config->Init.size_y));

	// inicialize debug manager
	SHOW_MILLIS("Initializing debug manager...");
	debug = new DebugManager();
	debug->inspection_grid=main_window->inspection_ctrl;
	
	// set top window and let the magic do the rest
	SetTopWindow(main_window);
	
#if !defined(__WIN32__) && !defined(__APPLE__)
//#ifdef DEBUG
	if (config->first_run) new mxIconInstaller(true);
//#else
//	if (config->first_run && 
//		!wxFileName::FileExists(DIR_PLUS_FILE(DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("Desktop")),_T("ZinjaI.desktop"))) &&
//		!wxFileName::FileExists(DIR_PLUS_FILE(DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("Escritorio")),_T("ZinjaI.desktop"))) &&
//		mxMD_YES == mxMessageDialog(main_window,LANG(APP_CREATE_ICON_QUESTION,"Desea crear un acceso rapido para ZinjaI en su escritorio?"),LANG(APP_WELCOME_TO_ZINJAI,"Bienvenido a ZinjaI"), mxMD_YES_NO).ShowModal()) {
//			if (wxFileName::DirExists(DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("Desktop"))))
//				utils->MakeDesktopIcon(DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("Desktop")));
//			if (wxFileName::DirExists(DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("Escritorio"))))
//				utils->MakeDesktopIcon(DIR_PLUS_FILE(wxFileName::GetHomeDir(),_T("Escritorio")));
//		}
//#endif
#endif
	
	// show startup tip
	if ( (config->Init.show_tip_on_startup && !config->Init.show_welcome) || config->Init.version!=VERSION) {
		main_window->Refresh();
		tips_window = new mxTipsWindow(main_window, wxID_ANY);
		if (splash) splash->ShouldClose();
	}
	
	// load files or create and empty one
	if (argc==1) {
		SHOW_MILLIS("Creating first page...");
		if (config->Init.show_welcome) {
			main_window->ShowWelcome(true);
		} else {
			if (config->Init.new_file==0)
				main_window->NewFileFromText(_T(""));
			else
				main_window->NewFileFromTemplate(config->Files.default_template);
			main_window->Refresh();
		} 
		if (splash) splash->ShouldClose();
	} else {
		SHOW_MILLIS("Loading files...");
		main_window->Refresh();
		wxYield();
		if (splash) splash->ShouldClose();
		for (int i=1; i<argc;i++)
			if (wxString(argv[i])=="--last-source" && config->Files.last_source[0].Len())
				main_window->OpenFileFromGui(wxString(config->Files.last_source[0]));
			else if (wxString(argv[i])=="--last-project" && config->Files.last_project[0].Len())
				main_window->OpenFileFromGui(wxString(config->Files.last_project[0]));
//			else if (wxString(argv[i])==".")
//				main_window->ShowExplorerTreePanel();
			else if (argv[i][0]!='\0')
				main_window->OpenFileFromGui(DIR_PLUS_FILE(cmd_path,argv[i]));
		if (!project && main_window->notebook_sources->GetPageCount()==0 && config->Init.show_welcome) {
			main_window->ShowWelcome(true);
		}
	}
	
	
	if ( (project || main_window->notebook_sources->GetPageCount()>0) && config->Init.show_beginner_panel && !config->Init.autohide_panels) {
		if (config->Init.show_beginner_panel && !beginner_panel) {
			beginner_panel = new mxBeginnerPanel(main_window);
			main_window->aui_manager.InsertPane(beginner_panel, wxAuiPaneInfo().Name(_T("beginner_panel")).Caption(_T("Panel de Asistencias")).Right().Layer(0).Show(), wxAUI_INSERT_ROW);
			main_window->menu.view_beginner_panel->Check(true);
			main_window->aui_manager.Update();
		}
		
	}
	
	// si estaba abriendo un proyecto el usuario puede haber cerrado la ventana antes de que el parser termine
	if (!main_window) return false;
	
	if (singleton) singleton->ProcessToOpenQueue();
	
	if (tips_window) tips_window->Raise();
	
	SHOW_MILLIS("Checking recovery system...");
	if (!mxErrorRecovering::RecoverSomething()) {
		if (config->Init.check_for_updates) 
			mxUpdatesChecker::BackgroundCheck();
	}
	
	SHOW_MILLIS("All done!");
	
//	cerr<<wxThread::GetCPUCount()<<endl;
	
	return true;
}
