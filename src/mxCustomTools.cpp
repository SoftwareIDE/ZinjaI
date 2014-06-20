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
#include "MenusAndToolsConfig.h"

#define _CAPTION LANG(CUSTOM_TOOLS_CAPTION,"Herramientas Personalizables")

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

mxCustomTools::mxCustomTools(bool for_project, int cual):wxDialog(main_window,wxID_ANY,_CAPTION,wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) {
	this->for_project=for_project;
	tool_count=for_project?MAX_PROJECT_CUSTOM_TOOLS:MAX_CUSTOM_TOOLS;
	tools = new CustomToolsPack(for_project?project->custom_tools:config->custom_tools);
	if (cual<0||cual>=tools->GetCount()) cual=0; 
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxArrayString array;
	for(int i=0;i<tools->GetCount();i++) array.Add(wxString(LANG(CUSTOM_TOOLS_ITEM,"Herramienta "))<<i);
	
	the_combo = utils->AddComboBox(sizer,this,LANG(CUSTOM_TOOLS_COMBO,"Herramienta a editar"),array,cual,mxID_CUSTOM_TOOLS_COMBO);
	name_ctrl  =utils->AddTextCtrl(sizer,this,LANG(CUSTOM_TOOLS_NAME,"Nombre"),"");
	command_ctrl = utils->AddDirCtrl(sizer,this,LANG(CUSTOM_TOOLS_COMMAND,"Comando"),"",mxID_CUSTOM_TOOLS_COMMAND);
	workdir_ctrl = utils->AddDirCtrl(sizer,this,LANG(CUSTOM_TOOLS_WORKDIR,"Directorio de trabajo"),"",mxID_CUSTOM_TOOLS_WORKDIR);
	
	wxArrayString pre_actions;
	pre_actions.Add(LANG(CUSTOM_TOOLS_PRE_NONE,"Ninguna"));
	pre_actions.Add(LANG(CUSTOM_TOOLS_PRE_SAVE_ONE,"Guardar el fuente actual."));
	pre_actions.Add(LANG(CUSTOM_TOOLS_PRE_SAVE_ALL,"Guardar todos los fuentes abiertos."));
	pre_actions.Add(LANG(CUSTOM_TOOLS_PRE_SAVE_PROJECT,"Guardar todo el proyecto."));
	pre_action_ctrl = utils->AddComboBox(sizer,this,LANG(CUSTOM_TOOLS_PRE,"Acción antes de ejecutar"),pre_actions,false);
		
	async_exec_ctrl = utils->AddCheckBox(sizer,this,LANG(CUSTOM_TOOLS_ASYNC_EXEC,"Ejecución asíncrona"),false);
		
	wxArrayString output_destinations;
	output_destinations.Add(LANG(CUSTOM_TOOLS_OUTPUT_HIDDEN,"Ocultas.")); // (en windows requiere redirect al pedo)
	output_destinations.Add(LANG(CUSTOM_TOOLS_OUTPUT_TERMINAL,"En terminal.")); // (en linux requiere lanzar con terminal)
	output_destinations.Add(LANG(CUSTOM_TOOLS_OUTPUT_DIALOG,"En cuadro de dialogo."));
	output_mode_ctrl = utils->AddComboBox(sizer,this,LANG(CUSTOM_TOOLS_OUTPUT,"Salidas (std y err)"),output_destinations,0);
		
	wxArrayString post_actions;
	post_actions.Add(LANG(CUSTOM_TOOLS_POST_NONE,"Ninguna."));
	post_actions.Add(LANG(CUSTOM_TOOLS_POST_RELOAD_ONE,"Recargar fuente actual."));
	post_actions.Add(LANG(CUSTOM_TOOLS_POST_RELOAD_ALL,"Recargar todos los fuentes."));
	post_action_ctrl = utils->AddComboBox(sizer,this,LANG(CUSTOM_TOOLS_POST,"Acción luego de ejecutar:"),post_actions,0);
	
	
	ontoolbar_ctrl = utils->AddCheckBox(sizer,this,LANG(CUSTOM_TOOLS_ONTOOLBAR_LINUX,"Mostrar en la barra de herramientas"),false);
	
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
	
	prev_sel=cual;
	ToolToDialog(cual);
	
	Show();
	
	the_combo->SetFocus();
	
}

