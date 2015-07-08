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

static cfgStyles old_config_styles; // aqu� para evitar tener que hacer el include de ConfigManager en el .h

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
	list[count++]=LinuxTerminalInfo("gnome-terminal","gnome-terminal --version","gnome-terminal --disable-factory --hide-menubar -t \"${TITLE}\" -x",true);
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

static mxPreferenceWindow *preference_window=nullptr;

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
	EVT_CLOSE(mxPreferenceWindow::OnClose)
	EVT_LISTBOX(mxID_SKIN_LIST,mxPreferenceWindow::OnSkinList)
	EVT_COMBOBOX(mxID_PREFERENCES_FONTNAME,mxPreferenceWindow::OnFontChange)
	EVT_TEXT(mxID_PREFERENCES_FONTSIZE,mxPreferenceWindow::OnFontChange)
END_EVENT_TABLE()

mxPreferenceWindow::mxPreferenceWindow(wxWindow* parent) : wxDialog(parent, wxID_ANY, LANG(PREFERENCES_CAPTION,"Preferencias"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) 
{

	ignore_styles_changes = true;
	
	toolbar_editor_file = nullptr;
	toolbar_editor_edit = nullptr;
	toolbar_editor_view = nullptr;
	toolbar_editor_tools = nullptr;
	toolbar_editor_debug = nullptr;
	toolbar_editor_run = nullptr;
	toolbar_editor_misc = nullptr;
	
	old_config_styles=config->Styles;
	
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
	notebook->AddPage(CreateSimplePanel(notebook), LANG(PREFERENCES_SIMPLE_PROGRAM,"Programa/Proyecto"),false,1);
	notebook->AddPage(CreateStylePanel(notebook), LANG(PREFERENCES_STYLE,"Estilo"),false,2);
	notebook->AddPage(CreateWritingPanels(notebook), LANG(PREFERENCES_ASSIST,"Asistencias"),false,3);
	notebook->AddPage(CreateSkinPanel(notebook), LANG(PREFERENCES_ICON_THEME,"Tema de Iconos"),false,4);
	notebook->AddPage(CreateToolbarsPanel(notebook), LANG(PREFERENCES_TOOLBARS,"Barras de Herram."),false,5);
	notebook->AddPage(CreateDebugPanels(notebook), LANG(PREFERENCES_DEBUGGING,"Depuraci�n"),false,6);
	notebook->AddPage(CreatePathsPanels(notebook), LANG(PREFERENCES_PATHS,"Rutas"),false,7); 

	mySizer->Add(notebook,sizers->Exp1);
	mySizer->Add(mxUT::MakeGenericButtonsSizer(this,true),sizers->Exp0);
	SetEscapeId(wxID_CANCEL);

	SetSizerAndFit(mySizer);

	ignore_styles_changes=false;
	
	SetFocus();
	Show();
}

wxPanel *mxPreferenceWindow::CreateGeneralPanel (mxBookCtrl *notebook) {

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
	init_lang_file = mxUT::AddComboBox(sizer,panel,LANG(PREFERENCES_GENERAL_GUI_LANGUAGE,"Idioma de la interfaz (*)"),a_langs, i_langs);
	init_show_tip_on_startup = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_TOOLTIPS,"Mostrar sugerencias al inicio"),config->Init.show_tip_on_startup);
	init_show_welcome = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_WELCOME_PANEL,"Mostrar panel de bienvenida"),config->Init.show_welcome);
	init_show_extra_panels = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_HELP_TABS,"Mostrar pesta�as de ayuda en cuadro de di�logo (*)"),config->Help.show_extra_panels);
	init_history_len = mxUT::AddTextCtrl(sizer,panel,LANG(PREFERENCES_GENERAL_HISTORY_LEN,"Archivos en el historial"),config->Init.history_len);
	init_show_explorer_tree = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_FILES_EXPLORER_ON_STARTUP,"Mostrar el explorador de archivos al iniciar"),config->Init.show_explorer_tree);
	init_left_panels = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_GROUP_TREES,"Agrupar arboles en un solo panel (*)"),config->Init.left_panels);
	init_autohide_panels = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_AUTOHIDE_PANELS,"Ocultar paneles automaticamente (*)"),config->Init.autohide_panels);
	init_singleton = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_SINGLETON,"Utilizar una sola instancia de ZinjaI al abrir archivos desde la linea de comandos"),config->Init.singleton);
	init_check_for_updates = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_CHECK_FOR_UPDATES,"Verificar si existen nuevas versiones al iniciar"),config->Init.check_for_updates);
#ifdef __linux__
	init_disable_ubuntu_tweaks = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_DISABLE_UBUNTU_TWEAKS,"Deshabilitar la interfaz de men�es y scrollbars especial de Unity (*)"),!wxFileExists(DIR_PLUS_FILE(config->home_dir,"ubuntu")));
#endif
	sizer->Add(new wxButton(panel,mxID_PREFERENCES_CUSTOMIZE_SHORTCUTS,LANG(PREFERENCES_CUSTOMIZE_SHORTCUTS,"Personalizar atajos de teclado...")),sizers->BA10);
	sizer->AddStretchSpacer(1);
	mxUT::AddStaticText(sizer,panel,LANG(PREFERENCES_GENERAL_ASTERIX_WILL_APPLY_NEXT_TIME,"(*) tendr� efecto la proxima vez que inicie ZinjaI"));
	panel->SetSizerAndFit(sizer);
	return panel;

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
	wxNotebook *nbk = new wxNotebook(notebook,wxID_ANY,wxDefaultPosition,wxDefaultSize);
	nbk->AddPage(CreateDebugPanel1(nbk), LANG(PREFERENCES_DEBUG_1,"Interfaz"),false);
	nbk->AddPage(CreateDebugPanel2(nbk), LANG(PREFERENCES_DEBUG_2,"Avanzado"),false);
	return nbk;
}


