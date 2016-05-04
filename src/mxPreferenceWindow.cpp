#include <wx/aui/auibook.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/dir.h>
#include <wx/treebook.h>
#include <wx/toolbook.h>
#include <wx/listbook.h>
#include <wx/imaglist.h>

#include "mxPreferenceWindow.h"
#include "mxShortcutsDialog.h"
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
#include <wx/fontenum.h>
#include "MenusAndToolsConfig.h"
#include "mxToolchainConfig.h"
#include "SimpleTemplates.h"
#include "mxThreeDotsUtils.h"

static cfgStyles s_old_config_styles; // aquí para evitar tener que hacer el include de ConfigManager en el .h

#ifndef __WIN32__
int LinuxTerminalInfo::count = 0;
LinuxTerminalInfo *LinuxTerminalInfo::list = nullptr;
void LinuxTerminalInfo::Initialize() {
	const int term_count=9;
	list = new LinuxTerminalInfo[term_count];
	count=0;
//	list[count++]=LinuxTerminalInfo("xterm","xterm -version","xterm -T \"${TITLE}\" -e");
	list[count++]=LinuxTerminalInfo("xterm","xterm -version","xterm -T \"${TITLE}\" -fa \"Liberation Mono\" -fs 12 -e");
	list[count++]=LinuxTerminalInfo("lxterminal","lxterminal -version","lxterminal -T \"${TITLE}\" -e");
	list[count++]=LinuxTerminalInfo("aterm","aterm --version","aterm -title \"${TITLE}\" -e");
	list[count++]=LinuxTerminalInfo("roxterm","roxterm --help","roxterm --separate --hide-menubar -T \"${TITLE}\" -e");
	list[count++]=LinuxTerminalInfo("xfce4-terminal","xfce4-terminal --version","xfce4-terminal --disable-server --hide-menubar --hide-toolbar -T \"${TITLE}\" -x");
	list[count++]=LinuxTerminalInfo("konsole (kde3)","konsole --version","konsole --nomenubar --notoolbar -T \"${TITLE}\" -e",false,"KDE: 3");
	list[count++]=LinuxTerminalInfo("mate-terminal","mate-terminal --version","mate-terminal --disable-factory --hide-menubar -t \"${TITLE}\" -x");
	list[count++]=LinuxTerminalInfo("konsole (kde4)","konsole --version","konsole -e",true,"~KDE: 3");
//	list[count++]=LinuxTerminalInfo("gnome-terminal","gnome-terminal --version","gnome-terminal --disable-factory --hide-menubar -t \"${TITLE}\" -x",true);
	list[count++]=LinuxTerminalInfo("gnome-terminal","gnome-terminal --version","gnome-terminal --hide-menubar -t \"${TITLE}\" -x",true);
#ifdef _ZINJAI_DEBUG
	if (term_count!=count) wxMessageBox("LinuxTerminalInfo::Init: term_count!=count");
#endif
}
bool LinuxTerminalInfo::Test() {
	wxString out = mxUT::GetOutput(test_command);
	if (extra_test.Len()) {
		if (extra_test[0]=='~')
			return out.Len() && !out.Contains(extra_test.Mid(1));
		else
			return out.Contains(extra_test);
	}
	else return out.Length();
}
#endif

static mxPreferenceWindow *s_preference_window = nullptr;

BEGIN_EVENT_TABLE(mxPreferenceWindow, wxDialog)
	EVT_BUTTON(wxID_OK,mxPreferenceWindow::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxPreferenceWindow::OnCancelButton)
	EVT_BUTTON(mxID_MAX_JOBS,mxPreferenceWindow::OnMaxJobsButton)
	EVT_BUTTON(mxID_COLORS_PICKER,mxPreferenceWindow::OnColoursButton)
	EVT_BUTTON(mxID_DEBUG_IMPROVE_INSPECTIONS_BY_TYPE,mxPreferenceWindow::OnImproveInspectionsByTypeButton)
	EVT_BUTTON(mxID_GDB_PATH,mxPreferenceWindow::OnGdbButton)
	EVT_BUTTON(mxID_XDOT_PATH,mxPreferenceWindow::OnXdotButton)
	EVT_BUTTON(mxID_PREFERENCES_TOOLCHAIN_OPTIONS,mxPreferenceWindow::OnToolchainButton)
	EVT_BUTTON(mxID_IMG_VIEWER_PATH,mxPreferenceWindow::OnImgBrowserButton)
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
	EVT_BUTTON(mxID_PREFERENCES_CUSTOMIZE_SHORTCUTS,mxPreferenceWindow::OnCustomizeShortcuts)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_FILE,mxPreferenceWindow::OnToolbarsFile)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_EDIT,mxPreferenceWindow::OnToolbarsEdit)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_RUN,mxPreferenceWindow::OnToolbarsRun)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_MISC,mxPreferenceWindow::OnToolbarsMisc)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_TOOLS,mxPreferenceWindow::OnToolbarsTools)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_DEBUG,mxPreferenceWindow::OnToolbarsDebug)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_VIEW,mxPreferenceWindow::OnToolbarsView)
	EVT_BUTTON(mxID_PREFERENCES_TOOLBAR_RESET,mxPreferenceWindow::OnToolbarsReset)
	EVT_BUTTON(mxID_PREFERENCES_CLEAR_SUBCMD_CACHE,mxPreferenceWindow::OnClearSubcmdCache)
#ifdef __WIN32__
//	EVT_BUTTON(mxID_MINGW_FOLDER,mxPreferenceWindow::OnMingwButton)
#else
	EVT_BUTTON(mxID_PREFERENCES_XDG,mxPreferenceWindow::OnXdgButton)
	EVT_BUTTON(mxID_TERMINALS_BUTTON,mxPreferenceWindow::OnTerminalButton)
	EVT_MENU_RANGE(mxID_LAST_ID, mxID_LAST_ID+50,mxPreferenceWindow::OnSelectTerminal)
	EVT_BUTTON(mxID_EXPLORERS_BUTTON,mxPreferenceWindow::OnExplorerButton)
	EVT_MENU(mxID_EXPLORERS_KONQUEROR,mxPreferenceWindow::OnExplorerKonqueror)
	EVT_MENU(mxID_EXPLORERS_DOLPHIN,mxPreferenceWindow::OnExplorerDolphin)
	EVT_MENU(mxID_EXPLORERS_THUNAR,mxPreferenceWindow::OnExplorerThunar)
	EVT_MENU(mxID_EXPLORERS_NAUTILUS,mxPreferenceWindow::OnExplorerNautilus)
#endif
	EVT_LISTBOX(mxID_SKIN_LIST,mxPreferenceWindow::OnSkinList)
	EVT_COMBOBOX(mxID_PREFERENCES_FONTNAME,mxPreferenceWindow::OnFontChange)
	EVT_TEXT(mxID_PREFERENCES_FONTSIZE,mxPreferenceWindow::OnFontChange)
END_EVENT_TABLE()

mxPreferenceWindow::mxPreferenceWindow(wxWindow* parent) : 
	mxDialog(parent, LANG(PREFERENCES_CAPTION,"Preferencias"), mxDialog::OCP_HIDE)
{

	ignore_styles_changes = true;
	
	toolbar_editor_file = nullptr;
	toolbar_editor_edit = nullptr;
	toolbar_editor_view = nullptr;
	toolbar_editor_tools = nullptr;
	toolbar_editor_debug = nullptr;
	toolbar_editor_run = nullptr;
	toolbar_editor_misc = nullptr;
	
	s_old_config_styles=config->Styles;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	notebook = new mxBookCtrl(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxBK_LEFT);
	wxImageList* imglist = new wxImageList(32, 32,true,8);
	imglist->Add(bitmaps->GetBitmap("pref_general.png"));
	imglist->Add(bitmaps->GetBitmap("pref_simple_program.png"));
	imglist->Add(bitmaps->GetBitmap("pref_style.png"));
	imglist->Add(bitmaps->GetBitmap("pref_writing.png"));
	imglist->Add(bitmaps->GetBitmap("pref_skin.png"));
	imglist->Add(bitmaps->GetBitmap("pref_toolbars.png"));
	imglist->Add(bitmaps->GetBitmap("pref_debug.png"));
	imglist->Add(bitmaps->GetBitmap("pref_paths.png"));
	if (config->Help.show_extra_panels) imglist->Add(bitmaps->GetBitmap("pref_help.png"));
	notebook->SetImageList(imglist);

	if (config->Help.show_extra_panels) 
		notebook->AddPage(CreateQuickHelpPanel(notebook), LANG(PREFERENCES_QUICK_HELP,"Ayuda Rapida"),false,8);
	notebook->AddPage(CreateGeneralPanel(notebook), LANG(PREFERENCES_GENERAL,"General"),false,0);
	notebook->AddPage(CreateCompilePanel(notebook), LANG(PREFERENCES_SIMPLE_PROGRAM,"Programa/Proyecto"),false,1);
	notebook->AddPage(CreateStylePanel(notebook), LANG(PREFERENCES_STYLE,"Estilo"),false,2);
	notebook->AddPage(CreateWritingPanels(notebook), LANG(PREFERENCES_ASSIST,"Asistencias"),false,3);
	notebook->AddPage(CreateSkinPanel(notebook), LANG(PREFERENCES_ICON_THEME,"Tema de Iconos"),false,4);
	notebook->AddPage(CreateToolbarsPanel(notebook), LANG(PREFERENCES_TOOLBARS,"Barras de Herram."),false,5);
	notebook->AddPage(CreateDebugPanels(notebook), LANG(PREFERENCES_DEBUGGING,"Depuración"),false,6);
	notebook->AddPage(CreatePathsPanels(notebook), LANG(PREFERENCES_PATHS,"Rutas"),false,7); 
	mySizer->Add(notebook,sizers->Exp1);
	
	ReuseSizer(this,mySizer)
		.BeginBottom().Help().Ok().Cancel().EndBottom(this)
		.SetAndFit();

	ignore_styles_changes=false;
	
	EnableOrDisableControls();
	SetFocus();
	Show();
}

