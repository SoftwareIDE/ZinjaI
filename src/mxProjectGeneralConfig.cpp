#include <wx/dir.h>
#include <wx/choicdlg.h>
#include "mxProjectGeneralConfig.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "ProjectManager.h"
#include "ids.h"
#include "ConfigManager.h"
#include "mxHelpWindow.h"
#include "CodeHelper.h"
#include "mxSource.h"
#include "mxMessageDialog.h"
#include "Language.h"
#include "Autocoder.h"
#include "mxMultipleChoiceEditor.h"
#include "mxBySourceCompilingOpts.h"
#include "mxInspectionsImprovingEditor.h"

BEGIN_EVENT_TABLE(mxProjectGeneralConfig, wxDialog)
	EVT_BUTTON(wxID_OK,mxProjectGeneralConfig::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxProjectGeneralConfig::OnCancelButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_AUTOCOMP_INDEXES,mxProjectGeneralConfig::OnIndexesButton)
	EVT_BUTTON(mxID_RUN_CONFIG,mxProjectGeneralConfig::OnCompileConfigButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_CUSTOM_TOOLS,mxProjectGeneralConfig::OnCustomToolsConfig)
	EVT_BUTTON(mxID_TOOLS_DOXY_CONFIG,mxProjectGeneralConfig::OnDoxygenConfigButton)
	EVT_BUTTON(mxID_DEBUG_MACROS,mxProjectGeneralConfig::OnDebugMacros)
	EVT_BUTTON(mxID_TOOLS_CPPCHECK_CONFIG,mxProjectGeneralConfig::OnCppCheckConfig)
	EVT_BUTTON(mxID_TOOLS_WXFB_CONFIG,mxProjectGeneralConfig::OnWxfbConfig)
	EVT_BUTTON(mxID_PROJECT_CONFIG_BYSRC,mxProjectGeneralConfig::OnBySrcCompilingOts)
	EVT_BUTTON(mxID_PROJECT_CONFIG_AUTOIMPROVE_TEMPLATES,mxProjectGeneralConfig::OnAutoimprovingInspections)
	EVT_BUTTON(mxID_TOOLS_DRAW_PROJECT,mxProjectGeneralConfig::OnDrawGraph)
	EVT_BUTTON(mxID_TOOLS_PROJECT_STATISTICS,mxProjectGeneralConfig::OnStatistics)
	EVT_MENU(mxID_DEBUG_MACROS_OPEN,mxProjectGeneralConfig::OnDebugMacrosOpen)
	EVT_MENU(mxID_DEBUG_MACROS_EDIT,mxProjectGeneralConfig::OnDebugMacrosEdit)
	EVT_BUTTON(mxID_AUTOCODES_FILE,mxProjectGeneralConfig::OnAutocodes)
	EVT_MENU(mxID_AUTOCODES_OPEN,mxProjectGeneralConfig::OnAutocodesOpen)
	EVT_MENU(mxID_AUTOCODES_EDIT,mxProjectGeneralConfig::OnAutocodesEdit)
	EVT_BUTTON(mxID_HELP_BUTTON,mxProjectGeneralConfig::OnHelpButton)
	EVT_CHECKBOX(mxID_PROJECT_CONFIG_CUSTOM_TABS,mxProjectGeneralConfig::OnCustomTabs)
	EVT_CLOSE(mxProjectGeneralConfig::OnClose)
END_EVENT_TABLE()