wxPanel *mxPreferenceWindow::CreateDebugPanel1 (wxNotebook *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
		
	debug_allow_edition = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_ALLOW_EDITION,"Permitir editar los fuentes durante la depuraci�n"),config->Debug.allow_edition);
	debug_autohide_panels = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_REORGANIZE_PANELS,"Reacomodar los paneles al iniciar/finalizar la depuraci�n"),config->Debug.autohide_panels);
	debug_inspections_on_right = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_INSPECTIONS_PANEL_ON_RIGHT,"Colocar el panel de inspecciones a la derecha (*)"),config->Debug.inspections_on_right);
	debug_show_thread_panel = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_SHOW_THREAD_LIST,"Mostrar lista de hilos de ejecuci�n"),config->Debug.show_thread_panel);
	debug_show_log_panel = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_SHOW_DEBUGGER_LOG,"Mostrar el panel de mensajes del depurador"),config->Debug.show_log_panel);
	debug_autohide_toolbars = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_ORGANIZE_TOOLBARS,"Reacomodar las barras de herramienta al iniciar/finalizar la depuraci�n"),config->Debug.autohide_toolbars);
	debug_raise_main_window = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_ACTIVATE_WINDOW_ON_INTERRUPTION,"Mostrar ZinjaI cuando se interrumpe la ejecuci�n"),config->Debug.raise_main_window);
	debug_return_focus_on_continue = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_RETURN_FOCUS_ON_CONTINUE,"Devolver el foco a la aplicaci�n en depuraci�n luego de una pausa"),config->Debug.return_focus_on_continue);
	
	panel->SetSizerAndFit(sizer);
	return panel;

}
wxPanel *mxPreferenceWindow::CreateDebugPanel2 (wxNotebook *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
		
#ifdef __linux__
	debug_enable_core_dump = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_ENABLE_CORE_DUMP,"Habilitar volcado de memoria al ejecutar sin depurador"),config->Debug.enable_core_dump);
#endif
	debug_readnow = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_LOAD_ALL_DEBUG_INFO,"Cargar toda la informaci�n de depuracion antes de comenzar"),config->Debug.readnow);
	debug_auto_solibs = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_LOAD_SHARED_LIBS_INFO,"Cargar informaci�n de depuracion de bibliotecas externas"),config->Debug.auto_solibs);
	debug_compile_again = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_RECOMPILE_AUTOMATICALLY,"Recompilar automaticamente antes de depurar si es necesario"),config->Debug.compile_again);
//	debug_close_on_normal_exit= mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_STOP_DEBBUGING_ON_ABNORMAL_TERMINATION,"Salir del modo depuraci�n si el programa finaliza normalmente"),config->Debug.close_on_normal_exit);
	debug_always_debug = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_ALWAYS_RUN_IN_DEBUGGER,"Siempre ejecutar en el depurador"),config->Debug.always_debug);
//	debug_use_colours_for_inspections = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_USE_COLOURS_FOR_INSPECTIONS,"Utilizar colores en la tabla de inspecciones"),config->Debug.use_colours_for_inspections);
	debug_inspections_can_have_side_effects = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_INSPECTIONS_CAN_HAVE_SIDE_EFFECTS,"Considerar side-effects al evaluar inspecciones"),config->Debug.inspections_can_have_side_effects);
	
	wxBoxSizer *type_replace_sizer = new wxBoxSizer(wxHORIZONTAL);
	debug_improve_inspections_by_type = new wxCheckBox(panel, wxID_ANY, wxString(LANG(PREFERENCES_DEBUG_IMPROVE_INSPECTIONS_BY_TYPE,"Mejorar inspecciones autom�ticamente segun tipo"))+_T("   "));
	debug_improve_inspections_by_type->SetValue(config->Debug.improve_inspections_by_type);
	type_replace_sizer->Add(debug_improve_inspections_by_type,sizers->Center);
	type_replace_sizer->Add(new wxButton(panel,mxID_DEBUG_IMPROVE_INSPECTIONS_BY_TYPE,LANG(PREFERENCES_DEBUG_IMPROVE_INSPECTIONS_SETTINGS,"Configurar...")),sizers->Center);
	sizer->Add(type_replace_sizer,sizers->BA5_Exp0);
	
	debug_macros_file = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de definiciones de macros para gdb"),config->Debug.macros_file,mxID_DEBUG_MACROS);
	
	wxBoxSizer *blacklist_sizer = new wxBoxSizer(wxHORIZONTAL);
	debug_use_blacklist = new wxCheckBox(panel, wxID_ANY, wxString(LANG(PREFERENCES_DEBUG_USE_BLACKLIST,"Utilizar lista negra en el paso a paso"))+_T("   "));
	blacklist_sizer->Add(debug_use_blacklist,sizers->Center);
	blacklist_sizer->Add(new wxButton(panel,mxID_DEBUG_BLACKLIST,LANG(PREFERENCES_DEBUG_BLACKLIST_BUTTON,"Configurar...")),sizers->Center);
	sizer->Add(blacklist_sizer,sizers->BA5_Exp0);
	debug_use_blacklist->SetValue(config->Debug.use_blacklist);
	
#ifdef __WIN32__
	debug_no_debug_heap = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_DEBUG_NO_DEBUG_HEAP,"Deshabilitar heap especial de widondows para deburaci�n"),config->Debug.no_debug_heap);
#endif
	
	
	panel->SetSizerAndFit(sizer);
	return panel;

}

