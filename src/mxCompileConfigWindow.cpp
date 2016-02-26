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
#include "mxCommonConfigControls.h"

BEGIN_EVENT_TABLE(mxCompileConfigWindow, wxDialog)
	EVT_BUTTON(wxID_OK,mxCompileConfigWindow::OnOkButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxCompileConfigWindow::OnHelpButton)
	EVT_BUTTON(wxID_CANCEL,mxCompileConfigWindow::OnCancelButton)
	EVT_BUTTON(mxID_WORKING_FOLDER,mxCompileConfigWindow::OnButtonFolder)
	EVT_BUTTON(mxID_COMPILE_OPTIONS_COMP_EXTRA,mxCompileConfigWindow::OnButtonCompilerOptions)
	EVT_MENU(mxID_ARGS_REPLACE_DIR,mxCompileConfigWindow::OnPopupReplaceDir)
	EVT_MENU(mxID_ARGS_DEFAULT,mxCompileConfigWindow::OnArgsDefault)
	EVT_MENU(mxID_ARGS_ADD_DIR,mxCompileConfigWindow::OnPopupAddDir)
	EVT_MENU(mxID_ARGS_REPLACE_FILE,mxCompileConfigWindow::OnPopupReplaceFile)
	EVT_MENU(mxID_ARGS_ADD_FILE,mxCompileConfigWindow::OnPopupAddFile)
	EVT_MENU(mxID_ARGS_EDIT_LIST,mxCompileConfigWindow::OnPopupEditList)
	EVT_MENU(mxID_ARGS_EDIT_TEXT,mxCompileConfigWindow::OnPopupEditText)
	EVT_MENU_RANGE(mxID_LAST_ID, mxID_LAST_ID+50,mxCompileConfigWindow::OnArgsFromTemplate)
	EVT_BUTTON(mxID_ARGS_BUTTON,mxCompileConfigWindow::OnArgsButton)
	EVT_CLOSE(mxCompileConfigWindow::OnClose)
END_EVENT_TABLE()


mxCompileConfigWindow::mxCompileConfigWindow(mxSource *a_source, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) 
	: wxDialog(parent, id, a_source->page_text, pos, size, style), m_source(a_source)
{
	mxCCC::MainSizer sizer = mxCCC::CreateMainSizer(this);
	
	sizer.BeginText( LANG(COMPILECONF_COMPILER_ARGS,"Parametros extra para el compilador") )
		.Value(m_source->GetCompilerOptions(false)).Button(mxID_COMPILE_OPTIONS_COMP_EXTRA).EndText(compiler_options_ctrl);
	
	sizer.BeginText( LANG(COMPILECONF_WORKDIR,"Directorio de trabajo") )
		.Value(m_source->working_folder.GetFullPath()).Button(mxID_WORKING_FOLDER).EndText(working_folder_ctrl);
	
	sizer.BeginText( LANG(COMPILECONF_RUNNING_ARGS,"Argumentos para la ejecucion") )
		.Value(m_source->exec_args).Button(mxID_ARGS_BUTTON).EndText(args_ctrl); 
	
	sizer.BeginCheck( LANG(COMPILECONF_ALWAYS_ASK_ARGS,"Siempre pedir argumentos al ejecutar") )
		.Value(m_source->config_running.always_ask_args).EndCheck(always_ask_args_ctrl);
	
	sizer.BeginCheck( LANG(COMPILECONF_WAIT_KEY,"Esperar una tecla luego de la ejecucion") )
		.Value(m_source->config_running.wait_for_key).EndCheck(wait_for_key_ctrl);
	
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

	m_source->exec_args = args_ctrl->GetValue();
	m_source->working_folder = working_folder_ctrl->GetValue();
	m_source->SetCompilerOptions(compiler_options_ctrl->GetValue());
	m_source->config_running.always_ask_args = always_ask_args_ctrl->GetValue();
	m_source->config_running.wait_for_key = wait_for_key_ctrl->GetValue();
	Destroy();

}

void mxCompileConfigWindow::OnCancelButton(wxCommandEvent &event){
	Destroy();
}

void mxCompileConfigWindow::OnClose(wxCloseEvent &event){
	Destroy();
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
	m_base_dir_for_popup = DIR_PLUS_FILE(m_source->GetPath(true),working_folder_ctrl->GetValue());
	m_text_ctrl_for_popup = args_ctrl;
	m_title_for_edit_helpers = LANG(COMPILECONF_RUNNING_ARGS,"Argumentos para la ejecucion");
	wxMenu menu;
	menu.Append(mxID_ARGS_EDIT_TEXT,LANG(GENERAL_POPUP_EDIT_AS_TEXT,"editar como texto"));
	menu.Append(mxID_ARGS_EDIT_LIST,LANG(GENERAL_POPUP_EDIT_AS_LIST,"editar como lista"));
	menu.Append(mxID_ARGS_REPLACE_FILE,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FILE,"reemplazar todo por archivo"));
	menu.Append(mxID_ARGS_ADD_FILE,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FILE,"reemplazar seleccion por archivo"));
	menu.Append(mxID_ARGS_REPLACE_DIR,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FOLDER,"reemplazar todo por directorio"));
	menu.Append(mxID_ARGS_ADD_DIR,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FOLDER,"reemplazar seleccion por directorio"));
	PopupMenu(&menu);
}

