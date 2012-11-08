#include <wx/wx.h>

#include <wx/aui/auibook.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/dir.h>
#include <wx/treebook.h>
#include <wx/listbook.h>
#include <wx/imaglist.h>

#include "mxPreferenceWindow.h"

#include "ConfigManager.h"
#include "mxHelpWindow.h"
#include "mxUtils.h"
#include "mxMainWindow.h"
#include "mxSource.h"
#include "mxBitmapButton.h"
#include "ids.h"
#include "CodeHelper.h"
#include "mxSizers.h"
#include "mxEnumerationEditor.h"
#include "mxToolbarEditor.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
#include "mxWelcomePanel.h"
#include "Language.h"
#include "mxSingleton.h"
#include "DebugManager.h"
#include "Autocoder.h"
#include "mxColoursEditor.h"
#include "mxIconInstaller.h"
#include "Toolchain.h"
#include "mxInspectionsImprovingEditor.h"

mxPreferenceWindow *preference_window=NULL;

BEGIN_EVENT_TABLE(mxPreferenceWindow, wxDialog)
	EVT_BUTTON(wxID_OK,mxPreferenceWindow::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxPreferenceWindow::OnCancelButton)
	EVT_BUTTON(mxID_MAX_JOBS,mxPreferenceWindow::OnMaxJobsButton)
	EVT_BUTTON(mxID_COLORS_PICKER,mxPreferenceWindow::OnColoursButton)
	EVT_BUTTON(mxID_DEBUG_IMPROVE_INSPECTIONS_BY_TYPE,mxPreferenceWindow::OnImproveInspectionsByTypeButton)
	EVT_BUTTON(mxID_GDB_PATH,mxPreferenceWindow::OnGdbButton)
	EVT_BUTTON(mxID_IMG_BROWSER_PATH,mxPreferenceWindow::OnImgBrowserButton)
	EVT_BUTTON(mxID_BROWSER_PATH,mxPreferenceWindow::OnBrowserButton)
	EVT_BUTTON(mxID_VALGRIND_PATH,mxPreferenceWindow::OnValgrindButton)
	EVT_BUTTON(mxID_CPPCHECK_PATH,mxPreferenceWindow::OnCppCheckButton)
	EVT_BUTTON(mxID_WXFB_PATH,mxPreferenceWindow::OnWxfbButton)
	EVT_BUTTON(mxID_DOXYGEN_PATH,mxPreferenceWindow::OnDoxygenButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxPreferenceWindow::OnHelpButton)
	EVT_BUTTON(mxID_TEMP_FOLDER,mxPreferenceWindow::OnTempButton)
	EVT_BUTTON(mxID_SKIN_APPLY,mxPreferenceWindow::OnSkinButton)
	EVT_BUTTON(mxID_WXHELP_FOLDER,mxPreferenceWindow::OnWxHelpButton)
	EVT_BUTTON(mxID_PROJECTS_FOLDER,mxPreferenceWindow::OnProjectButton)
	EVT_BUTTON(mxID_AUTOCODES_FILE,mxPreferenceWindow::OnAutocodesButton)
	EVT_MENU(mxID_AUTOCODES_OPEN,mxPreferenceWindow::OnAutocodesOpen)
	EVT_MENU(mxID_AUTOCODES_EDIT,mxPreferenceWindow::OnAutocodesEdit)
	EVT_BUTTON(mxID_DEBUG_MACROS,mxPreferenceWindow::OnDebugMacrosButton)
	EVT_MENU(mxID_DEBUG_MACROS_OPEN,mxPreferenceWindow::OnDebugMacrosOpen)
	EVT_MENU(mxID_DEBUG_MACROS_EDIT,mxPreferenceWindow::OnDebugMacrosEdit)
	EVT_BUTTON(mxID_DEBUG_BLACKLIST,mxPreferenceWindow::OnDebugBlacklistButton)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_FILE,mxPreferenceWindow::OnToolbarsFile)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_EDIT,mxPreferenceWindow::OnToolbarsEdit)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_RUN,mxPreferenceWindow::OnToolbarsRun)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_MISC,mxPreferenceWindow::OnToolbarsMisc)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_TOOLS,mxPreferenceWindow::OnToolbarsTools)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_DEBUG,mxPreferenceWindow::OnToolbarsDebug)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_VIEW,mxPreferenceWindow::OnToolbarsView)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_RESET,mxPreferenceWindow::OnToolbarsReset)
#if defined(_WIN32) || defined(__WIN32__)
	EVT_BUTTON(mxID_MINGW_FOLDER,mxPreferenceWindow::OnMingwButton)
#else
	EVT_BUTTON(mxID_PREFERENCES_XDG,mxPreferenceWindow::OnXdgButton)
	EVT_BUTTON(mxID_TERMINALS_BUTTON,mxPreferenceWindow::OnTerminalButton)
	EVT_MENU(mxID_TERMINALS_LX,mxPreferenceWindow::OnTerminalLX)
	EVT_MENU(mxID_TERMINALS_XTERM,mxPreferenceWindow::OnTerminalXTerm)
	EVT_MENU(mxID_TERMINALS_GNOME,mxPreferenceWindow::OnTerminalGnomeTerminal)
	EVT_MENU(mxID_TERMINALS_KONSOLE,mxPreferenceWindow::OnTerminalKonsole)
	EVT_BUTTON(mxID_EXPLORERS_BUTTON,mxPreferenceWindow::OnExplorerButton)
	EVT_MENU(mxID_EXPLORERS_KONQUEROR,mxPreferenceWindow::OnExplorerKonqueror)
	EVT_MENU(mxID_EXPLORERS_DOLPHIN,mxPreferenceWindow::OnExplorerDolphin)
	EVT_MENU(mxID_EXPLORERS_THUNAR,mxPreferenceWindow::OnExplorerThunar)
	EVT_MENU(mxID_EXPLORERS_NAUTILUS,mxPreferenceWindow::OnExplorerNautilus)
#endif
	EVT_CLOSE(mxPreferenceWindow::OnClose)
	EVT_LISTBOX(mxID_SKIN_LIST,mxPreferenceWindow::OnSkinList)
END_EVENT_TABLE()

mxPreferenceWindow::mxPreferenceWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, LANG(PREFERENCES_CAPTION,"Preferencias"), pos, size, style) {

	toolbar_editor_file = NULL;
	toolbar_editor_edit = NULL;
	toolbar_editor_view = NULL;
	toolbar_editor_tools = NULL;
	toolbar_editor_debug = NULL;
	toolbar_editor_run = NULL;
	toolbar_editor_misc = NULL;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	notebook = new wxListbook(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxBK_LEFT);
	wxImageList* imglist = new wxImageList(32, 32,true,8);
	imglist->Add(wxBitmap(SKIN_FILE(_T("pref_general.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("pref_simple_program.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("pref_style.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("pref_writing.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("pref_skin.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("pref_toolbars.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("pref_debug.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("pref_paths_1.png")),wxBITMAP_TYPE_PNG));
	imglist->Add(wxBitmap(SKIN_FILE(_T("pref_paths_2.png")),wxBITMAP_TYPE_PNG));
	if (config->Help.show_extra_panels) imglist->Add(wxBitmap(SKIN_FILE(_T("pref_help.png")),wxBITMAP_TYPE_PNG));
	notebook->SetImageList(imglist);

	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	SetEscapeId(wxID_CANCEL);
	wxButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	ok_button->SetDefault(); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));

	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);

	if (config->Help.show_extra_panels) 
		notebook->AddPage(CreateQuickHelpPanel(notebook), LANG(PREFERENCES_QUICK_HELP,"Ayuda Rapida"),false,9);
	notebook->AddPage(CreateGeneralPanel(notebook), LANG(PREFERENCES_GENERAL,"General"),false,0);
	notebook->AddPage(CreateSimplePanel(notebook), LANG(PREFERENCES_SIMPLE_PROGRAM,"Programa/Proyecto"),false,1);
	notebook->AddPage(CreateStylePanel(notebook), LANG(PREFERENCES_STYLE,"Estilo"),false,2);
	notebook->AddPage(CreateWritingPanel(notebook), LANG(PREFERENCES_ASSISTANCES,"Asistencias"),false,3);
	notebook->AddPage(CreateSkinPanel(notebook), LANG(PREFERENCES_ICON_THEME,"Tema de Iconos"),false,4);
	notebook->AddPage(CreateToolbarsPanel(notebook), LANG(PREFERENCES_TOOLBARS,"Barras de Herram."),false,5);
	notebook->AddPage(CreateDebugPanel(notebook), LANG(PREFERENCES_DEBUGGING,"Depuracion"),false,6);
	notebook->AddPage(CreatePathsPanel(notebook), LANG(PREFERENCES_PATHS_1,"Rutas 1"),false,7);
	notebook->AddPage(CreateCommandsPanel(notebook), LANG(PREFERENCES_PATHS_2,"Rutas 2"),false,8);

	mySizer->Add(notebook,sizers->Exp1);
	mySizer->Add(bottomSizer,sizers->Exp0);

	SetSizerAndFit(mySizer);

	SetFocus();
	Show();
}

wxPanel *mxPreferenceWindow::CreateGeneralPanel (wxListbook *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );

	wxDir dir("lang");
	wxString spec="*.pre", filename;
	bool cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
	wxArrayString a_langs;
	while ( cont ) {
		a_langs.Add(filename.BeforeLast('.'));
		cont = dir.GetNext(&filename);
	}
	if (a_langs.Index(_T("spanish"))==wxNOT_FOUND) a_langs.Add(_T("spanish"));
	int i_langs = a_langs.Index(config->Init.language_file);
	if (i_langs==wxNOT_FOUND) { i_langs = a_langs.Index("spanish"); config->Init.language_file=_T("spanish"); }
	init_lang_file = utils->AddComboBox(sizer,panel,LANG(PREFERENCES_GENERAL_GUI_LANGUAGE,"Idioma de la interfaz (*)"),a_langs, i_langs);
	init_show_splash = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_SPLASH,"Mostrar Splash mientras se carga el entorno"),config->Init.show_splash);
	init_show_tip_on_startup = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_TOOLTIPS,"Mostrar sugerencias al inicio"),config->Init.show_tip_on_startup);
	init_show_welcome = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_WELCOME_PANEL,"Mostrar panel de bienvenida"),config->Init.show_welcome);
	init_show_extra_panels = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_HELP_TABS,"Mostrar pestanas de ayuda en dialogos (*)"),config->Help.show_extra_panels);
	init_history_len = utils->AddTextCtrl(sizer,panel,LANG(PREFERENCES_GENERAL_HISTORY_LEN,"Archivos en el historial"),config->Init.history_len);
	init_show_explorer_tree = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_FILES_EXPLORER_ON_STARTUP,"Mostrar el explorador de archivos al iniciar"),config->Init.show_explorer_tree);
	init_left_panels = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_GROUP_TREES,"Agrupar arboles en un solo panel (*)"),config->Init.left_panels);
	init_autohide_panels = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_AUTOHIDE_PANELS,"Ocultar paneles automaticamente (experimental - *)"),config->Init.autohide_panels);
	init_singleton = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_SINGLETON,"Utilizar una sola instancia de ZinjaI al abrir archivos desde la linea de comandos"),config->Init.singleton);
	init_check_for_updates = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_CHECK_FOR_UPDATES,"Verificar si existen nuevas versiones al iniciar"),config->Init.check_for_updates);
#if defined(_WIN32) || defined(__WIN32__)
#else
	init_lang_es = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SPANISH_COMPILER_OUTPUT,"Mostrar errores de compilacion en Espanol (Ver Ayuda!) (*)"),config->Init.lang_es);
//	desktop_icon = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_CREATE_ZINJAI_DESKTOP_ICON,"Crear/Actualizar icono de ZinjaI en el escritorio"),false);
	wxButton *xdg_button = new wxButton(panel,mxID_PREFERENCES_XDG,LANG(PREFERENCES_CREATE_ICONS,"Crear/Actualizar accesos directos en el escritorio/menu del sistema"));
	sizer->Add(xdg_button,sizers->BA10);
#endif
	utils->AddStaticText(sizer,panel,LANG(PREFERENCES_GENERAL_ASTERIX_WILL_APPLY_NEXT_TIME,"(*) tendra efecto la proxima vez que inicie ZinjaI"));
	panel->SetSizerAndFit(sizer);
	return panel;

}