wxPanel *mxPreferenceWindow::CreateToolbarsPanel (mxBookCtrl *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	sizer->Add(new wxStaticText(panel, wxID_ANY, LANG(PREFERENCES_TOOLBARS_WICH,"Barra de herramientas a utilizar:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
	
	wxBoxSizer *tbs_sizer = new wxBoxSizer(wxVERTICAL);
	
	bool do_not_modify_toolbars = main_window->gui_debug_mode||main_window->gui_fullscreen_mode;
	wxArrayString poss; poss.Add(LANG(PREFERENCES_TOOLBARS_DOCK_TOP,"Arriba")); poss.Add(LANG(PREFERENCES_TOOLBARS_DOCK_LEFT,"Izquierda")); poss.Add(LANG(PREFERENCES_TOOLBARS_DOCK_RIGHT,"Derecha")); poss.Add(LANG(PREFERENCES_TOOLBARS_FLOAT,"Flotante"));
#define _aux_ctp_1(name,ID,label) { \
	MenusAndToolsConfig::toolbarPosition &position = menu_data->GetToolbarPosition(MenusAndToolsConfig::tb##ID); \
	wxBoxSizer *sz= new wxBoxSizer(wxHORIZONTAL); \
	sz->Add(20,1,0); \
	toolbars_wich_##name = new wxCheckBox(panel,wxID_ANY,label); \
	toolbars_wich_##name->SetValue(position.visible); \
	if (do_not_modify_toolbars) toolbars_wich_##name->Enable(false); \
	wxButton *bt = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_##ID,LANG(PREFERENCES_TOOLBARS_MODIFY,"Modificar...")); \
	sz->Add(toolbars_wich_##name,sizers->BA5_Center); sz->AddStretchSpacer(); sz->Add(bt,sizers->BLR10); \
	toolbars_side_##name = new wxComboBox(panel,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,poss,wxCB_READONLY); \
	toolbars_side_##name->SetSelection(position.top?0:(position.left?1:(position.right?2:3))); \
	if (do_not_modify_toolbars) toolbars_side_##name->Enable(false); \
	sz->Add(new wxStaticText(panel,wxID_ANY,"Ubicaci�n:"), sizers->BA5_Center); \
	sz->Add(toolbars_side_##name,sizers->BA5_Center); \
	tbs_sizer->Add(sz,sizers->Exp0); }
	
	_aux_ctp_1(file,FILE,LANG(CAPTION_TOOLBAR_FILE,"Archivo"));
	_aux_ctp_1(edit,EDIT,LANG(CAPTION_TOOLBAR_EDIT,"Editar"));
	_aux_ctp_1(view,VIEW,LANG(CAPTION_TOOLBAR_VIEW,"Ver"));
	_aux_ctp_1(run,RUN,LANG(CAPTION_TOOLBAR_RUN,"Ejecuci�n"));
	_aux_ctp_1(debug,DEBUG,LANG(CAPTION_TOOLBAR_DEBUG,"Depuraci�n"));
	_aux_ctp_1(tools,TOOLS,LANG(CAPTION_TOOLBAR_TOOLS,"Herramientas"));
	_aux_ctp_1(misc,MISC,LANG(CAPTION_TOOLBAR_MISC,"Misce�nea"));
	
	toolbars_wich_find = new wxCheckBox(panel,wxID_ANY,LANG(CAPTION_TOOLBAR_FIND,"Busqueda"));
	toolbars_wich_find->SetValue(_toolbar_visible(tbFIND));
	wxBoxSizer *szFind = new wxBoxSizer(wxHORIZONTAL);
	szFind->Add(20,1,0);
	szFind->Add(toolbars_wich_find,sizers->BA5_Center);
	tbs_sizer->Add(szFind/*,sizers->BA5*/);
	
	toolbars_wich_project = new wxCheckBox(panel,wxID_ANY,LANG(CAPTION_TOOLBAR_PROJECT,"Proyecto"));
	toolbars_wich_project->SetValue(_toolbar_visible(tbPROJECT));
	wxBoxSizer *szProject = new wxBoxSizer(wxHORIZONTAL);
	szProject->Add(20,1,0);
	szProject->Add(toolbars_wich_project,sizers->BA5_Center);
	tbs_sizer->Add(szProject/*,sizers->BA5*/);
	
	sizer->Add(tbs_sizer);
	
	wxButton *btReset = new wxButton(panel,mxID_PREFERENCES_TOOLBAR_RESET,LANG(PREFERENCES_TOOLBARS_RESET,"Reestablecer configuraci�n por defecto"));
	sizer->Add(btReset,sizers->BA5);
	
	wxArrayString icon_sizes;
	icon_sizes.Add("16x16");
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"24"))) icon_sizes.Add("24x24");
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"32"))) icon_sizes.Add("32x32");
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"48"))) icon_sizes.Add("48x48");
	if (wxFileName::DirExists(DIR_PLUS_FILE(config->Files.skin_dir,"64"))) icon_sizes.Add("64x64");
	wxString icsz = wxString()<<menu_data->icon_size<<"x"<<menu_data->icon_size;
	unsigned int idx_icsz = icon_sizes.Index(icsz);
	if (idx_icsz>=icon_sizes.GetCount()) idx_icsz=0;
	toolbar_icon_size = mxUT::AddComboBox(sizer,panel,LANG(PREFERENCES_TOOLBAR_ICON_SIZE,"Tama�o de icono"),icon_sizes,idx_icsz);
	
	if (do_not_modify_toolbars) {
		toolbars_wich_find->Enable(false);
		toolbars_wich_project->Enable(false);
		toolbar_icon_size->Enable(false);
		btReset->Enable(false);
	}
	
	panel->SetSizerAndFit(sizer);
	return (panel_toolbars=panel);

}

