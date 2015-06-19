#include "mxArgumentsDialog.h"
#include <wx/sizer.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include "mxBitmapButton.h"
#include "ids.h"
#include <wx/menu.h>
#include "mxSizers.h"
#include "mxLongTextEditor.h"
#include "mxEnumerationEditor.h"
#include "Language.h"

wxString mxArgumentsDialog::last_arguments;
wxString mxArgumentsDialog::last_workdir;
wxString mxArgumentsDialog::last_dir;
wxArrayString mxArgumentsDialog::list_for_combo_args;
wxArrayString mxArgumentsDialog::list_for_combo_work;


BEGIN_EVENT_TABLE(mxArgumentsDialog, wxDialog)
	EVT_BUTTON(wxID_YES,mxArgumentsDialog::OnYesButton)
	EVT_BUTTON(wxID_NO,mxArgumentsDialog::OnNoButton)
	EVT_BUTTON(wxID_CANCEL,mxArgumentsDialog::OnCancelButton)
	EVT_CHAR_HOOK(mxArgumentsDialog::OnCharHook)
	EVT_CLOSE(mxArgumentsDialog::OnClose)	
	EVT_MENU(mxID_ARGS_REPLACE_DIR,mxArgumentsDialog::OnArgsReplaceDir)
	EVT_MENU(mxID_ARGS_ADD_DIR,mxArgumentsDialog::OnArgsAddDir)
	EVT_MENU(mxID_ARGS_REPLACE_FILE,mxArgumentsDialog::OnArgsReplaceFile)
	EVT_MENU(mxID_ARGS_ADD_FILE,mxArgumentsDialog::OnArgsAddFile)
	EVT_MENU(mxID_ARGS_EDIT_LIST,mxArgumentsDialog::OnArgsEditList)
	EVT_MENU(mxID_ARGS_EDIT_TEXT,mxArgumentsDialog::OnArgsEditText)
	EVT_BUTTON(mxID_ARGS_BUTTON,mxArgumentsDialog::OnArgsButton)
	EVT_BUTTON(mxID_WORKING_FOLDER,mxArgumentsDialog::OnWorkdirButton)
END_EVENT_TABLE()

	
mxArgumentsDialog::mxArgumentsDialog(wxWindow *parent, const wxString &def_args, const wxString &def_dir) : wxDialog(parent,wxID_ANY,LANG(ARGUMENTS_CAPTION,"Argumentos para la ejecucion"),wxDefaultPosition,wxDefaultSize,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	
	mySizer->Add(new wxStaticText(this, wxID_ANY, LANG(COMPILECONF_RUNNING_ARGS,"Argumentos para la ejecucion:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5_Left);
	wxBoxSizer *comboSizer1 = new wxBoxSizer(wxHORIZONTAL);
	combo_args = new wxComboBox(this, wxID_ANY,def_args, wxDefaultPosition, wxDefaultSize, list_for_combo_args);
	comboSizer1->Add(combo_args, sizers->Exp1);
	comboSizer1->Add(new wxButton(this,mxID_ARGS_BUTTON,_T("..."),wxDefaultPosition,wxSize(30,10)), sizers->Exp0);
	mySizer->Add(comboSizer1, sizers->BA5_Exp1);
	
	mySizer->Add(new wxStaticText(this, wxID_ANY, LANG(COMPILECONF_WORKDIR,"Directorio de trabajo:"), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5_Left);
	wxBoxSizer *comboSizer2 = new wxBoxSizer(wxHORIZONTAL);
	combo_work = new wxComboBox(this, wxID_ANY,def_dir, wxDefaultPosition, wxDefaultSize, list_for_combo_work);
	comboSizer2->Add(combo_work, sizers->Exp1);
	comboSizer2->Add(new wxButton(this,mxID_WORKING_FOLDER,_T("..."),wxDefaultPosition,wxSize(30,10)), sizers->Exp0);
	mySizer->Add(comboSizer2, sizers->BA5_Exp1);
	
	check = new wxCheckBox(this, wxID_ANY,LANG(GENERAL_DONT_ASK_AGAIN,"no volver a preguntar"));
	mySizer->Add(check, sizers->BA5_Exp1);
	
	wxBoxSizer *bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	mxBitmapButton *button_yes = new mxBitmapButton(this,wxID_YES,bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	mxBitmapButton *button_no = new mxBitmapButton(this,wxID_NO,bitmaps->buttons.ok, LANG(ARGUMENTS_WITHOUT_ARGUMENTS,"&Sin Argumentos"));
	mxBitmapButton *button_cancel = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	bottomSizer->Add(button_cancel , sizers->BA5);
	bottomSizer->Add(button_no , sizers->BA5);
	bottomSizer->Add(button_yes , sizers->BA5);
	SetAffirmativeId(wxID_YES);
	SetEscapeId(wxID_CANCEL);
	mySizer->Add(bottomSizer,sizers->BA5_Right);
	
	SetMinSize(GetSize());
	SetSizerAndFit(mySizer);
	SetFocusFromKbd();
	
}

void mxArgumentsDialog::OnYesButton(wxCommandEvent &evt) {
	last_workdir=combo_work->GetValue();
	if (list_for_combo_work.GetCount()==0 || list_for_combo_work.Last()!=last_workdir)
		list_for_combo_work.Add(last_workdir);
	if (combo_args->GetValue().Len()) {
		last_arguments=combo_args->GetValue();
		if (list_for_combo_args.GetCount()==0 || list_for_combo_args.Last()!=last_arguments)
			list_for_combo_args.Add(last_arguments);
		EndModal(AD_ARGS|(check->GetValue()?AD_REMEMBER:0));
	} else {
		last_arguments="";
		EndModal(AD_EMPTY|(check->GetValue()?AD_REMEMBER:0));
	}
}
void mxArgumentsDialog::OnNoButton(wxCommandEvent &evt) {
	last_arguments="";
	EndModal(AD_EMPTY|(check->GetValue()?AD_REMEMBER:0));
}
void mxArgumentsDialog::OnCancelButton(wxCommandEvent &evt) {
	EndModal(AD_CANCEL);
}
void mxArgumentsDialog::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxArgumentsDialog::OnArgsButton(wxCommandEvent &evt) {
	wxMenu menu;
	menu.Append(mxID_ARGS_EDIT_TEXT,LANG(GENERAL_POPUP_EDIT_AS_TEXT,"editar como texto"));
	menu.Append(mxID_ARGS_EDIT_LIST,LANG(GENERAL_POPUP_EDIT_AS_LIST,"editar como list"));
	menu.Append(mxID_ARGS_REPLACE_FILE,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FILE,"reemplazar todo por archivo"));
	menu.Append(mxID_ARGS_ADD_FILE,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FILE,"reemplazar seleccion por archivo"));
	menu.Append(mxID_ARGS_REPLACE_DIR,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FOLDER,"reemplazar todo por directorio"));
	menu.Append(mxID_ARGS_ADD_DIR,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FOLDER,"reemplazar seleccion por directorio"));
	PopupMenu(&menu);
}

void mxArgumentsDialog::OnArgsReplaceFile(wxCommandEvent &evt) {
	wxString sel = combo_args->GetStringSelection();
	wxFileDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FILE,"Reemplazar todo por archivo:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		last_dir = dlg.GetPath();
		combo_args->SetValue(dlg.GetPath());
	}
}