wxPanel *mxPreferenceWindow::CreateGeneralPanel (mxBookCtrl *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	wxArrayString langs; mxUT::GetFilesFromDir(langs,"lang",true);
	for(unsigned int i=0;i<langs.GetCount();)
		if (langs[i].EndsWith(".pre")) { langs[i] = langs[i].BeforeFirst('.'); ++i; }
		else langs.RemoveAt(i,1);
	if (langs.Index("spanish")==wxNOT_FOUND) langs.Add("spanish");
	
	sizer.BeginCombo( LANG(PREFERENCES_GENERAL_GUI_LANGUAGE,"Idioma de la interfaz (*)") )
		.Bind(m_binder,config->Init.language_file).Add(langs).EndCombo();
	
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_SHOW_TOOLTIPS,"Mostrar sugerencias al inicio") )
		.Bind(m_binder,config->Init.show_tip_on_startup).EndCheck();
			
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_SHOW_WELCOME_PANEL,"Mostrar panel de bienvenida") )
		.Bind(m_binder,config->Init.show_welcome).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_SHOW_HELP_TABS,"Mostrar pestañas de ayuda en cuadro de diálogo (*)") )
		.Bind(m_binder,config->Help.show_extra_panels).EndCheck();
	
	sizer.BeginText( LANG(PREFERENCES_GENERAL_HISTORY_LEN,"Archivos en el historial") )
		.Bind(m_binder,config->Init.history_len).EndText();
	
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_SHOW_FILES_EXPLORER_ON_STARTUP,"Mostrar el explorador de archivos al iniciar") )
		.Bind(m_binder,config->Init.show_explorer_tree).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_GROUP_TREES,"Agrupar arboles en un solo panel (*)") )
		.Bind(m_binder,config->Init.left_panels).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_AUTOHIDE_PANELS,"Ocultar paneles automaticamente (*)") )
		.Bind(m_binder,config->Init.autohide_panels).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_SINGLETON,"Utilizar una sola instancia de ZinjaI al abrir archivos desde la linea de comandos") )
		.Bind(m_binder,config->Init.singleton).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_CHECK_FOR_UPDATES,"Verificar si existen nuevas versiones al iniciar") )
		.Bind(m_binder,config->Init.check_for_updates).EndCheck();
	
#ifdef __linux__
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_DISABLE_UBUNTU_TWEAKS,"Deshabilitar la interfaz de menúes y scrollbars especial de Unity (*)") )
		.Value(!wxFileExists(DIR_PLUS_FILE(config->config_dir,"ubuntu"))).EndCheck(init_disable_ubuntu_tweaks);
#endif
	
	sizer.BeginButton( LANG(PREFERENCES_CUSTOMIZE_SHORTCUTS,"Personalizar atajos de teclado...") )
		.Id(mxID_PREFERENCES_CUSTOMIZE_SHORTCUTS).EndButton();
	
	sizer.Spacer();
	sizer.BeginLabel( LANG(PREFERENCES_GENERAL_ASTERIX_WILL_APPLY_NEXT_TIME,"(*) tendrá efecto la proxima vez que inicie ZinjaI") )
		.Center().EndLabel();
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

wxPanel *mxPreferenceWindow::CreateQuickHelpPanel(mxBookCtrl *notebook) {
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	wxHtmlWindow *html = new wxHtmlWindow(panel,wxID_ANY);
	sizer->Add(html,sizers->Exp1);
	html->LoadFile(DIR_PLUS_FILE(config->Help.guihelp_dir,wxString(_T("prefs_help_"))<<config->Init.language_file<<_T(".html")));
	panel->SetSizerAndFit(sizer);
	return panel;	
}

wxNotebook *mxPreferenceWindow::CreateDebugPanels (mxBookCtrl *notebook) {
	return CreateNotebook(notebook)
		.AddPage(this,&mxPreferenceWindow::CreateDebugPanel1, LANG(PREFERENCES_DEBUG_1,"Interfaz"))
		.AddPage(this,&mxPreferenceWindow::CreateDebugPanel2, LANG(PREFERENCES_DEBUG_2,"Avanzado"))
		.EndNotebook();
}


wxPanel *mxPreferenceWindow::CreateDebugPanel1 (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
		
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_ALLOW_EDITION,"Permitir editar los fuentes durante la depuración") )
		.Bind(m_binder,config->Debug.allow_edition).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_REORGANIZE_PANELS,"Reacomodar los paneles al iniciar/finalizar la depuración") )
		.Bind(m_binder,config->Debug.autohide_panels).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_INSPECTIONS_PANEL_ON_RIGHT,"Colocar el panel de inspecciones a la derecha (*)") )
		.Bind(m_binder,config->Debug.inspections_on_right).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_SHOW_THREAD_LIST,"Mostrar lista de hilos de ejecución") )
		.Bind(m_binder,config->Debug.show_thread_panel).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_SHOW_DEBUGGER_LOG,"Mostrar el panel de mensajes del depurador") )
		.Bind(m_binder,config->Debug.show_log_panel).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_ORGANIZE_TOOLBARS,"Reacomodar las barras de herramienta al iniciar/finalizar la depuración") )
		.Bind(m_binder,config->Debug.autohide_toolbars).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_ACTIVATE_WINDOW_ON_INTERRUPTION,"Mostrar ZinjaI cuando se interrumpe la ejecución") )
		.Bind(m_binder,config->Debug.raise_main_window).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_RETURN_FOCUS_ON_CONTINUE,"Devolver el foco a la aplicación en depuración luego de una pausa") )
		.Bind(m_binder,config->Debug.return_focus_on_continue).EndCheck();
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

wxPanel *mxPreferenceWindow::CreateDebugPanel2 (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
		
#ifdef __linux__
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_ENABLE_CORE_DUMP,"Habilitar volcado de memoria al ejecutar sin depurador") )
		.Bind(m_binder,config->Debug.enable_core_dump).EndCheck();
#endif
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_LOAD_ALL_DEBUG_INFO,"Cargar toda la información de depuracion antes de comenzar") )
		.Bind(m_binder,config->Debug.readnow).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_LOAD_SHARED_LIBS_INFO,"Cargar información de depuracion de bibliotecas externas") )
		.Bind(m_binder,config->Debug.auto_solibs).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_RECOMPILE_AUTOMATICALLY,"Recompilar automaticamente antes de depurar si es necesario") )
		.Bind(m_binder,config->Debug.compile_again).EndCheck();
	
//	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_STOP_DEBBUGING_ON_ABNORMAL_TERMINATION,"Salir del modo depuración si el programa finaliza normalmente") )
//		.Bind(m_binder,config->Debug.close_on_normal_exit).EndCheck();

	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_ALWAYS_RUN_IN_DEBUGGER,"Siempre ejecutar en el depurador") )
		.Bind(m_binder,config->Debug.always_debug).EndCheck();
	