wxPanel *mxPreferenceWindow::CreateSimplePanel (mxBookCtrl *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxArrayString a_new_file;
	a_new_file.Add(LANG(PREFERENCES_SIMPLE_CREATE_EMPTY_FILE,"Crear archivo en blanco"));
	a_new_file.Add(LANG(PREFERENCES_SIMPLE_CREATE_FROM_TEMPLATE,"Crear a partir de plantilla"));
	a_new_file.Add(LANG(PREFERENCES_SIMPLE_SHOW_WIZARD,"Mostrar Asistente"));
	init_new_file = mxUT::AddComboBox(sizer,panel,LANG(PREFERENCES_SIMPLE_NEW_ACTION,"Accion para Nuevo Archivo"),a_new_file, config->Init.new_file);
	running_cpp_compiler_options = mxUT::AddTextCtrl(sizer,panel,LANG(PREFERENCES_SIMPLE_EXTRA_CPP_COMPILER_ARGUMENTS,"Par�metros adicionales por defecto para el compilador C++"),config->Running.cpp_compiler_options);
	running_c_compiler_options = mxUT::AddTextCtrl(sizer,panel,LANG(PREFERENCES_SIMPLE_EXTRA_C_COMPILER_ARGUMENTS,"Par�metros adicionales por defecto para el compilador C"),config->Running.c_compiler_options);
	running_wait_for_key = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_SIMPLE_WAIT_KEY_AFTER_RUNNING,"Esperar una tecla luego de la ejecuci�n"),config->Running.wait_for_key);
	running_always_ask_args = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_SIMPLE_ALWAYS_ASK_ARGS,"Siempre pedir argumentos al ejecutar"),config->Running.always_ask_args);
	init_always_add_extension = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_SIMPLE_ADD_CPP_EXTENSION,"Agregar la extension cpp si se omite al guardar"),config->Init.always_add_extension);
	
	init_max_jobs = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_GENERAL_MAX_JOBS,"Cantidad de pasos en paralelo al compilar"),wxString()<<config->Init.max_jobs,mxID_MAX_JOBS);
	init_stop_compiling_on_error = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_STOP_COMPILING_ON_ERROR,"Detener la compilaci�n de un proyecto al encontrar el primer error"),config->Init.stop_compiling_on_error);
	init_save_project = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SAVE_PROJECT_ON_CLOSE,"Guardar siempre el proyeto al salir"),config->Init.save_project);
	init_prefer_explorer_tree = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SHOW_FILES_EXPLORER_ON_PROJECT,"Mostrar el explorador de archivos al abrir un proyecto"),config->Init.prefer_explorer_tree);
	
	wxArrayString tc_array; Toolchain::GetNames(tc_array,false); int tc_i=tc_array.Index(config->Files.toolchain);
	files_toolchain = new wxComboBox(panel, wxID_ANY, config->Files.toolchain, wxDefaultPosition, wxDefaultSize, tc_array, wxCB_READONLY);
	files_toolchain->SetSelection(tc_i);
	wxBoxSizer *tc_sizer = new wxBoxSizer(wxHORIZONTAL);
	tc_sizer->Add(new wxStaticText(panel, wxID_ANY, wxString(LANG(PREFERENCES_TOOLCHAIN,"Herramientas de compilaci�n"))+": ", wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	tc_sizer->Add(files_toolchain , sizers->Exp1);
	tc_sizer->Add(new wxButton(panel,mxID_PREFERENCES_TOOLCHAIN_OPTIONS,"...",wxDefaultPosition,wxSize(30,10)),sizers->Exp0_Right);
	sizer->Add(tc_sizer,sizers->BA5_Exp0);
	
	
	wxBoxSizer *subcmd_cache_sizer = new wxBoxSizer(wxHORIZONTAL);
	init_use_cache_for_subcommands = new wxCheckBox(panel, wxID_ANY, wxString(LANG(PREFERENCES_USE_CACHE_FOR_SUBCMD,"Usar cache para la ejecuci�n de subcomandos"))+_T("   "));
	init_use_cache_for_subcommands->SetValue(config->Init.use_cache_for_subcommands);
	subcmd_cache_sizer->Add(init_use_cache_for_subcommands,sizers->Center);
	subcmd_cache_sizer->Add(new wxButton(panel,mxID_PREFERENCES_CLEAR_SUBCMD_CACHE,LANG(PREFERENCES_USE_CLEAR_SUBCMD_CACHE,"Limpiar")),sizers->Center);
	sizer->Add(subcmd_cache_sizer,sizers->BA5_Exp0);
	
	panel->SetSizerAndFit(sizer);
	return panel;

}





wxPanel *mxPreferenceWindow::CreateStylePanel (mxBookCtrl *notebook) {
	
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxBoxSizer *colours_sizer = new wxBoxSizer(wxHORIZONTAL);
	source_syntaxEnable = new wxCheckBox(panel, wxID_ANY, wxString(LANG(PREFERENCES_STYLE_SINTAX_HIGHLIGHT,"Colorear sintaxis"))+_T("   "));
	colours_sizer->Add(source_syntaxEnable,sizers->Center);
	colours_sizer->Add(new wxButton(panel,mxID_COLORS_PICKER,LANG(PREFERENCES_STYLE_DEFINE_COLOURS,"Definir colores...")),sizers->Center);
	sizer->Add(colours_sizer,sizers->BA5_Exp0);
	source_syntaxEnable->SetValue(config->Source.syntaxEnable);
	
	wxArrayString a_wrap;
	a_wrap.Add(LANG(PREFERENCES_STYLE_WRAP_NONE,"Nunca"));
	a_wrap.Add(LANG(PREFERENCES_STYLE_WRAP_ALL_BUT_SOURCES,"Todos menos fuentes"));
	a_wrap.Add(LANG(PREFERENCES_STYLE_WRAP_ALWAYS,"Siempre"));
	init_wrap_mode = mxUT::AddComboBox(sizer,panel,LANG(PREFERENCES_STYLE_LINE_WRAP,"Ajuste de linea din�mico"),a_wrap, config->Init.wrap_mode);
	source_whiteSpace = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_SHOW_EOF_AND_WHITE_SPACES,"Mostrar espacios y caracteres de fin de linea"),config->Source.whiteSpace);
	source_lineNumber = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_SHOW_LINE_NUMBERS,"Mostrar numeros de linea"),config->Source.lineNumber);
	source_foldEnable = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_ENABLE_CODE_FOLDING,"Habilitar plegado de c�digo"),config->Source.foldEnable);
	
	wxBoxSizer *tabs_sizer = new wxBoxSizer(wxHORIZONTAL);
	source_tabWidth = mxUT::AddTextCtrl(tabs_sizer,panel,LANG(PREFERENCES_STYLE_TAB_WIDTH,"Ancho del tabulado"),config->Source.tabWidth);
	source_tabUseSpaces = mxUT::AddCheckBox(tabs_sizer,panel,LANG(PREFERENCES_STYLE_SPACES_INTEAD_TABS,"Colocar espacios en lugar de tabs"),config->Source.tabUseSpaces);
	sizer->Add(tabs_sizer,sizers->Exp0);
	
	wxBoxSizer *edge_sizer = new wxBoxSizer(wxHORIZONTAL);
	source_edgeColumnCheck = mxUT::AddCheckBox(edge_sizer,panel,LANG(PREFERENCES_STYLE_EDGE_COLUM,"Mostrar linea gu�a en la columna"),config->Source.edgeColumn>0);
	source_edgeColumnPos = mxUT::AddTextCtrl(edge_sizer,panel,"",abs(config->Source.edgeColumn),false);
	sizer->Add(edge_sizer,sizers->Exp0);
	
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
	wxArrayString fonts; mxFontEnumerator f(fonts); fonts.Sort(); int def_font=fonts.Index(config->Styles.font_name,false); if (def_font==wxNOT_FOUND) def_font=-1;
	styles_font_name = mxUT::AddComboBox(sizer,panel,LANG(PREFERENCES_STYLE_FONT_NAME,"Fuente para el c�digo"),fonts,def_font,mxID_PREFERENCES_FONTNAME);
	
	wxBoxSizer *fonts_sizer = new wxBoxSizer(wxHORIZONTAL);
	styles_font_size = mxUT::AddTextCtrl(fonts_sizer,panel,LANG(PREFERENCES_STYLE_SCREEN_FONT_SIZE,"Tama�o de la fuente en pantalla"),config->Styles.font_size,false,mxID_PREFERENCES_FONTSIZE);
	styles_print_size = mxUT::AddTextCtrl(fonts_sizer,panel,LANG(PREFERENCES_STYLE_PRINTING_FONT_SIZE,"para impresion"),config->Styles.print_size);
	sizer->Add(fonts_sizer,sizers->Exp0);
	
	init_autohide_menus_fs = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_HIDE_MENUS_ON_FULLSCREEN,"Ocultar barra de menues al pasar a pantalla completa"),config->Init.autohide_menus_fs);
	init_autohide_toolbars_fs = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_HIDE_TOOLBARS_ON_FULLSCREEN,"Ocultar barras de herramientas al pasar a pantalla completa"),config->Init.autohide_toolbars_fs);
	init_autohide_panels_fs = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_STYLE_HIDE_PANELS_ON_FULLSCREEN,"Ocultar paneles al pasar a pantalla completa"),config->Init.autohide_panels_fs);
	
	panel->SetSizerAndFit(sizer);
	return panel;

}

wxNotebook *mxPreferenceWindow::CreateWritingPanels (mxBookCtrl *notebook) {
	wxNotebook *nbk = new wxNotebook(notebook,wxID_ANY,wxDefaultPosition,wxDefaultSize);
	nbk->AddPage(CreateWritingPanel1(nbk), LANG(PREFERENCES_ASSIST_1,"Generales"),false);
	nbk->AddPage(CreateWritingPanel2(nbk), LANG(PREFERENCES_ASSIST_2,"Autocompletado"),false);
	return nbk;
}

