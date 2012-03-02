#include "mxLongTextEditor.h"
#include "ids.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include "mxBitmapButton.h"
#include "ConfigManager.h"
#include "mxSizers.h"
#include <wx/combo.h>
#include "Language.h"

BEGIN_EVENT_TABLE(mxLongTextEditor, wxDialog)
	
	EVT_BUTTON(wxID_OK,mxLongTextEditor::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxLongTextEditor::OnCancelButton)

	EVT_BUTTON(mxID_ARGS_BUTTON,mxLongTextEditor::OnArgsButton)
	EVT_MENU(mxID_ARGS_REPLACE_FILE,mxLongTextEditor::OnReplaceFile)
	EVT_MENU(mxID_ARGS_ADD_FILE,mxLongTextEditor::OnAddFile)
	EVT_MENU(mxID_ARGS_REPLACE_DIR,mxLongTextEditor::OnReplaceDir)
	EVT_MENU(mxID_ARGS_ADD_DIR,mxLongTextEditor::OnAddDir)
	EVT_BUTTON(mxID_HELP_BUTTON,mxLongTextEditor::OnHelpButton)
	
	EVT_CLOSE(mxLongTextEditor::OnClose)
	
END_EVENT_TABLE()

mxLongTextEditor::mxLongTextEditor(wxWindow *parent, wxString title, wxComboBox *acombo) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(450,250) ,wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	text=NULL; combo=acombo;
	CreateCommonStuff(combo->GetValue());	
}
mxLongTextEditor::mxLongTextEditor(wxWindow *parent, wxString title, wxTextCtrl *atext) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(450,250) ,wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	text=atext; combo=NULL;
	CreateCommonStuff(text->GetValue());
}

void mxLongTextEditor::CreateCommonStuff(wxString value) {
	
	last_dir=config->Files.last_dir;
	
	wxBoxSizer *mid_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	my_text = new wxTextCtrl(this,wxID_ANY,value,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
	mid_sizer->Add(my_text,sizers->BA5_Exp1);
	wxButton *button_ok = new mxBitmapButton(this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	wxButton *button_cancel = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	wxButton *button_more = new wxButton(this,mxID_ARGS_BUTTON,LANG(LONGTEXT_MORE_OPTIONS,"Mas opciones..."));
	wxBitmapButton *button_help = new wxBitmapButton (this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	bottom_sizer->Add(button_help,sizers->BA5_Exp0);
	bottom_sizer->AddStretchSpacer();
	bottom_sizer->Add(button_more,sizers->BA5);
	bottom_sizer->AddStretchSpacer();
	bottom_sizer->Add(button_cancel,sizers->BA5);
	bottom_sizer->Add(button_ok,sizers->BA5);
	mid_sizer->Add(bottom_sizer,sizers->Exp0);
	
	button_ok->SetDefault(); 
	my_text->SetSelection(0,my_text->GetLastPosition());
	my_text->SetFocus();
	
	SetSizer(mid_sizer);
	ShowModal();
	
}


mxLongTextEditor::~mxLongTextEditor() {

}

void mxLongTextEditor::OnOkButton(wxCommandEvent &evt) {
	wxString str = my_text->GetValue();
	str.Replace(_T("\n"),_T(" "));
	str.Replace(_T("\r"),_T(""));
	if (text) text->SetValue(str);
	if (combo) combo->SetValue(str);
	Close();
}

void mxLongTextEditor::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxLongTextEditor::OnClose(wxCloseEvent &evt) {
	if (text) text->SetFocus();
	if (combo) combo->SetFocus();
	EndModal(0);
	Destroy();
}


void mxLongTextEditor::OnArgsButton(wxCommandEvent &evt) {
	wxMenu menu;
	menu.Append(mxID_ARGS_REPLACE_FILE,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FILE,"reemplazar todo por archivo"));
	menu.Append(mxID_ARGS_ADD_FILE,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FILE,"reemplazar seleccion por archivo"));
	menu.Append(mxID_ARGS_REPLACE_DIR,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FOLDER,"reemplazar todo por directorio"));
	menu.Append(mxID_ARGS_ADD_DIR,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FOLDER,"reemplazar seleccion por directorio"));
	PopupMenu(&menu);
}

void mxLongTextEditor::OnReplaceFile(wxCommandEvent &evt) {
	wxString sel = my_text->GetStringSelection();
	wxFileDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FILE,"Reemplazar todo por archivo:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		last_dir = dlg.GetPath();
		my_text->SetValue(dlg.GetPath());
	}
}

void mxLongTextEditor::OnAddFile(wxCommandEvent &evt) {
	wxString sel = my_text->GetStringSelection();
	wxFileDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FILE,"Reemplazar seleccion por archivo:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		long p1,p2;
		my_text->GetSelection(&p1,&p2);
		last_dir = dlg.GetPath();
		my_text->Replace(p1,p2,dlg.GetPath());
	}
}

void mxLongTextEditor::OnReplaceDir(wxCommandEvent &evt) {
	wxString sel = my_text->GetStringSelection();
	wxDirDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FOLDER,"Reemplazar todo por directorio:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		last_dir = dlg.GetPath();
		my_text->SetValue(dlg.GetPath());
	}
}

void mxLongTextEditor::OnAddDir(wxCommandEvent &evt) {
	wxString sel = my_text->GetStringSelection();
	wxDirDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FOLDER,"Reemplazar seleccion por directorio:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		long p1,p2;
		my_text->GetSelection(&p1,&p2);
		last_dir = dlg.GetPath();
		my_text->Replace(p1,p2,dlg.GetPath());
	}
}

void mxLongTextEditor::OnHelpButton(wxCommandEvent &evt) {
	
}