//	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_USE_COLOURS_FOR_INSPECTIONS,"Utilizar colores en la tabla de inspecciones") )
//		.Bind(m_binder,config->Debug.use_colours_for_inspections).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_INSPECTIONS_CAN_HAVE_SIDE_EFFECTS,"Considerar side-effects al evaluar inspecciones") )
		.Bind(m_binder,config->Debug.inspections_can_have_side_effects).EndCheck();
	
	sizer.BeginLine()
		.BeginCheck( LANG(PREFERENCES_DEBUG_IMPROVE_INSPECTIONS_BY_TYPE,"Mejorar inspecciones automáticamente segun tipo") )
			.Bind(m_binder,config->Debug.improve_inspections_by_type).EndCheck()
		.Space(15)
		.BeginButton( LANG(PREFERENCES_DEBUG_IMPROVE_INSPECTIONS_SETTINGS,"Configurar...") )
			.Id(mxID_DEBUG_IMPROVE_INSPECTIONS_BY_TYPE).EndButton()
		.EndLine();
	
	sizer.BeginText( LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de definiciones de macros para gdb") )
		.Bind(m_binder,config->Debug.macros_file).Button(mxID_DEBUG_MACROS).EndText(debug_macros_file);
	
	sizer.BeginLine()
		.BeginCheck( LANG(PREFERENCES_DEBUG_USE_BLACKLIST,"Utilizar lista negra en el paso a paso") )
			.Bind(m_binder,config->Debug.use_blacklist).EndCheck()
		.Space(15)
		.BeginButton( LANG(PREFERENCES_DEBUG_BLACKLIST_BUTTON,"Configurar...") )
			.Id(mxID_DEBUG_BLACKLIST).EndButton()
		.EndLine();
	
#ifdef __WIN32__
	sizer.BeginCheck( LANG(PREFERENCES_DEBUG_NO_DEBUG_HEAP,"Deshabilitar heap especial de widondows para deburación") )
		.Bind(m_binder,config->Debug.no_debug_heap).EndCheck();
#endif
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

wxPanel *mxPreferenceWindow::CreateToolbarsPanel (mxBookCtrl *notebook) {
	CreatePanelAndSizer sizer(notebook); panel_toolbars = sizer.GetPanel();
	sizer.BeginLabel(  LANG(PREFERENCES_TOOLBARS_WICH,"Barra de herramientas a utilizar:") ).EndLabel();
	
	wxBoxSizer *tbs_sizer = new wxBoxSizer(wxVERTICAL);

	wxArrayString poss; poss.Add(LANG(PREFERENCES_TOOLBARS_DOCK_TOP,"Arriba")); poss.Add(LANG(PREFERENCES_TOOLBARS_DOCK_LEFT,"Izquierda")); poss.Add(LANG(PREFERENCES_TOOLBARS_DOCK_RIGHT,"Derecha")); poss.Add(LANG(PREFERENCES_TOOLBARS_FLOAT,"Flotante"));
#define _aux_ctp_1(name,ID,label) { \
	MenusAndToolsConfig::toolbarPosition &position = menu_data->GetToolbarPosition(MenusAndToolsConfig::tb##ID); \
	wxBoxSizer *sz= new wxBoxSizer(wxHORIZONTAL); \
	sz->Add(20,1,0); \
	toolbars_wich_##name = new wxCheckBox(panel_toolbars,wxID_ANY,label); \
	toolbars_wich_##name->SetValue(position.visible); \
	wx_toolbars_widgets.Add(toolbars_wich_##name); \
	wxButton *bt = new wxButton(panel_toolbars,mxID_PREFERENCES_TOOLBAR_##ID,LANG(PREFERENCES_TOOLBARS_MODIFY,"Modificar...")); \
	sz->Add(toolbars_wich_##name,sizers->BA5_Center); sz->AddStretchSpacer(); sz->Add(bt,sizers->BLR10); \
	toolbars_side_##name = new wxComboBox(panel_toolbars,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,poss,wxCB_READONLY); \
	toolbars_side_##name->SetSelection(position.top?0:(position.left?1:(position.right?2:3))); \
	wx_toolbars_widgets.Add(toolbars_side_##name); \
	sz->Add(new wxStaticText(panel_toolbars,wxID_ANY,"Ubicación:"), sizers->BA5_Center); \
	sz->Add(toolbars_side_##name,sizers->BA5_Center); \
	tbs_sizer->Add(sz,sizers->Exp0); }
	
	_aux_ctp_1(file,FILE,LANG(CAPTION_TOOLBAR_FILE,"Archivo"));
	_aux_ctp_1(edit,EDIT,LANG(CAPTION_TOOLBAR_EDIT,"Editar"));
	_aux_ctp_1(view,VIEW,LANG(CAPTION_TOOLBAR_VIEW,"Ver"));
	_aux_ctp_1(run,RUN,LANG(CAPTION_TOOLBAR_RUN,"Ejecución"));
	_aux_ctp_1(debug,DEBUG,LANG(CAPTION_TOOLBAR_DEBUG,"Depuración"));
	_aux_ctp_1(tools,TOOLS,LANG(CAPTION_TOOLBAR_TOOLS,"Herramientas"));
	_aux_ctp_1(misc,MISC,LANG(CAPTION_TOOLBAR_MISC,"Misceánea"));
	
	toolbars_wich_find = new wxCheckBox(panel_toolbars,wxID_ANY,LANG(CAPTION_TOOLBAR_FIND,"Busqueda"));
	toolbars_wich_find->SetValue(_toolbar_visible(tbFIND));
	wxBoxSizer *szFind = new wxBoxSizer(wxHORIZONTAL);
	szFind->Add(20,1,0);
	szFind->Add(toolbars_wich_find,sizers->BA5_Center);
	tbs_sizer->Add(szFind/*,sizers->BA5*/);
	
	toolbars_wich_project = new wxCheckBox(panel_toolbars,wxID_ANY,LANG(CAPTION_TOOLBAR_PROJECT,"Proyecto"));
	toolbars_wich_project->SetValue(_toolbar_visible(tbPROJECT));
	wxBoxSizer *szProject = new wxBoxSizer(wxHORIZONTAL);
	szProject->Add(20,1,0);
	szProject->Add(toolbars_wich_project,sizers->BA5_Center);
	tbs_sizer->Add(szProject/*,sizers->BA5*/);
	
	sizer.GetSizer()->Add(tbs_sizer);
	
	sizer.BeginButton( LANG(PREFERENCES_TOOLBARS_RESET,"Reestablecer configuración por defecto") )
		.Id(mxID_PREFERENCES_TOOLBAR_RESET).RegisterIn(wx_toolbars_widgets).EndButton();
	
	wxArrayString icon_sizes;
	wxString icsz = wxString()<<menu_data->icon_size<<"x"<<menu_data->icon_size;
	
	sizer.BeginCombo( LANG(PREFERENCES_TOOLBAR_ICON_SIZE,"Tamaño de icono") )
		.Add("16x16")
		.AddIf(wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"24")),"24x24")
		.AddIf(wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"32")),"32x32")
		.AddIf(wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"48")),"48x48")
		.AddIf(wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"64")),"64x64")
		.Add(icon_sizes).Select(icsz,0).RegisterIn(wx_toolbars_widgets).EndCombo(toolbar_icon_size);
	
	wx_toolbars_widgets.Add(toolbars_wich_find);
	wx_toolbars_widgets.Add(toolbars_wich_project);
	
	sizer.SetAndFit();
	return panel_toolbars;
}

wxNotebook *mxPreferenceWindow::CreateCompilePanel(mxBookCtrl *notebook) {
	return CreateNotebook(notebook)
		.AddPage(this,&mxPreferenceWindow::CreateCompilePanelSimple, LANG(PREFERENCES_SIMPLE,"Programa simple") )
		.AddPage(this,&mxPreferenceWindow::CreateCompilePanelProject, LANG(PREFERENCES_PROJECT,"Proyecto") )
		.EndNotebook();
}

wxPanel *mxPreferenceWindow::CreateCompilePanelProject (wxBookCtrl *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	sizer.BeginText( LANG(PREFERENCES_GENERAL_MAX_JOBS,"Cantidad de pasos en paralelo al compilar") )
		.Bind(m_binder,config->Init.max_jobs).Button(mxID_MAX_JOBS).EndText();
	
	
	sizer.BeginCheck( LANG(PREFERENCES_STOP_COMPILING_ON_ERROR,"Detener la compilación de un proyecto al encontrar el primer error") )
		.Bind(m_binder,config->Init.stop_compiling_on_error).EndCheck();
		
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_SAVE_PROJECT_ON_CLOSE,"Guardar siempre el proyeto al salir") )
		.Bind(m_binder,config->Init.save_project).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_SHOW_FILES_EXPLORER_ON_PROJECT,"Mostrar el explorador de archivos al abrir un proyecto") )
		 .Bind(m_binder,config->Init.prefer_explorer_tree).EndCheck();

	sizer.BeginLine()
		.BeginCheck( LANG(PREFERENCES_USE_CACHE_FOR_SUBCMD,"Usar cache para la ejecución de subcomandos") )
			.Bind(m_binder,config->Init.use_cache_for_subcommands).EndCheck()
		.Space(15)
		.BeginButton( LANG(PREFERENCES_USE_CLEAR_SUBCMD_CACHE,"Limpiar") )
			.Id(mxID_PREFERENCES_CLEAR_SUBCMD_CACHE).EndButton()
		.EndLine();
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