wxPanel *mxPreferenceWindow::CreateWritingPanel1 (wxNotebook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	source_smartIndent = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_INTELLIGENT_EDITING,"Edici�n inteligente"),config->Source.smartIndent);
	source_bracketInsertion = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_AUTOCLOSE_BRACKETS,"Cerrar llaves autom�ticamente al presionar Enter"),config->Source.bracketInsertion);
	source_autocloseStuff = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_AUTOCLOSE_STUFF,"Siempre cerrar llaves, par�ntesis y comillas autom�ticamente"),config->Source.autocloseStuff);
	source_indentPaste = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_INDENT_ON_PASTE,"Corregir indentado al pegar"),config->Source.indentPaste);
	source_avoidNoNewLineWarning = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_CHECK_FOR_EMPTY_LAST_LINE,"Controlar que quede una linea en blanco al final de cada archivo"),config->Source.avoidNoNewLineWarning);
	source_toolTips = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_SHOW_TOOLTIPS_FOR_VAR_TYPES,"Utilizar tooltips para identificar tipos de variables"),config->Source.toolTips);
	init_beautifyCompilerErrors  = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_BEAUTIFY_COMPILER_ERRORS,"Simplificar mensajes de error del compilador"),config->Init.beautify_compiler_errors);
	files_autocode = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_WRITTING_AUTOCODES_FILE,"Archivo de definiciones de plantillas de auto-c�digo"),config->Files.autocodes_file,mxID_AUTOCODES_FILE);
#ifndef __WIN32__
	init_lang_es = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_GENERAL_SPANISH_COMPILER_OUTPUT,"Mostrar errores de compilaci�n en Espa�ol (Ver Ayuda!) (*)"),config->Init.lang_es);
#endif
	
	panel->SetSizerAndFit(sizer);
	return panel;
	
}

wxPanel *mxPreferenceWindow::CreateWritingPanel2 (wxNotebook *notebook) {
	
	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	
	wxArrayString autocomp_methods;
	autocomp_methods.Add(LANG(PREFERENCES_WRITTING_AUTOCOMP_NONE,"Deshabilitado"));
	autocomp_methods.Add(LANG(PREFERENCES_WRITTING_AUTOCOMP_START,"Habilitado, por comienzo de palabra"));
	autocomp_methods.Add(LANG(PREFERENCES_WRITTING_AUTOCOMP_FIND,"Habilitado, por cualquier parte de la palabra"));
	autocomp_methods.Add(LANG(PREFERENCES_WRITTING_AUTOCOMP_FUZZY,"Habilitado, por similaridad"));
	source_autoCompletion = mxUT::AddComboBox(sizer,panel,LANG(PREFERENCES_WRITING_AUTOCOMPLETION,"Autocompletado"),autocomp_methods,config->Source.autoCompletion);
	source_autocompFilters = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_AUTOCOM_FILTERS,"Filtrar resultados de autocompletado al continuar escribiendo"),config->Source.autocompFilters);
	source_autocompTips = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_ENABLE_AUTOCOMP_CALLTIPS,"Mostrar info. adicional junto al men� de autocompletado"),config->Source.autocompTips);
	source_callTips = mxUT::AddCheckBox(sizer,panel,LANG(PREFERENCES_WRITING_ENABLE_CALLTIPS,"Mostrar ayuda de llamadas a funciones y m�todos"),config->Source.callTips);
	
	sizer->Add(new wxStaticText(panel, wxID_ANY, LANG(PREFERENCES_WRITING_AUTOCOMPLETION_INDEXES,"�ndices de autocompletado a utilizar"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
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
	sizer->Add(help_autocomp_indexes,sizers->BA5_DL_Exp1);

	panel->SetSizerAndFit(sizer);
	return panel;
	
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
	wxNotebook *nbk = new wxNotebook(notebook,wxID_ANY,wxDefaultPosition,wxDefaultSize);
	nbk->AddPage(CreatePathsPanel1(nbk), LANG(PREFERENCES_PATHS_1,"Rutas 1"),false);
	nbk->AddPage(CreatePathsPanel2(nbk), LANG(PREFERENCES_PATHS_1,"Rutas 2"),false);
	return nbk;
}

wxPanel *mxPreferenceWindow::CreatePathsPanel1 (wxNotebook *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );

	files_temp_dir = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_TEMP,"Directorio temporal"),config->Files.temp_dir, mxID_TEMP_FOLDER);
#ifdef __WIN32__
//	files_mingw_dir = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_MINGW,"Directorio de MinGW (requiere reiniciar ZinjaI)"),config->Files.mingw_dir, mxID_MINGW_FOLDER);
#endif
	files_project_folder = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_PROJECTS,"Directorio de proyectos"),config->Files.project_folder, mxID_PROJECTS_FOLDER);
	
	help_wxhelp_index = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_PATHS_WXWIDGETS_HELP_INDEX,"Indice de ayuda wxWidgets"),config->Help.wxhelp_index, mxID_WXHELP_FOLDER);
	
//	files_compiler_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_GPP,"Comando del compilador C++"),config->Files.compiler_command,mxID_GPP_PATH);
//	files_compiler_c_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_GCC,"Comando del compilador C"),config->Files.compiler_c_command,mxID_GCC_PATH);
	files_debugger_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_GDB,"Comando del depurador"),config->Files.debugger_command,mxID_GDB_PATH);
	files_terminal_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_CONSOLE,"Comando de la terminal"),config->Files.terminal_command,mxID_TERMINALS_BUTTON);
	
#ifndef __WIN32__
	sizer->Add(new wxButton(panel,mxID_PREFERENCES_XDG,LANG(PREFERENCES_CREATE_ICONS,"Crear/Actualizar accesos directos en el escritorio/men� del sistema...")),sizers->BA10);
#endif
	
	sizer->AddStretchSpacer(1);
	mxUT::AddStaticText(sizer,panel,/*LANG(PREFERENCES_GENERAL_ASTERIX_WILL_APPLY_NEXT_TIME,*/"Nota: Las configuraci�n de las rutas relacionadas al compilador se realiza\n"
																								"desde la pesta�a \"Programa/Proyecto\" utilizando el bot�n \"...\" de la\n"
																								"opci�n \"Herramientas de compilaci�n\"."/*)*/);
	
	panel->SetSizerAndFit(sizer);
	return panel;

}

