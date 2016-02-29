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

#include "mxCommonConfigControls.h"

BEGIN_EVENT_TABLE(mxCustomTools, wxDialog)
	EVT_BUTTON(wxID_OK,mxCustomTools::OnButtonOk)
	EVT_BUTTON(mxID_CUSTOM_TOOLS_RUN,mxCustomTools::OnButtonTest)
	EVT_BUTTON(wxID_CANCEL,mxCustomTools::OnButtonCancel)
	EVT_BUTTON(mxID_HELP_BUTTON,mxCustomTools::OnButtonHelp)
	EVT_COMBOBOX(mxID_CUSTOM_TOOLS_COMBO,mxCustomTools::OnComboChange)
	EVT_BUTTON(mxID_CUSTOM_TOOLS_COMMAND,mxCustomTools::OnCommandPopup)
	EVT_BUTTON(mxID_CUSTOM_TOOLS_WORKDIR,mxCustomTools::OnWorkdirPopup)
END_EVENT_TABLE()

mxCustomTools::mxCustomTools(bool for_project, int cual)
	: mxDialog(main_window, LANG(CUSTOM_TOOLS_CAPTION,"Herramientas Personalizables") ),
	  m_tools(new CustomToolsPack(for_project?project->custom_tools:config->custom_tools)),
	  m_tool_count(for_project?MAX_PROJECT_CUSTOM_TOOLS:MAX_CUSTOM_TOOLS),
	  m_for_project(for_project)
{
	if (cual<0||cual>=m_tools->GetCount()) cual=0; 
	
	CreateSizer sizer(this);
	
	wxArrayString array; for(int i=0;i<m_tools->GetCount();i++) array.Add(wxString(LANG(CUSTOM_TOOLS_ITEM,"Herramienta "))<<i);
	sizer.BeginCombo( LANG(CUSTOM_TOOLS_COMBO,"Herramienta a editar") )
		.Add(array).Select(cual).Id(mxID_CUSTOM_TOOLS_COMBO).EndCombo(the_combo);
	
	sizer.BeginText( LANG(CUSTOM_TOOLS_NAME,"Nombre") ).EndText(name_ctrl);
	sizer.BeginText( LANG(CUSTOM_TOOLS_COMMAND,"Comando") ).Button(mxID_CUSTOM_TOOLS_COMMAND).EndText(command_ctrl);
	sizer.BeginText( LANG(CUSTOM_TOOLS_WORKDIR,"Directorio de trabajo") ).Button(mxID_CUSTOM_TOOLS_WORKDIR).EndText(workdir_ctrl);
	
	sizer.BeginCombo( LANG(CUSTOM_TOOLS_PRE,"Acción antes de ejecutar") )
		.Add(LANG(CUSTOM_TOOLS_PRE_NONE,"Ninguna"))
		.Add(LANG(CUSTOM_TOOLS_PRE_SAVE_ONE,"Guardar el fuente actual"))
		.Add(LANG(CUSTOM_TOOLS_PRE_SAVE_ALL,"Guardar todos los fuentes abiertos"))
		.Add(LANG(CUSTOM_TOOLS_PRE_SAVE_PROJECT,"Guardar todo el proyecto"))
		.Add(LANG(CUSTOM_TOOLS_PRE_COMPILE,"Compilar el programa/proyecto"))
		.EndCombo(pre_action_ctrl);
	
	sizer.BeginCheck( LANG(CUSTOM_TOOLS_ASYNC_EXEC,"Ejecución asíncrona") )
		.Value(false).EndCheck(async_exec_ctrl);

	sizer.BeginCombo( LANG(CUSTOM_TOOLS_OUTPUT,"Salidas (std y err)") )
		.Add(LANG(CUSTOM_TOOLS_OUTPUT_HIDDEN,"Ocultas")) // (en windows requiere redirect al pedo)
		.Add(LANG(CUSTOM_TOOLS_OUTPUT_TERMINAL,"En terminal")) // (en linux requiere lanzar con terminal)
		.Add(LANG(CUSTOM_TOOLS_OUTPUT_TERMINAL_WAIT,"En terminal (esperar tecla)")) // (en linux requiere lanzar con terminal)
		.Add(LANG(CUSTOM_TOOLS_OUTPUT_DIALOG,"En cuadro de dialogo"))
		.Select(0).EndCombo(output_mode_ctrl);
		
	sizer.BeginCombo( LANG(CUSTOM_TOOLS_POST,"Acción luego de ejecutar:") )
		.Add(LANG(CUSTOM_TOOLS_POST_NONE,"Ninguna"))
		.Add(LANG(CUSTOM_TOOLS_POST_RELOAD_ONE,"Recargar fuente actual"))
		.Add(LANG(CUSTOM_TOOLS_POST_RELOAD_ALL,"Recargar todos los fuentes"))
		.Add(LANG(CUSTOM_TOOLS_POST_RUN,"Ejecutar el programa/proyecto"))
		.Add(LANG(CUSTOM_TOOLS_POST_DEBUG,"Depurar el programa/proyecto"))
		.Select(0).EndCombo(post_action_ctrl);
	
	sizer.BeginCheck( LANG(CUSTOM_TOOLS_ONTOOLBAR_LINUX,"Mostrar en la barra de herramientas") )
		.Value(false).EndCheck(ontoolbar_ctrl);
	
	sizer.BeginBottom()
		.Extra(mxID_CUSTOM_TOOLS_RUN,LANG(CUSTOM_TOOLS_TEST,"&Ejecutar"),bitmaps->buttons.ok)
		.Ok().Cancel().Help().EndBottom(this);
	
	sizer.SetAndFit();
	
	m_prev_sel = cual;
	ToolToDialog(cual);
	
	Show();
	the_combo->SetFocus();
}