mxProjectGeneralConfig::mxProjectGeneralConfig() : wxDialog(main_window, wxID_ANY, LANG(PROJECTGENERAL_CAPTION,"Configuracion de Proyecto"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxNotebook *notebook = new wxNotebook(this,wxID_ANY);
	notebook->AddPage(CreateTabGeneral(notebook), LANG(PROJECTCONFIG_TAB_GENERAL,"General"));
	notebook->AddPage(CreateTabAdvanced(notebook), LANG(PROJECTCONFIG_TAB_MORE,"Más"));
	mySizer->Add(notebook,sizers->Exp1);
	mySizer->Add(mxUT::MakeGenericButtonsSizer(this,true),sizers->Exp0);
	SetSizerAndFit(mySizer);
	Show(); SetFocus();
}

wxPanel *mxProjectGeneralConfig::CreateTabGeneral(wxNotebook *notebook) {
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	project_name = mxUT::AddTextCtrl(sizer,panel,LANG(PROJECTGENERAL_NAME,"Nombre del proyecto"),project->project_name);
	custom_tab = mxUT::AddCheckBox(sizer,panel,LANG(PROJECTGENERAL_CUSTOM_TABS,"Utilizar tabulado propio"),project->custom_tabs!=0,mxID_PROJECT_CONFIG_CUSTOM_TABS);
	tab_width = mxUT::AddTextCtrl(sizer,panel,LANG(PROJECTGENERAL_TAB_WIDTH,"Ancho del tabulado"),project->custom_tabs==0?config->Source.tabWidth:project->custom_tabs,true);
	tab_use_spaces = mxUT::AddCheckBox(sizer,panel,LANG(PROJECTGENERAL_TAB_SPACE,"Colocar espacios en lugar de tabs"),project->custom_tabs==0?config->Source.tabUseSpaces:project->tab_use_spaces,wxID_ANY,true);
	
	mxUT::AddStaticText(sizer,panel,LANG(PROJECTGENERAL_PREFERRED_EXTENSIONS,"Extensiones preferidas:"),false);
	wxBoxSizer *ext_sizer = new wxBoxSizer(wxHORIZONTAL);
	ext_sizer->AddSpacer(10);
	default_fext_source = mxUT::AddShortTextCtrl(ext_sizer,panel,LANG(PROJECTGENERAL_DEFAULT_EXTENSIONS_SOURCE,"Fuentes"),project->default_fext_source);
	default_fext_header = mxUT::AddShortTextCtrl(ext_sizer,panel,LANG(PROJECTGENERAL_DEFAULT_EXTENSIONS_HEADERS,"Cabeceras"),project->default_fext_header);
	sizer->Add(ext_sizer,sizers->Exp0);
	
	project_autocomp = mxUT::AddDirCtrl(sizer,panel,LANG(PROJECTGENERAL_AUTOCOMP_EXTRA,"Indices de autocompletado adicionales"),project->autocomp_extra,mxID_PROJECT_CONFIG_AUTOCOMP_INDEXES);
	project_autocodes = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_WRITTING_AUTOCODES_FILE,"Archivo de definiciones de autocódigos"),project->autocodes_file,mxID_AUTOCODES_FILE);
	project_debug_macros = mxUT::AddDirCtrl(sizer,panel,LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros para gdb"),project->macros_file,mxID_DEBUG_MACROS);
	tab_width->Enable(custom_tab->GetValue());
	tab_use_spaces->Enable(custom_tab->GetValue());
	sizer->Add(new wxButton(panel,mxID_PROJECT_CONFIG_AUTOIMPROVE_TEMPLATES,LANG(PROJECTGENERAL_AUTOIMPROVE_TEMPLATES," Mejora de inspecciones según tipo ")),sizers->BA5);
	panel->SetSizer(sizer);
	return panel;
}

