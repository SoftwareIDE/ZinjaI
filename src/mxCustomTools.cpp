#include "mxCustomTools.h"
#include "mxMainWindow.h"
#include "mxSizers.h"
#include "mxUtils.h"
#include "ids.h"
#include "Language.h"
#include <wx/combobox.h>
#include "mxBitmapButton.h"
#include "ConfigManager.h"
#include "mxHelpWindow.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"

BEGIN_EVENT_TABLE(mxCustomTools, wxDialog)
	EVT_BUTTON(wxID_OK,mxCustomTools::OnButtonOk)
	EVT_BUTTON(mxID_CUSTOM_TOOLS_RUN,mxCustomTools::OnButtonTest)
	EVT_BUTTON(wxID_CANCEL,mxCustomTools::OnButtonCancel)
	EVT_BUTTON(mxID_HELP_BUTTON,mxCustomTools::OnButtonHelp)
	EVT_COMBOBOX(mxID_CUSTOM_TOOLS_COMBO,mxCustomTools::OnComboChange)
	EVT_BUTTON(mxID_CUSTOM_TOOLS_COMMAND,mxCustomTools::OnCommandPopup)
	EVT_BUTTON(mxID_CUSTOM_TOOLS_WORKDIR,mxCustomTools::OnWorkdirPopup)
	EVT_MENU_RANGE(mxID_POPUPS_INSERT_FIRST, mxID_POPUPS_INSERT_LAST, mxCustomTools::OnPopup)
	EVT_CLOSE(mxCustomTools::OnClose)
END_EVENT_TABLE()

int mxCustomTools::prev_sel=0;
	