wxPanel *mxPreferenceWindow::CreateQuickHelpPanel(wxListbook *notebook) {
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	wxHtmlWindow *html = new wxHtmlWindow(panel,wxID_ANY);
	sizer->Add(html,sizers->Exp1);
	html->LoadFile(DIR_PLUS_FILE(config->Help.guihelp_dir,wxString(_T("prefs_help_"))<<config->Init.language_file<<_T(".html")));
	panel->SetSizerAndFit(sizer);
	return panel;	
}

wxPanel *mxPreferenceWindow::CreateDebugPanel (wxListbook *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
		
	debug_allow_edition = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_ALLOW_EDITION,"Permitir editar los fuentes durante la depuracion"),config->Debug.allow_edition);
	debug_autohide_panels = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_REORGANIZE_PANELS,"Reacomodar los paneles al iniciar/finalizar la depuracion"),config->Debug.autohide_panels);
	debug_inspections_on_right = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_INSPECTIONS_PANEL_ON_RIGHT,"Colocar el panel de inspecciones a la derecha (*)"),config->Debug.inspections_on_right);
	debug_show_thread_panel = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_SHOW_THREAD_LIST,"Mostrar lista de hilos de ejecucion"),config->Debug.show_thread_panel);
	debug_show_log_panel = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_SHOW_DEBUGGER_LOG,"Mostrar el panel de mensajes del depurador"),config->Debug.show_log_panel);
	debug_autohide_toolbars = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_ORGANIZE_TOOLBARS,"Reacomodar las barras de herramienta al iniciar/finalizar la depuracion"),config->Debug.autohide_toolbars);
//	debug_autoupdate_backtrace = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_AUTOUPDATE_BACKTRACE,"Actualizar automaticamente el panel de trazado inverso en cada paso"),config->Debug.autoupdate_backtrace);
	debug_readnow = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_LOAD_ALL_DEBUG_INFO,"Cargar toda la informacion de depuracion antes de comenzar"),config->Debug.readnow);
	debug_auto_solibs = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_LOAD_SHARED_LIBS_INFO,"Cargar informacion de depuracion de librerias externas"),config->Debug.auto_solibs);
	debug_compile_again = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_RECOMPILE_AUTOMATICALLY,"Recompilar automaticamente antes de depurar si es necesario"),config->Debug.compile_again);
	debug_close_on_normal_exit= utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_STOP_DEBBUGING_ON_ABNORMAL_TERMINATION,"Salir del modo depuracion si el programa finaliza normalmente"),config->Debug.close_on_normal_exit);
	debug_always_debug = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_ALWAYS_RUN_IN_DEBUGGER,"Siempre ejecutar en el depurador"),config->Debug.always_debug);
	debug_raise_main_window = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_ACTIVATE_WINDOW_ON_INTERRUPTION,"Mostrar ZinjaI cuando se interrumpe la ejecucion"),config->Debug.raise_main_window);
	debug_select_modified_inspections = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_SELECT_UPDATES_IN_INSPECTIONS,"Utilizar colores en la tabla de inspecciones"),config->Debug.select_modified_inspections);
	
	wxBoxSizer *type_replace_sizer = new wxBoxSizer(wxHORIZONTAL);
	improve_inspections_by_type = new wxCheckBox(panel, wxID_ANY, wxString(LANG(PREFERENCES_DEBUG_IMPROVE_INSPECTIONS_BY_TYPE,"Mejorar inspecciones automáticamente segun tipo"))+_T("   "));
	improve_inspections_by_type->SetValue(config->Debug.improve_inspections_by_type);
	type_replace_sizer->Add(improve_inspections_by_type,sizers->Center);
	type_replace_sizer->Add(new wxButton(panel,mxID_DEBUG_IMPROVE_INSPECTIONS_BY_TYPE,"Configurar..."),sizers->Center);
	sizer->Add(type_replace_sizer,sizers->BA5_Exp0);
	
	debug_macros_file = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de definiciones de macros para gdb"),config->Debug.macros_file,mxID_DEBUG_MACROS);
	debug_blacklist = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_DEBUG_SOURCES_BLACKLIST,"Fuentes a evitar (para el step in)"),config->Debug.blacklist,mxID_DEBUG_BLACKLIST);
	panel->SetSizerAndFit(sizer);
	return panel;

}

wxPanel *mxPreferenceWindow::CreateToolbarsPanel (wxListbook *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	sizer->Add(new wxStaticText(panel, wxID_ANY, LANG(PREFERENCES_TOOLBARS_WICH,"Barra de herramientas a utilizar:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
	
	wxBoxSizer *szFile= new wxBoxSizer(wxHORIZONTAL);
	toolbars_wich_file = new wxCheckBox(panel,wxID_ANY,LANG(PREFERENCES_TOOLBARS_FILE,"Archivo"));
	toolbars_wich_file->SetValue(config->Toolbars.wich_ones.file);
	wxButton *btFile = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_FILE,LANG(PREFERENCES_TOOLBARS_MODIFY,"Modificar..."));
	szFile->Add(toolbars_wich_file,sizers->BLR10); szFile->Add(btFile,sizers->BLR10);
	szFile->Add(GetSize().GetWidth()/2,1,0);
	sizer->Add(szFile,sizers->BA5);
	
	wxBoxSizer *szEdit  = new wxBoxSizer(wxHORIZONTAL);
	toolbars_wich_edit = new wxCheckBox(panel,wxID_ANY,LANG(PREFERENCES_TOOLBARS_EDIT,"Editar"));
	toolbars_wich_edit->SetValue(config->Toolbars.wich_ones.edit);
	wxButton *btEdit = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_EDIT,LANG(PREFERENCES_TOOLBARS_MODIFY,"Modificar..."));
	szEdit->Add(toolbars_wich_edit,sizers->BLR10); szEdit->Add(btEdit,sizers->BLR10);
	sizer->Add(szEdit,sizers->BA5);
	
	wxBoxSizer *szView  = new wxBoxSizer(wxHORIZONTAL);
	toolbars_wich_view = new wxCheckBox(panel,wxID_ANY,LANG(PREFERENCES_TOOLBARS_VIEW,"Ver"));
	toolbars_wich_view->SetValue(config->Toolbars.wich_ones.view);
	wxButton *btView = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_VIEW,LANG(PREFERENCES_TOOLBARS_MODIFY,"Modificar..."));
	szView->Add(toolbars_wich_view,sizers->BLR10); szView->Add(btView,sizers->BLR10);
	sizer->Add(szView,sizers->BA5);
	
	wxBoxSizer *szRun= new wxBoxSizer(wxHORIZONTAL);
	toolbars_wich_run = new wxCheckBox(panel,wxID_ANY,LANG(PREFERENCES_TOOLBARS_RUN,"Ejecucion"));
	toolbars_wich_run->SetValue(config->Toolbars.wich_ones.run);
	wxButton *btRun = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_RUN,LANG(PREFERENCES_TOOLBARS_MODIFY,"Modificar..."));
	szRun->Add(toolbars_wich_run,sizers->BLR10); szRun->Add(btRun,sizers->BLR10);
	sizer->Add(szRun,sizers->BA5);
	
	wxBoxSizer *szDebug= new wxBoxSizer(wxHORIZONTAL);
	toolbars_wich_debug = new wxCheckBox(panel,wxID_ANY,LANG(PREFERENCES_TOOLBARS_DEBUG,"Depuracion"));
	toolbars_wich_debug->SetValue(config->Toolbars.wich_ones.debug);
	wxButton *btDebug = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_DEBUG,LANG(PREFERENCES_TOOLBARS_MODIFY,"Modificar..."));
	szDebug->Add(toolbars_wich_debug,sizers->BLR10); szDebug->Add(btDebug,sizers->BLR10);
	sizer->Add(szDebug,sizers->BA5);
	
	wxBoxSizer *szTools= new wxBoxSizer(wxHORIZONTAL);
	toolbars_wich_tools = new wxCheckBox(panel,wxID_ANY,LANG(PREFERENCES_TOOLBARS_TOOLS,"Herramientas"));
	toolbars_wich_tools->SetValue(config->Toolbars.wich_ones.tools);
	wxButton *btTools = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_TOOLS,LANG(PREFERENCES_TOOLBARS_MODIFY,"Modificar..."));
	szTools->Add(toolbars_wich_tools,sizers->BLR10); szTools->Add(btTools,sizers->BLR10);
	szTools->Add(GetSize().GetWidth()/2,1,0);
	sizer->Add(szTools,sizers->BA5);
	
	wxBoxSizer *szMisc = new wxBoxSizer(wxHORIZONTAL);
	toolbars_wich_misc = new wxCheckBox(panel,wxID_ANY,LANG(PREFERENCES_TOOLBARS_MISC,"Miscelanea"));
	toolbars_wich_misc->SetValue(config->Toolbars.wich_ones.misc);
	wxButton *btMisc = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_MISC,LANG(PREFERENCES_TOOLBARS_MODIFY,"Modificar..."));
	szMisc->Add(toolbars_wich_misc,sizers->BLR10); szMisc->Add(btMisc,sizers->BLR10);
	szMisc->Add(GetSize().GetWidth()/2,1,0);
	sizer->Add(szMisc,sizers->BA5);

	toolbars_wich_find = new wxCheckBox(panel,wxID_ANY,LANG(PREFERENCES_TOOLBARS_FIND,"Busqueda"));
	toolbars_wich_find->SetValue(config->Toolbars.wich_ones.find);
	wxBoxSizer *szFind = new wxBoxSizer(wxHORIZONTAL);
	szFind->Add(toolbars_wich_find,sizers->BLR10);
	sizer->Add(szFind,sizers->BA5);
	
	wxButton *btReset = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_RESET,LANG(PREFERENCES_TOOLBARS_RESET,"Reestablecer configuracion por defecto"));
	sizer->Add(btReset,sizers->BA5);
	
	wxArrayString icon_sizes;
	icon_sizes.Add(_T("16x16"));
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,_T("24")))) icon_sizes.Add(_T("24x24"));
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,_T("32")))) icon_sizes.Add(_T("32x32"));
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,_T("48")))) icon_sizes.Add(_T("48x48"));
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,_T("64")))) icon_sizes.Add(_T("64x64"));
	wxString icsz = wxString()<<config->Toolbars.icon_size<<_T("x")<<config->Toolbars.icon_size;
	unsigned int idx_icsz = icon_sizes.Index(icsz);
	if (idx_icsz>=icon_sizes.GetCount()) idx_icsz=0;
	toolbar_icon_size = utils->AddComboBox(sizer,panel,LANG(PREFERENCES_TOOLBAR_ICON_SIZE,"Tamaño de icono"),icon_sizes,idx_icsz);
	
	panel->SetSizerAndFit(sizer);
	return (panel_toolbars=panel);

}

wxPanel *mxPreferenceWindow::CreateSimplePanel (wxListbook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxArrayString a_new_file;
	a_new_file.Add(LANG(PREFERENCES_SIMPLE_CREATE_EMPTY_FILE,"Crear archivo en blanco"));
	a_new_file.Add(LANG(PREFERENCES_SIMPLE_CREATE_FROM_TEMPLATE,"Crear a partir de plantilla"));
	a_new_file.Add(LANG(PREFERENCES_SIMPLE_SHOW_WIZARD,"Mostrar Asistente"));
	init_new_file = utils->AddComboBox(sizer,panel,LANG(PREFERENCES_SIMPLE_NEW_ACTION,"Accion para Nuevo Archivo"),a_new_file, config->Init.new_file);
//	files_templates_dir = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_SIMPLE_TEMPLATES_FOLDER,"Carpeta de plantillas (necesita reiniciar)"),config->Files.templates_dir,mxID_TEMPLATES_FOLDER);
	running_compiler_options = utils->AddTextCtrl(sizer,panel,LANG(PREFERENCES_SIMPLE_EXTRA_COMPILER_ARGUMENTS,"Parametros extra para el compilador"),config->Running.compiler_options);
	running_wait_for_key = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_SIMPLE_WAIT_KEY_AFTER_RUNNING,"Esperar una tecla luego de la ejecucion"),config->Running.wait_for_key);
	running_always_ask_args = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_SIMPLE_ALWAYS_ASK_ARGS,"Siempre pedir argumentos al ejecutar"),config->Running.always_ask_args);
	init_always_add_extension = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_SIMPLE_ADD_CPP_EXTENSION,"Agregar la extension cpp si se omite al guardar"),config->Init.always_add_extension);
	running_dont_run_headers = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_SIMPLE_RUN_LAST_CPP_INSTEAD_HEADER,"Ejecutar el ultimo cpp si se intenta ejecutar una cabecera"),config->Running.dont_run_headers);
	
	init_max_jobs = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_GENERAL_MAX_JOBS,"Cantidad de pasos en paralelo al compilar"),wxString()<<config->Init.max_jobs,mxID_MAX_JOBS);
	init_stop_compiling_on_error = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STOP_COMPILING_ON_ERROR,"Detener la compilación de un proyecto al encontrar el primer error"),config->Init.stop_compiling_on_error);
	init_save_project = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SAVE_PROJECT_ON_CLOSE,"Guardar siempre el proyeto al salir"),config->Init.save_project);
	init_close_files_for_project = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_CLOSE_FILES_ON_PROJECT_OPENING,"Cerrar todos los demas archivos al abrir un proyecto"),config->Init.close_files_for_project);
	init_prefer_explorer_tree = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_FILES_EXPLORER_ON_PROJECT,"Mostrar el explorador de archivos al abrir un proyecto"),config->Init.prefer_explorer_tree);
	
	panel->SetSizerAndFit(sizer);
	return panel;

}


