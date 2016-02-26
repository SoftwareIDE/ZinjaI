#include "mxCommonConfigControls.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include "mxSizers.h"
#include <wx/valtext.h>
#include "widgetDisabler.h"

static wxStaticText *s_last_label; ///< guarda la ultima etiqueta que se uso en alguno de los AddAlgo
wxStaticText * mxCCC::GetLastLabel ( ) { return s_last_label; }

static wxButton *s_last_button; ///< guarda el ultimo boton colocado por AddDirCtrl
wxButton * mxCCC::GetLastButton ( ) { return s_last_button; }

static wxBoxSizer *s_last_sizer; ///< guarda el último sizer creado por los AddAlgo
wxSizer * mxCCC::GetLastSizer ( ) { return s_last_sizer; }


wxCheckBox *mxCCC::AddCheckBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool value, wxWindowID id, bool margin) {
	wxCheckBox *checkbox = new wxCheckBox(panel, id, text+_T("   "));
	if (margin) {
		wxBoxSizer *sizerRow = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
		sizerRow->AddSpacer(15);
		sizerRow->Add(checkbox, sizers->Exp1);
		sizer->Add(sizerRow, sizers->BLRT5_Exp0);
	} else 
		sizer->Add(checkbox,sizers->BA5_Exp0);
	checkbox->SetValue(value);
	s_last_button=nullptr;
	return checkbox;	
}

wxTextCtrl *mxCCC::AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, int id) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(s_last_label=new wxStaticText(panel,wxID_ANY,text+_T(":   "), wxDefaultPosition, wxDefaultSize, 0),sizers->BLRT5_Exp0);
	sizer->Add(textctrl, sizers->BLRB5_Exp0);
	textctrl->SetValue(value);
	s_last_button=nullptr;
	return textctrl;
}

wxTextCtrl *mxCCC::AddLongTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	sizer->Add(s_last_label=new wxStaticText(panel,wxID_ANY,text+_T(":   "), wxDefaultPosition, wxDefaultSize, 0),sizers->BLRT5_Exp0);
	sizer->Add(textctrl, sizers->BLRB5_Exp1);
	textctrl->SetValue(value);
	s_last_button=nullptr;
	return textctrl;
}

wxTextCtrl *mxCCC::AddDirCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, wxWindowID id, wxString button_text, bool margin) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	wxBoxSizer *sizerRow = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *button = s_last_button = new wxButton(panel,id,button_text,wxDefaultPosition,button_text== _T("...")?wxSize(30,10):wxSize(-1,10));
	if (margin) sizerRow->AddSpacer(15);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizerRow->Add(button, sizers->Exp0_Right);
	sizer->Add(s_last_label=new wxStaticText(panel,wxID_ANY,text+_T(":   "), wxDefaultPosition, wxDefaultSize, 0),sizers->BLRT5_Exp0);
	sizer->Add(sizerRow, sizers->BLRB5_Exp0);
	textctrl->SetValue(value);
	return textctrl;
}

wxTextCtrl *mxCCC::AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int value, bool margin, int id) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxTextValidator(wxFILTER_NUMERIC));
	wxBoxSizer *sizerRow = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(15);
	if (!text.IsEmpty())
		sizerRow->Add(s_last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	textctrl->SetValue(wxString::Format(_T("%d"), value));
	s_last_button=nullptr;
	return textctrl;
}

wxStaticText* mxCCC::AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString str, bool margin) {
	wxStaticText *textctrl = new wxStaticText(panel, wxID_ANY, str, wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *sizerRow = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizerRow->AddStretchSpacer(1);
	sizerRow->Add(textctrl, sizers->Exp0);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	s_last_button=nullptr;
	return textctrl;	
}

wxTextCtrl *mxCCC::AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString str, bool margin, wxWindowID id) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, id, str, wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *sizerRow = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(s_last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	s_last_button=nullptr;
	return textctrl;
}