wxPanel *mxPreferenceWindow::CreateCompilePanelSimple (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	sizer.BeginCombo( LANG(PREFERENCES_SIMPLE_NEW_ACTION,"Accion para Nuevo Archivo") )
		.Add(LANG(PREFERENCES_SIMPLE_CREATE_EMPTY_FILE,"Crear archivo en blanco"))
		.Add(LANG(PREFERENCES_SIMPLE_CREATE_FROM_TEMPLATE,"Crear a partir de plantilla"))
		.Add(LANG(PREFERENCES_SIMPLE_SHOW_WIZARD,"Mostrar Asistente"))
		.Bind(m_binder,config->Init.new_file).EndCombo();
	
	SimpleTemplates::Initialize(); // ensures g_templates!=nullptr
	wxArrayString cpp_templates_names, cpp_templates_files;
	g_templates->GetNamesList(cpp_templates_names,false,true);
	g_templates->GetFilesList(cpp_templates_files,false,true);
	wxArrayString c_templates_names, c_templates_files;
	g_templates->GetNamesList(c_templates_names,true,false);
	g_templates->GetFilesList(c_templates_files,true,false);
	
	sizer.BeginCombo( LANG(PREFERENCES_SIMPLE_DEFAULT_CPP_TEMPLATE,"Plantilla por defecto para abrir fuentes C++") )
		.Add(cpp_templates_names).Select(cpp_templates_files.Index(config->Files.cpp_template))
		.EndCombo(simple_default_cpp_template);
	
	sizer.BeginText( LANG(PREFERENCES_SIMPLE_EXTRA_CPP_COMPILER_ARGUMENTS,"Parámetros adicionales para el compilador C++") )
		.Bind(m_binder,config->Running.cpp_compiler_options).EndText();
	
	sizer.BeginCombo( LANG(PREFERENCES_SIMPLE_DEFAULT_C_TEMPLATE,"Plantilla por defecto para abrir fuentes C") )
		.Add(c_templates_names).Select(c_templates_files.Index(config->Files.c_template))
		.EndCombo(simple_default_c_template);
	
	sizer.BeginText( LANG(PREFERENCES_SIMPLE_EXTRA_C_COMPILER_ARGUMENTS,"Parámetros adicionales para el compilador C") )
		.Bind(m_binder,config->Running.c_compiler_options).EndText();
	
	sizer.BeginCheck( LANG(PREFERENCES_SIMPLE_WAIT_KEY_AFTER_RUNNING,"Esperar una tecla luego de la ejecución") )
		.Bind(m_binder,config->Running.wait_for_key).EndCheck();
	sizer.BeginCheck( LANG(PREFERENCES_SIMPLE_ALWAYS_ASK_ARGS,"Siempre pedir argumentos al ejecutar") )
		.Bind(m_binder,config->Running.always_ask_args).EndCheck();
	sizer.BeginCheck( LANG(PREFERENCES_SIMPLE_ADD_CPP_EXTENSION,"Agregar la extension cpp si se omite al guardar") )
		.Bind(m_binder,config->Init.always_add_extension).EndCheck();
	
	wxArrayString tc_array; Toolchain::GetNames(tc_array,false);	
	sizer.BeginLine()
		.BeginCombo( LANG(PREFERENCES_TOOLCHAIN,"Herramientas de compilación") )
			.Add(tc_array).Bind(m_binder,config->Files.toolchain).EndCombo(files_toolchain)
		.BeginButton("...").Id(mxID_PREFERENCES_TOOLCHAIN_OPTIONS).EndButton()
		.EndLine();
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

wxPanel *mxPreferenceWindow::CreateStylePanel (mxBookCtrl *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	sizer.BeginLine()
		.BeginCheck( LANG(PREFERENCES_STYLE_SINTAX_HIGHLIGHT,"Colorear sintaxis") )
			.Bind(m_binder,config->Source.syntaxEnable).EndCheck()
		.Space(15)
		.BeginButton( LANG(PREFERENCES_STYLE_DEFINE_COLOURS,"Definir colores...") )
			.Id(mxID_COLORS_PICKER).EndButton()
		.EndLine();
	
	sizer.BeginCombo( LANG(PREFERENCES_STYLE_LINE_WRAP,"Ajuste de linea dinámico") )
		.Add(LANG(PREFERENCES_STYLE_WRAP_NONE,"Nunca"))
		.Add(LANG(PREFERENCES_STYLE_WRAP_ALL_BUT_SOURCES,"Todos menos fuentes"))
		.Add(LANG(PREFERENCES_STYLE_WRAP_ALWAYS,"Siempre"))
		.Bind(m_binder,config->Init.wrap_mode).EndCombo();

	sizer.BeginCheck( LANG(PREFERENCES_STYLE_SHOW_EOF_AND_WHITE_SPACES,"Mostrar espacios y caracteres de fin de linea") )
		.Bind(m_binder,config->Source.whiteSpace).EndCheck();
		
	sizer.BeginCheck( LANG(PREFERENCES_STYLE_SHOW_LINE_NUMBERS,"Mostrar numeros de linea") )
		.Bind(m_binder,config->Source.lineNumber).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_STYLE_ENABLE_CODE_FOLDING,"Habilitar plegado de código") )
		.Bind(m_binder,config->Source.foldEnable).EndCheck();
	
	sizer.BeginLine()
		.BeginText( LANG(PREFERENCES_STYLE_TAB_WIDTH,"Ancho del tabulado") )
			.Bind(m_binder,config->Source.tabWidth).EndText()
		.Space(15)
		.BeginCheck( LANG(PREFERENCES_STYLE_SPACES_INTEAD_TABS,"Colocar espacios en lugar de tabs") )
			.Bind(m_binder,config->Source.tabUseSpaces).EndCheck()
		.EndLine();
	
	sizer.BeginLine()
		.BeginCheck( LANG(PREFERENCES_STYLE_EDGE_COLUM,"Mostrar linea guía en la columna") )
			.Value(config->Source.edgeColumn>0).EndCheck(source_edgeColumnCheck)
		.BeginText(" ").Value(abs(config->Source.edgeColumn)).EndText(source_edgeColumnPos)
		.EndLine();
	
	class mxFontEnumerator: public wxFontEnumerator {
		wxArrayString *array;
	public:
		mxFontEnumerator(wxArrayString &the_array):array(&the_array){ 
			wxFontEnumerator::EnumerateFacenames(wxFONTENCODING_SYSTEM,true);
		}
		bool OnFacename(const wxString& font) {
			array->Add(font);
			return true;
		}
	};
	wxArrayString fonts; mxFontEnumerator f(fonts); fonts.Sort(); 
//	int def_font=fonts.Index(config->Styles.font_name,false); if (def_font==wxNOT_FOUND) def_font=-1;
	sizer.BeginCombo( LANG(PREFERENCES_STYLE_FONT_NAME,"Fuente para el código") )
		.Add(fonts).Bind(m_binder,config->Styles.font_name,-1).Id(mxID_PREFERENCES_FONTNAME).EndCombo(styles_font_name);
	
	sizer.BeginLine()
		.BeginText( LANG(PREFERENCES_STYLE_SCREEN_FONT_SIZE,"Tamaño de la fuente en pantalla") )
			.Bind(m_binder,config->Styles.font_size).Id(mxID_PREFERENCES_FONTSIZE).EndText(styles_font_size)
		.Space(15)
		.BeginText( LANG(PREFERENCES_STYLE_PRINTING_FONT_SIZE,"para impresion") )
			.Bind(m_binder,config->Styles.print_size).EndText()
		.EndLine();
	
	sizer.BeginCheck( LANG(PREFERENCES_STYLE_HIDE_MENUS_ON_FULLSCREEN,"Ocultar barra de menues al pasar a pantalla completa") )
		.Bind(m_binder,config->Init.autohide_menus_fs).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_STYLE_HIDE_TOOLBARS_ON_FULLSCREEN,"Ocultar barras de herramientas al pasar a pantalla completa") )
		.Bind(m_binder,config->Init.autohide_toolbars_fs).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_STYLE_HIDE_PANELS_ON_FULLSCREEN,"Ocultar paneles al pasar a pantalla completa") )
		.Bind(m_binder,config->Init.autohide_panels_fs).EndCheck();
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

wxNotebook *mxPreferenceWindow::CreateWritingPanels (mxBookCtrl *notebook) {
	return CreateNotebook(notebook)
		.AddPage(this,&mxPreferenceWindow::CreateWritingPanel1, LANG(PREFERENCES_ASSIST_1,"Generales") )
		.AddPage(this,&mxPreferenceWindow::CreateWritingPanel2, LANG(PREFERENCES_ASSIST_2,"Autocompletado") )
		.EndNotebook();
}

wxPanel *mxPreferenceWindow::CreateWritingPanel1 (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_INTELLIGENT_EDITING,"Edición inteligente") )
		.Bind(m_binder,config->Source.smartIndent).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_AUTOCLOSE_BRACKETS,"Cerrar llaves automáticamente al presionar Enter") )
		.Bind(m_binder,config->Source.bracketInsertion).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_AUTOCLOSE_STUFF,"Siempre cerrar llaves, paréntesis y comillas automáticamente") )
		.Bind(m_binder,config->Source.autocloseStuff).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_INDENT_ON_PASTE,"Corregir indentado al pegar") )
		.Bind(m_binder,config->Source.indentPaste).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_CHECK_FOR_EMPTY_LAST_LINE,"Controlar que quede una linea en blanco al final de cada archivo") )
		.Bind(m_binder,config->Source.avoidNoNewLineWarning).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_SHOW_TOOLTIPS_FOR_VAR_TYPES,"Utilizar tooltips para identificar tipos de variables") )
		.Bind(m_binder,config->Source.toolTips).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_BEAUTIFY_COMPILER_ERRORS,"Simplificar mensajes de error del compilador") )
		.Bind(m_binder,config->Init.beautify_compiler_errors).EndCheck();
	
	sizer.BeginText( LANG(PREFERENCES_WRITTING_AUTOCODES_FILE,"Archivo de definiciones de plantillas de auto-código") )
		.Bind(m_binder,config->Files.autocodes_file).Button(mxID_AUTOCODES_FILE).EndText(files_autocode);