wxPanel *mxPreferenceWindow::CreateStylePanel (wxListbook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxBoxSizer *colours_sizer = new wxBoxSizer(wxHORIZONTAL);
	source_syntaxEnable = new wxCheckBox(panel, wxID_ANY, wxString(LANG(PREFERENCES_STYLE_SINTAX_HIGHLIGHT,"Colorear sintaxis"))+_T("   "));
	colours_sizer->Add(source_syntaxEnable,sizers->Center);
	colours_sizer->Add(new wxButton(panel,mxID_COLORS_PICKER,"Definir colores..."),sizers->Center);
	sizer->Add(colours_sizer,sizers->BA5_Exp0);
	source_syntaxEnable->SetValue(config->Source.syntaxEnable);
	
//	source_syntaxEnable = utils->AddCheckBox(sizer,panel,,config->Source.syntaxEnable);
	
//	styles_dark = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_INVERTED_COLOURS,"Colores inversos (fondo negro)"),config->Styles.dark);
//	source_wrapMode = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_LINE_WRAP,"Ajuste de linea dinamico"),config->Source.wrapMode);
	wxArrayString a_wrap;
	a_wrap.Add(LANG(PREFERENCES_STYLE_WRAP_NONE,"Nunca"));
	a_wrap.Add(LANG(PREFERENCES_STYLE_WRAP_ALL_BUT_SOURCES,"Todos menos fuentes"));
	a_wrap.Add(LANG(PREFERENCES_STYLE_WRAP_ALWAYS,"Siempre"));
	init_wrap_mode = utils->AddComboBox(sizer,panel,LANG(PREFERENCES_STYLE_LINE_WRAP,"Ajuste de linea dinamico"),a_wrap, config->Init.wrap_mode);
	source_whiteSpace = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_SHOW_EOF_AND_WHITE_SPACES,"Mostrar espacios y caracteres de fin de linea"),config->Source.whiteSpace);
	source_lineNumber = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_SHOW_LINE_NUMBERS,"Mostrar numeros de linea"),config->Source.lineNumber);
	source_foldEnable = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_ENABLE_CODE_FOLDING,"Habilitar plegado de codigo"),config->Source.foldEnable);
	source_tabWidth = utils->AddTextCtrl(sizer,panel,LANG(PREFERENCES_STYLE_TAB_WIDTH,"Ancho del tabulado"),config->Source.tabWidth);
	source_tabUseSpaces = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_SPACES_INTEAD_TABS,"Colocar espacios en lugar de tabs"),config->Source.tabUseSpaces);
	styles_font_size = utils->AddTextCtrl(sizer,panel,LANG(PREFERENCES_STYLE_SCREEN_FONT_SIZE,"Tamaño de la fuente en pantalla"),config->Styles.font_size);
	styles_print_size = utils->AddTextCtrl(sizer,panel,LANG(PREFERENCES_STYLE_PRINTING_FONT_SIZE,"Tamaño de la fuente para impresion"),config->Styles.print_size);
	init_autohide_menus_fs = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_HIDE_MENUS_ON_FULLSCREEN,"Ocultar barra de menues al pasar a pantalla completa"),config->Init.autohide_menus_fs);
	init_autohide_toolbars_fs = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_HIDE_TOOLBARS_ON_FULLSCREEN,"Ocultar barras de herramientas al pasar a pantalla completa"),config->Init.autohide_toolbars_fs);
	init_autohide_panels_fs = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_HIDE_PANELS_ON_FULLSCREEN,"Ocultar paneles al pasar a pantalla completa"),config->Init.autohide_panels_fs);
	
	panel->SetSizerAndFit(sizer);
	return panel;

}

wxPanel *mxPreferenceWindow::CreateWritingPanel (wxListbook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	source_smartIndent = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_INTELLIGENT_EDITING,"Edicion inteligente"),config->Source.smartIndent);
	source_bracketInsertion = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_AUTOCLOSE_BRACKETS,"Cerrar llaves automaticamente al presionar Enter"),config->Source.bracketInsertion);
	source_autocloseStuff = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_AUTOCLOSE_STUFF,"Siempre cerrar llaves, parentesis y comillas al ingresar"),config->Source.autocloseStuff);
	source_indentPaste = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_INDENT_ON_PASTE,"Corregir indentado al pegar"),config->Source.indentPaste);
	source_avoidNoNewLineWarning = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_CHECK_FOR_EMPTY_LAST_LINE,"Controlar que quede una linea en blanco al final de cada archivo"),config->Source.avoidNoNewLineWarning);
	source_toolTips = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_SHOW_TOOLTIPS_FOR_VAR_TYPES,"Utilizar tooltips para identificar tipos de variables"),config->Source.toolTips);
	source_autoCompletion = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_ENABLE_AUTOCOMPLETION,"Utilizar autocompletado mientras escribe"),config->Source.autoCompletion);
	source_callTips = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_ENABLE_CALLTIPS,"Mostrar ayuda de llamadas a funciones y metodos"),config->Source.callTips);
	init_beautifyCompilerErrors  = utils->AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_BEAUTIFY_COMPILER_ERRORS,"Simplificar mensajes de error del compilador"),config->Init.beautify_compiler_errors);
	files_autocode = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_WRITTING_AUTOCODES_FILE,"Archivo de definiciones de plantillas de auto-codigo"),config->Files.autocodes_file,mxID_AUTOCODES_FILE);
	
	sizer->Add(new wxStaticText(panel, wxID_ANY, LANG(PREFERENCES_WRITING_AUTOCOMPLETION_INDEXES,"Indices de autocompletado a utilizar"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
	help_autocomp_indexes = new wxCheckListBox(panel,mxID_AUTOCOMP_LIST,wxDefaultPosition,wxSize(100,100),0,NULL,wxLB_SORT);
	
	wxArrayString autocomp_array_all, autocomp_array_user;
	utils->GetFilesFromDir(autocomp_array_all,config->Help.autocomp_dir);
	utils->GetFilesFromDir(autocomp_array_all,DIR_PLUS_FILE(config->home_dir,"autocomp"));
	utils->Split(config->Help.autocomp_indexes,autocomp_array_user);
	utils->Unique(autocomp_array_all,true);
	utils->Unique(autocomp_array_user,true);
	
	for (unsigned int i=0;i<autocomp_array_all.GetCount();i++) {
		int n=help_autocomp_indexes->Append(autocomp_array_all[i]);
		if (autocomp_array_user.Index(autocomp_array_all[i])!=wxNOT_FOUND) 
			help_autocomp_indexes->Check(n,true);
	}
	sizer->Add(help_autocomp_indexes,sizers->BA5_DL_Exp1);

	panel->SetSizerAndFit(sizer);
	return panel;
	
}


wxPanel *mxPreferenceWindow::CreateSkinPanel (wxListbook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	sizer->Add(new wxStaticText(panel, wxID_ANY, LANG(PREFERENCES_SKIN_AVAILABLE_THEMES,"Temas disponibles"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
	skin_list = new wxListBox(panel,mxID_SKIN_LIST,wxDefaultPosition,wxSize(100,100),0,NULL,0);
	wxDir dir(_T("skins"));
	if ( dir.IsOpened() ) {
		wxArrayString autocomp_array;
		utils->Split(config->Help.autocomp_indexes,autocomp_array);
		wxString filename;
		wxString spec;
		bool cont = dir.GetFirst(&filename, spec , wxDIR_DIRS);
		skin_list->Append(LANG(PREFERENCES_SKIN_DEFAULT_THEME,"<Tema por defecto>"));
		skin_paths.Add(_T("imgs"));
		skin_list->Select(0);
		while ( cont ) {
			if (wxFileName::FileExists(DIR_PLUS_FILE(DIR_PLUS_FILE(_T("skins"),filename),_T("descripcion.txt")))) {
				skin_list->Append(filename);
				filename = DIR_PLUS_FILE(_T("skins"),filename);
				skin_paths.Add(filename);
				if (filename==config->Files.skin_dir)
					skin_list->SetSelection(skin_list->GetCount()-1);
			}
			cont = dir.GetNext(&filename);
		}	
	}
	
	sizer->Add(skin_list,sizers->BA5_DL_Exp1);

	skin_text = new wxTextCtrl(panel,wxID_ANY,_T(""),wxDefaultPosition,wxDefaultSize,wxTE_READONLY|wxTE_MULTILINE);
	sizer->Add(skin_text,sizers->BA10_Exp0);
	if (wxFileName::FileExists(SKIN_FILE_OPT(_T("skin_preview.png"))))
		skin_image = new wxStaticBitmap(panel,wxID_ANY,wxBitmap(SKIN_FILE_OPT(_T("skin_preview.png")),wxBITMAP_TYPE_PNG));
	else
		skin_image = new wxStaticBitmap(panel,wxID_ANY,wxBitmap(SKIN_FILE(_T("skin_no_preview.png")),wxBITMAP_TYPE_PNG));
	sizer->Add(skin_image,sizers->BA10_Exp0);
	wxButton *apply_button = new mxBitmapButton(panel,mxID_SKIN_APPLY,bitmaps->buttons.ok,LANG(PREFERENCES_SKIN_APPLY,"Aplicar"));
	sizer->Add(apply_button,sizers->BA10_Exp0_Right);
	
	wxCommandEvent ce;
	OnSkinList(ce);
	
	panel->SetSizerAndFit(sizer);
	return panel;
	
}


wxPanel *mxPreferenceWindow::CreatePathsPanel (wxListbook *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );

//	help_quickhelp_dir = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_QUICKHELP,"Ubicacion de la ayuda rapida"),config->Help.quickhelp_dir,mxID_QUICKHELP_FOLDER);
//	help_autocomp_dir = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_AUTOCOMP,"Ubicacion de los indices de autocompletado"),config->Help.autocomp_dir,mxID_AUTOCOMP_FOLDER);
	files_temp_dir = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_TEMP,"Directorio temporal"),config->Files.temp_dir, mxID_TEMP_FOLDER);
#if defined(_WIN32) || defined(__WIN32__)
	files_mingw_dir = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_MINGW,"Directorio de MinGW (requiere reiniciar ZinjaI)"),config->Files.mingw_dir, mxID_MINGW_FOLDER);
#endif
	files_project_folder = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_PROJECTS,"Directorio de proyectos"),config->Files.project_folder, mxID_PROJECTS_FOLDER);
	
	help_wxhelp_index = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_WXWIDGETS_HELP_INDEX,"Indice de ayuda wxWidgets"),config->Help.wxhelp_index, mxID_WXHELP_FOLDER);
	
//	files_compiler_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_GPP,"Comando del compilador C++"),config->Files.compiler_command,mxID_GPP_PATH);
//	files_compiler_c_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_GCC,"Comando del compilador C"),config->Files.compiler_c_command,mxID_GCC_PATH);
	files_debugger_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_GDB,"Comando del depurador"),config->Files.debugger_command,mxID_GDB_PATH);
	files_terminal_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_CONSOLE,"Comando de la terminal"),config->Files.terminal_command,mxID_TERMINALS_BUTTON);
	
	wxArrayString tc_array; Toolchain::GetNames(tc_array,true); int tc_i=tc_array.Index(config->Files.toolchain);
	files_toolchain = utils->AddComboBox(sizer,panel,LANG(PREFERENCES_TOOLCHAIN,"Herramientas de compilación"),tc_array,tc_i);
	
	panel->SetSizerAndFit(sizer);
	return panel;

}

