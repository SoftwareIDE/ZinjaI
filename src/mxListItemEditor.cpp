#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include "mxListItemEditor.h"
#include "ids.h"
#include "mxBitmapButton.h"
#include "ConfigManager.h"
#include "mxSizers.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxListItemEditor, wxDialog)
	
	EVT_BUTTON(wxID_OK,mxListItemEditor::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxListItemEditor::OnCancelButton)
	
	EVT_BUTTON(mxID_ARGS_BUTTON,mxListItemEditor::OnArgsButton)
	EVT_MENU(mxID_ARGS_REPLACE_FILE,mxListItemEditor::OnReplaceFile)
	EVT_MENU(mxID_ARGS_ADD_FILE,mxListItemEditor::OnAddFile)
	EVT_MENU(mxID_ARGS_REPLACE_DIR,mxListItemEditor::OnReplaceDir)
	EVT_MENU(mxID_ARGS_ADD_DIR,mxListItemEditor::OnAddDir)
	EVT_BUTTON(mxID_HELP_BUTTON,mxListItemEditor::OnHelpButton)
	
	EVT_CLOSE(mxListItemEditor::OnClose)
	
END_EVENT_TABLE()

mxListItemEditor::mxListItemEditor(wxWindow *parent, wxString title, wxListBox *alist, int asel) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(400,100) ,wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	
	list=alist; sel=asel;
	
	last_dir=config->Files.last_dir;
	
	wxBoxSizer *mid_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxString value;
	if (sel!=wxNOT_FOUND)
		value=list->GetString(sel);
	my_text = new wxTextCtrl(this,wxID_ANY,value,wxDefaultPosition,wxDefaultSize/*,wxTE_MULTILINE*/);
	mid_sizer->Add(my_text,sizers->BA5_Exp1);
	wxButton *button_ok = new mxBitmapButton(this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	wxButton *button_cancel = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	wxButton *button_more = new wxButton(this,mxID_ARGS_BUTTON,LANG(LONGTEXT_MORE_OPTIONS,"Mas opciones..."));
	wxBitmapButton *button_help = new wxBitmapButton (this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	bottom_sizer->Add(button_help,sizers->BA5);
	bottom_sizer->AddStretchSpacer();
	bottom_sizer->Add(button_more,sizers->BA5);
	bottom_sizer->AddStretchSpacer();
	bottom_sizer->Add(button_cancel,sizers->BA5);
	bottom_sizer->Add(button_ok,sizers->BA5);
	mid_sizer->Add(bottom_sizer,sizers->Exp0);
	
	button_ok->SetDefault(); 
	my_text->SetSelection(0,my_text->GetLastPosition());
	my_text->SetFocus();
	
	SetSizerAndFit(mid_sizer);
	ShowModal();
	
}


mxListItemEditor::~mxListItemEditor() {
	
}

void mxListItemEditor::OnOkButton(wxCommandEvent &evt) {
	wxString str = my_text->GetValue();
	str.Replace("\n"," ");
	str.Replace("\r","");
	if (sel==wxNOT_FOUND) {
		if (list->GetSelection()!=wxNOT_FOUND)
			list->Insert(str,list->GetSelection());
		else
			list->Append(str);
	} else
		list->SetString(sel,str);
	Close();
}

void mxListItemEditor::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxListItemEditor::OnClose(wxCloseEvent &evt) {
	list->SetFocus();
	EndModal(0);
	Destroy();
}


void mxListItemEditor::OnArgsButton(wxCommandEvent &evt) {
	wxMenu menu;
	menu.Append(mxID_ARGS_REPLACE_FILE,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FILE,"reemplazar todo por archivo"));
	menu.Append(mxID_ARGS_ADD_FILE,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FILE,"reemplazar seleccion por archivo"));
	menu.Append(mxID_ARGS_REPLACE_DIR,LANG(GENERAL_POPUP_REPLACE_ALL_WITH_FOLDER,"reemplazar todo por directorio"));
	menu.Append(mxID_ARGS_ADD_DIR,LANG(GENERAL_POPUP_REPLACE_SELECTION_WITH_FOLDER,"reemplazar seleccion por directorio"));
	PopupMenu(&menu);
}

void mxListItemEditor::OnReplaceFile(wxCommandEvent &evt) {
	wxString sel = my_text->GetStringSelection();
	wxFileDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FILE,"Reemplazar todo por archivo:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		last_dir = dlg.GetPath();
		my_text->SetValue(dlg.GetPath());
	}
}

void mxListItemEditor::OnAddFile(wxCommandEvent &evt) {
	wxString sel = my_text->GetStringSelection();
	wxFileDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FILE,"Reemplazar seleccion por archivo:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		long p1,p2;
		my_text->GetSelection(&p1,&p2);
		last_dir = dlg.GetPath();
		my_text->Replace(p1,p2,dlg.GetPath());
	}
}

void mxListItemEditor::OnReplaceDir(wxCommandEvent &evt) {
	wxString sel = my_text->GetStringSelection();
	wxDirDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_ALL_WITH_FOLDER,"Reemplazar todo por directorio:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		last_dir = dlg.GetPath();
		my_text->SetValue(dlg.GetPath());
	}
}

void mxListItemEditor::OnAddDir(wxCommandEvent &evt) {
	wxString sel = my_text->GetStringSelection();
	wxDirDialog dlg(this,LANG(GENERAL_FILEDLG_REPLACE_SELECTION_WITH_FOLDER,"Reemplazar seleccion por directorio:"),sel.Len()?sel:last_dir);
	if (wxID_OK==dlg.ShowModal()) {
		long p1,p2;
		my_text->GetSelection(&p1,&p2);
		last_dir = dlg.GetPath();
		my_text->Replace(p1,p2,dlg.GetPath());
	}
}

void mxListItemEditor::OnHelpButton(wxCommandEvent &evt) {
	
}