wxPanel *mxProjectGeneralConfig::CreateTabAdvanced(wxNotebook *notebook) {
	wxPanel *panel = new wxPanel(notebook, wxID_ANY );
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(new wxButton(panel,mxID_RUN_CONFIG,LANG(PROJECTGENERAL_COMPILE_AND_RUN," Compilacion y Ejecucion (generales)... ")),sizers->BA5_Center);
	sizer->Add(new wxButton(panel,mxID_PROJECT_CONFIG_BYSRC,LANG(PROJECTGENERAL_BYSRC_OPTIONS," Opciones de Compilacion (por fuente)... ")),sizers->BA5_Center);
	sizer->Add(new wxButton(panel,mxID_TOOLS_DOXY_CONFIG,LANG(PROJECTGENERAL_DOXYGEN," Configuración Doxygen... ")),sizers->BA5_Center);
	sizer->Add(new wxButton(panel,mxID_TOOLS_CPPCHECK_CONFIG,LANG(PROJECTGENERAL_CPPCHECK," Configuración CppCheck... ")),sizers->BA5_Center);
	sizer->Add(new wxButton(panel,mxID_TOOLS_WXFB_CONFIG,LANG(PROJECTGENERAL_WXFB," Integración con wxFormBuilder... ")),sizers->BA5_Center);
	sizer->Add(new wxButton(panel,mxID_PROJECT_CONFIG_CUSTOM_TOOLS,LANG(PROJECTGENERAL_CUSTOM_TOOLS," Herramientas Personalizadas... ")),sizers->BA5_Center);
	sizer->Add(new wxButton(panel,mxID_TOOLS_PROJECT_STATISTICS,LANG(PROJECTGENERAL_STATISTICS," Estadísticas... ")),sizers->BA5_Center);
	sizer->Add(new wxButton(panel,mxID_TOOLS_DRAW_PROJECT,LANG(PROJECTGENERAL_GRAPH," Grafo de archivos... ")),sizers->BA5_Center);
	panel->SetSizer(sizer);
	return panel;
	
}

void mxProjectGeneralConfig::OnOkButton(wxCommandEvent &evt) {
	if (!project) {Close(); return;}
	if (project->project_name!=project_name->GetValue()) {
		project->project_name=project_name->GetValue();
		main_window->SetOpenedFileName(project->project_name);
	}
	if (custom_tab->GetValue()) {
		long l=0;
		tab_width->GetValue().ToLong(&l); project->custom_tabs=l;
		project->tab_use_spaces=tab_use_spaces->GetValue();
		wxAuiNotebook *ns=main_window->notebook_sources;
		for (unsigned int i=0;i<ns->GetPageCount();i++)
			((mxSource*)(ns->GetPage(i)))->LoadSourceConfig();
			
	} else 
		project->custom_tabs=0;
//	project->use_wxfb = use_wxfb->GetValue();
	project->macros_file = project_debug_macros->GetValue();
	if (project->autocomp_extra != project_autocomp->GetValue()) {
		project->autocomp_extra = project_autocomp->GetValue();
		g_code_helper->ReloadIndexes(config->Help.autocomp_indexes+" "+project->autocomp_extra);
	}
//	main_window->menu.tools_wxfb_activate->Check(project->use_wxfb);
	if (project->autocodes_file != project_autocodes->GetValue()) {
		project->autocodes_file = project_autocodes->GetValue();
		g_autocoder->Reset(DIR_PLUS_FILE(project->path,project->autocodes_file));
	}
	project->default_fext_header = default_fext_header->GetValue();
	project->default_fext_source = default_fext_source->GetValue();
	Close();
}

void mxProjectGeneralConfig::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxProjectGeneralConfig::OnCompileConfigButton(wxCommandEvent &evt) {
	main_window->OnRunCompileConfig(evt);
}

void mxProjectGeneralConfig::OnDoxygenConfigButton(wxCommandEvent &evt) {
	main_window->OnToolsDoxyConfig(evt);
}

void mxProjectGeneralConfig::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxProjectGeneralConfig::OnHelpButton(wxCommandEvent &event) {
	mxHelpWindow::ShowHelp("project1_general_config.html");
}

void mxProjectGeneralConfig::OnIndexesButton(wxCommandEvent &evt) {
	wxArrayString autocomp_array_all;
	mxUT::GetFilesFromBothDirs(autocomp_array_all,"autocomp");
	mxMultipleChoiceEditor(this,LANG(PROJECTGENERAL_AUTOCOMPLETION,"Autocompletado"),LANG(PROJECTGENERAL_INDEXES,"Indices:"),project_autocomp,autocomp_array_all);
}