wxPanel *mxPreferenceWindow::CreateCommandsPanel (wxListbook *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );

	files_explorer_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_EXPLORER,"Comando del explorador de archivos"),config->Files.explorer_command,mxID_EXPLORERS_BUTTON);
	files_img_browser_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_IMAGE_VIEWER,"Comando del visor de imagenes"),config->Files.img_browser,mxID_IMG_BROWSER_PATH);
	files_browser_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_BROWSER,"Comando del navegador Web"),config->Files.browser_command,mxID_BROWSER_PATH);
	files_doxygen_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_DOXYGEN,"Ubicacion del ejecutable de Doxygen"),config->Files.doxygen_command,mxID_DOXYGEN_PATH);
	files_wxfb_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_WXFORMBUILDER,"Ubicacion del ejecutable de wxFormBuilder"),config->Files.wxfb_command,mxID_WXFB_PATH);
#ifdef __WIN32__
	files_valgrind_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_VALGRIND,"Ubicacion del ejecutable de Valgrind"),config->Files.valgrind_command,mxID_VALGRIND_PATH);
#endif
	files_cppcheck_command = utils->AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_CPPCHECK,"Ubicacion del ejecutable de CppCheck"),config->Files.cppcheck_command,mxID_CPPCHECK_PATH);
	
	panel->SetSizerAndFit(sizer);
	return panel;

}


void mxPreferenceWindow::OnOkButton(wxCommandEvent &event){
	
	long int l;
	config->Init.left_panels = init_left_panels->GetValue();
	config->Init.check_for_updates = init_check_for_updates->GetValue();
	config->Help.show_extra_panels = init_show_extra_panels->GetValue();
	config->Init.language_file = init_lang_file->GetValue();
	config->Init.show_welcome= init_show_welcome->GetValue();
	config->Init.show_tip_on_startup = init_show_tip_on_startup->GetValue();
	if (config->Init.show_welcome && !welcome_panel) {
		welcome_panel=new mxWelcomePanel(main_window);
		main_window->aui_manager.AddPane(welcome_panel, wxAuiPaneInfo().Name(_T("welcome_panel")).CenterPane().PaneBorder(false).Hide());
		if (!project && !main_window->notebook_sources->GetPageCount()) main_window->ShowWelcome(true);
	} else if (!config->Init.show_welcome && welcome_panel) {
		main_window->ShowWelcome(false);
		main_window->aui_manager.DetachPane(welcome_panel);
		welcome_panel->Destroy();
		welcome_panel=NULL;
	}
	config->Init.show_splash = init_show_splash->GetValue();
	config->Init.always_add_extension = init_always_add_extension->GetValue();
#if defined(_WIN32) || defined(__WIN32__)
#else
	config->Init.lang_es = init_lang_es->GetValue();
#endif
	wxSetEnv(_T("LANG"),(config->Init.lang_es?_T("es_ES"):_T("en_US")));
	config->Init.save_project = init_save_project->GetValue();
	init_history_len->GetValue().ToLong(&l); config->Init.history_len=l;
	if (config->Init.history_len>30) config->Init.history_len=30;
	else if (config->Init.history_len<5) config->Init.history_len=5;
	init_max_jobs->GetValue().ToLong(&l); config->Init.max_jobs=l;
	if (config->Init.max_jobs<1) config->Init.max_jobs=1;
	config->Init.autohide_panels_fs = init_autohide_panels_fs->GetValue();
	config->Init.autohide_toolbars_fs = init_autohide_toolbars_fs->GetValue();
	config->Init.close_files_for_project = init_close_files_for_project->GetValue();
	config->Init.prefer_explorer_tree = init_prefer_explorer_tree->GetValue();
	config->Init.show_explorer_tree = init_show_explorer_tree->GetValue();
	config->Init.wrap_mode = init_wrap_mode->GetSelection();
	config->Init.new_file = init_new_file->GetSelection();
	config->Running.wait_for_key = running_wait_for_key->GetValue();
	config->Running.always_ask_args = running_always_ask_args->GetValue();
	config->Running.compiler_options = running_compiler_options->GetValue();
	config->Running.dont_run_headers = running_dont_run_headers->GetValue();
	config->Init.singleton = init_singleton->GetValue();
	if (!config->Init.singleton && singleton->IsRunning()) singleton->Stop();
	else if (config->Init.singleton && !project && !singleton->IsRunning()) singleton->Start();
	config->Init.autohide_panels = init_autohide_panels->GetValue();
	config->Init.stop_compiling_on_error = init_stop_compiling_on_error->GetValue();
	config->Files.temp_dir = files_temp_dir->GetValue();
	config->Files.debugger_command = files_debugger_command->GetValue();
	config->Files.toolchain = files_toolchain->GetValue();
	config->Files.terminal_command = files_terminal_command->GetValue();
	config->Files.explorer_command = files_explorer_command->GetValue();
	config->Files.img_browser = files_img_browser_command->GetValue();
	config->Files.browser_command = files_browser_command->GetValue();
#ifdef __WIN32__
	config->Files.valgrind_command = files_valgrind_command->GetValue();
#endif
	config->Files.cppcheck_command = files_cppcheck_command->GetValue();
	config->Files.wxfb_command = files_wxfb_command->GetValue();
	config->Files.doxygen_command = files_doxygen_command->GetValue();
	config->Files.project_folder = files_project_folder->GetValue();
	config->Source.syntaxEnable = source_syntaxEnable->GetValue();
	source_tabWidth->GetValue().ToLong(&l); config->Source.tabWidth=l;
	config->Source.tabUseSpaces = source_tabUseSpaces->GetValue();
	config->Source.foldEnable = source_foldEnable->GetValue();
	config->Source.bracketInsertion = (source_smartIndent->GetValue() && source_bracketInsertion->GetValue());
	config->Source.smartIndent = (source_smartIndent->GetValue() && source_syntaxEnable->GetValue());
	config->Source.indentPaste = (source_smartIndent->GetValue() && source_indentPaste->GetValue());
	config->Source.avoidNoNewLineWarning = source_avoidNoNewLineWarning->GetValue();
	config->Source.whiteSpace = source_whiteSpace->GetValue();
	config->Source.whiteSpace = source_whiteSpace->GetValue();
	config->Source.lineNumber = source_lineNumber->GetValue();
	config->Source.toolTips = source_toolTips->GetValue();
	config->Init.beautify_compiler_errors = init_beautifyCompilerErrors->GetValue();
	config->Source.callTips = source_callTips->GetValue();
	config->Source.autoCompletion = source_autoCompletion->GetValue();
	config->Source.autocloseStuff = source_autocloseStuff->GetValue();
	config->Help.wxhelp_index = help_wxhelp_index->GetValue();
	
	wxString autocomp_indexes;
	for (unsigned int i=0;i<help_autocomp_indexes->GetCount();i++) {
		if (help_autocomp_indexes->IsChecked(i)) {
			wxString index_string=help_autocomp_indexes->GetString(i);
			if (index_string.Contains(_T(" ")))
				index_string=wxString(_T("\""))<<index_string<<_T("\"");
			if (autocomp_indexes.Len())
				autocomp_indexes<<_T(",")<<index_string;
			else
				autocomp_indexes = help_autocomp_indexes->GetString(i);
		}
	}
	if (config->Help.autocomp_indexes != autocomp_indexes) {
		config->Help.autocomp_indexes = autocomp_indexes;
		if (project) autocomp_indexes<<_T(" ")<<project->autocomp_extra;
		code_helper->ReloadIndexes(autocomp_indexes);
	}
	
	styles_print_size->GetValue().ToLong(&l); config->Styles.print_size=l;
	styles_font_size->GetValue().ToLong(&l); config->Styles.font_size=l;
#if defined(_WIN32) || defined(__WIN32__)
	config->Files.mingw_dir = files_mingw_dir->GetValue();
#endif
	wxAuiNotebook *ns=main_window->notebook_sources;
	for (unsigned int i=0;i<ns->GetPageCount();i++)
		((mxSource*)(ns->GetPage(i)))->LoadSourceConfig();
	main_window->SetStatusBarFields(); // ocultar/mostrar nro de linea
		
	config->Debug.auto_solibs = debug_auto_solibs->GetValue();
	config->Debug.readnow = debug_readnow->GetValue();
	config->Debug.allow_edition = debug_allow_edition->GetValue();
	if (debug->debugging) {
		for (unsigned int i=0;i<ns->GetPageCount();i++)
			((mxSource*)(ns->GetPage(i)))->SetDebugTime(!config->Debug.allow_edition);
	}
	config->Debug.autohide_panels = debug_autohide_panels->GetValue();
	config->Debug.autohide_toolbars = debug_autohide_panels->GetValue();
//	config->Debug.autoupdate_backtrace = debug_autoupdate_backtrace->GetValue();
	config->Debug.compile_again = debug_compile_again->GetValue();
	config->Debug.close_on_normal_exit = debug_close_on_normal_exit->GetValue();
	config->Debug.always_debug = debug_always_debug->GetValue();
	config->Debug.raise_main_window = debug_raise_main_window->GetValue();
	config->Debug.inspections_on_right = debug_inspections_on_right->GetValue();
	config->Debug.show_thread_panel = debug_show_thread_panel->GetValue();
	config->Debug.show_log_panel = debug_show_log_panel->GetValue();
	config->Debug.select_modified_inspections = debug_select_modified_inspections->GetValue();
	main_window->inspection_ctrl->ResetChangeHightlights(); // aplica los colores
	config->Files.autocodes_file = files_autocode->GetValue();
	autocoder->Reset(project?project->autocodes_file:"");
	config->Debug.improve_inspections_by_type = improve_inspections_by_type->GetValue();
	config->Debug.macros_file = debug_macros_file->GetValue();
	config->Debug.blacklist = debug_blacklist->GetValue();
	config->Save();
	
	if (toolbars_wich_file->GetValue()!=config->Toolbars.wich_ones.file) {
		config->Toolbars.wich_ones.file=toolbars_wich_file->GetValue();
		if (config->Toolbars.wich_ones.file) {
			main_window->menu.view_toolbar_file->Check(true);
			main_window->aui_manager.GetPane(main_window->toolbar_file).Show();
		} else {
			main_window->menu.view_toolbar_file->Check(false);
			main_window->aui_manager.GetPane(main_window->toolbar_file).Hide();
		}
		main_window->aui_manager.Update();
	}
	if (toolbars_wich_edit->GetValue()!=config->Toolbars.wich_ones.edit) {
		config->Toolbars.wich_ones.edit=toolbars_wich_edit->GetValue();
		if (config->Toolbars.wich_ones.edit) {
			main_window->menu.view_toolbar_edit->Check(true);
			main_window->aui_manager.GetPane(main_window->toolbar_edit).Show();
		} else {
			main_window->menu.view_toolbar_edit->Check(false);
			main_window->aui_manager.GetPane(main_window->toolbar_edit).Hide();
		}
		main_window->aui_manager.Update();
	}
	if (toolbars_wich_view->GetValue()!=config->Toolbars.wich_ones.view) {
		config->Toolbars.wich_ones.view=toolbars_wich_view->GetValue();
		if (config->Toolbars.wich_ones.view) {
			main_window->menu.view_toolbar_view->Check(true);
			main_window->aui_manager.GetPane(main_window->toolbar_view).Show();
		} else {
			main_window->menu.view_toolbar_view->Check(false);
			main_window->aui_manager.GetPane(main_window->toolbar_view).Hide();
		}
		main_window->aui_manager.Update();
	}
	if (toolbars_wich_run->GetValue()!=config->Toolbars.wich_ones.run) {
		config->Toolbars.wich_ones.run=toolbars_wich_run->GetValue();
		if (config->Toolbars.wich_ones.run) {
			main_window->menu.view_toolbar_run->Check(true);
			main_window->aui_manager.GetPane(main_window->toolbar_run).Show();
		} else {
			main_window->menu.view_toolbar_run->Check(false);
			main_window->aui_manager.GetPane(main_window->toolbar_run).Hide();
		}
		main_window->aui_manager.Update();
	}
	if (toolbars_wich_debug->GetValue()!=config->Toolbars.wich_ones.debug) {
		config->Toolbars.wich_ones.debug=toolbars_wich_debug->GetValue();
		if (config->Toolbars.wich_ones.debug) {
			main_window->menu.view_toolbar_debug->Check(true);
			main_window->aui_manager.GetPane(main_window->toolbar_debug).Show();
		} else {
			main_window->menu.view_toolbar_debug->Check(false);
			main_window->aui_manager.GetPane(main_window->toolbar_debug).Hide();
		}
		main_window->aui_manager.Update();
	}
	if (toolbars_wich_misc->GetValue()!=config->Toolbars.wich_ones.misc) {
		config->Toolbars.wich_ones.misc=toolbars_wich_misc->GetValue();
		if (config->Toolbars.wich_ones.misc) {
			main_window->menu.view_toolbar_misc->Check(true);
			main_window->aui_manager.GetPane(main_window->toolbar_misc).Show();
		} else {
			main_window->menu.view_toolbar_misc->Check(false);
			main_window->aui_manager.GetPane(main_window->toolbar_misc).Hide();
		}
		main_window->aui_manager.Update();
	}
	if (toolbars_wich_tools->GetValue()!=config->Toolbars.wich_ones.tools) {
		config->Toolbars.wich_ones.tools=toolbars_wich_tools->GetValue();
		if (config->Toolbars.wich_ones.tools) {
			main_window->menu.view_toolbar_tools->Check(true);
			main_window->aui_manager.GetPane(main_window->toolbar_tools).Show();
		} else {
			main_window->menu.view_toolbar_tools->Check(false);
			main_window->aui_manager.GetPane(main_window->toolbar_tools).Hide();
		}
		main_window->aui_manager.Update();
	}
	if (toolbars_wich_find->GetValue()!=config->Toolbars.wich_ones.find) {
		config->Toolbars.wich_ones.find=toolbars_wich_find->GetValue();
		if (config->Toolbars.wich_ones.find) {
			main_window->menu.view_toolbar_find->Check(true);
			main_window->aui_manager.GetPane(main_window->toolbar_find).Show();
		} else {
			main_window->menu.view_toolbar_find->Check(false);
			main_window->aui_manager.GetPane(main_window->toolbar_find).Hide();
		}
		main_window->aui_manager.Update();
	}
	if (toolbar_icon_size->GetValue().BeforeFirst('x').ToLong(&l)) {
		if (l!=config->Toolbars.icon_size) {
			config->Toolbars.icon_size=l;
			main_window->CreateToolbars(main_window->toolbar_file);
			main_window->CreateToolbars(main_window->toolbar_edit);
			main_window->CreateToolbars(main_window->toolbar_view);
			main_window->CreateToolbars(main_window->toolbar_run);
			main_window->CreateToolbars(main_window->toolbar_tools);
			main_window->CreateToolbars(main_window->toolbar_misc);
			main_window->CreateToolbars(main_window->toolbar_debug);
		}
	} else
		config->Toolbars.icon_size=16;
	
	config->RecalcStuff();
	Toolchain::SelectToolchain();
	
	Close();

}