#ifndef __WIN32__
	sizer.BeginCheck( LANG(PREFERENCES_GENERAL_SPANISH_COMPILER_OUTPUT,"Mostrar errores de compilación en Español (Ver Ayuda!) (*)") )
		.Bind(m_binder,config->Init.lang_es).EndCheck();
#endif
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

wxPanel *mxPreferenceWindow::CreateWritingPanel2 (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook); wxPanel *panel = sizer.GetPanel();
	
	sizer.BeginCombo( LANG(PREFERENCES_WRITING_AUTOCOMPLETION,"Autocompletado") )
		.Add(LANG(PREFERENCES_WRITTING_AUTOCOMP_NONE,"Deshabilitado"))
		.Add(LANG(PREFERENCES_WRITTING_AUTOCOMP_START,"Habilitado, por comienzo de palabra"))
		.Add(LANG(PREFERENCES_WRITTING_AUTOCOMP_FIND,"Habilitado, por cualquier parte de la palabra"))
		.Add(LANG(PREFERENCES_WRITTING_AUTOCOMP_FUZZY,"Habilitado, por similaridad"))
		.Bind(m_binder,config->Source.autoCompletion).EndCombo();
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_AUTOCOM_FILTERS,"Filtrar resultados de autocompletado al continuar escribiendo") )
		.Bind(m_binder,config->Source.autocompFilters).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_ENABLE_AUTOCOMP_CALLTIPS,"Mostrar info. adicional junto al menú de autocompletado") )
		.Bind(m_binder,config->Source.autocompTips).EndCheck();
	
	sizer.BeginCheck( LANG(PREFERENCES_WRITING_ENABLE_CALLTIPS,"Mostrar ayuda de llamadas a funciones y métodos") )
		.Bind(m_binder,config->Source.callTips).EndCheck();
	
	sizer.Add(new wxStaticText(panel, wxID_ANY, LANG(PREFERENCES_WRITING_AUTOCOMPLETION_INDEXES,"Índices de autocompletado a utilizar"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
	help_autocomp_indexes = new wxCheckListBox(panel,mxID_AUTOCOMP_LIST,wxDefaultPosition,wxSize(100,100),0,nullptr,wxLB_SORT);
	
	wxArrayString autocomp_array_all, autocomp_array_user;
	mxUT::GetFilesFromBothDirs(autocomp_array_all,"autocomp");
	mxUT::Split(config->Help.autocomp_indexes,autocomp_array_user);
	mxUT::Unique(autocomp_array_user,true);
	for (unsigned int i=0;i<autocomp_array_all.GetCount();i++) {
		int n=help_autocomp_indexes->Append(autocomp_array_all[i]);
		if (autocomp_array_user.Index(autocomp_array_all[i])!=wxNOT_FOUND) 
			help_autocomp_indexes->Check(n,true);
	}
	sizer.Add(help_autocomp_indexes,sizers->BA5_DL_Exp1);

	sizer.SetAndFit();
	return sizer.GetPanel();
}


wxPanel *mxPreferenceWindow::CreateSkinPanel (mxBookCtrl *notebook) {
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	sizer->Add(new wxStaticText(panel, wxID_ANY, LANG(PREFERENCES_SKIN_AVAILABLE_THEMES,"Temas disponibles"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
	skin_list = new wxListBox(panel,mxID_SKIN_LIST,wxDefaultPosition,wxSize(100,100),0,nullptr,0);
	wxDir dir(_T("skins"));
	if ( dir.IsOpened() ) {
		wxArrayString autocomp_array;
		mxUT::Split(config->Help.autocomp_indexes,autocomp_array);
		wxString filename;
		wxString spec;
		bool cont = dir.GetFirst(&filename, spec , wxDIR_DIRS);
		skin_list->Append(LANG(PREFERENCES_SKIN_DEFAULT_THEME,"<Tema por defecto>"));
		skin_paths.Add(_T("imgs"));
		skin_list->Select(0);
		while ( cont ) {
			if (wxFileName::FileExists(DIR_PLUS_FILE_2("skins",filename,"descripcion.txt"))) {
				skin_list->Append(filename);
				filename = DIR_PLUS_FILE("skins",filename);
				skin_paths.Add(filename);
				if (filename==config->Files.skin_dir)
					skin_list->SetSelection(skin_list->GetCount()-1);
			}
			cont = dir.GetNext(&filename);
		}	
	}
	
	sizer->Add(skin_list,sizers->BA5_DL_Exp1);

	skin_text = new wxTextCtrl(panel,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_READONLY|wxTE_MULTILINE);
	sizer->Add(skin_text,sizers->BA10_Exp0);
	if (bitmaps->HasBitmap("skin_preview.png",true))
		skin_image = new wxStaticBitmap(panel,wxID_ANY,bitmaps->GetBitmap("skin_preview.png",true));
	else
		skin_image = new wxStaticBitmap(panel,wxID_ANY,bitmaps->GetBitmap("skin_no_preview.png"));
	sizer->Add(skin_image,sizers->BA10_Exp0);
	wxButton *apply_button = new mxBitmapButton(panel,mxID_SKIN_APPLY,bitmaps->buttons.ok,LANG(PREFERENCES_SKIN_APPLY,"Aplicar"));
	sizer->Add(apply_button,sizers->BA10_Exp0_Right);
	
	wxCommandEvent ce;
	OnSkinList(ce);
	
	panel->SetSizerAndFit(sizer);
	return panel;
}

wxNotebook *mxPreferenceWindow::CreatePathsPanels (mxBookCtrl *notebook) {
	return CreateNotebook(notebook)
		.AddPage(this,&mxPreferenceWindow::CreatePathsPanel1, LANG(PREFERENCES_PATHS_1,"Rutas 1") )
		.AddPage(this,&mxPreferenceWindow::CreatePathsPanel2, LANG(PREFERENCES_PATHS_1,"Rutas 2") )
		.EndNotebook();
}

wxPanel *mxPreferenceWindow::CreatePathsPanel1 (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);

	sizer.BeginText( LANG(PREFERENCES_PATHS_TEMP,"Directorio temporal") )
		.Bind(m_binder,config->Files.temp_dir).Button(mxID_TEMP_FOLDER).EndText(files_temp_dir);

	sizer.BeginText( LANG(PREFERENCES_PATHS_PROJECTS,"Directorio de proyectos") )
		.Bind(m_binder,config->Files.project_folder).Button(mxID_PROJECTS_FOLDER).EndText(files_project_folder);
	
	sizer.BeginText( LANG(PREFERENCES_PATHS_WXWIDGETS_HELP_INDEX,"Indice de ayuda wxWidgets") )
		.Bind(m_binder,config->Help.wxhelp_index).Button(mxID_WXHELP_FOLDER).EndText(help_wxhelp_index);
	
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_GDB,"Comando del depurador") )
		.Bind(m_binder,config->Files.debugger_command).Button(mxID_GDB_PATH).EndText(files_debugger_command);
	
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_CONSOLE,"Comando de la terminal") )
		.Bind(m_binder,config->Files.terminal_command).Button(mxID_TERMINALS_BUTTON).EndText(files_terminal_command);
	
#ifndef __WIN32__
	sizer.BeginButton( LANG(PREFERENCES_CREATE_ICONS,"Crear/Actualizar accesos directos en el escritorio/menú del sistema...") )
		.Id(mxID_PREFERENCES_XDG).EndButton();
#endif
	
	sizer.Spacer();
	sizer.BeginLabel( LANG(PREFERENCES_NOTE_GCC_PATHS,""
						   "Nota: La configuración de las rutas relacionadas al compilador\n"
						   "se realiza desde la pestaña \"Programa/Proyecto\" utilizando\n"
						   "el botón \"...\" de la opción \"Herramientas de compilación\".") ).Center().EndLabel();
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

wxPanel *mxPreferenceWindow::CreatePathsPanel2 (wxNotebook *notebook) {
	CreatePanelAndSizer sizer(notebook);
	
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_EXPLORER,"Comando del explorador de archivos") )
		.Bind(m_binder,config->Files.explorer_command).Button(mxID_EXPLORERS_BUTTON).EndText(files_explorer_command);
	
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_IMAGE_VIEWER,"Comando del visor de imagenes") )
		.Bind(m_binder,config->Files.img_viewer).Button(mxID_IMG_VIEWER_PATH).EndText(files_img_viewer_command);
	
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_BROWSER,"Comando del navegador Web") )
		.Bind(m_binder,config->Files.browser_command).Button(mxID_BROWSER_PATH).EndText(files_browser_command);
	
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_XDOT_COMMAND,"Comando del visor de grafos (xdot)") )
		.Bind(m_binder,config->Files.xdot_command).Button(mxID_XDOT_PATH).EndText(files_xdot_command);
	
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_DOXYGEN,"Ubicación del ejecutable de Doxygen") )
		.Bind(m_binder,config->Files.doxygen_command).Button(mxID_DOXYGEN_PATH).EndText(files_doxygen_command);	
	
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_WXFORMBUILDER,"Ubicación del ejecutable de wxFormBuilder") )
		.Bind(m_binder,config->Files.wxfb_command).Button(mxID_WXFB_PATH).EndText(files_wxfb_command);