void mxCustomTools::OnCommandPopup(wxCommandEvent &event) {
	CommonPopup(command_ctrl).Caption(LANG(CUSTOM_TOOLS_COMMAND,"Comando"))
		.AddEditAsText().AddEditAsList().AddFilename().AddPath()
		.AddCurrentFile().AddCurrentDir().AddProjectBin().AddProjectDir()
		.AddArgs().AddTempDir().AddMinGWDir().AddZinjaiDir()
		.AddBrowerCommand().AddShellCommand()
		.Run(this);
}

void mxCustomTools::OnWorkdirPopup(wxCommandEvent &event) {
	CommonPopup(workdir_ctrl).SelectAll().Caption(LANG(CUSTOM_TOOLS_WORKDIR,"Directorio de trabajo"))
		.AddPath().AddCurrentDir().AddProjectDir()
		.AddTempDir().AddMinGWDir().AddZinjaiDir().AddWorkDir()
		.Run(this);
}

void mxCustomTools::OnButtonOk(wxCommandEvent &event) {
	if (m_for_project && !project) { Close(); return; }
	DialogToTool(m_prev_sel);
	bool someone_ontoolbar=false;
	CustomToolsPack &orig=m_for_project?project->custom_tools:config->custom_tools;
	for (int i=0;i<m_tool_count;i++) {
		orig.GetTool(i)=m_tools->GetTool(i);
		someone_ontoolbar|=orig.GetTool(i).on_toolbar;
	}
	int tb_id = m_for_project?MenusAndToolsConfig::tbPROJECT:MenusAndToolsConfig::tbTOOLS;
	int wx_id = m_for_project?mxID_VIEW_TOOLBAR_PROJECT:mxID_VIEW_TOOLBAR_TOOLS;
	if (someone_ontoolbar && !menu_data->GetToolbarPosition(tb_id).visible && 
		mxMD_YES==mxMessageDialog(this,LANG(CUSTOM_TOOLS_SHOW_TOOLBAR,"La barra de herramientas \"Herramientas\" no esta visible.\n"
		"¿Desea activarla para ver los controles personalizados?"),GetCaption(), mxMD_YES_NO).ShowModal()) {
			main_window->OnToggleToolbar(wx_id,tb_id,true);
		}
	menu_data->TransferStatesFromConfig();
	menu_data->UpdateToolbar(tb_id,true);
	main_window->UpdateCustomTools(m_for_project);
	Close();
}

void mxCustomTools::OnButtonCancel(wxCommandEvent &event) {
	Close();
}

void mxCustomTools::OnComboChange(wxCommandEvent &event) {
	int i=the_combo->GetSelection();
	if (i<0||i>9) return;
	DialogToTool(m_prev_sel);
	m_prev_sel=i;
	ToolToDialog(i);
}

void mxCustomTools::OnButtonHelp(wxCommandEvent &evt) {
	mxHelpWindow::ShowHelp("custom_tools.html");
}

void mxCustomTools::OnButtonTest(wxCommandEvent &evt) {
	DialogToTool(m_prev_sel);
	m_tools->Run(m_prev_sel);
}

void mxCustomTools::ToolToDialog (int i) {
	OneCustomTool &tool = m_tools->GetTool(i);
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
	OneCustomTool &tool = m_tools->GetTool(i);
	tool.name=name_ctrl->GetValue();
	tool.command=command_ctrl->GetValue();
	tool.workdir=workdir_ctrl->GetValue();
	tool.pre_action=pre_action_ctrl->GetSelection();
	tool.post_action=post_action_ctrl->GetSelection();
	tool.output_mode=output_mode_ctrl->GetSelection();
	tool.async_exec=async_exec_ctrl->GetValue();
	tool.on_toolbar=ontoolbar_ctrl->GetValue();
}

mxCustomTools::~mxCustomTools ( ) {
	delete m_tools;
}

