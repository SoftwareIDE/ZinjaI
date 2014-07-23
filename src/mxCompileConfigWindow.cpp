#include <wx/wx.h>
#include <wx/aui/auibook.h>
#include <wx/clipbrd.h>
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

BEGIN_EVENT_TABLE(mxCompileConfigWindow, wxDialog)
	EVT_BUTTON(wxID_OK,mxCompileConfigWindow::OnOkButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxCompileConfigWindow::OnHelpButton)
	EVT_BUTTON(wxID_CANCEL,mxCompileConfigWindow::OnCancelButton)
	EVT_BUTTON(mxID_WORKING_FOLDER,mxCompileConfigWindow::OnButtonFolder)
	EVT_BUTTON(mxID_COMPILE_OPTIONS_COMP_EXTRA,mxCompileConfigWindow::OnButtonCompilerOptions)
	EVT_MENU(mxID_ARGS_REPLACE_DIR,mxCompileConfigWindow::OnArgsReplaceDir)
	EVT_MENU(mxID_ARGS_DEFAULT,mxCompileConfigWindow::OnArgsDefault)
	EVT_MENU(mxID_ARGS_ADD_DIR,mxCompileConfigWindow::OnArgsAddDir)
	EVT_MENU(mxID_ARGS_REPLACE_FILE,mxCompileConfigWindow::OnArgsReplaceFile)
	EVT_MENU(mxID_ARGS_ADD_FILE,mxCompileConfigWindow::OnArgsAddFile)
	EVT_MENU(mxID_ARGS_EDIT_LIST,mxCompileConfigWindow::OnArgsEditList)
	EVT_MENU(mxID_ARGS_EDIT_TEXT,mxCompileConfigWindow::OnArgsEditText)
	EVT_MENU_RANGE(mxID_LAST_ID, mxID_LAST_ID+50,mxCompileConfigWindow::OnArgsFromTemplate)
	EVT_BUTTON(mxID_ARGS_BUTTON,mxCompileConfigWindow::OnArgsButton)
	EVT_CLOSE(mxCompileConfigWindow::OnClose)
END_EVENT_TABLE()


mxCompileConfigWindow::mxCompileConfigWindow(mxSource *a_source, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, a_source->page_text, pos, size, style) {
	source=a_source;

	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton *cancel_button = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	SetEscapeId(wxID_CANCEL); 
	wxButton *ok_button = new mxBitmapButton(this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	ok_button->SetDefault(); 
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*bitmaps->buttons.help);

	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);

	compiler_options_ctrl = mxUT::AddDirCtrl(mySizer,this,LANG(COMPILECONF_COMPILER_ARGS,"Parametros extra para el compilador"),source->GetCompilerOptions(false),mxID_COMPILE_OPTIONS_COMP_EXTRA);
	working_folder_ctrl = mxUT::AddDirCtrl(mySizer,this,LANG(COMPILECONF_WORKDIR,"Directorio de trabajo"),source->working_folder.GetFullPath(),mxID_WORKING_FOLDER);
	args_ctrl = mxUT::AddDirCtrl(mySizer,this,LANG(COMPILECONF_RUNNING_ARGS,"Argumentos para la ejecucion"),source->exec_args,mxID_ARGS_BUTTON);
	always_ask_args_ctrl = mxUT::AddCheckBox(mySizer,this,LANG(COMPILECONF_ALWAYS_ASK_ARGS,"Siempre pedir argumentos al ejecutar"),source->config_running.always_ask_args);
	wait_for_key_ctrl = mxUT::AddCheckBox(mySizer,this,LANG(COMPILECONF_WAIT_KEY,"Esperar una tecla luego de la ejecucion"),source->config_running.wait_for_key);
	mySizer->Add(new wxStaticText(this,wxID_ANY," "),sizers->Exp0);

	last_dir=source->working_folder.GetFullPath();
	
	args_ctrl->SetValue(source->exec_args);
	compiler_options_ctrl->SetValue(source->GetCompilerOptions());
	always_ask_args_ctrl->SetValue(source->config_running.always_ask_args);
	wait_for_key_ctrl->SetValue(source->config_running.wait_for_key);

	mySizer->Add(bottomSizer,sizers->Exp0);

	SetSizerAndFit(mySizer);

	args_ctrl->SetFocus();

	Show();

}


void mxCompileConfigWindow::OnOkButton(wxCommandEvent &event){

	if (wxNOT_FOUND==main_window->notebook_sources->GetPageIndex(source)) {
		mxMessageDialog(this,LANG(COMPILECONF_SOURCE_CLOSED,"El fuente en cual se debe aplicar esta configuracion ha sido cerrado."), LANG(GENERAL_ERROR,"Error"), mxMD_ERROR|mxMD_OK).ShowModal();
		return;
	}
	
	if (working_folder_ctrl->GetValue()=="")
		working_folder_ctrl->SetValue(wxFileName::GetHomeDir());
	else if (!wxFileName::DirExists(working_folder_ctrl->GetValue())) {
		mxMessageDialog(this,LANG(COMPILECONF_INVALID_WORKDIR,"El directorio de trabajo no es valido."), LANG(GENERAL_ERROR,"Error"), mxMD_ERROR|mxMD_OK).ShowModal();
		return;
	}

	source->exec_args = args_ctrl->GetValue();
	source->working_folder = working_folder_ctrl->GetValue();
	source->SetCompilerOptions(compiler_options_ctrl->GetValue());
	source->config_running.always_ask_args = always_ask_args_ctrl->GetValue();
	source->config_running.wait_for_key = wait_for_key_ctrl->GetValue();
	Destroy();

}

