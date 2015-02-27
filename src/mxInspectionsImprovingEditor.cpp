#include <wx/sizer.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include "ConfigManager.h"
#include "mxInspectionsImprovingEditor.h"
#include "mxBitmapButton.h"
#include "ids.h"
#include "mxSizers.h"
#include "Language.h"
#include "mxHelpWindow.h"

BEGIN_EVENT_TABLE(mxInspectionsImprovingEditor,wxDialog)
	EVT_LISTBOX(wxID_ANY,mxInspectionsImprovingEditor::OnList)
	EVT_BUTTON(wxID_OK,mxInspectionsImprovingEditor::OnOk)
	EVT_BUTTON(wxID_CANCEL,mxInspectionsImprovingEditor::OnCancel)
	EVT_BUTTON(mxID_INSPECTION_TEMPLATES_UP,mxInspectionsImprovingEditor::OnUp)
	EVT_BUTTON(mxID_INSPECTION_TEMPLATES_DOWN,mxInspectionsImprovingEditor::OnDown)
	EVT_BUTTON(mxID_INSPECTION_TEMPLATES_ADD,mxInspectionsImprovingEditor::OnAdd)
	EVT_BUTTON(mxID_INSPECTION_TEMPLATES_DEL,mxInspectionsImprovingEditor::OnDel)
	EVT_TEXT(mxID_INSPECTION_TEMPLATES_FROM,mxInspectionsImprovingEditor::OnFrom)
	EVT_TEXT(mxID_INSPECTION_TEMPLATES_TO,mxInspectionsImprovingEditor::OnTo)
	EVT_BUTTON(mxID_HELP_BUTTON,mxInspectionsImprovingEditor::OnHelp)
END_EVENT_TABLE()
	
mxInspectionsImprovingEditor::mxInspectionsImprovingEditor(wxWindow *parent, wxArrayString &a_from, wxArrayString &a_to, const wxString &type, const wxString &expr) 
	: wxDialog(parent,wxID_ANY,"Reemplaza automático de inspecciones",wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER) 
{
	
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	wxSizer *sizer_top=new wxBoxSizer(wxVERTICAL);
	
	array_from=a_from; orig_array_from=&a_from;
	array_to=a_to; orig_array_to=&a_to;
	
	list=new wxListBox(this,mxID_INSPECTION_TEMPLATES_LIST,wxDefaultPosition,wxDefaultSize,array_from);
	sizer_top->Add(list,sizers->BA5_Exp1);
	
	wxSizer *sizer_options=new wxBoxSizer(wxVERTICAL);
	sizer_options->Add(new wxStaticText(this,wxID_ANY,"Tipo"));
	sizer_options->Add(type_from=new wxTextCtrl(this,mxID_INSPECTION_TEMPLATES_FROM,""),sizers->BA5_Exp0);
	sizer_options->Add(new wxStaticText(this,wxID_ANY,"Expresion"));
	sizer_options->Add(expression_to=new wxTextCtrl(this,mxID_INSPECTION_TEMPLATES_TO,""),sizers->BA5_Exp0);
	wxSizer *sizer_add_del=new wxBoxSizer(wxHORIZONTAL);
	sizer_add_del->Add(new wxButton(this,mxID_INSPECTION_TEMPLATES_UP,"Move Up"),sizers->BA5);
	sizer_add_del->Add(new wxButton(this,mxID_INSPECTION_TEMPLATES_DOWN,"Move Down"),sizers->BA5);
	sizer_add_del->AddStretchSpacer(1);
	sizer_add_del->Add(new wxButton(this,mxID_INSPECTION_TEMPLATES_ADD,"Add"),sizers->BA5);
	sizer_add_del->Add(new wxButton(this,mxID_INSPECTION_TEMPLATES_DEL,"Del"),sizers->BA5);
	sizer_options->Add(sizer_add_del,sizers->Exp0);
	
	sizer_top->Add(sizer_options,sizers->Exp0);
	sizer->Add(sizer_top,sizers->Exp1);
	
	wxSizer *sizer_buttons=new wxBoxSizer(wxHORIZONTAL);
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	mxBitmapButton *bt_ok=new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	mxBitmapButton *bt_cancel=new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	sizer_buttons->Add(help_button,sizers->BA5); 
	sizer_buttons->AddStretchSpacer(1);
	sizer_buttons->Add(bt_ok,sizers->BA5); 
	sizer_buttons->Add(bt_cancel,sizers->BA5);
	sizer->Add(sizer_buttons,sizers->Exp0);
	bt_cancel->SetDefault();
	SetEscapeId(wxID_CANCEL);
	
	SetSizerAndFit(sizer);
	
//	if (list->GetCount()) {
//		list->Select(selected=0);
//		wxCommandEvent evt; OnList(evt);
//		type_from->SetFocus();
//	} else {
		selected=-1;
		type_from->Enable(false);
		expression_to->Enable(false);
//	}
	
	if (!type.IsEmpty()) {
		int sel_pos=0;
		if (array_from.Index(type)!=wxNOT_FOUND) {
			sel_pos=array_from.Index(type);
		} else {
			array_from.Insert(type,0);
			array_to.Insert(expr+" (use ${EXP} as the original expression)",0);
			list->Insert(array_from.Last(),0);
		}
		list->Select(sel_pos);
		wxCommandEvent evt; OnList(evt);
	}
	
	ShowModal();
	
}

