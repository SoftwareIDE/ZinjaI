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

BEGIN_EVENT_TABLE(mxProjectGeneralConfig, wxDialog)
	EVT_BUTTON(wxID_OK,mxProjectGeneralConfig::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxProjectGeneralConfig::OnCancelButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_AUTOCOMP_INDEXES,mxProjectGeneralConfig::OnIndexesButton)
	EVT_BUTTON(mxID_RUN_CONFIG,mxProjectGeneralConfig::OnCompileConfigButton)
	EVT_BUTTON(mxID_PROJECT_CONFIG_CUSTOM_TOOLS,mxProjectGeneralConfig::OnCustomToolsConfig)
	EVT_BUTTON(mxID_TOOLS_DOXY_CONFIG,mxProjectGeneralConfig::OnDoxigenConfigButton)
	EVT_BUTTON(mxID_DEBUG_MACROS,mxProjectGeneralConfig::OnDebugMacros)
	EVT_MENU(mxID_DEBUG_MACROS_OPEN,mxProjectGeneralConfig::OnDebugMacrosOpen)
	EVT_MENU(mxID_DEBUG_MACROS_EDIT,mxProjectGeneralConfig::OnDebugMacrosEdit)
	EVT_BUTTON(mxID_AUTOCODES_FILE,mxProjectGeneralConfig::OnAutocodes)
	EVT_MENU(mxID_AUTOCODES_OPEN,mxProjectGeneralConfig::OnAutocodesOpen)
	EVT_MENU(mxID_AUTOCODES_EDIT,mxProjectGeneralConfig::OnAutocodesEdit)
	EVT_BUTTON(mxID_HELP_BUTTON,mxProjectGeneralConfig::OnHelpButton)
	EVT_CLOSE(mxProjectGeneralConfig::OnClose)
END_EVENT_TABLE()

mxProjectGeneralConfig::mxProjectGeneralConfig() : wxDialog(main_window, wxID_ANY, LANG(PROJECTGENERAL_CAPTION,"Configuracion de Proyecto"), wxDefaultPosition, wxDefaultSize ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);

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
	
	project_name = utils->AddTextCtrl(mySizer,this,LANG(PROJECTGENERAL_NAME,"Nombre del proyecto"),project->project_name);
	custom_tab = utils->AddCheckBox(mySizer,this,LANG(PROJECTGENERAL_CUSTOM_TABS,"Utilizar tabulado propio"),project->custom_tabs!=0);
	tab_width = utils->AddTextCtrl(mySizer,this,LANG(PROJECTGENERAL_TAB_WIDTH,"Ancho del tabulado"),project->custom_tabs==0?config->Source.tabWidth:project->custom_tabs,true);
	tab_use_spaces = utils->AddCheckBox(mySizer,this,LANG(PROJECTGENERAL_TAB_SPACE,"Colocar espacios en lugar de tabs"),project->custom_tabs==0?config->Source.tabUseSpaces:project->tab_use_spaces,wxID_ANY);
	project_autocomp = utils->AddDirCtrl(mySizer,this,LANG(PROJECTGENERAL_AUTOCOMP_EXTRA,"Indices de autocompletado adicionales"),project->autocomp_extra,mxID_PROJECT_CONFIG_AUTOCOMP_INDEXES);
	project_autocodes = utils->AddDirCtrl(mySizer,this,LANG(PREFERENCES_WRITTING_AUTOCODES_FILE,"Archivo de definiciones de autocódigos"),project->autocodes_file,mxID_AUTOCODES_FILE);
	project_debug_macros = utils->AddDirCtrl(mySizer,this,LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros para gdb"),project->macros_file,mxID_DEBUG_MACROS);
	
	mySizer->Add(new wxButton(this,mxID_RUN_CONFIG,LANG(PROJECTGENERAL_COMPILE_AND_RUN," Compilacion y Ejecucion... ")),sizers->BA5);
	mySizer->Add(new wxButton(this,mxID_TOOLS_DOXY_CONFIG,LANG(PROJECTGENERAL_DOXYGEN," Opciones Doxygen... ")),sizers->BA5);
	mySizer->Add(new wxButton(this,mxID_PROJECT_CONFIG_CUSTOM_TOOLS,LANG(PROJECTGENERAL_CUSTOM_TOOLS," Herramientas Personalizadas... ")),sizers->BA5);
//	use_wxfb = utils->AddCheckBox(mySizer,this,LANG(PROJECTGENERAL_ACTIVATE_WXFORMBUILDER,"Activar integracion con wxFormBuilder"),project->use_wxfb,wxID_ANY);
	
	mySizer->Add(bottomSizer,sizers->Exp0);
	
	SetSizerAndFit(mySizer);
	
	SetFocus();
	Show();
	
}