wxPanel *mxPreferenceWindow::CreatePathsPanel2 (wxNotebook *notebook) {

	wxBoxSizer *sizer= new wxBoxSizer(wxVERTICAL);
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );

	files_explorer_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_EXPLORER,"Comando del explorador de archivos"),config->Files.explorer_command,mxID_EXPLORERS_BUTTON);
	files_img_viewer_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_IMAGE_VIEWER,"Comando del visor de imagenes"),config->Files.img_viewer,mxID_IMG_VIEWER_PATH);
	files_browser_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_BROWSER,"Comando del navegador Web"),config->Files.browser_command,mxID_BROWSER_PATH);
	files_xdot_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_XDOT_COMMAND,"Comando del visor de grafos (xdot)"),config->Files.xdot_command,mxID_XDOT_PATH);
	files_doxygen_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_DOXYGEN,"Ubicaci�n del ejecutable de Doxygen"),config->Files.doxygen_command,mxID_DOXYGEN_PATH);
	files_wxfb_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_WXFORMBUILDER,"Ubicaci�n del ejecutable de wxFormBuilder"),config->Files.wxfb_command,mxID_WXFB_PATH);
#ifdef __WIN32__
	files_valgrind_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_VALGRIND,"Ubicaci�n del ejecutable de Valgrind"),config->Files.valgrind_command,mxID_VALGRIND_PATH);
#endif
	files_cppcheck_command = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_COMMANDS_CPPCHECK,"Ubicaci�n del ejecutable de CppCheck"),config->Files.cppcheck_command,mxID_CPPCHECK_PATH);
	
	panel->SetSizerAndFit(sizer);
	return panel;

}

static void RecreateAllToolbars ( ) {
	for(int tb_id=0;tb_id<MenusAndToolsConfig::tbPROJECT;tb_id++)
		menu_data->UpdateToolbar(tb_id,true);
	if (project) menu_data->UpdateToolbar(MenusAndToolsConfig::tbPROJECT,true);
}

void mxPreferenceWindow::OnOkButton(wxCommandEvent &event) {
	
	if (Toolchain::GetToolchain(files_toolchain->GetValue())->IsExtern()) {
		mxMessageDialog(this,LANG(PREFERENCES_CANNOT_DEFAULT_EXTERN_TOOLCHAIN,"La herramienta de compilaci�n por defecto (pesta�a Programa/Proyecto) no puede ser de tipo externa"),LANG(PREFERENCES_CAPTION,"Preferencias"),mxMD_WARNING|mxMD_OK).ShowModal();
		return;
	}
	
#ifdef __linux__
	if ( init_disable_ubuntu_tweaks->GetValue() == wxFileExists(DIR_PLUS_FILE(config->home_dir,"ubuntu")) ) {
		if (init_disable_ubuntu_tweaks->GetValue()) wxRemoveFile( DIR_PLUS_FILE(config->home_dir,"ubuntu") );
		else { wxTextFile fil( DIR_PLUS_FILE(config->home_dir,"ubuntu") ); fil.Create(); fil.Write(); }
	}
#endif
	
	long int l;
	config->Init.left_panels = init_left_panels->GetValue();
	config->Init.check_for_updates = init_check_for_updates->GetValue();
	config->Help.show_extra_panels = init_show_extra_panels->GetValue();
	config->Init.language_file = init_lang_file->GetValue();
	config->Init.show_welcome= init_show_welcome->GetValue();
	config->Init.show_tip_on_startup = init_show_tip_on_startup->GetValue();
	if (config->Init.show_welcome && !welcome_panel) {
		welcome_panel=new mxWelcomePanel(main_window);
		main_window->aui_manager.AddPane(welcome_panel, wxAuiPaneInfo().Name("welcome_panel").CenterPane().PaneBorder(false).Hide());
		if (!project && !main_window->notebook_sources->GetPageCount()) main_window->ShowWelcome(true);
	} else if (!config->Init.show_welcome && welcome_panel) {
		main_window->ShowWelcome(false);
		main_window->aui_manager.DetachPane(welcome_panel);
		welcome_panel->Destroy();
		welcome_panel=nullptr;
	}
	config->Init.always_add_extension = init_always_add_extension->GetValue();
#ifdef __WIN32__
#else
	config->Init.lang_es = init_lang_es->GetValue();
#endif
	wxSetEnv("LANG",(config->Init.lang_es?"es_ES":"en_US"));
	config->Init.save_project = init_save_project->GetValue();
	if (init_history_len->GetValue().ToLong(&l)) config->Init.history_len=l;
	if (config->Init.history_len>30) config->Init.history_len=30;
	else if (config->Init.history_len<5) config->Init.history_len=5;
	if (init_max_jobs->GetValue().ToLong(&l)) config->Init.max_jobs=l;
	if (config->Init.max_jobs<1) config->Init.max_jobs=1;
	config->Init.autohide_panels_fs = init_autohide_panels_fs->GetValue();
	config->Init.autohide_toolbars_fs = init_autohide_toolbars_fs->GetValue();
	config->Init.prefer_explorer_tree = init_prefer_explorer_tree->GetValue();
	config->Init.show_explorer_tree = init_show_explorer_tree->GetValue();
	config->Init.wrap_mode = init_wrap_mode->GetSelection();
	config->Init.new_file = init_new_file->GetSelection();
	config->Running.wait_for_key = running_wait_for_key->GetValue();
	config->Running.always_ask_args = running_always_ask_args->GetValue();
	config->Running.cpp_compiler_options = running_cpp_compiler_options->GetValue();
	config->Running.c_compiler_options = running_c_compiler_options->GetValue();
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
	config->Files.xdot_command = files_xdot_command->GetValue();
	config->Files.img_viewer = files_img_viewer_command->GetValue();
	config->Files.browser_command = files_browser_command->GetValue();
#ifdef __WIN32__
	config->Files.valgrind_command = files_valgrind_command->GetValue();
#endif
	config->Files.cppcheck_command = files_cppcheck_command->GetValue();
	config->Files.wxfb_command = files_wxfb_command->GetValue();
	config->Files.doxygen_command = files_doxygen_command->GetValue();
	config->Files.project_folder = files_project_folder->GetValue();
	config->Source.syntaxEnable = source_syntaxEnable->GetValue();
	if (source_tabWidth->GetValue().ToLong(&l)) config->Source.tabWidth=l;
	config->Source.tabUseSpaces = source_tabUseSpaces->GetValue();
	if (source_edgeColumnPos->GetValue().ToLong(&l)) config->Source.edgeColumn=l;
	if (!source_edgeColumnCheck->GetValue()) config->Source.edgeColumn = -config->Source.edgeColumn;
	config->Source.foldEnable = source_foldEnable->GetValue();
	config->Source.bracketInsertion = (source_smartIndent->GetValue() && source_bracketInsertion->GetValue());
	config->Source.smartIndent = (source_smartIndent->GetValue() && source_syntaxEnable->GetValue());
	config->Source.indentPaste = (source_smartIndent->GetValue() && source_indentPaste->GetValue());
	config->Source.avoidNoNewLineWarning = source_avoidNoNewLineWarning->GetValue();
	config->Source.whiteSpace = source_whiteSpace->GetValue();
	config->Source.whiteSpace = source_whiteSpace->GetValue();
	config->Source.lineNumber = source_lineNumber->GetValue();
	config->Source.toolTips = source_toolTips->GetValue();
	config->Init.use_cache_for_subcommands = init_use_cache_for_subcommands->GetValue();
	config->Init.beautify_compiler_errors = init_beautifyCompilerErrors->GetValue();
	config->Source.callTips = source_callTips->GetValue();
	config->Source.autocompTips = source_autocompTips->GetValue();
	config->Source.autoCompletion = source_autoCompletion->GetSelection();
	code_helper->SetAutocompletionMatchingMode(config->Source.autoCompletion);
	config->Source.autocompFilters = source_autocompFilters->GetValue();
	config->Source.autocloseStuff = source_autocloseStuff->GetValue();
	config->Help.wxhelp_index = help_wxhelp_index->GetValue();
	
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
		code_helper->ReloadIndexes(autocomp_indexes);
	}
	
	if (styles_print_size->GetValue().ToLong(&l)) config->Styles.print_size=l;
	if (styles_font_size->GetValue().ToLong(&l)) config->Styles.font_size=l;
	config->Styles.font_name=styles_font_name->GetValue();