void mxCustomTools::OnCommandPopup(wxCommandEvent &event) {
	utils->ShowTextPopUp(this,LANG(CUSTOM_TOOLS_COMMAND,"Comando"),command_ctrl,"TEXT|LIST|FILE|DIR|TEMP_DIR|SHELL_EXECUTE|MINGW_DIR|BROWSER|PROJECT_PATH|PROJECT_BIN|CURRENT_FILE|CURRENT_DIR|ZINJAI_DIR|MINGW_DIR");
}

void mxCustomTools::OnWorkdirPopup(wxCommandEvent &event) {
	utils->ShowTextPopUp(this,LANG(CUSTOM_TOOLS_WORKDIR,"Directorio de trabajo"),workdir_ctrl,"REPLACE|DIR|TEMP_DIR|MINGW_DIR|PROJECT_PATH|CURRENT_DIR|ZINJAI_DIR|MINGW_DIR|WORK_DIR");
}


void mxCustomTools::OnPopup(wxCommandEvent &evt) {
	utils->ProcessTextPopup(evt.GetId());
}

void mxCustomTools::OnButtonOk(wxCommandEvent &event) {
	if (for_project && !project) { Close(); return; }
	DialogToTool(prev_sel);
	bool someone_ontoolbar=false;
	CustomToolsPack &orig=for_project?project->custom_tools:config->custom_tools;
	for (int i=0;i<tool_count;i++) {
		orig.GetTool(i)=tools->GetTool(i);
		someone_ontoolbar|=orig.GetTool(i).on_toolbar;
	}
	int tb_id = for_project?MenusAndToolsConfig::tbPROJECT:MenusAndToolsConfig::tbTOOLS;
	int wx_id = for_project?mxID_VIEW_TOOLBAR_PROJECT:mxID_VIEW_TOOLBAR_TOOLS;
	if (someone_ontoolbar && !menu_data->GetToolbarPosition(tb_id).visible && 
		mxMD_YES==mxMessageDialog(this,LANG(CUSTOM_TOOLS_SHOW_TOOLBAR,"La barra de herramientas \"Herramientas\" no esta visible.\n"
		"¿Desea activarla para ver los controles personalizados?"),_CAPTION, mxMD_YES_NO).ShowModal()) {
			main_window->OnToggleToolbar(wx_id,tb_id,true);
		}
	menu_data->TransferStatesFromConfig();
	menu_data->UpdateToolbar(tb_id,true);
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
	DialogToTool(prev_sel);
	prev_sel=i;
	ToolToDialog(i);
}

void mxCustomTools::OnButtonHelp(wxCommandEvent &evt) {
	mxHelpWindow::ShowHelp("custom_tools.html");
}

void mxCustomTools::OnButtonTest(wxCommandEvent &evt) {
	DialogToTool(prev_sel);
	tools->Run(prev_sel);
}

void mxCustomTools::ToolToDialog (int i) {
	OneCustomTool &tool = tools->GetTool(i);
	name_ctrl->SetValue(tool.name);
	command_ctrl->SetValue(tool.command);
	workdir_ctrl->SetValue(tool.workdir);
	pre_action_ctrl->SetSelection(tool.pre_action);
	post_action_ctrl->SetSelection(tool.post_action);
	output_mode_ctrl->SetSelection(tool.output_mode);
	async_exec_ctrl->SetValue(tool.async_exec);
	ontoolbar_ctrl->SetValue(tool.on_toolbar);
}

void mxCustomTools::DialogToTool (int i) {
	OneCustomTool &tool = tools->GetTool(i);
	tool.name=name_ctrl->GetValue();
	tool.command=command_ctrl->GetValue();
	tool.workdir=workdir_ctrl->GetValue();
	tool.pre_action=pre_action_ctrl->GetSelection();
	tool.post_action=post_action_ctrl->GetSelection();
	tool.output_mode=output_mode_ctrl->GetSelection();
	tool.async_exec=async_exec_ctrl->GetValue();
	tool.on_toolbar=ontoolbar_ctrl->GetValue();
}