wxTextCtrl *mxCCC::AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int n, wxString foot, bool margin) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, wxID_ANY, wxString()<<n, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxTextValidator(wxFILTER_NUMERIC));
	wxBoxSizer *sizerRow = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(s_last_label=new wxStaticText(panel, wxID_ANY, text+" ", wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizerRow->Add(new wxStaticText(panel, wxID_ANY, wxString(" ")<<foot, wxDefaultPosition, wxDefaultSize, 0), sizers->Right);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	s_last_button=nullptr;
	return textctrl;
}

wxComboBox *mxCCC::AddComboBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxArrayString &values, int def, wxWindowID id, bool margin, bool editable) {
	wxString sdef=def==-1?"":values[def];
	wxComboBox *combo = new wxComboBox(panel, id, sdef, wxDefaultPosition, wxDefaultSize, values, editable?0:wxCB_READONLY);
	combo->SetSelection(def);
	wxBoxSizer *sizerRow = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(s_last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(combo, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	s_last_button=nullptr;
	return combo;
}

wxStaticText *mxCCC::AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool center) {
	wxStaticText *statictext;
	sizer->Add( statictext = new wxStaticText(panel, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, center?wxALIGN_CENTRE:0), center?sizers->BA5_Center:sizers->BA5);
	return statictext;
}


template<typename T1, typename T2>
void mxCCC::MainSizer::BaseControl<T1,T2>::RegisterInDisablers(wxControl *ctrl1, wxControl *ctrl2, wxControl *ctrl3) {
	if (m_disabler_1) {
		if (ctrl1) m_disabler_1->Add(ctrl1,false);
		if (ctrl2) m_disabler_1->Add(ctrl2,false);
		if (ctrl3) m_disabler_1->Add(ctrl3,false);
	}
	if (m_disabler_2) {
		if (ctrl1) m_disabler_2->Add(ctrl1,false);
		if (ctrl2) m_disabler_2->Add(ctrl2,false);
		if (ctrl3) m_disabler_2->Add(ctrl3,false);
	}
}

mxCCC::MainSizer &mxCCC::MainSizer::MainText::EndText(wxTextCtrl *&text_ctrl) {
	wxWindow *parent = m_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_sizer; wxButton *button = nullptr;
	text_ctrl = new wxTextCtrl(parent, m_id, m_value, wxDefaultPosition, wxDefaultSize, 0);
	wxStaticText *static_text = new wxStaticText(parent,wxID_ANY,m_label+":   ", wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(static_text,sizers->BLRT5_Exp0);
	if (m_button_id!=mxID_NULL) {
		wxBoxSizer *inner_sizer = new wxBoxSizer(wxHORIZONTAL);
		inner_sizer->Add(text_ctrl, sizers->Exp1);
		button = new wxButton(parent,m_button_id,m_button_text,wxDefaultPosition,m_button_text== _T("...")?wxSize(30,10):wxSize(-1,10));
		inner_sizer->Add(button, sizers->Exp0_Right);
		sizer->Add(inner_sizer, sizers->BLRB5_Exp0);
	} else {
		sizer->Add(text_ctrl, sizers->BLRB5_Exp0);
	}
	RegisterInDisablers(static_text,text_ctrl,button);
	return *m_sizer;
}

mxCCC::MainSizer &mxCCC::MainSizer::MainCombo::EndCombo(wxComboBox *&combo_box) {
	wxWindow *parent = m_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_sizer;
	wxBoxSizer *innter_sizer = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *static_text = new wxStaticText(parent, wxID_ANY, m_label+_T(": "), wxDefaultPosition, wxDefaultSize, 0);
	if (m_selection<0) m_selection=0; 
	else if (m_selection>=int(m_items.GetCount())) m_selection=m_items.GetCount()-1;
	combo_box = new wxComboBox(parent, m_id, m_items.GetCount()?m_items[m_selection]:"", 
								wxDefaultPosition, wxDefaultSize, m_items, m_editable?0:wxCB_READONLY);
	if (m_items.GetCount()) combo_box->SetSelection(m_selection);
	innter_sizer->Add(static_text, sizers->Center);
	innter_sizer->Add(combo_box, sizers->Exp1);
	sizer->Add(innter_sizer, sizers->BA5_Exp0);
	RegisterInDisablers(static_text,combo_box);
	return *m_sizer;
}

void mxCCC::MainSizer::Done() { 
	if (m_set_and_fit) m_parent->SetSizerAndFit(m_sizer); 
}

mxCCC::MainSizer &mxCCC::MainSizer::InnerSizer::EndInnerSizer() {
	m_outher_sizer->m_sizer->Add(m_inner_sizer, sizers->BA5_Exp0);
	return *m_outher_sizer;
}

mxCCC::MainSizer::InnerSizer &mxCCC::MainSizer::InnerSizer::InnerLabel::EndLabel() {
	wxWindow *parent = m_sizer->m_outher_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_inner_sizer;
	wxStaticText *static_text = new wxStaticText(parent,m_id,m_label);
	sizer->Add(static_text,sizers->Center); RegisterInDisablers(static_text);
	return *m_sizer;
}

mxCCC::MainSizer::InnerSizer &mxCCC::MainSizer::InnerSizer::InnerCombo::EndCombo(wxComboBox *&combo_box) {
	wxWindow *parent = m_sizer->m_outher_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_inner_sizer;
	wxStaticText *static_text = m_label.IsEmpty() ? nullptr : new wxStaticText(parent,wxID_ANY,m_label+": ");
	if (static_text) sizer->Add(static_text,sizers->Center);
	if (m_selection<0) m_selection=0; 
	else if (m_selection>=int(m_items.GetCount())) m_selection=m_items.GetCount()-1;
	combo_box = new wxComboBox(parent, m_id, m_items.GetCount()?m_items[m_selection]:"",
								wxDefaultPosition, wxDefaultSize, m_items, m_editable?0:wxCB_READONLY);
	if (m_items.GetCount()) combo_box->SetSelection(m_selection);
	combo_box->SetMinSize(wxSize(50,-1));
	sizer->Add(combo_box,sizers->Exp1);
	RegisterInDisablers(static_text, combo_box);
	return *m_sizer;
}	

mxCCC::MainSizer::InnerSizer &mxCCC::MainSizer::InnerSizer::InnerCheck::EndCheck(wxCheckBox *&check_box) {
	wxWindow *parent = m_sizer->m_outher_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_inner_sizer;
	check_box = new wxCheckBox(parent,m_id,m_label); 
	check_box->SetValue(m_value);
	sizer->Add(check_box, sizers->Center);
	RegisterInDisablers(check_box);
	return *m_sizer;
}