//#ifdef __WIN32__
//	config->Files.mingw_dir = files_mingw_dir->GetValue();
//#endif
	wxAuiNotebook *ns=main_window->notebook_sources;
	for (unsigned int i=0;i<ns->GetPageCount();i++)
		((mxSource*)(ns->GetPage(i)))->LoadSourceConfig();
	main_window->SetStatusBarFields(); // ocultar/mostrar nro de linea
		
#ifdef __linux__
	config->Debug.enable_core_dump = debug_enable_core_dump->GetValue();
#endif
#ifdef __WIN32__
	config->Debug.no_debug_heap = debug_no_debug_heap->GetValue();
#endif
	config->Debug.auto_solibs = debug_auto_solibs->GetValue();
	config->Debug.readnow = debug_readnow->GetValue();
	config->Debug.allow_edition = debug_allow_edition->GetValue();
	if (debug->IsDebugging()) {
		for (unsigned int i=0;i<ns->GetPageCount();i++)
			((mxSource*)(ns->GetPage(i)))->SetReadOnlyMode(config->Debug.allow_edition?ROM_DEBUG:ROM_ADD_DEBUG);
	}
	config->Debug.autohide_panels = debug_autohide_panels->GetValue();
	config->Debug.autohide_toolbars = debug_autohide_panels->GetValue();
	config->Debug.compile_again = debug_compile_again->GetValue();
//	config->Debug.close_on_normal_exit = debug_close_on_normal_exit->GetValue();
	config->Debug.always_debug = debug_always_debug->GetValue();
	config->Debug.raise_main_window = debug_raise_main_window->GetValue();
	config->Debug.inspections_on_right = debug_inspections_on_right->GetValue();
	config->Debug.show_thread_panel = debug_show_thread_panel->GetValue();
	config->Debug.show_log_panel = debug_show_log_panel->GetValue();
//	config->Debug.use_colours_for_inspections = debug_use_colours_for_inspections->GetValue();
	config->Debug.inspections_can_have_side_effects = debug_inspections_can_have_side_effects->GetValue();
	config->Files.autocodes_file = files_autocode->GetValue();
	autocoder->Reset(project?project->autocodes_file:"");
	config->Debug.return_focus_on_continue = debug_return_focus_on_continue->GetValue();
	config->Debug.improve_inspections_by_type = debug_improve_inspections_by_type->GetValue();
	config->Debug.macros_file = debug_macros_file->GetValue();
	config->Debug.blacklist = temp_debug_blacklist;
	config->Debug.use_blacklist = debug_use_blacklist;
	debug->SetBlacklist();
	
	old_config_styles=config->Styles;
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
	config->Styles=old_config_styles;
	main_window->UpdateStylesInSources();
	Close();
}

void mxPreferenceWindow::OnClose(wxCloseEvent &event){
	Hide();
	event.Veto();
}