#ifdef __WIN32__
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_VALGRIND,"Ubicación del ejecutable de Valgrind") )
		.Bind(m_binder,config->Files.valgrind_command).Button(mxID_VALGRIND_PATH).EndText(files_valgrind_command);
#endif
	
	sizer.BeginText( LANG(PREFERENCES_COMMANDS_CPPCHECK,"Ubicación del ejecutable de CppCheck") )
		.Bind(m_binder,config->Files.cppcheck_command).Button(mxID_CPPCHECK_PATH).EndText(files_cppcheck_command);
	
	sizer.SetAndFit();
	return sizer.GetPanel();
}

static void RecreateAllToolbars ( ) {
	for(int tb_id=0;tb_id<MenusAndToolsConfig::tbPROJECT;tb_id++)
		menu_data->UpdateToolbar(tb_id,true);
	if (project) menu_data->UpdateToolbar(MenusAndToolsConfig::tbPROJECT,true);
}

void mxPreferenceWindow::OnOkButton(wxCommandEvent &event) {
	
	if (Toolchain::GetToolchain(files_toolchain->GetValue())->IsExtern()) {
		mxMessageDialog(this,LANG(PREFERENCES_CANNOT_DEFAULT_EXTERN_TOOLCHAIN,""
								  "La herramienta de compilación por defecto (pestaña\n"
								  "Programa/Proyecto) no puede ser de tipo externa"))
			.Title(LANG(PREFERENCES_CAPTION,"Preferencias")).IconWarning().Run();
		return;
	}
	
	m_binder.FromWidgets();
	
#ifdef __linux__
	if ( init_disable_ubuntu_tweaks->GetValue() == wxFileExists(DIR_PLUS_FILE(config->config_dir,"ubuntu")) ) {
		if (init_disable_ubuntu_tweaks->GetValue()) wxRemoveFile( DIR_PLUS_FILE(config->config_dir,"ubuntu") );
		else { wxTextFile fil( DIR_PLUS_FILE(config->config_dir,"ubuntu") ); fil.Create(); fil.Write(); }
	}
#endif
	
	long int l;
	if (config->Init.show_welcome && !g_welcome_panel) {
		g_welcome_panel=new mxWelcomePanel(main_window);
		main_window->aui_manager.AddPane(g_welcome_panel, wxAuiPaneInfo().Name("g_welcome_panel").CenterPane().PaneBorder(false).Hide());
		if (!project && !main_window->notebook_sources->GetPageCount()) main_window->ShowWelcome(true);
	} else if (!config->Init.show_welcome && g_welcome_panel) {
		main_window->ShowWelcome(false);
		main_window->aui_manager.DetachPane(g_welcome_panel);
		g_welcome_panel->Destroy();
		g_welcome_panel=nullptr;
	}
	wxSetEnv("LANG",(config->Init.lang_es?"es_ES":"en_US"));
	if (config->Init.history_len>30) config->Init.history_len=30;
	else if (config->Init.history_len<5) config->Init.history_len=5;
	if (config->Init.max_jobs<1) config->Init.max_jobs=1;
	
	wxArrayString cpp_templates; g_templates->GetFilesList(cpp_templates,false,true);
	wxArrayString c_templates; g_templates->GetFilesList(c_templates,true,false);
	int cpp_idx = simple_default_cpp_template->GetSelection(), c_idx = simple_default_c_template->GetSelection();
	if (cpp_idx!=wxNOT_FOUND) config->Files.cpp_template = cpp_templates[cpp_idx];
	if (c_idx!=wxNOT_FOUND) config->Files.c_template = c_templates[c_idx];
		
	if (!config->Init.singleton && g_singleton->IsRunning()) g_singleton->Stop();
	else if (config->Init.singleton && !project && !g_singleton->IsRunning()) g_singleton->Start();
	if (source_edgeColumnPos->GetValue().ToLong(&l)) config->Source.edgeColumn=l;
	if (!source_edgeColumnCheck->GetValue()) config->Source.edgeColumn = -config->Source.edgeColumn;
	if (!config->Source.syntaxEnable) config->Source.smartIndent = false;
	if(!config->Source.smartIndent) config->Source.bracketInsertion =  config->Source.indentPaste = false;
	g_code_helper->SetAutocompletionMatchingMode(config->Source.autoCompletion);
	
	wxString autocomp_indexes;
	for (unsigned int i=0;i<help_autocomp_indexes->GetCount();i++) {
		if (help_autocomp_indexes->IsChecked(i)) {
			wxString index_string=help_autocomp_indexes->GetString(i);
			if (index_string.Contains(" "))
				index_string=wxString("\"")<<index_string<<"\"";
			if (autocomp_indexes.Len())
				autocomp_indexes<<","<<index_string;
			else
				autocomp_indexes = help_autocomp_indexes->GetString(i);
		}
	}
	if (config->Help.autocomp_indexes != autocomp_indexes) {
		config->Help.autocomp_indexes = autocomp_indexes;
		if (project) autocomp_indexes<<" "<<project->autocomp_extra;
		g_code_helper->ReloadIndexes(autocomp_indexes);
	}
	
	wxAuiNotebook *ns=main_window->notebook_sources;
	for (unsigned int i=0;i<ns->GetPageCount();i++)
		((mxSource*)(ns->GetPage(i)))->LoadSourceConfig();
	main_window->SetStatusBarFields(); // ocultar/mostrar nro de linea
		
	if (debug->IsDebugging()) {
		for (unsigned int i=0;i<ns->GetPageCount();i++)
			((mxSource*)(ns->GetPage(i)))->SetReadOnlyMode(config->Debug.allow_edition?ROM_DEBUG:ROM_ADD_DEBUG);
	}
	g_autocoder->Reset(project?project->autocodes_file:"");
	config->Debug.blacklist = temp_debug_blacklist;
	debug->SetBlacklist();
	
	s_old_config_styles=config->Styles;
	config->Save();
	
	bool toolbar_changed=false;
#define _update_toolbar_visibility_0(NAME,name) \
	if (toolbars_wich_##name->GetValue()!=_toolbar_visible(tb##NAME)) { toolbar_changed=true; \
	main_window->OnToggleToolbar(mxID_VIEW_TOOLBAR_##NAME,MenusAndToolsConfig::tb##NAME,false); }
#define _update_toolbar_visibility(NAME,name) _update_toolbar_visibility_0(NAME,name); \
	{ int s=toolbars_side_##name->GetSelection(); \
	MenusAndToolsConfig::toolbarPosition &position = menu_data->GetToolbarPosition(MenusAndToolsConfig::tb##NAME); \
	if ( (s==0&&!position.top) || (s==1&&!position.left) || (s==2&&!position.right) || (s==3&&(position.right||position.left||position.top)) ) { \
		if (s==3) { position.right=false; position.left=position.top=false; } \
		else if (s==2) { position.right=true; position.left=position.top=false; } \
		else if (s==1) { position.left=true; position.top=position.right=false; } \
		else { position.top=true; position.left=position.right=false; } \
		menu_data->UpdateToolbar(MenusAndToolsConfig::tb##NAME,false); toolbar_changed=true; } \
	}
	_update_toolbar_visibility(FILE,file);
	_update_toolbar_visibility(EDIT,edit);
	_update_toolbar_visibility(VIEW,view);
	_update_toolbar_visibility(RUN,run);
	_update_toolbar_visibility(DEBUG,debug);
	_update_toolbar_visibility(TOOLS,tools);
	_update_toolbar_visibility(MISC,misc);
	_update_toolbar_visibility_0(FIND,find);
	if (project) { _update_toolbar_visibility_0(PROJECT,project); } else _toolbar_visible(tbPROJECT)=toolbars_wich_project->GetValue();
	if (toolbar_icon_size->GetValue().BeforeFirst('x').ToLong(&l)) {
		if (l!=menu_data->icon_size) {
			toolbar_changed=true;
			menu_data->icon_size=l;
			RecreateAllToolbars();
		}
	} else
		menu_data->icon_size=16;
	if (toolbar_changed) main_window->SortToolbars(true);
	
	Toolchain::SelectToolchain();
	config->RecalcStuff();
	
	Close();
}

void mxPreferenceWindow::OnCancelButton(wxCommandEvent &event){
	config->Styles=s_old_config_styles;
	main_window->UpdateStylesInSources();
	Close();
}

void mxPreferenceWindow::OnSkinButton(wxCommandEvent &event){
	int selection = skin_list->GetSelection();
	if (selection<0) return;
	config->Files.skin_dir=skin_paths[selection];
	RecreateAllToolbars(); main_window->SortToolbars(true);
	mxMessageDialog(main_window,LANG(PREFERENCES_THEME_WILL_APPLY_ON_RESTART,""
									 "El tema seleccionado se aplicara completamente\n"
									 "la proxima vez que reinicie ZinjaI"))
		.Title(LANG(PREFERENCES_CAPTION,"Preferencias")).IconInfo().Run();
}

void mxPreferenceWindow::OnWxHelpButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceAllWithFile(this,help_wxhelp_index,config->zinjai_dir,
										 LANG(PREFERENCES_PATHS_WXWIDGETS_HELP_INDEX,"Indice de ayuda wxWidgets") );
}

void mxPreferenceWindow::OnTempButton(wxCommandEvent &event){
	mxThreeDotsUtils::ReplaceAllWithDirectory(this,files_temp_dir,"",
											  LANG(PREFERENCES_PATHS_TEMP,"Directorio temporal") );
}

void mxPreferenceWindow::OnProjectButton(wxCommandEvent &event){
	mxThreeDotsUtils::ReplaceAllWithDirectory(this,files_project_folder,"",
											  LANG(PREFERENCES_PATHS_PROJECTS,"Directorio de proyectos") );
}

void mxPreferenceWindow::OnGdbButton(wxCommandEvent &event){
	mxThreeDotsUtils::ReplaceAllWithFile(this,files_debugger_command,config->zinjai_dir,
										 LANG(PREFERENCES_COMMANDS_GDB,"Comando del depurador"));
}

void mxPreferenceWindow::OnImgBrowserButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceAllWithFile(this,files_img_viewer_command,config->zinjai_dir,
										 LANG(PREFERENCES_COMMANDS_IMAGE_VIEWER,"Comando del visor de imagenes") );
}

void mxPreferenceWindow::OnXdotButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceAllWithFile(this,files_xdot_command,config->zinjai_dir,
										 LANG(PREFERENCES_COMMANDS_XDOT_COMMAND,"Comando del visor de grafos (xdot)") );
}

void mxPreferenceWindow::OnBrowserButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceAllWithFile(this,files_browser_command,config->zinjai_dir,
										 LANG(PREFERENCES_COMMANDS_BROWSER,"Comando del navegador Web") );
}

void mxPreferenceWindow::OnDoxygenButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceAllWithFile(this,files_doxygen_command,config->zinjai_dir,
										 LANG(PREFERENCES_COMMANDS_DOXYGEN,"Ubicación del ejecutable de Doxygen") );
}