void mxPreferenceWindow::OnCancelButton(wxCommandEvent &event){
	Close();
}

void mxPreferenceWindow::OnClose(wxCloseEvent &event){
//	main_window->preference_window=NULL;
//	Destroy();
	Hide();
	event.Veto();
}

void mxPreferenceWindow::OnSkinButton(wxCommandEvent &event){
	int selection = skin_list->GetSelection();
	if (selection<0) return;
	config->Files.skin_dir=skin_paths[selection];
	mxMessageDialog(main_window,_T("El tema seleccionado se aplicara la proxima vez que cierre y vuelva a iniciar ZinjaI"),_T("Tema de iconos"), mxMD_OK|mxMD_INFO).ShowModal();
}

void mxPreferenceWindow::OnWxHelpButton(wxCommandEvent &event){
	wxFileDialog dlg(this,_T("Indice de ayuda wxWidgets:"),help_wxhelp_index->GetValue());
	if (wxID_OK==dlg.ShowModal())
		help_wxhelp_index->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnTempButton(wxCommandEvent &event){
	wxDirDialog dlg(this,_T("Directorio temporal:"),files_temp_dir->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_temp_dir->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnProjectButton(wxCommandEvent &event){
	wxDirDialog dlg(this,_T("Directorio de proyectos:"),files_project_folder->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_project_folder->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnGdbButton(wxCommandEvent &event){
	wxFileDialog dlg(this,_T("Comando del depurador"),files_debugger_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_debugger_command->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnImgBrowserButton(wxCommandEvent &event){
	wxFileDialog dlg(this,_T("Comando del visor de imagenes"),files_img_browser_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_img_browser_command->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnBrowserButton(wxCommandEvent &event){
	wxFileDialog dlg(this,_T("Comando del navegador Web"),files_browser_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_browser_command->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnDoxygenButton(wxCommandEvent &event){
	wxFileDialog dlg(this,_T("Ubicacion del ejecutable de Doxygen"),files_doxygen_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_doxygen_command->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnWxfbButton(wxCommandEvent &event){
	wxFileDialog dlg(this,_T("Ubicacion del ejecutable de wxFormBuilder"),files_wxfb_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_wxfb_command->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnValgrindButton(wxCommandEvent &event){
	wxFileDialog dlg(this,_T("Ubicacion del ejecutable de Valgrind"),files_valgrind_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_valgrind_command->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnCppCheckButton(wxCommandEvent &event){
	wxFileDialog dlg(this,_T("Ubicacion del ejecutable de CppCheck"),files_cppcheck_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_cppcheck_command->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnHelpButton(wxCommandEvent &event){
	SHOW_HELP(_T("preferences.html"));
}

#if defined(_WIN32) || defined(__WIN32__)

void mxPreferenceWindow::OnMingwButton(wxCommandEvent &event){
	wxDirDialog dlg(this,_T("Directorio de binarios de MinGW:"),files_mingw_dir->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_mingw_dir->SetValue(dlg.GetPath());
}

#else

void mxPreferenceWindow::OnExplorerButton(wxCommandEvent &event) {
	wxMenu menu;
	int count=0;
	if (utils->GetOutput(_T("dolphin --version")).Len()) {
		count++;
		menu.Append(mxID_EXPLORERS_DOLPHIN,_T("dolphin (kde 4)"));
	}
	if (utils->GetOutput(_T("konqueror --version")).Len()) {
		count++;
		menu.Append(mxID_EXPLORERS_KONQUEROR,_T("konqueror (kde 3)"));
	}
	if (utils->GetOutput(_T("nautilus --version")).Len()) {
		count++;
		menu.Append(mxID_EXPLORERS_NAUTILUS,_T("nautilus (gnome)"));
	}
	if (utils->GetOutput(_T("thunar --version")).Len()) {
		count++;
		menu.Append(mxID_EXPLORERS_THUNAR,_T("thunar (xfce)"));
	}
	if (count) {
		PopupMenu(&menu);
	} else {
		mxMessageDialog(main_window,_T("No se ha encontrado ningun explorador de archivos conocido."),_T("Explorador de archivos"), mxMD_OK|mxMD_WARNING).ShowModal();
	}	
}

void mxPreferenceWindow::OnExplorerDolphin(wxCommandEvent &event) {
	files_explorer_command->SetValue(_T("dolphin"));
}

void mxPreferenceWindow::OnExplorerKonqueror(wxCommandEvent &event) {
	files_explorer_command->SetValue(_T("konqueror"));
}

void mxPreferenceWindow::OnExplorerNautilus(wxCommandEvent &event) {
	files_explorer_command->SetValue(_T("nautilus"));
}

void mxPreferenceWindow::OnExplorerThunar(wxCommandEvent &event) {
	files_explorer_command->SetValue(_T("thunar"));
}

void mxPreferenceWindow::OnTerminalButton(wxCommandEvent &event) {
	wxMenu menu;
	int count=0;
	if (utils->GetOutput(_T("xterm -version")).Len()) {
		count++;
		menu.Append(mxID_TERMINALS_XTERM,_T("xterm (consola basica)"));
	}
	if (utils->GetOutput(_T("lxterminal --version")).Len()) {
		count++;
		menu.Append(mxID_TERMINALS_XTERM,_T("xterm (consola basica)"));
	}
//	if (utils->GetOutput(_T("xfterm4 --version")).Len()) {
//		count++;
//		menu.Append(mxID_TERMINALS_XFCE,_T("xterm (consola basica)"));
//	}
	if (utils->GetOutput(_T("konsole --version")).Len()) {
		count++;
		menu.Append(mxID_TERMINALS_KONSOLE,_T("konsole (consola de kde)"));
	}
	if (utils->GetOutput(_T("gnome-terminal --version")).Len()) {
		count++;
		menu.Append(mxID_TERMINALS_GNOME,_T("gnome-terminal (consola de gnome)"));
	}
	if (count) {
		PopupMenu(&menu);
	} else {
		mxMessageDialog(main_window,_T("No se ha encontrado una terminal conocida.\nInstale xterm,konsole o gnome-terminal; o\nconfigure el parametro \"Comando del\nTerminal\" en el cuadro de Preferencias.\""),_T("Terminal de ejecucion"), mxMD_OK|mxMD_WARNING).ShowModal();
	}	
}

void mxPreferenceWindow::OnTerminalXTerm(wxCommandEvent &event) {
	files_terminal_command->SetValue(_T(TERM_XTERM));
}

void mxPreferenceWindow::OnTerminalLX(wxCommandEvent &event) {
	files_terminal_command->SetValue(_T(TERM_LX));
}

//void mxPreferenceWindow::OnTerminalXfce(wxCommandEvent &event) {
//	files_terminal_command->SetValue(_T(TERM_XFCE));
//}

void mxPreferenceWindow::OnTerminalKonsole(wxCommandEvent &event) {
	if (utils->GetOutput(_T("konsole --version")).Len()) {
		if (utils->GetOutput(_T("konsole --version")).Find(_T("KDE: 3"))==wxNOT_FOUND)
			files_terminal_command->SetValue(_T(TERM_KDE4));
		else
			files_terminal_command->SetValue(_T(TERM_KDE3));
	}
}

void mxPreferenceWindow::OnTerminalGnomeTerminal(wxCommandEvent &event) {
	files_terminal_command->SetValue(_T(TERM_GNOME));
}

#endif

void mxPreferenceWindow::OnDebugBlacklistButton(wxCommandEvent &event) {
	new mxEnumerationEditor(this,_T("Parametros extra para el compilador"),debug_blacklist,true);
}

void mxPreferenceWindow::OnAutocodesButton(wxCommandEvent &event) {
	wxMenu menu;
	menu.Append(mxID_AUTOCODES_OPEN,LANG(PREFERENCES_OPEN_FILE,"&Buscar archivo..."));
	menu.Append(mxID_AUTOCODES_EDIT,LANG(PREFERENCES_EDIT_FILE,"&Editar archivo..."));
	PopupMenu(&menu);
}
void mxPreferenceWindow::OnAutocodesOpen(wxCommandEvent &event) {
	wxFileDialog dlg(this,_T("Archivo de definiciones de autocodigos"),_T(""),DIR_PLUS_FILE(config->zinjai_dir,files_autocode->GetValue()));
	if (wxID_OK==dlg.ShowModal()) {
		wxFileName fn(dlg.GetPath());
		fn.MakeRelativeTo(config->zinjai_dir);
		files_autocode->SetValue(fn.GetFullPath());
	}
}

void mxPreferenceWindow::OnAutocodesEdit(wxCommandEvent &event) {
	int i=2;
	wxString file=files_autocode->GetValue();
	if (wxFileName(file).FileExists())
		main_window->OpenFileFromGui(file,&i);
	else
		mxMessageDialog(main_window,_T("El archivo no existe"),_T("Archivo de definiciones de autocódigos"), mxMD_OK|mxMD_WARNING).ShowModal();	
}

void mxPreferenceWindow::OnDebugMacrosButton(wxCommandEvent &event) {
	wxMenu menu;
	menu.Append(mxID_DEBUG_MACROS_OPEN,LANG(PREFERENCES_OPEN_FILE,"&Buscar archivo..."));
	menu.Append(mxID_DEBUG_MACROS_EDIT,LANG(PREFERENCES_EDIT_FILE,"&Editar archivo..."));
	PopupMenu(&menu);
}

void mxPreferenceWindow::OnDebugMacrosOpen(wxCommandEvent &event) {
	wxFileDialog dlg(this,LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros para gdb"),_T(""),DIR_PLUS_FILE(config->zinjai_dir,debug_macros_file->GetValue()));
	if (wxID_OK==dlg.ShowModal()) {
		wxFileName fn(dlg.GetPath());
		fn.MakeRelativeTo(config->zinjai_dir);
		debug_macros_file->SetValue(fn.GetFullPath());
	}
}

void mxPreferenceWindow::OnDebugMacrosEdit(wxCommandEvent &event) {
	int i=2;
	wxString file=debug_macros_file->GetValue();
	if (wxFileName(file).FileExists())
		main_window->OpenFileFromGui(file,&i);
	else
		mxMessageDialog(main_window,_T("El archivo no existe"),LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros para gdb"), mxMD_OK|mxMD_WARNING).ShowModal();	
}

void mxPreferenceWindow::ShowUp() {
	ResetChanges();
	Show();
	Raise();
}

void mxPreferenceWindow::OnToolbarsFile(wxCommandEvent &evt) {
	
	if (toolbar_editor_file)
		toolbar_editor_file->ShowUp();
	else {
		toolbar_editor_file = new mxToolbarEditor(main_window->toolbar_file,LANG(PREFERENCES_EDIT_TOOLBAR_FILE_CAPTION,"Barra de Herramientas Archivo"));

		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_NEW,"Nuevo..."),_T("nuevo.png"),config->Toolbars.file.new_file);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_NEW_PROJECT,"Nuevo Proyecto..."),_T("proyecto.png"),config->Toolbars.file.new_project);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_OPEN,"Abrir..."),_T("abrir.png"),config->Toolbars.file.open);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_RECENT_SOURCES,"Fuentes Recientes..."),_T("recentSimple.png"),config->Toolbars.file.recent_simple);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_RECENT_PROJECTS,"Proyectos Recientes..."),_T("recentProject.png"),config->Toolbars.file.recent_project);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_OPEN_H,"Abrir h/cpp Complementario"),_T("abrirp.png"),config->Toolbars.file.open_header);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_OPEN_SELECTED,"Abrir Seleccionado"),_T("abrirh.png"),config->Toolbars.file.open_selected);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_SAVE,"Guardar"),_T("guardar.png"),config->Toolbars.file.save);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_SAVE_AS,"Guardar Como..."),_T("guardarComo.png"),config->Toolbars.file.save_as);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_SAVE_ALL,"Guardar Todo"),_T("guardarTodo.png"),config->Toolbars.file.save_all);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_SAVE_PROJECT,"Guardar Proyecto"),_T("guardarProyecto.png"),config->Toolbars.file.save_project);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_EXPORT_HTML,"Exportar a HTML..."),_T("exportHtml.png"),config->Toolbars.file.export_html);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_PRINT,"Imprimir..."),_T("imprimir.png"),config->Toolbars.file.print);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_RELOAD,"Recargar"),_T("recargar.png"),config->Toolbars.file.reload);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_CLOSE,"Cerrar"),_T("cerrar.png"),config->Toolbars.file.close);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_CLOSE_ALL,"Cerrar Todo"),_T("cerrarTodo.png"),config->Toolbars.file.close_all);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_CLOSE_PROJECT,"Cerrar Proyecto"),_T("cerrarProyecto.png"),config->Toolbars.file.close_project);
		toolbar_editor_file->Add(LANG(TOOLBAR_CAPTION_FILE_PROJECT_CONFIG,"Configuracion de Proyecto..."),_T("projectConfig.png"),config->Toolbars.file.project_config);

		toolbar_editor_file->ShowUp();
	}
}