void mxPreferenceWindow::OnSkinButton(wxCommandEvent &event){
	int selection = skin_list->GetSelection();
	if (selection<0) return;
	config->Files.skin_dir=skin_paths[selection];
	RecreateAllToolbars(); main_window->SortToolbars(true);
	mxMessageDialog(main_window,LANG(PREFERENCES_THEME_WILL_APPLY_ON_RESTART,"El tema seleccionado se aplicara completamente la proxima vez que reinicie ZinjaI"),LANG(PREFERENCES_CAPTION,"Preferencias"), mxMD_OK|mxMD_INFO).ShowModal();
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
	wxFileDialog dlg(this,_T("Comando del visor de imagenes"),files_img_viewer_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_img_viewer_command->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnXdotButton(wxCommandEvent &event){
	wxFileDialog dlg(this,_T("Comando del visor de grafos xdot"),files_xdot_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_xdot_command->SetValue(dlg.GetPath());
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
	wxFileDialog dlg(this,"Ubicacion del ejecutable de CppCheck",files_cppcheck_command->GetValue());
	if (wxID_OK==dlg.ShowModal())
		files_cppcheck_command->SetValue(dlg.GetPath());
}

void mxPreferenceWindow::OnHelpButton(wxCommandEvent &event){
	mxHelpWindow::ShowHelp("preferences.html");
}

#ifdef __WIN32__
//
//void mxPreferenceWindow::OnMingwButton(wxCommandEvent &event){
//	wxDirDialog dlg(this,_T("Directorio de binarios de MinGW:"),files_mingw_dir->GetValue());
//	if (wxID_OK==dlg.ShowModal())
//		files_mingw_dir->SetValue(dlg.GetPath());
// }

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
		mxMessageDialog(main_window,"No se ha encontrado una terminal conocida.\nInstale xterm,konsole o gnome-terminal; o\nconfigure el parametro \"Comando del\nTerminal\" en el cuadro de Preferencias.\"","Terminal de ejecucion", mxMD_OK|mxMD_WARNING).ShowModal();
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
	if (wxFileName(file).FileExists())
		main_window->OpenFileFromGui(file,&i);
	else
		mxMessageDialog(main_window,_T("El archivo no existe"),_T("Archivo de definiciones de autoc�digos"), mxMD_OK|mxMD_WARNING).ShowModal();	
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
	if (wxFileName(file).FileExists())
		main_window->OpenFileFromGui(file,&i);
	else
		mxMessageDialog(main_window,_T("El archivo no existe"),LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros para gdb"), mxMD_OK|mxMD_WARNING).ShowModal();	
}

mxPreferenceWindow *mxPreferenceWindow::ShowUp() {
	if (preference_window) {
		preference_window->ResetChanges();
		preference_window->Show();
		preference_window->Raise();
	} else {
		preference_window = new mxPreferenceWindow(main_window);
	}
	return preference_window;
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
	if (mxMD_YES == mxMessageDialog(main_window,LANG(PREFERENCES_TOOLBARS_RESET_WARNING,"Perdera todas las modificiaciones realizadas a las barras\n de herramientas. �Desea continuar?"),LANG(PREFERENCES_CAPTION,"Preferencias"), mxMD_YES_NO).ShowModal()) {
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
	old_config_styles=config->Styles;
	
	// paths
	help_wxhelp_index->SetValue(config->Help.wxhelp_index);
	files_temp_dir->SetValue(config->Files.temp_dir);
//#ifdef __WIN32__
//	files_mingw_dir->SetValue(config->Files.mingw_dir);
//#endif
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
	init_use_cache_for_subcommands->SetValue(config->Init.use_cache_for_subcommands);
	init_beautifyCompilerErrors->SetValue(config->Init.beautify_compiler_errors);
	source_autoCompletion->SetSelection(config->Source.autoCompletion);
	source_autocloseStuff->SetValue(config->Source.autocloseStuff);
	source_autocompFilters->SetValue(config->Source.autocompFilters);
	source_callTips->SetValue(config->Source.callTips);
	source_autocompTips->SetValue(config->Source.autocompTips);
	
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
	source_syntaxEnable->SetValue(config->Source.syntaxEnable);
	source_whiteSpace->SetValue(config->Source.whiteSpace);
	source_lineNumber->SetValue(config->Source.lineNumber);
	source_foldEnable->SetValue(config->Source.foldEnable);
	source_tabWidth->SetValue(wxString()<<config->Source.tabWidth);
	source_tabUseSpaces->SetValue(config->Source.tabUseSpaces);
	source_edgeColumnCheck->SetValue(config->Source.edgeColumn>0);
	source_edgeColumnPos->SetValue(wxString()<<abs(config->Source.edgeColumn));
	styles_print_size->SetValue(wxString()<<int(config->Styles.print_size));
	styles_font_size->SetValue(wxString()<<int(config->Styles.font_size));
	styles_font_name->SetValue(config->Styles.font_name); 
	wxCommandEvent cmd_evt; OnFontChange(cmd_evt);
	init_autohide_menus_fs->SetValue(config->Init.autohide_menus_fs);
	init_autohide_toolbars_fs->SetValue(config->Init.autohide_toolbars_fs);
	init_autohide_panels_fs->SetValue(config->Init.autohide_panels_fs);
	files_autocode->SetValue(config->Files.autocodes_file);
	
	// simple
	running_cpp_compiler_options->SetValue(config->Running.cpp_compiler_options);
	running_c_compiler_options->SetValue(config->Running.c_compiler_options);
	running_wait_for_key->SetValue(config->Running.wait_for_key);
	running_always_ask_args->SetValue(config->Running.always_ask_args);
	init_always_add_extension->SetValue(config->Init.always_add_extension);
	init_autohide_panels->SetValue(config->Init.autohide_panels);
	init_stop_compiling_on_error->SetValue(config->Init.stop_compiling_on_error);
	init_singleton->SetValue(config->Init.singleton);
	init_max_jobs->SetValue(wxString()<<config->Init.max_jobs);
	
	// general
	init_show_extra_panels->SetValue(config->Help.show_extra_panels);
	init_lang_file->SetValue(config->Init.language_file);
	init_show_welcome->SetValue(config->Init.show_welcome);
	init_show_tip_on_startup->SetValue(config->Init.show_tip_on_startup);
	init_wrap_mode->SetSelection(config->Init.wrap_mode);
	init_new_file->SetSelection(config->Init.new_file);
	init_history_len->SetValue(wxString()<<config->Init.history_len);
	init_save_project->SetValue(config->Init.save_project);
//	init_close_files_for_project->SetValue(config->Init.close_files_for_project);
	init_prefer_explorer_tree->SetValue(config->Init.prefer_explorer_tree);
	init_show_explorer_tree->SetValue(config->Init.show_explorer_tree);
	init_left_panels->SetValue(config->Init.left_panels);
	init_check_for_updates->SetValue(config->Init.check_for_updates);
#ifdef __linux__
	init_disable_ubuntu_tweaks->SetValue(!wxFileExists(DIR_PLUS_FILE(config->home_dir,"ubuntu")));
#endif
#ifndef __WIN32__
	init_lang_es->SetValue(config->Init.lang_es);
//	desktop_icon->SetValue(false);
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
#ifdef __linux__
	debug_enable_core_dump->SetValue(config->Debug.enable_core_dump);
#endif
#ifdef __WIN32__
	debug_no_debug_heap->SetValue(config->Debug.no_debug_heap);
#endif
	debug_auto_solibs->SetValue(config->Debug.auto_solibs);
	debug_readnow->SetValue(config->Debug.readnow);
	debug_allow_edition->SetValue(config->Debug.allow_edition);
	debug_autohide_panels->SetValue(config->Debug.autohide_panels);
	debug_autohide_panels->SetValue(config->Debug.autohide_toolbars);
	debug_compile_again->SetValue(config->Debug.compile_again);
//	debug_close_on_normal_exit->SetValue(config->Debug.close_on_normal_exit);
	debug_always_debug->SetValue(config->Debug.always_debug);
	debug_raise_main_window->SetValue(config->Debug.raise_main_window);
	debug_inspections_on_right->SetValue(config->Debug.inspections_on_right);
	debug_show_thread_panel->SetValue(config->Debug.show_thread_panel);
	debug_show_log_panel->SetValue(config->Debug.show_log_panel);
//	debug_use_colours_for_inspections->SetValue(config->Debug.use_colours_for_inspections);
	debug_inspections_can_have_side_effects->SetValue(config->Debug.inspections_can_have_side_effects);
	debug_return_focus_on_continue->SetValue(config->Debug.return_focus_on_continue);
	debug_improve_inspections_by_type->SetValue(config->Debug.improve_inspections_by_type);
	debug_macros_file->SetValue(config->Debug.macros_file);
	debug_use_blacklist->SetValue(config->Debug.use_blacklist);
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
		"m�s detalles. ZinjaI detecta en el sistema actual <{1}> nucleos",wxString()<<wxThread::GetCPUCount()),
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
	if (preference_window) preference_window->Destroy();
	preference_window=nullptr;
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
