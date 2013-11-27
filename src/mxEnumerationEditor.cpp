#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/arrstr.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include "mxEnumerationEditor.h"
#include "mxUtils.h"
#include "ids.h"
#include "mxBitmapButton.h"
#include "mxListItemEditor.h"
#include "mxSizers.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxEnumerationEditor, wxDialog)
	EVT_BUTTON(wxID_OK,mxEnumerationEditor::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxEnumerationEditor::OnCancelButton)
	EVT_BUTTON(mxID_ENUMED_ADD,mxEnumerationEditor::OnAdd)
	EVT_BUTTON(mxID_ENUMED_DELETE,mxEnumerationEditor::OnDelete)
	EVT_BUTTON(mxID_ENUMED_EDIT,mxEnumerationEditor::OnEdit)
	EVT_BUTTON(mxID_ENUMED_UP,mxEnumerationEditor::OnUp)
	EVT_BUTTON(mxID_ENUMED_DOWN,mxEnumerationEditor::OnDown)
	EVT_CLOSE(mxEnumerationEditor::OnClose)
END_EVENT_TABLE()
	

mxEnumerationEditor::mxEnumerationEditor(wxWindow *parent, wxString title, wxComboBox *acombo, bool comma_splits) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(450,400) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	text=NULL; combo=acombo;
	CreateCommonStuff(combo->GetValue(),comma_splits);
}

mxEnumerationEditor::mxEnumerationEditor(wxWindow *parent, wxString title, wxTextCtrl *atext, bool comma_splits) : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(450,400) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	text=atext; combo=NULL;
	CreateCommonStuff(text->GetValue(),comma_splits);
}
void mxEnumerationEditor::CreateCommonStuff(wxString value, bool comma_splits) {
	
	wxBoxSizer *mid_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *right_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	
	list = new wxListBox(this,wxID_ANY);
	mid_sizer->Add(list,sizers->BA5_Exp1);
	wxButton *button_up = new wxButton(this,mxID_ENUMED_UP,LANG(ENUMEDITOR_MOVE_UP,"Mover Arriba"));
	wxButton *button_down = new wxButton(this,mxID_ENUMED_DOWN,LANG(ENUMEDITOR_MOVE_DOWN,"Mover Abajo"));
	wxButton *button_edit = new wxButton(this,mxID_ENUMED_EDIT,LANG(ENUMEDITOR_EDIT,"Editar..."));
	wxButton *button_add = new wxButton(this,mxID_ENUMED_ADD,LANG(ENUMEDITOR_ADD,"Agregar..."));
	wxButton *button_delete = new wxButton(this,mxID_ENUMED_DELETE,LANG(ENUMEDITOR_REMOVE,"Quitar"));
	
	wxButton *button_ok = new mxBitmapButton(this,wxID_OK,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	wxButton *button_cancel = new mxBitmapButton(this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	right_sizer->Add(button_up,sizers->BA5_Exp0);
	right_sizer->Add(button_down,sizers->BA5_Exp0);
	right_sizer->Add(button_edit,sizers->BA5_Exp0);
	right_sizer->Add(button_add,sizers->BA5_Exp0);
	right_sizer->Add(button_delete,sizers->BA5_Exp0);
	mid_sizer->Add(right_sizer,sizers->Exp0_Right);
	bottom_sizer->Add(button_cancel,sizers->BA5_Exp0);
	bottom_sizer->Add(button_ok,sizers->BA5_Exp0);
	main_sizer->Add(mid_sizer,sizers->Exp1);
	main_sizer->Add(bottom_sizer,sizers->Center);
	
	wxArrayString array;
	this->comma_splits=comma_splits;
	utils->Split(value,array,comma_splits,false);
	list->InsertItems(array,0);
	list->SetFocus();
	SetSizer(main_sizer);
	ShowModal();
}

mxEnumerationEditor::~mxEnumerationEditor() {
	
}

void mxEnumerationEditor::OnOkButton(wxCommandEvent &evt) {
	wxString str;
	for (unsigned int i=0;i<list->GetCount();i++) {
		if (comma_splits)
			str<<utils->QuotizeEx(list->GetString(i))<<' ';
		else
			str<<utils->Quotize(list->GetString(i))<<' ';
	}
	if (str.Len()) str.RemoveLast();
	if (text) text->SetValue(str);
	if (combo) combo->SetValue(str);
	Close();
}

void mxEnumerationEditor::OnCancelButton(wxCommandEvent &evt) {
	Close();
}

void mxEnumerationEditor::OnClose(wxCloseEvent &evt) {
	if (text) text->SetFocus();
	if (combo) combo->SetFocus();
	EndModal(0);
	Destroy();
}

void mxEnumerationEditor::OnAdd(wxCommandEvent &evt) {
	new mxListItemEditor(this,LANG(ENUMEDITOR_NEW_ITEM,"Nuevo item"),list,wxNOT_FOUND);
}

void mxEnumerationEditor::OnDelete(wxCommandEvent &evt) {
	wxArrayInt ai;
	list->GetSelections(ai);
	if (ai.GetCount())
		for (int i=(int)ai.GetCount()-1;i>=0;i--)
			list->Delete(ai[i]);
	if (ai[0]<int(list->GetCount())) list->SetSelection(ai[0]);
}

void mxEnumerationEditor::OnEdit(wxCommandEvent &evt) {
	if (list->GetSelection()!=wxNOT_FOUND) 
		new mxListItemEditor(this,LANG(ENUMEDITOR_EDIT_ITEM,"Editar item"),list,list->GetSelection());
}

void mxEnumerationEditor::OnUp(wxCommandEvent &evt) {
	int n=list->GetSelection();
	if (n!=wxNOT_FOUND && n>0) {
		wxString aux = list->GetString(n);		
		list->SetString(n,list->GetString(n-1));
		list->SetString(n-1,aux);
		list->SetSelection(n-1);
	}
}

void mxEnumerationEditor::OnDown(wxCommandEvent &evt) {
	int n=list->GetSelection();
	if (n!=wxNOT_FOUND && n+1<(int)list->GetCount()) {
		wxString aux = list->GetString(n);		
		list->SetString(n,list->GetString(n+1));
		list->SetString(n+1,aux);
		list->SetSelection(n+1);
	}
}