mxCustomTools::mxCustomTools(int cual):wxDialog(main_window,wxID_ANY,LANG(CUSTOM_TOOLS_CAPTION,"Herramientas Personalizables"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	if (cual<0) cual=prev_sel; else prev_sel=cual;
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxArrayString array;
	for (int i=0;i<10;i++) {
		names[i]=config->CustomTools.names[i];
		commands[i]=config->CustomTools.commands[i];
		console[i]=config->CustomTools.console[i];
		ontoolbar[i]=config->Toolbars.tools.custom[i];
		array.Add(wxString(LANG(CUSTOM_TOOLS_ITEM,"Herramienta "))<<i);
	}
	the_combo = utils->AddComboBox(sizer,this,LANG(CUSTOM_TOOLS_COMBO,"Herramienta a editar"),array,cual,mxID_CUSTOM_TOOLS_COMBO);
	name_ctrl  =utils->AddTextCtrl(sizer,this,LANG(CUSTOM_TOOLS_NAME,"Nombre"),config->CustomTools.names[cual]);
	command_ctrl = utils->AddDirCtrl(sizer,this,LANG(CUSTOM_TOOLS_COMMAND,"Comando"),config->CustomTools.commands[cual],mxID_CUSTOM_TOOLS_COMMAND);
	workdir_ctrl = utils->AddDirCtrl(sizer,this,LANG(CUSTOM_TOOLS_WORKDIR,"Directorio de trabajo"),config->CustomTools.workdirs[cual],mxID_CUSTOM_TOOLS_WORKDIR);
#if defined(__WIN32__)
	console_ctrl = utils->AddCheckBox(sizer,this,LANG(CUSTOM_TOOLS_CONSOLE_WINDOWS,"Ocultar ventana"),config->CustomTools.console[cual]);
#else
	console_ctrl = utils->AddCheckBox(sizer,this,LANG(CUSTOM_TOOLS_CONSOLE_LINUX,"Correr en terminal"),config->CustomTools.console[cual]);
#endif
	ontoolbar_ctrl = utils->AddCheckBox(sizer,this,LANG(CUSTOM_TOOLS_ONTOOLBAR_LINUX,"Mostrar en la barra de herramientas"),config->Toolbars.tools.custom[cual]);
	
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	SetEscapeId(wxID_CANCEL);
	wxButton *ok_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	wxButton *run_button = new mxBitmapButton (this,mxID_CUSTOM_TOOLS_RUN,bitmaps->buttons.ok,LANG(CUSTOM_TOOLS_TEST,"&Ejecutar"));
	ok_button->SetDefault(); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	
	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(run_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	sizer->Add(bottomSizer,sizers->Exp0);
	
	SetSizerAndFit(sizer);
	
	Show();
	
	the_combo->SetFocus();
	
}

void mxCustomTools::OnCommandPopup(wxCommandEvent &event) {
	utils->ShowTextPopUp(this,LANG(CUSTOM_TOOLS_COMMAND,"Comando"),command_ctrl,"TEXT|LIST|FILE|DIR|TEMP_DIR|MINGW_DIR|BROWSER|PROJECT_PATH|PROJECT_BIN|CURRENT_FILE|CURRENT_DIR|ZINJAI_DIR");
}

void mxCustomTools::OnWorkdirPopup(wxCommandEvent &event) {
	utils->ShowTextPopUp(this,LANG(CUSTOM_TOOLS_WORKDIR,"Directorio de trabajo"),workdir_ctrl,"REPLACE|DIR|TEMP_DIR|MINGW_DIR|PROJECT_PATH|CURRENT_DIR|ZINJAI_DIR");
}


void mxCustomTools::OnPopup(wxCommandEvent &evt) {
	utils->ProcessTextPopup(evt.GetId());
}

void mxCustomTools::OnButtonOk(wxCommandEvent &event) {
	commands[prev_sel]=command_ctrl->GetValue();
	names[prev_sel]=name_ctrl->GetValue();
	console[prev_sel]=console_ctrl->GetValue();
	ontoolbar[prev_sel]=ontoolbar_ctrl->GetValue();
	bool someone_ontoolbar=false;
	for (int i=0;i<10;i++) {
		config->CustomTools.names[i]=names[i];
		config->CustomTools.commands[i]=commands[i];
		config->CustomTools.console[i]=console[i];
		config->Toolbars.tools.custom[i]=ontoolbar[i];
		someone_ontoolbar|=ontoolbar[i];
	}
	if (someone_ontoolbar && !config->Toolbars.wich_ones.tools && 
		mxMD_YES==mxMessageDialog(this,LANG(CUSTOM_TOOLS_SHOW_TOOLBAR,"La barra de herramientas \"Herramientas\" no esta visible.\n"
		"¿Desea activarla para ver los controles personalizados?"), _T("Reiniciar Barras de Herramientas"), mxMD_YES_NO).ShowModal()) {
		config->Toolbars.wich_ones.tools=true;
			main_window->menu.view_toolbar_tools->Check(true);
			main_window->aui_manager.GetPane(main_window->toolbar_tools).Show();
		}
	main_window->CreateToolbars(main_window->toolbar_tools);
	main_window->UpdateCustomTools();
	Close();
}

void mxCustomTools::OnButtonCancel(wxCommandEvent &event) {
	Close();
}

void mxCustomTools::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxCustomTools::OnComboChange(wxCommandEvent &event) {
	int i=the_combo->GetSelection();
	if (i<0||i>9) return;
	
	commands[prev_sel]=command_ctrl->GetValue();
	names[prev_sel]=name_ctrl->GetValue();
	console[prev_sel]=console_ctrl->GetValue();
	ontoolbar[prev_sel]=ontoolbar_ctrl->GetValue();
	
	prev_sel=i;
	command_ctrl->SetValue(commands[prev_sel]);
	name_ctrl->SetValue(names[prev_sel]);
	console_ctrl->SetValue(console[prev_sel]);
	ontoolbar_ctrl->SetValue(ontoolbar[prev_sel]);
}

void mxCustomTools::OnButtonHelp(wxCommandEvent &evt) {
	SHOW_HELP(_T("custom_tools.html"));	
}

void mxCustomTools::OnButtonTest(wxCommandEvent &evt) {
	main_window->RunCustomTool(name_ctrl->GetValue(),workdir_ctrl->GetValue(),command_ctrl->GetValue(),console_ctrl->GetValue());
}