mxProjectGeneralConfig::~mxProjectGeneralConfig() {
	
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
		code_helper->ReloadIndexes(config->Help.autocomp_indexes+_T(" ")+project->autocomp_extra);
	}
//	main_window->menu.tools_wxfb_activate->Check(project->use_wxfb);
	if (project->autocodes_file != project_autocodes->GetValue()) {
		project->autocodes_file = project_autocodes->GetValue();
		autocoder->Reset(DIR_PLUS_FILE(project->path,project->autocodes_file));
	}
	Close();
}

void mxProjectGeneralConfig::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxProjectGeneralConfig::OnCompileConfigButton(wxCommandEvent &evt) {
	main_window->OnRunCompileConfig(evt);
}

void mxProjectGeneralConfig::OnDoxigenConfigButton(wxCommandEvent &evt) {
	main_window->OnToolsDoxyConfig(evt);
}

void mxProjectGeneralConfig::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxProjectGeneralConfig::OnHelpButton(wxCommandEvent &event) {
	SHOW_HELP(_T("project1_general_config.html"));
}

void mxProjectGeneralConfig::OnIndexesButton(wxCommandEvent &evt) {

	wxArrayString autocomp_array_all;
	utils->GetFilesFromBothDirs(autocomp_array_all,"autocomp");
	
	mxMultipleChoiceEditor(this,LANG(PROJECTGENERAL_AUTOCOMPLETION,"Autocompletado"),LANG(PROJECTGENERAL_INDEXES,"Indices:"),project_autocomp,autocomp_array_all);
	
}

void mxProjectGeneralConfig::OnDebugMacrosEdit(wxCommandEvent &evt) {
	wxString file=project_debug_macros->GetValue();
	if (wxFileName(DIR_PLUS_FILE(project->path,file)).FileExists())
		main_window->OpenFileFromGui(DIR_PLUS_FILE(project->path,file),NULL);
	else
		mxMessageDialog(main_window,LANG(PROJECTGENERAL_FILE_NOT_EXISTS,"El archivo no existe"),LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros"), mxMD_OK|mxMD_WARNING).ShowModal();	
}

void mxProjectGeneralConfig::OnDebugMacrosOpen(wxCommandEvent &evt) {
	wxFileDialog dlg(this,LANG(PREFERENCES_DEBUG_GDB_MACROS_FILE,"Archivo de macros para gdb"),_T(""),DIR_PLUS_FILE(project->path,project_debug_macros->GetValue()));
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
		main_window->OpenFileFromGui(DIR_PLUS_FILE(project->path,file),NULL);
	else
		mxMessageDialog(main_window,LANG(PROJECTGENERAL_FILE_NOT_EXISTS,"El archivo no existe"),LANG(PREFERENCES_WRITTING_AUTOCODES_FILE,"Archivo con definiciones de autocodigos"), mxMD_OK|mxMD_WARNING).ShowModal();	
}

void mxProjectGeneralConfig::OnAutocodesOpen(wxCommandEvent &evt) {
	wxFileDialog dlg(this,LANG(PREFERENCES_WRITTING_AUTOCODES_FILE,"Archivo con definiciones de autocodigos"),_T(""),DIR_PLUS_FILE(project->path,project_autocodes->GetValue()));
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