void mxInspectionsImprovingEditor::OnFrom (wxCommandEvent & evt) {
	if (selected<0) return;
	array_from[selected]=type_from->GetValue();
	list->SetString(selected,array_from[selected]);
}

void mxInspectionsImprovingEditor::OnTo (wxCommandEvent & evt) {
	if (selected<0) return;
	array_to[selected]=expression_to->GetValue();
}

void mxInspectionsImprovingEditor::OnAdd (wxCommandEvent & evt) {
	array_from.Add("<new type>");
	array_to.Add("${EXP}");
	list->Append(array_from.Last());
	list->Select(list->GetCount()-1);
	OnList(evt);
}

void mxInspectionsImprovingEditor::OnDel (wxCommandEvent & evt) {
	if (selected<0) return;
	array_from.RemoveAt(selected);
	array_to.RemoveAt(selected);
	int old_selected=selected;
	list->Delete(selected);
	selected=old_selected;
	if (selected<int(list->GetCount())) {
		list->Select(selected);
		wxCommandEvent evt;
		OnList(evt);
	} else {
		selected=-1;
		type_from->Enable(false);
		expression_to->Enable(false);
	}
}

void mxInspectionsImprovingEditor::OnOk (wxCommandEvent & evt) {
	(*orig_array_from) = array_from;
	(*orig_array_to) = array_to;
	EndModal(1);
}

void mxInspectionsImprovingEditor::OnCancel (wxCommandEvent & evt) {
	EndModal(0);
}

void mxInspectionsImprovingEditor::OnList (wxCommandEvent & evt) {
	selected = list->GetSelection();
	if (selected<0||selected>=int(list->GetCount())) return; // delete method can launch this event
	type_from->Enable();
	expression_to->Enable();
	type_from->SetValue(array_from[selected]);
	expression_to->SetValue(array_to[selected]);
}

void mxInspectionsImprovingEditor::OnUp (wxCommandEvent & evt) {
	if (selected<1) return;
	swap(array_to[selected],array_to[selected-1]);
	swap(array_from[selected],array_from[selected-1]);
	list->SetString(selected,array_from[selected]);
	list->SetString(selected-1,array_from[selected-1]);
	selected--; list->Select(selected); OnList(evt);
}

void mxInspectionsImprovingEditor::OnDown (wxCommandEvent &evt) {
	if (selected==-1 || selected>int(list->GetCount())-2) return;
	swap(array_to[selected],array_to[selected+1]);
	swap(array_from[selected],array_from[selected+1]);
	list->SetString(selected,array_from[selected]);
	list->SetString(selected+1,array_from[selected+1]);
	selected++; list->Select(selected); OnList(evt);
}

void mxInspectionsImprovingEditor::OnHelp (wxCommandEvent & evt) {
	mxHelpWindow::ShowHelp("inspections_improving.html",this);
}