void mxProjectGeneralConfig::OnDebugMacrosEdit(wxCommandEvent &evt) {
	wxString file=project_debug_macros->GetValue();
	if (wxFileName(DIR_PLUS_FILE(project->path,file)).FileExists())
		main_window->OpenFileFromGui(DIR_PLUS_FILE(project->path,file),nullptr);
	else
		mxMessageDialog(main_window,LANG(PROJECTGENERAL_FILE_NOT_EXISTS,"El archivo no existe"),LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros"), mxMD_OK|mxMD_WARNING).ShowModal();	
}

void mxProjectGeneralConfig::OnDebugMacrosOpen(wxCommandEvent &evt) {
	wxFileDialog dlg(this,LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros para gdb"),"",DIR_PLUS_FILE(project->path,project_debug_macros->GetValue()));
	if (wxID_OK==dlg.ShowModal()) {
		wxFileName fn(dlg.GetPath());
		fn.MakeRelativeTo(project->path);
		project_debug_macros->SetValue(fn.GetFullPath());
	}
}

void mxProjectGeneralConfig::OnDebugMacros(wxCommandEvent &evt) {
	wxMenu menu;
	menu.Append(mxID_DEBUG_MACROS_OPEN,LANG(PREFERENCES_OPEN_FILE,"&Buscar archivo..."));
	menu.Append(mxID_DEBUG_MACROS_EDIT,LANG(PREFERENCES_EDIT_FILE,"&Editar archivo..."));
	PopupMenu(&menu);
}

void mxProjectGeneralConfig::OnAutocodesEdit(wxCommandEvent &evt) {
	wxString file=project_autocodes->GetValue();
	if (wxFileName(DIR_PLUS_FILE(project->path,file)).FileExists())
		main_window->OpenFileFromGui(DIR_PLUS_FILE(project->path,file),nullptr);
	else
		mxMessageDialog(main_window,LANG(PROJECTGENERAL_FILE_NOT_EXISTS,"El archivo no existe"),LANG(PREFERENCES_WRITTING_AUTOCODES_FILE,"Archivo con definiciones de autocodigos"), mxMD_OK|mxMD_WARNING).ShowModal();	
}

void mxProjectGeneralConfig::OnAutocodesOpen(wxCommandEvent &evt) {
	wxFileDialog dlg(this,LANG(PREFERENCES_WRITTING_AUTOCODES_FILE,"Archivo con definiciones de autocodigos"),"",DIR_PLUS_FILE(project->path,project_autocodes->GetValue()));
	if (wxID_OK==dlg.ShowModal()) {
		wxFileName fn(dlg.GetPath());
		fn.MakeRelativeTo(project->path);
		project_autocodes->SetValue(fn.GetFullPath());
	}
}

void mxProjectGeneralConfig::OnAutocodes(wxCommandEvent &evt) {
	wxMenu menu;
	menu.Append(mxID_AUTOCODES_OPEN,LANG(PREFERENCES_OPEN_FILE,"&Buscar archivo..."));
	menu.Append(mxID_AUTOCODES_EDIT,LANG(PREFERENCES_EDIT_FILE,"&Editar archivo..."));
	PopupMenu(&menu);
}

void mxProjectGeneralConfig::OnCustomToolsConfig (wxCommandEvent & evt) {
	new mxCustomTools(true,-1);
}

void mxProjectGeneralConfig::OnCustomTabs (wxCommandEvent & evt) {
	evt.Skip();
	tab_width->Enable(custom_tab->GetValue());
	tab_use_spaces->Enable(custom_tab->GetValue());
}

void mxProjectGeneralConfig::OnWxfbConfig (wxCommandEvent & evt) {
	main_window->OnToolsWxfbConfig(evt);
}

void mxProjectGeneralConfig::OnCppCheckConfig (wxCommandEvent & evt) {
	main_window->OnToolsCppCheckConfig(evt);
}

void mxProjectGeneralConfig::OnBySrcCompilingOts (wxCommandEvent & evt) {
	new mxBySourceCompilingOpts(main_window,nullptr);
}

void mxProjectGeneralConfig::OnDrawGraph (wxCommandEvent & evt) {
	main_window->OnToolsDrawProject(evt);
}

void mxProjectGeneralConfig::OnStatistics (wxCommandEvent & evt) {
	main_window->OnToolsProjectStatistics(evt);
}

void mxProjectGeneralConfig::OnAutoimprovingInspections (wxCommandEvent & evt) {
	mxInspectionsImprovingEditor(this,
		project->inspection_improving_template_from,project->inspection_improving_template_to);
}

