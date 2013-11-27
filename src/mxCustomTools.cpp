#include <wx/combobox.h>
#include "mxArt.h"
#include "mxCustomTools.h"
#include "mxMainWindow.h"
#include "mxSizers.h"
#include "mxUtils.h"
#include "ids.h"
#include "Language.h"
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
	
mxCustomTools::mxCustomTools(bool for_project, int cual):wxDialog(main_window,wxID_ANY,LANG(CUSTOM_TOOLS_CAPTION,"Herramientas Personalizables"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	this->for_project=for_project;
	if (cual<0) cual=prev_sel; else prev_sel=cual;
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxArrayString array;
	tool_count=for_project?MAX_PROJECT_CUSTOM_TOOLS:MAX_CUSTOM_TOOLS;
	tools=new cfgCustomTool[tool_count];
	orig=for_project?project->custom_tools:config->CustomTools;
	for (int i=0;i<tool_count;i++) {
		tools[i].name=orig[i].name;
		tools[i].command=orig[i].command;
		tools[i].workdir=orig[i].workdir;
		tools[i].console=orig[i].console;
		tools[i].on_toolbar=orig[i].on_toolbar;
		array.Add(wxString(LANG(CUSTOM_TOOLS_ITEM,"Herramienta "))<<i);
	}
	the_combo = utils->AddComboBox(sizer,this,LANG(CUSTOM_TOOLS_COMBO,"Herramienta a editar"),array,cual,mxID_CUSTOM_TOOLS_COMBO);
	name_ctrl  =utils->AddTextCtrl(sizer,this,LANG(CUSTOM_TOOLS_NAME,"Nombre"),tools[cual].name);
	command_ctrl = utils->AddDirCtrl(sizer,this,LANG(CUSTOM_TOOLS_COMMAND,"Comando"),tools[cual].command,mxID_CUSTOM_TOOLS_COMMAND);
	workdir_ctrl = utils->AddDirCtrl(sizer,this,LANG(CUSTOM_TOOLS_WORKDIR,"Directorio de trabajo"),tools[cual].workdir,mxID_CUSTOM_TOOLS_WORKDIR);
#if defined(__WIN32__)
	console_ctrl = utils->AddCheckBox(sizer,this,LANG(CUSTOM_TOOLS_CONSOLE_WINDOWS,"Ocultar ventana"),tools[cual].console);
#else
	console_ctrl = utils->AddCheckBox(sizer,this,LANG(CUSTOM_TOOLS_CONSOLE_LINUX,"Correr en terminal"),tools[cual].console);
#endif
	ontoolbar_ctrl = utils->AddCheckBox(sizer,this,LANG(CUSTOM_TOOLS_ONTOOLBAR_LINUX,"Mostrar en la barra de herramientas"),tools[cual].on_toolbar);
	
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
	utils->ShowTextPopUp(this,LANG(CUSTOM_TOOLS_COMMAND,"Comando"),command_ctrl,"TEXT|LIST|FILE|DIR|TEMP_DIR|SHELL_EXECUTE|MINGW_DIR|BROWSER|PROJECT_PATH|PROJECT_BIN|CURRENT_FILE|CURRENT_DIR|ZINJAI_DIR|MINGW_DIR");
}

void mxCustomTools::OnWorkdirPopup(wxCommandEvent &event) {
	utils->ShowTextPopUp(this,LANG(CUSTOM_TOOLS_WORKDIR,"Directorio de trabajo"),workdir_ctrl,"REPLACE|DIR|TEMP_DIR|MINGW_DIR|PROJECT_PATH|CURRENT_DIR|ZINJAI_DIR|MINGW_DIR");
}


void mxCustomTools::OnPopup(wxCommandEvent &evt) {
	utils->ProcessTextPopup(evt.GetId());
}

void mxCustomTools::OnButtonOk(wxCommandEvent &event) {
	if (for_project && !project) { Close(); return; }
	tools[prev_sel].command=command_ctrl->GetValue();
	tools[prev_sel].name=name_ctrl->GetValue();
	tools[prev_sel].console=console_ctrl->GetValue();
	tools[prev_sel].workdir=workdir_ctrl->GetValue();
	tools[prev_sel].on_toolbar=ontoolbar_ctrl->GetValue();
	bool someone_ontoolbar=false;
	for (int i=0;i<tool_count;i++) {
		orig[i].name=tools[i].name;
		orig[i].command=tools[i].command;
		orig[i].workdir=tools[i].workdir;
		orig[i].console=tools[i].console;
		orig[i].on_toolbar=tools[i].on_toolbar;
		someone_ontoolbar|=tools[i].on_toolbar;
	}
	bool &visible=for_project?config->Toolbars.positions.project.visible:config->Toolbars.positions.tools.visible;
	if (someone_ontoolbar && !visible && 
		mxMD_YES==mxMessageDialog(this,LANG(CUSTOM_TOOLS_SHOW_TOOLBAR,"La barra de herramientas \"Herramientas\" no esta visible.\n"
		"¿Desea activarla para ver los controles personalizados?"), _T("Reiniciar Barras de Herramientas"), mxMD_YES_NO).ShowModal()) {
			visible=true;
			if (for_project) {
				main_window->OnToggleToolbar(main_window->menu.view_toolbar_project,main_window->toolbar_project,visible,true);
			} else {
				main_window->OnToggleToolbar(main_window->menu.view_toolbar_tools,main_window->toolbar_tools,visible,true);
			}
		}
	main_window->CreateToolbars(for_project?main_window->toolbar_project:main_window->toolbar_tools);
	main_window->UpdateCustomTools(for_project);
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
	
	tools[prev_sel].command=command_ctrl->GetValue();
	tools[prev_sel].workdir=workdir_ctrl->GetValue();
	tools[prev_sel].name=name_ctrl->GetValue();
	tools[prev_sel].console=console_ctrl->GetValue();
	tools[prev_sel].on_toolbar=ontoolbar_ctrl->GetValue();
	
	prev_sel=i;
	command_ctrl->SetValue(tools[prev_sel].command);
	name_ctrl->SetValue(tools[prev_sel].name);
	console_ctrl->SetValue(tools[prev_sel].console);
	ontoolbar_ctrl->SetValue(tools[prev_sel].on_toolbar);
	workdir_ctrl->SetValue(tools[prev_sel].workdir);
}

void mxCustomTools::OnButtonHelp(wxCommandEvent &evt) {
	SHOW_HELP(_T("custom_tools.html"));	
}

void mxCustomTools::OnButtonTest(wxCommandEvent &evt) {
	cfgCustomTool t;
	t.name=name_ctrl->GetValue();
	t.workdir=workdir_ctrl->GetValue();
	t.command=command_ctrl->GetValue();
	t.console=console_ctrl->GetValue();
	main_window->RunCustomTool(t);
}