void mxCompileConfigWindow::OnCancelButton(wxCommandEvent &event){
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData( new wxTextDataObject(compiler_options_ctrl->GetValue()) );
		wxTheClipboard->Close();
	}
	
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
	text_for_edit=args_ctrl;
	wxMenu menu;
	menu.Append(mxID_ARGS_EDIT_TEXT,LANG(GENERAL_POPUP_EDIT_AS_TEXT,"editar como texto"));
	menu.Append(mxID_ARGS_EDIT_LIST,LANG(GENERAL_POPUP_EDIT_AS_LIST,"editar como lista"));
	menu.Append(mxID_ARGS_REPLACE_FILE,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FILE,"reemplazar todo por archivo"));
	menu.Append(mxID_ARGS_ADD_FILE,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FILE,"reemplazar seleccion por archivo"));
	menu.Append(mxID_ARGS_REPLACE_DIR,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FOLDER,"reemplazar todo por directorio"));
	menu.Append(mxID_ARGS_ADD_DIR,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FOLDER,"reemplazar seleccion por directorio"));
	PopupMenu(&menu);
}

void mxCompileConfigWindow::OnArgsReplaceFile(wxCommandEvent &evt) {
	wxString sel = text_for_edit->GetStringSelection();
	wxFileDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FILE,"Reemplazar todo por archivo:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		last_dir = wxFileName(dlg.GetPath()).GetPath();
		wxString file=dlg.GetPath();
		if (file.Contains(' ')) file = wxString("\"")<<file<<"\"";
		text_for_edit->SetValue(file);
	}
}

void mxCompileConfigWindow::OnArgsAddFile(wxCommandEvent &evt) {
	wxString sel = text_for_edit->GetStringSelection();
	wxFileDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FILE,"Reemplazar seleccion por archivo:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		long p1,p2;
		text_for_edit->GetSelection(&p1,&p2);
		last_dir = wxFileName(dlg.GetPath()).GetPath();
		wxString file=dlg.GetPath();
		if (file.Contains(' ')) file = wxString("\"")<<file<<"\"";
		text_for_edit->Replace(p1,p2,file);
	}
}

void mxCompileConfigWindow::OnArgsReplaceDir(wxCommandEvent &evt) {
	wxString sel = text_for_edit->GetStringSelection();
	wxDirDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FOLDER,"Reemplazar todo por directorio:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		last_dir = dlg.GetPath();
		wxString file=dlg.GetPath();
		if (file.Contains(' ')) file = wxString("\"")<<file<<"\"";
		text_for_edit->SetValue(file);
	}
}

void mxCompileConfigWindow::OnArgsDefault(wxCommandEvent &evt) {
	compiler_options_ctrl->SetValue(config->GetDefaultCompilerOptions(source->IsCppOrJustC()));
}

void mxCompileConfigWindow::OnArgsAddDir(wxCommandEvent &evt) {
	wxString sel = text_for_edit->GetStringSelection();
	wxDirDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FOLDER,"Reemplazar seleccion por directorio:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		long p1,p2;
		text_for_edit->GetSelection(&p1,&p2);
		last_dir = dlg.GetPath();
		wxString file=dlg.GetPath();
		if (file.Contains(' ')) file = wxString("\"")<<file<<"\"";
		text_for_edit->Replace(p1,p2,file);
	}
}

void mxCompileConfigWindow::OnArgsFromTemplate(wxCommandEvent &evt) {
	wxString val=opts_list[evt.GetId()-mxID_LAST_ID];
	val.Replace("${DEFAULT}",config->GetDefaultCompilerOptions(source->IsCppOrJustC()),true);
	compiler_options_ctrl->SetValue(val);
}

void mxCompileConfigWindow::OnButtonCompilerOptions(wxCommandEvent &evt) {
	text_for_edit=compiler_options_ctrl;
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

void mxCompileConfigWindow::OnArgsEditText(wxCommandEvent &evt) {
	new mxLongTextEditor(this,LANG(COMPILECONF_COMPILER_ARGS_DLG,"Parametros extra para el compilador"),text_for_edit);
}

void mxCompileConfigWindow::OnArgsEditList(wxCommandEvent &evt) {
	new mxEnumerationEditor(this,LANG(COMPILECONF_COMPILER_ARGS_DLG,"Parametros extra para el compilador"),text_for_edit,false);
}