void mxArgumentsDialog::OnArgsAddFile(wxCommandEvent &evt) {
	wxString sel = combo_args->GetStringSelection();
	wxFileDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FILE,"Reemplazar seleccion por archivo:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		long p1,p2;
		combo_args->GetSelection(&p1,&p2);
		last_dir = dlg.GetPath();
		combo_args->Replace(p1,p2,dlg.GetPath());
	}
}

void mxArgumentsDialog::OnArgsReplaceDir(wxCommandEvent &evt) {
	wxString sel = combo_args->GetStringSelection();
	wxDirDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FOLDER,"Reemplazar todo por directorio:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		last_dir = dlg.GetPath();
		combo_args->SetValue(dlg.GetPath());
	}
}

void mxArgumentsDialog::OnArgsAddDir(wxCommandEvent &evt) {
	wxString sel = combo_args->GetStringSelection();
	wxDirDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FOLDER,"Reemplazar seleccion por directorio:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		long p1,p2;
		combo_args->GetSelection(&p1,&p2);
		last_dir = dlg.GetPath();
		combo_args->Replace(p1,p2,dlg.GetPath());
	}
}
	
void mxArgumentsDialog::OnCharHook(wxKeyEvent &evt)	{
	if (evt.GetKeyCode()==WXK_RETURN || evt.GetKeyCode()==WXK_NUMPAD_ENTER) {
		wxCommandEvent evt;
		OnYesButton(evt);
	} else evt.Skip();
}

void mxArgumentsDialog::OnArgsEditText(wxCommandEvent &evt) {
	new mxLongTextEditor(this,LANG(ARGUMENTS_CAPTION,"Argumentos para la ejecucion"),combo_args);
}

void mxArgumentsDialog::OnArgsEditList(wxCommandEvent &evt) {
	new mxEnumerationEditor(this,LANG(ARGUMENTS_CAPTION,"Argumentos para la ejecucion"),combo_args,false);
}
	

void mxArgumentsDialog::OnWorkdirButton (wxCommandEvent & evt) {
	wxDirDialog dlg(this,LANG(COMPILECONF_WORKDIR_DLG,"Directorio de trabajo"),combo_args->GetValue());
	if (wxID_OK==dlg.ShowModal())
		combo_args->SetValue(dlg.GetPath());
}