void mxPreferenceWindow::OnToolbarsEdit(wxCommandEvent &evt) {
	if (toolbar_editor_edit)
		toolbar_editor_edit->ShowUp();
	else {
		toolbar_editor_edit = new mxToolbarEditor(main_window->toolbar_edit,LANG(PREFERENCES_EDIT_TOOLBAR_EDIT_CAPTION,"Barra de Herramientas \"Edicion\""));
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_UNDO,"Deshacer"),_T("deshacer.png"),config->Toolbars.edit.undo);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_REDO,"Rehacer"),_T("rehacer.png"),config->Toolbars.edit.redo);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_COPY,"Copiar"),_T("copiar.png"),config->Toolbars.edit.copy);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_CUT,"Cortar"),_T("cortar.png"),config->Toolbars.edit.cut);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_PASTE,"Pegar"),_T("pegar.png"),config->Toolbars.edit.paste);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_LINES_UP,"Mover Hacia Arriba"),_T("toggleLinesUp.png"),config->Toolbars.edit.move_up);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_LINES_DOWN,"Mover Hacia Abajo"),_T("toggleLinesDown.png"),config->Toolbars.edit.move_down);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_DUPLICATE_LINES,"Duplicar Linea(s)"),_T("duplicarLineas.png"),config->Toolbars.edit.duplicate);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_DELETE_LINES,"Eliminar Linea(s)"),_T("borrarLineas.png"),config->Toolbars.edit.delete_lines);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_GOTO_LINE,"Ir a Linea..."),_T("irALinea.png"),config->Toolbars.edit.goto_line);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_GOTO_FUNCTION,"Ir a Fucion/Clase/Metodo..."),_T("irAFuncion.png"),config->Toolbars.edit.goto_class);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_GOTO_FILE,"Ir a Archivo..."),_T("irAArchivo.png"),config->Toolbars.edit.goto_file);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_FIND,"Buscar..."),_T("buscar.png"),config->Toolbars.edit.find);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_FIND_PREV,"Buscar Anterior"),_T("buscarAnterior.png"),config->Toolbars.edit.find_prev);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_FIND_NEXT,"Buscar Siguiente"),_T("buscarSiguiente.png"),config->Toolbars.edit.find_next);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_REPLACE,"Reemplazar..."),_T("reemplazar.png"),config->Toolbars.edit.replace);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_INSERT_INCLUDE,"Insertar #include Correspondiente"),_T("insertarInclude.png"),config->Toolbars.edit.inser_header);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_COMMENT,"Comentar"),_T("comentar.png"),config->Toolbars.edit.comment);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_UNCOMMENT,"Descomentar"),_T("descomentar.png"),config->Toolbars.edit.uncomment);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_INDENT,"Indentar"),_T("indent.png"),config->Toolbars.edit.indent);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_BRACEMATCH,"Seleccionar Bloque"),_T("mostrarLlave.png"),config->Toolbars.edit.select_block);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_SELECT_ALL,"Seleccionar Todo"),_T("seleccionarTodo.png"),config->Toolbars.edit.select_all);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_HIGHLIGHT_LINES,"Resaltar Linea(s)/Quitar Resaltado"),_T("marcar.png"),config->Toolbars.edit.toggle_user_mark);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_FIND_HIGHLIGHTS,"Buscar Resaltado"),_T("irAMarca.png"),config->Toolbars.edit.find_user_mark);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_LIST_HIGHLIGHTS,"Listar Lineas Resaltadas"),_T("listarMarcas.png"),config->Toolbars.edit.list_user_marks);
		toolbar_editor_edit->Add(LANG(TOOLBAR_CAPTION_EDIT_FORCE_AUTOCOMPLETE,"Autocompletar"),_T("autocompletar.png"),config->Toolbars.edit.autocomplete);
		toolbar_editor_edit->ShowUp();
	}	
}

void mxPreferenceWindow::OnToolbarsMisc(wxCommandEvent &evt) {
	if (toolbar_editor_misc)
		toolbar_editor_misc->ShowUp();
	else {
		toolbar_editor_misc = new mxToolbarEditor(main_window->toolbar_misc,LANG(PREFERENCES_EDIT_TOOLBAR_MISC_CAPTION,"Barra de Herramientas \"Miscelanea\""));
		toolbar_editor_misc->Add(LANG(TOOLBAR_CAPTION_FILE_PREFERENCES,"Preferencias..."),_T("preferencias.png"),config->Toolbars.misc.preferences);
		toolbar_editor_misc->Add(LANG(TOOLBAR_CAPTION_HELP_TUTORIALS,"Tutoriales..."),_T("tutoriales.png"),config->Toolbars.misc.tutorials);
		toolbar_editor_misc->Add(LANG(TOOLBAR_CAPTION_HELP_ZINJAI,"Ayuda Sobre ZinjaI..."),_T("ayuda.png"),config->Toolbars.misc.help_ide);
		toolbar_editor_misc->Add(LANG(TOOLBAR_CAPTION_HELP_CPP,"Ayuda Sobre C++..."),_T("referencia.png"),config->Toolbars.misc.help_cpp);
		toolbar_editor_misc->Add(LANG(TOOLBAR_CAPTION_HELP_TIPS,"Sugerencias..."),_T("tip.png"),config->Toolbars.misc.show_tips);
		toolbar_editor_misc->Add(LANG(TOOLBAR_CAPTION_HELP_ABOUT,"Acerca de..."),_T("acercaDe.png"),config->Toolbars.misc.about);
		toolbar_editor_misc->Add(LANG(TOOLBAR_CAPTION_HELP_OPINION,"Opina sobre ZinjaI..."),_T("opinion.png"),config->Toolbars.misc.opinion);
		toolbar_editor_misc->Add(LANG(TOOLBAR_CAPTION_HELP_UPDATES,"Buscar Actualizaciones..."),_T("updates.png"),config->Toolbars.misc.find_updates);
		toolbar_editor_misc->Add(LANG(TOOLBAR_CAPTION_FILE_EXIT,"Salir de ZinjaI"),_T("salir.png"),config->Toolbars.misc.exit);
		toolbar_editor_misc->ShowUp();
	}	
}

void mxPreferenceWindow::OnToolbarsView(wxCommandEvent &evt) {
	if (toolbar_editor_view)
		toolbar_editor_view->ShowUp();
	else {
		toolbar_editor_view = new mxToolbarEditor(main_window->toolbar_view,LANG(PREFERENCES_EDIT_TOOLBAR_VIEW_CAPTION,"Barra de Herramientas \"ver\""));
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_SPLIT_VIEW,"Duplicar Vista"),_T("duplicarVista.png"),config->Toolbars.view.split_view);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_LINE_WRAP,"Ajuste de Linea"),_T("lineWrap.png"),config->Toolbars.view.line_wrap);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_WHITE_SPACES,"Mostrar Espacios y Caracteres de Fin de Linea"),_T("whiteSpace.png"),config->Toolbars.view.white_space);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_SYNTAX_HIGHLIGHT,"Colorear Sintaxis"),_T("syntaxColour.png"),config->Toolbars.view.sintax_colour);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_SYMBOLS_TREE,"Arbol de Simbolos"),_T("symbolsTree.png"),config->Toolbars.view.update_symbols);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_EXPLORER_TREE,"Explorador de Archivos"),_T("explorerTree.png"),config->Toolbars.view.explorer_tree);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_PROJECT_TREE,"Arbol de Proyecto"),_T("projectTree.png"),config->Toolbars.view.project_tree);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_COMPILER_TREE,"Resultados de La Compilacion"),_T("compilerTree.png"),config->Toolbars.view.compiler_tree);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_FULLSCREEN,"Pantalla Completa"),_T("fullScreen.png"),config->Toolbars.view.full_screen);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_BEGINNER_PANEL,"Panel de Mini-Plantillas"),_T("beginnerPanel.png"),config->Toolbars.view.beginner_panel);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_PREV_ERROR,"Ir a Error Anterior"),_T("errorPrev.png"),config->Toolbars.view.prev_error);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_VIEW_NEXT_ERROR,"Ir a siguiente error"),_T("errorNext.png"),config->Toolbars.view.next_error);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_FOLD_ALL_LEVELS,"Plegar Todos los Niveles"),_T("foldAll.png"),config->Toolbars.view.fold_all);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_FOLD_LEVEL_1,"Plegar el Primer Nivel"),_T("fold1.png"),config->Toolbars.view.fold_1);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_FOLD_LEVEL_2,"Plegar el Segundo Nivel"),_T("fold2.png"),config->Toolbars.view.fold_2);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_FOLD_LEVEL_3,"Plegar el Tercer Nivel"),_T("fold3.png"),config->Toolbars.view.fold_3);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_UNFOLD_ALL_LEVELS,"Desplegar Todos los Niveles"),_T("unfoldAll.png"),config->Toolbars.view.unfold_all);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_1,"Desplegar el Primer Nivel"),_T("unfold1.png"),config->Toolbars.view.unfold_1);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_2,"Desplegar el Segundo Nivel"),_T("unfold2.png"),config->Toolbars.view.unfold_2);
		toolbar_editor_view->Add(LANG(TOOLBAR_CAPTION_UNFOLD_LEVEL_3,"Desplegar el Tercer Nivel"),_T("unfold3.png"),config->Toolbars.view.unfold_3);
		toolbar_editor_view->ShowUp();
	}	
}

