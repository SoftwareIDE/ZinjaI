#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/dir.h>
#include <wx/textfile.h>
#include "mxCompileConfigWindow.h"
#include "ConfigManager.h"
#include "mxUtils.h"
#include "mxHelpWindow.h"
#include "mxSource.h"
#include "mxArt.h"
#include "mxBitmapButton.h"
#include "mxMessageDialog.h"
#include "ids.h"
#include "mxLongTextEditor.h"
#include "mxMainWindow.h"
#include "mxEnumerationEditor.h"
#include "mxSizers.h"
#include "Language.h"
#include "mxThreeDotsUtils.h"
#include "SimpleTemplates.h"

BEGIN_EVENT_TABLE(mxCompileConfigWindow, wxDialog)
	EVT_BUTTON(wxID_OK,mxCompileConfigWindow::OnOkButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxCompileConfigWindow::OnHelpButton)
	EVT_BUTTON(wxID_CANCEL,mxCompileConfigWindow::OnCancelButton)
	EVT_BUTTON(mxID_WORKING_FOLDER,mxCompileConfigWindow::OnButtonFolder)
	EVT_BUTTON(mxID_COMPILE_OPTIONS_COMP_EXTRA,mxCompileConfigWindow::OnButtonCompilerOptions)
	EVT_MENU(mxID_ARGS_DEFAULT,mxCompileConfigWindow::OnArgsDefault)
	EVT_BUTTON(mxID_ARGS_BUTTON,mxCompileConfigWindow::OnArgsButton)
	EVT_MENU_RANGE(mxID_LAST_ID, mxID_LAST_ID+50,mxCompileConfigWindow::OnArgsFromTemplate)
END_EVENT_TABLE()


mxCompileConfigWindow::mxCompileConfigWindow(wxWindow* parent, mxSource *a_source)
	: mxDialog(parent, a_source->page_text), m_source(a_source)
{
	CreateSizer sizer(this);
	
	sizer.BeginText( LANG(COMPILECONF_COMPILER_ARGS,"Parametros extra para el compilador") )
		.Value(m_source->GetCompilerOptions(false)).Button(mxID_COMPILE_OPTIONS_COMP_EXTRA).EndText(compiler_options_ctrl);
	
	sizer.BeginText( LANG(COMPILECONF_WORKDIR,"Directorio de trabajo") )
		.Value(m_source->working_folder.GetFullPath()).Button(mxID_WORKING_FOLDER).EndText(working_folder_ctrl);
	
	sizer.BeginText( LANG(COMPILECONF_RUNNING_ARGS,"Argumentos para la ejecucion") )
		.Bind(m_binder,m_source->exec_args).Button(mxID_ARGS_BUTTON).EndText(args_ctrl); 
	
	sizer.BeginCheck( LANG(COMPILECONF_ALWAYS_ASK_ARGS,"Siempre pedir argumentos al ejecutar") )
		.Bind(m_binder,m_source->config_running.always_ask_args).EndCheck(/*always_ask_args_ctrl*/);
	
	sizer.BeginCheck( LANG(COMPILECONF_WAIT_KEY,"Esperar una tecla luego de la ejecucion") )
		.Bind(m_binder,m_source->config_running.wait_for_key).EndCheck(/*wait_for_key_ctrl*/);
	
	sizer
		.BeginLabel(" ").EndLabel()
		.BeginBottom().Ok().Cancel().Help().EndBottom(this)
		.SetAndFit();
	
	args_ctrl->SetFocus();
	Show();
}

void mxCompileConfigWindow::OnOkButton(wxCommandEvent &event){

	if (wxNOT_FOUND==main_window->notebook_sources->GetPageIndex(m_source)) {
		mxMessageDialog(this,LANG(COMPILECONF_SOURCE_CLOSED,"El fuente en cual se debe aplicar esta configuracion ha sido cerrado."), LANG(GENERAL_ERROR,"Error"), mxMD_ERROR|mxMD_OK).ShowModal();
		return;
	}
	
	if (working_folder_ctrl->GetValue()=="")
		working_folder_ctrl->SetValue(wxFileName::GetHomeDir());
	else if (!wxFileName::DirExists(working_folder_ctrl->GetValue())) {
		mxMessageDialog(this,LANG(COMPILECONF_INVALID_WORKDIR,"El directorio de trabajo no es valido."), LANG(GENERAL_ERROR,"Error"), mxMD_ERROR|mxMD_OK).ShowModal();
		return;
	}

	m_source->working_folder = working_folder_ctrl->GetValue();
	m_source->SetCompilerOptions(compiler_options_ctrl->GetValue());
	m_binder.FromWidgets();
	Close();
}

void mxCompileConfigWindow::OnCancelButton(wxCommandEvent &event){
	Close();
}

void mxCompileConfigWindow::OnButtonFolder(wxCommandEvent &event){
	wxDirDialog dlg(this,LANG(COMPILECONF_WORKDIR_DLG,"Directorio de trabajo:"),working_folder_ctrl->GetValue());
	if (wxID_OK==dlg.ShowModal())
		working_folder_ctrl->SetValue(dlg.GetPath());
}

void mxCompileConfigWindow::OnHelpButton(wxCommandEvent &evt) {
	mxHelpWindow::ShowHelp("compile_config.html");
}

void mxCompileConfigWindow::OnArgsButton(wxCommandEvent &evt) {
	CommonPopup(args_ctrl)
		.Caption( LANG(COMPILECONF_RUNNING_ARGS,"Argumentos para la ejecucion") )
		.BasePath( DIR_PLUS_FILE(m_source->GetPath(true),working_folder_ctrl->GetValue()) )
		.AddEditAsText().AddEditAsList().AddFilename().AddPath().Run(this);
}

void mxCompileConfigWindow::OnArgsDefault(wxCommandEvent &evt) {
	compiler_options_ctrl->SetValue(config->GetDefaultCompilerOptions(m_source->IsCppOrJustC()));
}

void mxCompileConfigWindow::OnArgsFromTemplate(wxCommandEvent &evt) {
	wxString val=opts_list[evt.GetId()-mxID_LAST_ID];
	val.Replace("${DEFAULT}",config->GetDefaultCompilerOptions(m_source->IsCppOrJustC()),true);
	compiler_options_ctrl->SetValue(val);
}

void mxCompileConfigWindow::OnButtonCompilerOptions(wxCommandEvent &evt) {
	SimpleTemplates::Initialize();
	wxArrayString files; opts_list.Clear();
	g_templates->GetFilesList(files,true,true);
	wxMenu *templates = new wxMenu; 
	for(unsigned int i=0;i<files.GetCount();i++) {
		wxString args = g_templates->GetArgsFromFile(files[i]);
		if (args.IsEmpty()||args=="${DEFAULT}") continue;
		templates->Append(mxID_LAST_ID+opts_list.GetCount(),g_templates->GetNameFromFile(files[i])); // para mostrar en el dialogo
		opts_list.Add(args); // para uso interno
	}
	
	CommonPopup(compiler_options_ctrl)
		.Caption( LANG(COMPILECONF_COMPILER_ARGS_DLG,"Parametros extra para el compilador") )
		.BasePath( m_source->GetPath(true) )
		.AddEditAsText().AddEditAsList().AddFilename().AddPath()
		.AddSeparator()
		.Add(mxID_ARGS_DEFAULT, LANG(COMPILECONF_POPUP_DEFAULT,"valores por defecto") )
		.Add(templates, LANG(COMPILECONF_POPUP_COPY_FROM_TEMPLATE,"copiar de plantilla...") )
		.Run(this);
}