void mxPreferenceWindow::OnWxfbButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceAllWithFile(this,files_wxfb_command,config->zinjai_dir,
										 LANG(PREFERENCES_COMMANDS_WXFORMBUILDER,"Ubicación del ejecutable de wxFormBuilder") );
}

void mxPreferenceWindow::OnValgrindButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceAllWithFile(this,files_valgrind_command,config->zinjai_dir,
										 LANG(PREFERENCES_COMMANDS_VALGRIND,"Ubicación del ejecutable de Valgrind") );
}

void mxPreferenceWindow::OnCppCheckButton(wxCommandEvent &event) {
	mxThreeDotsUtils::ReplaceAllWithFile(this,files_cppcheck_command,config->zinjai_dir,
										 LANG(PREFERENCES_COMMANDS_CPPCHECK,"Ubicación del ejecutable de CppCheck") );
}

void mxPreferenceWindow::OnHelpButton(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("preferences.html");
}

#ifdef __WIN32__

#else

void mxPreferenceWindow::OnExplorerButton(wxCommandEvent &event) {
	wxMenu menu;
	int count=0;
	if (mxUT::GetOutput(_T("dolphin --version")).Len()) {
		count++;
		menu.Append(mxID_EXPLORERS_DOLPHIN,_T("dolphin (kde 4)"));
	}
	if (mxUT::GetOutput(_T("konqueror --version")).Len()) {
		count++;
		menu.Append(mxID_EXPLORERS_KONQUEROR,_T("konqueror (kde 3)"));
	}
	if (mxUT::GetOutput(_T("nautilus --version")).Len()) {
		count++;
		menu.Append(mxID_EXPLORERS_NAUTILUS,_T("nautilus (gnome)"));
	}
	if (mxUT::GetOutput(_T("thunar --version")).Len()) {
		count++;
		menu.Append(mxID_EXPLORERS_THUNAR,_T("thunar (xfce)"));
	}
	if (count) {
		PopupMenu(&menu);
	} else {
		mxMessageDialog(main_window,"No se ha encontrado ningun explorador de archivos conocido.")
			.Title("Explorador de archivos").IconWarning().Run();
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
	LinuxTerminalInfo::Initialize();
	wxMenu menu; int count=0;
	for(int i=0;i<LinuxTerminalInfo::count;i++) { 
		if (LinuxTerminalInfo::list[i].Test()) {
			menu.Append(mxID_LAST_ID+i,wxString("Utilizar ")+LinuxTerminalInfo::list[i].name);
			count++;
		}
	}
	if (count) {
		PopupMenu(&menu);
	} else {
		mxMessageDialog(main_window,"No se ha encontrado una terminal conocida.\n"
						            "Instale xterm,konsole o gnome-terminal; o\n"
									"configure el parametro \"Comando del\n"
									"Terminal\" en el cuadro de Preferencias.\"")
			.Title("Terminal de ejecucion").IconWarning().Run();
	}	
}

void mxPreferenceWindow::OnSelectTerminal(wxCommandEvent &event) {
	files_terminal_command->SetValue(LinuxTerminalInfo::list[event.GetId()-mxID_LAST_ID].run_command);
}

#endif

void mxPreferenceWindow::OnDebugBlacklistButton(wxCommandEvent &event) {
	new mxEnumerationEditor(this,LANG(PREFERENCES_DEBUG_SETUP_BLACKLIST,"Fuentes y funciones a omitir en el paso a paso"),&temp_debug_blacklist);
}

void mxPreferenceWindow::OnAutocodesButton(wxCommandEvent &event) {
	wxMenu menu;
	menu.Append(mxID_AUTOCODES_OPEN,LANG(PREFERENCES_OPEN_FILE,"&Buscar archivo..."));
	menu.Append(mxID_AUTOCODES_EDIT,LANG(PREFERENCES_EDIT_FILE,"&Editar archivo..."));
	PopupMenu(&menu);
}
void mxPreferenceWindow::OnAutocodesOpen(wxCommandEvent &event) {
	wxFileDialog dlg(this,_T("Archivo de definiciones de autocodigos"),"",DIR_PLUS_FILE(config->zinjai_dir,files_autocode->GetValue()));
	if (wxID_OK==dlg.ShowModal()) {
		wxFileName fn(dlg.GetPath());
		fn.MakeRelativeTo(config->zinjai_dir);
		files_autocode->SetValue(fn.GetFullPath());
	}
}

void mxPreferenceWindow::OnAutocodesEdit(wxCommandEvent &event) {
	int i=2;
	wxString file=files_autocode->GetValue();
	if (wxFileName(file).FileExists()) {
		main_window->OpenFileFromGui(file,&i);
	} else {
		mxMessageDialog(main_window,"El archivo no existe")
			.Title("Archivo de definiciones de autocódigos").IconWarning().Run();
	}
}

void mxPreferenceWindow::OnDebugMacrosButton(wxCommandEvent &event) {
	wxMenu menu;
	menu.Append(mxID_DEBUG_MACROS_OPEN,LANG(PREFERENCES_OPEN_FILE,"&Buscar archivo..."));
	menu.Append(mxID_DEBUG_MACROS_EDIT,LANG(PREFERENCES_EDIT_FILE,"&Editar archivo..."));
	PopupMenu(&menu);
}

void mxPreferenceWindow::OnDebugMacrosOpen(wxCommandEvent &event) {
	wxFileDialog dlg(this,LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros para gdb"),"",DIR_PLUS_FILE(config->zinjai_dir,debug_macros_file->GetValue()));
	if (wxID_OK==dlg.ShowModal()) {
		wxFileName fn(dlg.GetPath());
		fn.MakeRelativeTo(config->zinjai_dir);
		debug_macros_file->SetValue(fn.GetFullPath());
	}
}

void mxPreferenceWindow::OnDebugMacrosEdit(wxCommandEvent &event) {
	int i=2;
	wxString file=debug_macros_file->GetValue();
	if (wxFileName(file).FileExists()) {
		main_window->OpenFileFromGui(file,&i);
	} else {
		mxMessageDialog(main_window,"El archivo no existe")
			.Title(LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros para gdb"))
			.IconWarning().Run();	
	}
}

mxPreferenceWindow *mxPreferenceWindow::ShowUp() {
	if (s_preference_window) {
		s_preference_window->ResetChanges();
		s_preference_window->EnableOrDisableControls();
		s_preference_window->Show();
		s_preference_window->Raise();
	} else {
		s_preference_window = new mxPreferenceWindow(main_window);
	}
	return s_preference_window;
}

void mxPreferenceWindow::OnToolbarsCommon(mxToolbarEditor *wx_toolbar, int tb_id) {
	if (!wx_toolbar) {
		MenusAndToolsConfig::myToolbar &tb=menu_data->toolbars[tb_id];
		wx_toolbar = new mxToolbarEditor(this,tb_id,LANG1(PREFERENCES_EDIT_TOOLBAR,"Barra de Herramientas <{1}>",tb.label));
		for(unsigned int i=0;i<tb.items.size();i++)
			if (!tb.items[i].key.IsEmpty()) 
				wx_toolbar->Add(tb.items[i].label,tb.items[i].icon,tb.items[i].visible);
	}
	wx_toolbar->ShowUp();
}
void mxPreferenceWindow::OnToolbarsFile(wxCommandEvent &evt) {
	OnToolbarsCommon(toolbar_editor_file,MenusAndToolsConfig::tbFILE);
}

void mxPreferenceWindow::OnToolbarsEdit(wxCommandEvent &evt) {
	OnToolbarsCommon(toolbar_editor_edit,MenusAndToolsConfig::tbEDIT);
}

void mxPreferenceWindow::OnToolbarsMisc(wxCommandEvent &evt) {
	OnToolbarsCommon(toolbar_editor_misc,MenusAndToolsConfig::tbMISC);
}

void mxPreferenceWindow::OnToolbarsView(wxCommandEvent &evt) {
	OnToolbarsCommon(toolbar_editor_view,MenusAndToolsConfig::tbVIEW);
}

void mxPreferenceWindow::OnToolbarsTools(wxCommandEvent &evt) {
	OnToolbarsCommon(toolbar_editor_tools,MenusAndToolsConfig::tbTOOLS);
}

void mxPreferenceWindow::OnToolbarsDebug(wxCommandEvent &evt) {
	OnToolbarsCommon(toolbar_editor_debug,MenusAndToolsConfig::tbDEBUG);
}

void mxPreferenceWindow::OnToolbarsRun(wxCommandEvent &evt) {
	OnToolbarsCommon(toolbar_editor_run,MenusAndToolsConfig::tbRUN);
}

void mxPreferenceWindow::SetToolbarPage(const wxString &edit_one) {
	for (unsigned int i=0;i<notebook->GetPageCount();i++)
		if (notebook->GetPage(i)==panel_toolbars) {
			notebook->SetSelection(i);
			break;
		}
	if (edit_one.Len()) {
		wxCommandEvent evt;
		if (edit_one=="file") OnToolbarsFile(evt);
		else if (edit_one=="edit") OnToolbarsEdit(evt);
		else if (edit_one=="view") OnToolbarsView(evt);
		else if (edit_one=="debug") OnToolbarsDebug(evt);
		else if (edit_one=="run") OnToolbarsRun(evt);
		else if (edit_one=="misc") OnToolbarsMisc(evt);
		else if (edit_one=="tools") OnToolbarsTools(evt);
	}
}

void mxPreferenceWindow::OnToolbarsReset(wxCommandEvent &evt) {
	if ( mxMessageDialog(main_window,LANG(PREFERENCES_TOOLBARS_RESET_WARNING,""
										  "Perdera todas las modificiaciones realizadas a las\n"
										  "barras de herramientas. ¿Desea continuar?"))
			.Title(LANG(PREFERENCES_CAPTION,"Preferencias")).ButtonsYesNo().Run().yes) 
	{
#define _aux_on_toolbar_reset(NAME,name) \
		bool baux_##name = _toolbar_visible(tb##NAME); \
		toolbars_wich_##name->SetValue(_toolbar_visible(tb##NAME)); \
		menu_data->UpdateToolbar(MenusAndToolsConfig::tb##NAME,true); \
		_toolbar_visible(tb##NAME)=baux_##name;
	
		_aux_on_toolbar_reset(FILE,file);
		_aux_on_toolbar_reset(EDIT,edit);
		_aux_on_toolbar_reset(VIEW,view);
		_aux_on_toolbar_reset(RUN,run);
		_aux_on_toolbar_reset(DEBUG,debug);
		_aux_on_toolbar_reset(TOOLS,tools);
		_aux_on_toolbar_reset(MISC,misc);
		_aux_on_toolbar_reset(FIND,find);
		main_window->SortToolbars();
	}
}
	
void mxPreferenceWindow::ResetChanges() {

	ignore_styles_changes=true;
	s_old_config_styles=config->Styles;
	
	m_binder.ToWidgets();
	
	help_autocomp_indexes->Clear();
	wxArrayString autocomp_array_all, autocomp_array_user;
	mxUT::GetFilesFromBothDirs(autocomp_array_all,"autocomp");
	mxUT::Split(config->Help.autocomp_indexes,autocomp_array_user);
	mxUT::Unique(autocomp_array_user,true);
	for (unsigned int i=0;i<autocomp_array_all.GetCount();i++) {
		int n=help_autocomp_indexes->Append(autocomp_array_all[i]);
		if (autocomp_array_user.Index(autocomp_array_all[i])!=wxNOT_FOUND) 
			help_autocomp_indexes->Check(n,true);
	}
	
	// style
	source_edgeColumnCheck->SetValue(config->Source.edgeColumn>0);
	source_edgeColumnPos->SetValue(wxString()<<abs(config->Source.edgeColumn));
	wxCommandEvent cmd_evt; OnFontChange(cmd_evt);
	
	// compile
	wxArrayString cpp_templates; g_templates->GetFilesList(cpp_templates,false,true);
	int cpp_idx = cpp_templates.Index(config->Files.cpp_template);
	if (cpp_idx!=wxNOT_FOUND) simple_default_cpp_template->SetSelection(cpp_idx);
	wxArrayString c_templates; g_templates->GetFilesList(c_templates,true,false);
	int c_idx = c_templates.Index(config->Files.c_template);
	if (c_idx!=wxNOT_FOUND) simple_default_c_template->SetSelection(c_idx);
	
	// general
#ifdef __linux__
	init_disable_ubuntu_tweaks->SetValue(!wxFileExists(DIR_PLUS_FILE(config->config_dir,"ubuntu")));
#endif

	// toolbars
	toolbars_wich_file->SetValue(_toolbar_visible(tbFILE));
	toolbars_wich_edit->SetValue(_toolbar_visible(tbEDIT));
	toolbars_wich_run->SetValue(_toolbar_visible(tbRUN));
	toolbars_wich_debug->SetValue(_toolbar_visible(tbDEBUG));
	toolbars_wich_tools->SetValue(_toolbar_visible(tbTOOLS));
	toolbars_wich_misc->SetValue(_toolbar_visible(tbMISC));
	toolbars_wich_find->SetValue(_toolbar_visible(tbFIND));
	toolbars_wich_view->SetValue(_toolbar_visible(tbVIEW));
	wxArrayString icon_sizes;
	icon_sizes.Add("16x16");
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"24"))) icon_sizes.Add("24x24");
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"32"))) icon_sizes.Add("32x32");
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"48"))) icon_sizes.Add("48x48");
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"64"))) icon_sizes.Add("64x64");
	wxString icsz = wxString()<<menu_data->icon_size<<"x"<<menu_data->icon_size;
	unsigned int idx_icsz = icon_sizes.Index(icsz);
	if (idx_icsz>=icon_sizes.GetCount()) idx_icsz=0;
	toolbar_icon_size->SetSelection(idx_icsz);
	
	// debug
	temp_debug_blacklist=config->Debug.blacklist;
	
	ignore_styles_changes=false;
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
		skin_image->SetBitmap(bitmaps->GetBitmap("skin_no_preview.png"));
}