void mxPreferenceWindow::OnToolbarsTools(wxCommandEvent &evt) {
	if (toolbar_editor_tools)
		toolbar_editor_tools->ShowUp();
	else {
		toolbar_editor_tools = new mxToolbarEditor(main_window->toolbar_tools,LANG(PREFERENCES_EDIT_TOOLBAR_TOOLS_CAPTION,"Barra de Herramientas \"Herramientas\""));
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DRAW_FLOWCHART,"Dibujar Diagrama de Flujo..."),_T("flujo.png"),config->Toolbars.tools.draw_flow);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DRAW_CLASS_HIERARCHY,"Dibujar Jerarquia de Clases..."),_T("clases.png"),config->Toolbars.tools.draw_classes);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_COMMENTS_COPY_CODE_FROM_H,"Implementar Metodos/Funciones Faltantes..."),_T("copy_code_from_h.png"),config->Toolbars.tools.copy_code_from_h);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_COMMENTS_ALIGN_COMMENTS,"Alinear Comentarios..."),_T("align_comments.png"),config->Toolbars.tools.align_comments);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_COMMENTS_DELETE_COMMENTS,"Eliminar Comentarios..."),_T("remove_comments.png"),config->Toolbars.tools.remove_comments);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_MARK_VALID,"Marcar Lineas No Compiladas..."),_T("preproc_mark_valid.png"),config->Toolbars.tools.preproc_mark_valid);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_UNMARK_ALL,"Borrar Marcas..."),_T("preproc_unmark_all.png"),config->Toolbars.tools.preproc_unmark_all);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_PREPROC_EXPAND_MACROS,"Expandir Macros..."),_T("preproc_expand_macros.png"),config->Toolbars.tools.preproc_expand_macros);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_GENERATE_MAKEFILE,"Generar Makefile..."),_T("makefile.png"),config->Toolbars.tools.generate_makefile);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_OPEN_TERMINAL,"Abrir Consola..."),_T("console.png"),config->Toolbars.tools.open_terminal);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_EXE_INFO,"Propiedades del Ejecutable..."),_T("exeinfo.png"),config->Toolbars.tools.exe_info);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_SHARE_OPEN,"Abrir Archivo Compartido..."),_T("abrirs.png"),config->Toolbars.tools.open_shared);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_SHARE_SHARE,"Compartir Fuente..."),_T("compartir.png"),config->Toolbars.tools.share_source);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_SHARE_LIST,"Ver Lista de Compartidos Propios..."),_T("share_list.png"),config->Toolbars.tools.share_list);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_PROJECT_STATISTICS,"Estadisticas del Proyecto..."),_T("proystats.png"),config->Toolbars.tools.proy_stats);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DRAW_PROJECT,"Grafo del Proyecto..."),_T("draw_project.png"),config->Toolbars.tools.draw_classes);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DIFF_TWO,"Compara Dos Fuentes Abiertos..."),_T("diff_sources.png"),config->Toolbars.tools.diff_two_sources);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISK,"Comparar Fuente con Archivo en Disco..."),_T("diff_source_file.png"),config->Toolbars.tools.diff_other_file);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DIFF_HIMSELF,"Comparar Cambios en el Fuente con su Version en Disco..."),_T("diff_source_himself.png"),config->Toolbars.tools.diff_himself);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DIFF_SHOW,"Mostrar Cambio"),_T("diff_show.png"),config->Toolbars.tools.diff_show);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DIFF_APPLY,"Aplicar Cambio"),_T("diff_apply.png"),config->Toolbars.tools.diff_apply);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DIFF_DISCARD,"Descartar Cambio"),_T("diff_discard.png"),config->Toolbars.tools.diff_discard);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DIFF_CLEAR,"Borrar Marcas de Comparacion"),_T("diff_clear.png"),config->Toolbars.tools.diff_clear);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_GENERATE,"Generar Documentacion Doxygen..."),_T("doxy_run.png"),config->Toolbars.tools.doxy_generate);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_CONFIGURE,"Configurar Documentacion Doxygen..."),_T("doxy_config.png"),config->Toolbars.tools.doxy_config);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_DOXYGEN_VIEW,"Ver Documentacion Doxygen..."),_T("doxy_view.png"),config->Toolbars.tools.doxy_view);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_WXFB_ACTIVATE,"Activar Integracion wxFormBuilder"),_T("wxfb_activate.png"),config->Toolbars.tools.wxfb_activate);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_WXFB_NEW_RESOURCE,"Adjuntar un Nuevo Proyecto wxFB"),_T("wxfb_new_res.png"),config->Toolbars.tools.wxfb_new_res);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_WXFB_LOAD_RESOURCE,"Adjuntar un Proyecto wxFB Existente"),_T("wxfb_load_res.png"),config->Toolbars.tools.wxfb_load_res);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REGENERATE,"Regenerar Proyectos wxFB"),_T("wxfb_regen.png"),config->Toolbars.tools.wxfb_regen);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_WXFB_INHERIT,"Generar Clase Heredada..."),_T("wxfb_inherit.png"),config->Toolbars.tools.wxfb_inherit);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_WXFB_UPDATE_INHERIT,"Actualizar Clase Heredada..."),_T("wxfb_update_inherit.png"),config->Toolbars.tools.wxfb_update_inherit);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_WXFB_REFERENCE,"Referencia wxWidgets..."),_T("ayuda_wx.png"),config->Toolbars.tools.wxfb_help_wx);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_GPROF_ACTIVATE,"Activar Perfilado de Ejecucion"),_T("comp_for_prof.png"),config->Toolbars.tools.gprof_activate);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_GPROF_SHOW,"Graficar Resultados del Perfilado de Ejecucion..."),_T("showgprof.png"),config->Toolbars.tools.gprof_show_graph);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_GPROF_LIST,"Listar Resultados del Perfilado de Ejecucion"),_T("listgprof.png"),config->Toolbars.tools.gprof_list_output);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_RUN,"Ejecutar Para Analisis Estatico"),_T("cppcheck_run.png"),config->Toolbars.tools.cppcheck_run);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_CONFIG,"Configurar Analisis Estatico"),_T("cppcheck_config.png"),config->Toolbars.tools.cppcheck_config);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_CPPCHECK_VIEW,"Mostrar Resultados del Analisis Dinamico"),_T("cppcheck_view.png"),config->Toolbars.tools.cppcheck_view);
#if !defined(_WIN32) && !defined(__WIN32__)
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_VALGRIND_RUN,"Ejecutar Para Analisis Dinamico"),_T("valgrind_run.png"),config->Toolbars.tools.valgrind_run);
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_VALGRIND_VIEW,"Mostrar Resultados del Analisis Dinamico"),_T("valgrind_view.png"),config->Toolbars.tools.valgrind_view);
#endif
		for (int i=0;i<10;i++) {
			if (config->CustomTools.names[i].Len())
				toolbar_editor_tools->Add(wxString(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada "))<<i<<" ("<<config->CustomTools.names[i]<<")",wxString("customTool")<<i<<".png",config->Toolbars.tools.custom[i]);
			else
				toolbar_editor_tools->Add(wxString(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_TOOL,"Herramienta Personalizada "))<<i<<" ("<<LANG(CUSTOM_TOOLS_NOT_CONFIGURED,"no configurada")<<")",wxString("customTool")<<i<<".png",config->Toolbars.tools.custom[i]);
		}
		toolbar_editor_tools->Add(LANG(TOOLBAR_CAPTION_TOOLS_CUSTOM_SETTINGS,"Configurar Herramientas Personalizadas"),_T("customToolsSettings.png"),config->Toolbars.tools.custom_settings);
		toolbar_editor_tools->ShowUp();
	}	
}

void mxPreferenceWindow::OnToolbarsDebug(wxCommandEvent &evt) {
	if (toolbar_editor_debug)
		toolbar_editor_debug->ShowUp();
	else {
		toolbar_editor_debug = new mxToolbarEditor(main_window->toolbar_debug,LANG(PREFERENCES_EDIT_TOOLBAR_DEBUG_CAPTION,"Barra de Herramientas \"Depuracion\""));
		toolbar_editor_debug->Add(_T("Comenzar/Continuar Depuracion"),_T("depurar.png"),config->Toolbars.debug.start);
		toolbar_editor_debug->Add(_T("Interrumpir Ejecucion en Depuracion"),_T("pausar.png"),config->Toolbars.debug.pause);
		toolbar_editor_debug->Add(_T("Detener Depuracion"),_T("detener.png"),config->Toolbars.debug.stop);
		toolbar_editor_debug->Add(_T("Step In"),_T("step_in.png"),config->Toolbars.debug.step_in);
		toolbar_editor_debug->Add(_T("Step Over"),_T("step_over.png"),config->Toolbars.debug.step_over);
		toolbar_editor_debug->Add(_T("Step Out"),_T("step_out.png"),config->Toolbars.debug.step_out);
		toolbar_editor_debug->Add(_T("Return..."),_T("return.png"),config->Toolbars.debug.function_return);
		toolbar_editor_debug->Add(_T("Ejecutar Hasta El Cursor"),_T("run_until.png"),config->Toolbars.debug.run_until);
		toolbar_editor_debug->Add(_T("Continuar Desde Aqui"),_T("debug_jump.png"),config->Toolbars.debug.jump);
#if !defined(__WIN32__)
		toolbar_editor_debug->Add(_T("Habilitar Ejecucion Hacia Atras"),_T("reverse_enable.png"),config->Toolbars.debug.enable_inverse_exec);
		toolbar_editor_debug->Add(_T("Ejecutar Hacia Atras"),_T("reverse_toggle.png"),config->Toolbars.debug.inverse_exec);
#endif
		toolbar_editor_debug->Add(_T("Panel de Inspecciones"),_T("inspect.png"),config->Toolbars.debug.inspections);
		toolbar_editor_debug->Add(_T("Trazado Inverso"),_T("backtrace.png"),config->Toolbars.debug.backtrace);
		toolbar_editor_debug->Add(_T("Hilos de Ejecucion"),_T("threadlsit.png"),config->Toolbars.debug.threadlist);
		toolbar_editor_debug->Add(_T("Agregar/Quitar Breakpoint"),_T("breakpoint.png"),config->Toolbars.debug.break_toggle);
		toolbar_editor_debug->Add(_T("Opciones del Breakpoint..."),_T("breakpoint_options.png"),config->Toolbars.debug.break_options);
		toolbar_editor_debug->Add(_T("Listar Watch/Break points..."),_T("breakpoint_list.png"),config->Toolbars.debug.break_list);
		toolbar_editor_debug->Add(_T("Mostrar Mensajes del Depurador"),_T("show_log_panel.png"),config->Toolbars.debug.log_panel);
		toolbar_editor_debug->ShowUp();
	}		
}

void mxPreferenceWindow::OnToolbarsRun(wxCommandEvent &evt) {
	if (toolbar_editor_run)
		toolbar_editor_run->ShowUp();
	else {
		toolbar_editor_run = new mxToolbarEditor(main_window->toolbar_run,LANG(PREFERENCES_EDIT_TOOLBAR_RUN_CAPTION,"Barra de Herramientas \"Ejecucion\""));
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_RUN_COMPILE,"Compilar"),_T("compilar.png"),config->Toolbars.run.compile);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_RUN_RUN,"Guardar, Compilar y Ejecutar..."),_T("ejecutar.png"),config->Toolbars.run.run);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_RUN_OLD,"Ejecutar Sin Recompilar..."),_T("ejecutar_old.png"),config->Toolbars.run.run_old);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_RUN_STOP,"Detener"),_T("detener.png"),config->Toolbars.run.stop);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_RUN_CLEAN,"Limpiar"),_T("limpiar.png"),config->Toolbars.run.clean);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_RUN_OPTIONS,"Opciones..."),_T("opciones.png"),config->Toolbars.run.options);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_DEBUG_RUN,"Depurar"),_T("depurar.png"),config->Toolbars.run.debug);
#if !defined(__WIN32__)
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_DEBUG_ATTACH,"Adjuntar Depurador..."),_T("debug_attach.png"),config->Toolbars.run.debug_attach);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_DEBUG_CORE_DUMP,"Cargar Volcado de Memoria..."),_T("core_dump.png"),config->Toolbars.run.load_core_dump);
#endif
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_DEBUG_TOGGLE_BREAKPOINT,"Agregar/Quitar Breakpoint"),_T("breakpoint.png"),config->Toolbars.run.break_toggle);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_DEBUG_BREAKPOINT_OPTIONS,"Opciones del Breakpoint..."),_T("breakpoint_options.png"),config->Toolbars.run.break_options);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_DEBUG_LIST_BREAKPOINTS,"Listar Breakpoints..."),_T("breakpoint_list.png"),config->Toolbars.run.break_list);
		toolbar_editor_run->Add(LANG(TOOLBAR_CAPTION_INSPECT,"Panel de Inspecciones"),_T("inspect.png"),config->Toolbars.run.inspections);
		toolbar_editor_run->ShowUp();
	}	
}