void mxCompileConfigWindow::OnPopupReplaceFile(wxCommandEvent &evt) {
	mxThreeDotsUtils::ReplaceAllWithFile(this,m_text_ctrl_for_popup,m_base_dir_for_popup);
}

void mxCompileConfigWindow::OnPopupAddFile(wxCommandEvent &evt) {
	mxThreeDotsUtils::ReplaceSelectionWithFile(this,m_text_ctrl_for_popup,m_base_dir_for_popup);
}

void mxCompileConfigWindow::OnPopupReplaceDir(wxCommandEvent &evt) {
	mxThreeDotsUtils::ReplaceAllWithDirectory(this,m_text_ctrl_for_popup,m_base_dir_for_popup);
}


void mxCompileConfigWindow::OnPopupAddDir(wxCommandEvent &evt) {
	mxThreeDotsUtils::ReplaceSelectionWithDirectory(this,m_text_ctrl_for_popup,m_base_dir_for_popup);
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
	m_base_dir_for_popup = m_source->GetPath(true);
	m_text_ctrl_for_popup = compiler_options_ctrl;
	m_title_for_edit_helpers = LANG(COMPILECONF_COMPILER_ARGS_DLG,"Parametros extra para el compilador");
	wxMenu menu;
	menu.Append(mxID_ARGS_EDIT_TEXT,LANG(GENERAL_POPUP_EDIT_AS_TEXT,"editar como texto"));
	menu.Append(mxID_ARGS_EDIT_LIST,LANG(GENERAL_POPUP_EDIT_AS_LIST,"editar como lista"));
	menu.Append(mxID_ARGS_REPLACE_FILE,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FILE,"reemplazar todo por archivo"));
	menu.Append(mxID_ARGS_ADD_FILE,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FILE,"reemplazar seleccion por archivo"));
	menu.Append(mxID_ARGS_REPLACE_DIR,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FOLDER,"reemplazar todo por directorio"));
	menu.Append(mxID_ARGS_ADD_DIR,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FOLDER,"reemplazar seleccion por directorio"));
	menu.AppendSeparator();
	menu.Append(mxID_ARGS_DEFAULT,LANG(COMPILECONF_POPUP_DEFAULT,"valores por defecto"));
//	menu.Append(mxID_ARGS_FROM_TEMPLATE,LANG(COMPILECONF_POPUP_COPY_FROM_TEMPLATE,"copiar de plantilla..."));

	wxMenu *templates=new wxMenu; opts_list.Clear();
	wxArrayString templates_list;
	mxUT::GetFilesFromBothDirs(templates_list,"templates");
	for(unsigned int i=0;i<templates_list.GetCount();i++) {
		wxString name=templates_list[i], options;
		wxString filename = mxUT::WichOne(name,"templates",true);
		wxTextFile file(filename);
		file.Open();
		if (file.IsOpened()) { 
			// buscar si tiene nombre
			wxString line = file.GetFirstLine();
			while (!file.Eof() && line.Left(7)=="// !Z! ") {
				if (line.Left(15)=="// !Z! Options:") {
					options = line.Mid(15).Trim(false).Trim(true);
				}
				if (line.Left(12)=="// !Z! Name:") {
					name = line.Mid(12).Trim(false).Trim(true);
				}
				line = file.GetNextLine();
			}
			file.Close();
			if (options.Len()) {
				templates->Append(mxID_LAST_ID+opts_list.GetCount(),name); // para mostrar en el dialogo
				opts_list.Add(options); // para uso interno
			}
		}
	}
	if (opts_list.GetCount()) menu.AppendSubMenu(templates,LANG(COMPILECONF_POPUP_COPY_FROM_TEMPLATE,"copiar de plantilla..."));
	PopupMenu(&menu);
}

void mxCompileConfigWindow::OnPopupEditText(wxCommandEvent &evt) {
	mxLongTextEditor(this,m_title_for_edit_helpers,m_text_ctrl_for_popup);
}

void mxCompileConfigWindow::OnPopupEditList(wxCommandEvent &evt) {
	mxEnumerationEditor(this,m_title_for_edit_helpers,m_text_ctrl_for_popup,false);
}