void mxPreferenceWindow::OnMaxJobsButton(wxCommandEvent &event) {
	wxMessageBox(LANG1(LANG_MAX_JOBS_TIP,""
		"Este campo define la cantidad de objetos que se compilan en simultaneo "
		"al construir un proyecto. Si su pc tiene mas de un nucleo, la cantidad "
		"de nucleos, o la mitad son valores recomendables. Consulte la ayuda para "
		"más detalles. ZinjaI detecta en el sistema actual <{1}> nucleos.",wxString()<<wxThread::GetCPUCount()),
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
	mxInspectionsImprovingEditor(this,
		config->Debug.inspection_improving_template_from,config->Debug.inspection_improving_template_to);
}

void mxPreferenceWindow::OnToolchainButton(wxCommandEvent &evt) {
	wxArrayString tc_prev; Toolchain::GetNames(tc_prev,false);
	mxToolchainConfig(this,files_toolchain->GetValue()).ShowModal();
	wxArrayString tc_post; Toolchain::GetNames(tc_post,false);
	for(unsigned int i=0;i<tc_post.GetCount();i++) {
		if (tc_prev.Index(tc_post[i])==wxNOT_FOUND)
			files_toolchain->Append(tc_post[i]);
	}
}

void mxPreferenceWindow::Delete ( ) {
	if (s_preference_window) s_preference_window->Destroy();
	s_preference_window = nullptr;
}

void mxPreferenceWindow::OnFontChange (wxCommandEvent & evt) {
	if (ignore_styles_changes) return;
	config->Styles.font_name = styles_font_name->GetValue();
	long l=0; if (styles_font_size->GetValue().ToLong(&l)&&l>0) config->Styles.font_size=l;
	main_window->UpdateStylesInSources();
}

void mxPreferenceWindow::OnCustomizeShortcuts (wxCommandEvent & evt) {
	mxShortcutsDialog(this);
}

void mxPreferenceWindow::OnClearSubcmdCache(wxCommandEvent &event) {
	mxUT::GetOutput("",false,true);
}


void mxPreferenceWindow::EnableOrDisableControls ( ) {
	if (main_window->gui_debug_mode||main_window->gui_fullscreen_mode) 
		wx_toolbars_widgets.DisableAll();
	else
		wx_toolbars_widgets.EnableAll();
}

