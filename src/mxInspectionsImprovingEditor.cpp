#include <wx/sizer.h>
#include <wx/listbox.h>
#include <wx/stattext.h>
#include "ConfigManager.h"
#include "mxInspectionsImprovingEditor.h"
#include "mxBitmapButton.h"
#include "ids.h"
#include "mxSizers.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxInspectionsImprovingEditor,wxDialog)
	EVT_LISTBOX(wxID_ANY,mxInspectionsImprovingEditor::OnList)
	EVT_BUTTON(wxID_OK,mxInspectionsImprovingEditor::OnOk)
	EVT_BUTTON(wxID_CANCEL,mxInspectionsImprovingEditor::OnCancel)
	EVT_BUTTON(mxID_INSPECTION_TEMPLATES_ADD,mxInspectionsImprovingEditor::OnAdd)
	EVT_BUTTON(mxID_INSPECTION_TEMPLATES_DEL,mxInspectionsImprovingEditor::OnDel)
	EVT_TEXT(mxID_INSPECTION_TEMPLATES_FROM,mxInspectionsImprovingEditor::OnFrom)
	EVT_TEXT(mxID_INSPECTION_TEMPLATES_TO,mxInspectionsImprovingEditor::OnTo)
END_EVENT_TABLE()
	
mxInspectionsImprovingEditor::mxInspectionsImprovingEditor(wxWindow *parent):wxDialog(parent,wxID_ANY,"Reemplaza automático de inspecciones",wxDefaultPosition,wxDefaultSize) {
	
	wxSizer *sizer=new wxBoxSizer(wxVERTICAL);
	wxSizer *sizer_top=new wxBoxSizer(wxHORIZONTAL);
	
	array_from=config->Debug.inspection_improving_template_from;
	array_to=config->Debug.inspection_improving_template_to;
	list=new wxListBox(this,mxID_INSPECTION_TEMPLATES_LIST,wxDefaultPosition,wxDefaultSize,array_from);
	sizer_top->Add(list,sizers->BA5_Exp1);
	
	wxSizer *sizer_options=new wxBoxSizer(wxVERTICAL);
	sizer_options->Add(new wxStaticText(this,wxID_ANY,"Tipo"));
	sizer_options->Add(type_from=new wxTextCtrl(this,mxID_INSPECTION_TEMPLATES_FROM,""),sizers->BA5_Exp0);
	sizer_options->Add(new wxStaticText(this,wxID_ANY,"Expresion"));
	sizer_options->Add(expression_to=new wxTextCtrl(this,mxID_INSPECTION_TEMPLATES_TO,""),sizers->BA5_Exp0);
	wxSizer *sizer_add_del=new wxBoxSizer(wxHORIZONTAL);
	sizer_add_del->Add(new wxButton(this,mxID_INSPECTION_TEMPLATES_ADD,"Add"),sizers->BA5);
	sizer_add_del->Add(new wxButton(this,mxID_INSPECTION_TEMPLATES_DEL,"Del"),sizers->BA5);
	sizer_options->Add(sizer_add_del,sizers->Exp0);
	
	sizer_top->Add(sizer_options,sizers->Exp1);
	sizer->Add(sizer_top,sizers->Exp1);
	
	wxSizer *sizer_buttons=new wxBoxSizer(wxHORIZONTAL);
	mxBitmapButton *bt_ok=new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	mxBitmapButton *bt_cancel=new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(GENERAL_OK_BUTTON,"&Aceptar"));
	sizer_buttons->Add(bt_ok,sizers->BA5); 
	sizer_buttons->Add(bt_cancel,sizers->BA5);
	sizer->Add(sizer_buttons,sizers->Right);
	bt_cancel->SetDefault();
	SetEscapeId(wxID_CANCEL);
	
	SetSizerAndFit(sizer);
	
	if (list->GetCount()) {
		list->Select(selected=0);
		wxCommandEvent evt;
		OnList(evt);
		type_from->SetFocus();
	} else {
		selected=-1;
		type_from->Enable(false);
		expression_to->Enable(false);
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
	config->Debug.inspection_improving_template_from=array_from;
	config->Debug.inspection_improving_template_to=array_to;
	EndModal(1);
}

void mxInspectionsImprovingEditor::OnCancel (wxCommandEvent & evt) {
	EndModal(0);
}

void mxInspectionsImprovingEditor::OnList (wxCommandEvent & evt) {
	selected=list->GetSelection();
	if (selected<0||selected>=int(list->GetCount())) return; // delete method can launch this event
	type_from->Enable();
	expression_to->Enable();
	type_from->SetValue(array_from[selected]);
	expression_to->SetValue(array_to[selected]);
}