void mxPreferenceWindow::SetToolbarPage() {
	for (unsigned int i=0;i<notebook->GetPageCount();i++)
		if (notebook->GetPage(i)==panel_toolbars) {
			notebook->SetSelection(i);
			break;
		}
}

void mxPreferenceWindow::OnToolbarsReset(wxCommandEvent &evt) {
	if (mxMD_YES == mxMessageDialog(main_window,_T("Perdera todas las modificiaciones realizadas a las barras\n de herramientas. Desea continuar?"), _T("Reiniciar Barras de Herramientas"), mxMD_YES_NO).ShowModal()) {
		bool 
			tbed=config->Toolbars.wich_ones.edit,
			tbrn=config->Toolbars.wich_ones.run,
			tbvw=config->Toolbars.wich_ones.view,
			tbfn=config->Toolbars.wich_ones.find,
			tbfl=config->Toolbars.wich_ones.file,
			tbms=config->Toolbars.wich_ones.misc,
			tbdb=config->Toolbars.wich_ones.debug,
			tbtl=config->Toolbars.wich_ones.tools;
		config->LoadToolBarsDefaults();
		toolbars_wich_edit->SetValue(config->Toolbars.wich_ones.edit);
		toolbars_wich_run->SetValue(config->Toolbars.wich_ones.run);
		toolbars_wich_view->SetValue(config->Toolbars.wich_ones.view);
		toolbars_wich_find->SetValue(config->Toolbars.wich_ones.find);
		toolbars_wich_file->SetValue(config->Toolbars.wich_ones.file);
		toolbars_wich_misc->SetValue(config->Toolbars.wich_ones.misc);
		toolbars_wich_debug->SetValue(config->Toolbars.wich_ones.debug);
		toolbars_wich_tools->SetValue(config->Toolbars.wich_ones.tools);
		main_window->CreateToolbars(main_window->toolbar_debug);
		main_window->CreateToolbars(main_window->toolbar_file);
		main_window->CreateToolbars(main_window->toolbar_view);
		main_window->CreateToolbars(main_window->toolbar_edit);
		main_window->CreateToolbars(main_window->toolbar_misc);
		main_window->CreateToolbars(main_window->toolbar_tools);
		main_window->CreateToolbars(main_window->toolbar_run);
		config->Toolbars.wich_ones.edit=tbed;
		config->Toolbars.wich_ones.run=tbrn;
		config->Toolbars.wich_ones.view=tbvw;
		config->Toolbars.wich_ones.find=tbfn;
		config->Toolbars.wich_ones.file=tbfl;
		config->Toolbars.wich_ones.misc=tbms;
		config->Toolbars.wich_ones.debug=tbdb;
		config->Toolbars.wich_ones.tools=tbtl;
	}
}
	
void mxPreferenceWindow::ResetChanges() {

	// paths
	help_wxhelp_index->SetValue(config->Help.wxhelp_index);
	files_temp_dir->SetValue(config->Files.temp_dir);
#if defined(_WIN32) || defined(__WIN32__)
	files_mingw_dir->SetValue(config->Files.mingw_dir);
#endif
	files_project_folder->SetValue(config->Files.project_folder);
	files_wxfb_command->SetValue(config->Files.wxfb_command);
#ifdef __WIN32__
	files_valgrind_command->SetValue(config->Files.valgrind_command);
#endif
	files_cppcheck_command->SetValue(config->Files.cppcheck_command);
	
	// writting
	source_smartIndent->SetValue(config->Source.smartIndent);
	source_bracketInsertion->SetValue(config->Source.bracketInsertion);
	source_indentPaste->SetValue(config->Source.indentPaste);
	source_avoidNoNewLineWarning->SetValue(config->Source.avoidNoNewLineWarning);
	source_toolTips->SetValue(config->Source.toolTips);
	init_beautifyCompilerErrors->SetValue(config->Init.beautify_compiler_errors);
	source_autoCompletion->SetValue(config->Source.autoCompletion);
	source_autocloseStuff->SetValue(config->Source.autocloseStuff);
	source_callTips->SetValue(config->Source.callTips);
	help_autocomp_indexes->Clear();
	wxDir dir(config->Help.autocomp_dir);
	if ( dir.IsOpened() ) {
		wxArrayString autocomp_array;
		utils->Split(config->Help.autocomp_indexes,autocomp_array);
		wxString filename;
		wxString spec;
		bool cont = dir.GetFirst(&filename, spec , wxDIR_FILES);
		int n;
		while ( cont ) {
			n = help_autocomp_indexes->Append(filename);
			if (autocomp_array.Index(filename)!=wxNOT_FOUND)
				help_autocomp_indexes->Check(n,true);
			cont = dir.GetNext(&filename);
		}	
	}
	
	// style
	source_syntaxEnable->SetValue(config->Source.syntaxEnable);
	source_whiteSpace->SetValue(config->Source.whiteSpace);
//	source_wrapMode->SetValue(config->Source.wrapMode);
	source_lineNumber->SetValue(config->Source.lineNumber);
	source_foldEnable->SetValue(config->Source.foldEnable);
	source_tabWidth->SetValue(wxString()<<config->Source.tabWidth);
	source_tabUseSpaces->SetValue(config->Source.tabUseSpaces);
	styles_print_size->SetValue(wxString()<<int(config->Styles.print_size));
	styles_font_size->SetValue(wxString()<<int(config->Styles.font_size));
	init_autohide_menus_fs->SetValue(config->Init.autohide_menus_fs);
	init_autohide_toolbars_fs->SetValue(config->Init.autohide_toolbars_fs);
	init_autohide_panels_fs->SetValue(config->Init.autohide_panels_fs);
	files_autocode->SetValue(config->Files.autocodes_file);
	
	// simple
//	files_templates_dir->SetValue(config->Files.templates_dir);
	running_compiler_options->SetValue(config->Running.compiler_options);
	running_wait_for_key->SetValue(config->Running.wait_for_key);
	running_always_ask_args->SetValue(config->Running.always_ask_args);
	init_always_add_extension->SetValue(config->Init.always_add_extension);
	running_dont_run_headers->SetValue(config->Running.dont_run_headers);
	init_autohide_panels->SetValue(config->Init.autohide_panels);
	init_stop_compiling_on_error->SetValue(config->Init.stop_compiling_on_error);
	init_singleton->SetValue(config->Init.singleton);
	init_max_jobs->SetValue(wxString()<<config->Init.max_jobs);
	
	// general
	init_show_splash->SetValue(config->Init.show_splash);
	init_show_extra_panels->SetValue(config->Help.show_extra_panels);
	init_lang_file->SetValue(config->Init.language_file);
	init_show_welcome->SetValue(config->Init.show_welcome);
	init_show_tip_on_startup->SetValue(config->Init.show_tip_on_startup);
	init_wrap_mode->SetSelection(config->Init.wrap_mode);
	init_new_file->SetSelection(config->Init.new_file);
	init_history_len->SetValue(wxString()<<config->Init.history_len);
	init_save_project->SetValue(config->Init.save_project);
	init_close_files_for_project->SetValue(config->Init.close_files_for_project);
	init_prefer_explorer_tree->SetValue(config->Init.prefer_explorer_tree);
	init_show_explorer_tree->SetValue(config->Init.show_explorer_tree);
	init_left_panels->SetValue(config->Init.left_panels);
	init_check_for_updates->SetValue(config->Init.check_for_updates);
#if defined(_WIN32) || defined(__WIN32__)
#else
	init_lang_es->SetValue(config->Init.lang_es);
//	desktop_icon->SetValue(false);
#endif

	// toolbars
	toolbars_wich_file->SetValue(config->Toolbars.wich_ones.file);
	toolbars_wich_edit->SetValue(config->Toolbars.wich_ones.edit);
	toolbars_wich_run->SetValue(config->Toolbars.wich_ones.run);
	toolbars_wich_debug->SetValue(config->Toolbars.wich_ones.debug);
	toolbars_wich_tools->SetValue(config->Toolbars.wich_ones.tools);
	toolbars_wich_misc->SetValue(config->Toolbars.wich_ones.misc);
	toolbars_wich_find->SetValue(config->Toolbars.wich_ones.find);
	toolbars_wich_view->SetValue(config->Toolbars.wich_ones.view);
	wxArrayString icon_sizes;
	icon_sizes.Add(_T("16x16"));
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,_T("24")))) icon_sizes.Add(_T("24x24"));
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,_T("32")))) icon_sizes.Add(_T("32x32"));
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,_T("48")))) icon_sizes.Add(_T("48x48"));
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,_T("64")))) icon_sizes.Add(_T("64x64"));
	wxString icsz = wxString()<<config->Toolbars.icon_size<<_T("x")<<config->Toolbars.icon_size;
	unsigned int idx_icsz = icon_sizes.Index(icsz);
	if (idx_icsz>=icon_sizes.GetCount()) idx_icsz=0;
	toolbar_icon_size->SetSelection(idx_icsz);
	
	// debug
	debug_auto_solibs->SetValue(config->Debug.auto_solibs);
	debug_readnow->SetValue(config->Debug.readnow);
	debug_allow_edition->SetValue(config->Debug.allow_edition);
	debug_autohide_panels->SetValue(config->Debug.autohide_panels);
	debug_autohide_panels->SetValue(config->Debug.autohide_toolbars);
//	debug_autoupdate_backtrace->SetValue(config->Debug.autoupdate_backtrace);
	debug_compile_again->SetValue(config->Debug.compile_again);
	debug_close_on_normal_exit->SetValue(config->Debug.close_on_normal_exit);
	debug_always_debug->SetValue(config->Debug.always_debug);
	debug_raise_main_window->SetValue(config->Debug.raise_main_window);
	debug_inspections_on_right->SetValue(config->Debug.inspections_on_right);
	debug_show_thread_panel->SetValue(config->Debug.show_thread_panel);
	debug_show_log_panel->SetValue(config->Debug.show_log_panel);
	debug_select_modified_inspections->SetValue(config->Debug.select_modified_inspections);
	improve_inspections_by_type->SetValue(config->Debug.improve_inspections_by_type);
	debug_macros_file->SetValue(config->Debug.macros_file);
	debug_blacklist->SetValue(config->Debug.blacklist);
	
}

void mxPreferenceWindow::OnSkinList(wxCommandEvent &event) {
	static int last_selection=-1;
	int selection = skin_list->GetSelection();
	if (selection==last_selection || selection<0) return;
	last_selection=selection;
	skin_text->LoadFile(DIR_PLUS_FILE(skin_paths[selection],_T("descripcion.txt")));
	if (wxFileName::FileExists(DIR_PLUS_FILE(skin_paths[selection],_T("skin_preview.png"))))
		skin_image->SetBitmap(wxBitmap(DIR_PLUS_FILE(skin_paths[selection],_T("skin_preview.png")),wxBITMAP_TYPE_PNG));
	else
		skin_image->SetBitmap(wxBitmap(SKIN_FILE(_T("skin_no_preview.png")),wxBITMAP_TYPE_PNG));
}

void mxPreferenceWindow::OnMaxJobsButton(wxCommandEvent &event) {
	wxMessageBox(wxString(LANG(LANG_MAX_JOBS_TIP_PRE,""
		"Este campo define la cantidad de objetos que se compilan en simultaneo "
		"al construir un proyecto. Si su pc tiene mas de un nucleo, la cantidad "
		"de nucleos, o la mitad son valores recomendables. Consulte la ayuda para "
		"más detalles. ZinjaI detecta en el sistema actual "))<<wxThread::GetCPUCount()
		<<LANG(LANG_MAX_JOBS_TIP_POST," nucleos"),
		LANG(PREFERENCES_GENERAL_MAX_JOBS,"Cantidad de pasos en paralelo al compilar"),wxOK,this);	
}

void mxPreferenceWindow::OnColoursButton (wxCommandEvent & event) {
	Hide(); new mxColoursEditor(this);
}

void mxPreferenceWindow::OnXdgButton(wxCommandEvent &evt) {
#if !defined(__WIN32__) && !defined(__APPLE__)
	new mxIconInstaller(false);
#endif
}

void mxPreferenceWindow::OnImproveInspectionsByTypeButton (wxCommandEvent & event) {
	mxInspectionsImprovingEditor(this);
}

