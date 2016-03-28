#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/valtext.h>
#include <wx/bmpbuttn.h>
#include <wx/notebook.h>
#include "mxCommonConfigControls.h"
#include "mxSizers.h"
#include "widgetDisabler.h"
#include "mxBitmapButton.h"
#include "Language.h"

static wxStaticText *s_last_label; ///< guarda la ultima etiqueta que se uso en alguno de los AddAlgo
static wxButton *s_last_button; ///< guarda el ultimo boton colocado por AddDirCtrl
static wxBoxSizer *s_last_sizer; ///< guarda el último sizer creado por los AddAlgo

template<typename TSizer, typename TControl> template<typename wxCtrl_t, typename value_t> 
void mxDialog::MainSizer::BaseControl<TSizer,TControl>::DoBind(wxCtrl_t *ctrl) {
	if (m_binder) m_binder->Add(ctrl,*(reinterpret_cast<value_t*>(m_bind_value)));
}


wxCheckBox *mxDialog::AddCheckBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool value, wxWindowID id, bool margin) {
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

wxTextCtrl *mxDialog::AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, int id) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(s_last_label=new wxStaticText(panel,wxID_ANY,text+_T(":   "), wxDefaultPosition, wxDefaultSize, 0),sizers->BLRT5_Exp0);
	sizer->Add(textctrl, sizers->BLRB5_Exp0);
	textctrl->SetValue(value);
	s_last_button=nullptr;
	return textctrl;
}

wxTextCtrl *mxDialog::AddDirCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString value, wxWindowID id, wxString button_text, bool margin) {
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

wxTextCtrl *mxDialog::AddTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int value, bool margin, int id) {
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

wxStaticText* mxDialog::AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString str, bool margin) {
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

wxTextCtrl *mxDialog::AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxString str, bool margin, wxWindowID id) {
	wxTextCtrl *textctrl = new wxTextCtrl(panel, id, str, wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *sizerRow = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
	if (margin) sizerRow->AddSpacer(10);
	sizerRow->Add(s_last_label=new wxStaticText(panel, wxID_ANY, text+_T(": "), wxDefaultPosition, wxDefaultSize, 0), sizers->Center);
	sizerRow->Add(textctrl, sizers->Exp1);
	sizer->Add(sizerRow,sizers->BA5_Exp0);
	s_last_button=nullptr;
	return textctrl;
}

wxTextCtrl *mxDialog::AddShortTextCtrl (wxBoxSizer *sizer, wxWindow *panel, wxString text, int n, wxString foot, bool margin) {
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

wxComboBox *mxDialog::AddComboBox (wxBoxSizer *sizer, wxWindow *panel, wxString text, wxArrayString &values, int def, wxWindowID id, bool margin, bool editable) {
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

wxStaticText *mxDialog::AddStaticText (wxBoxSizer *sizer, wxWindow *panel, wxString text, bool center) {
	wxStaticText *statictext;
	sizer->Add( statictext = new wxStaticText(panel, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, center?wxALIGN_CENTRE:0), center?sizers->BA5_Center:sizers->BA5);
	return statictext;
}


template<typename T1, typename T2>
void mxDialog::MainSizer::BaseControl<T1,T2>::RegisterInDisablers(wxControl *ctrl1, wxControl *ctrl2, wxControl *ctrl3) {
	if (m_disabler_1) {
		if (ctrl1) m_disabler_1->Add(ctrl1);
		if (ctrl2) m_disabler_1->Add(ctrl2);
		if (ctrl3) m_disabler_1->Add(ctrl3);
	}
	if (m_disabler_2) {
		if (ctrl1) m_disabler_2->Add(ctrl1);
		if (ctrl2) m_disabler_2->Add(ctrl2);
		if (ctrl3) m_disabler_2->Add(ctrl3);
	}
}

mxDialog::MainSizer &mxDialog::MainSizer::MainText::EndText(wxTextCtrl *&text_ctrl) {
	wxWindow *parent = m_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_sizer; wxButton *button = nullptr;
	if (!m_is_numeric) text_ctrl = new wxTextCtrl(parent, m_id, m_value, wxDefaultPosition, wxDefaultSize, (m_multiline?wxTE_MULTILINE:0)|(m_readonly?wxTE_READONLY:0));
	else text_ctrl = new wxTextCtrl(parent, m_id, m_value, wxDefaultPosition, wxDefaultSize,  wxTE_RIGHT|(m_readonly?wxTE_READONLY:0), wxTextValidator(wxFILTER_NUMERIC));
	wxStaticText *static_text = new wxStaticText(parent,wxID_ANY,m_label+":   ", wxDefaultPosition, wxDefaultSize, 0);
	if (!m_one_line) sizer->Add(static_text,sizers->BLRT5_Exp0);
	if (m_button_id!=mxID_NULL || m_one_line) {
		wxBoxSizer *inner_sizer = new wxBoxSizer(wxHORIZONTAL);
		if (m_one_line) inner_sizer->Add(static_text,sizers->Center);
		inner_sizer->Add(text_ctrl, sizers->Exp1);
		if (m_button_id!=mxID_NULL) {
			button = new wxButton(parent,m_button_id,m_button_text,wxDefaultPosition,m_button_text== _T("...")?wxSize(30,10):wxSize(-1,10));
			inner_sizer->Add(button, sizers->Exp0_Right);
		}
		sizer->Add(inner_sizer, m_one_line?sizers->BA5_Exp0:sizers->BLRB5_Exp0);
	} else {
		sizer->Add(text_ctrl, m_multiline?sizers->BLRB5_Exp1:sizers->BLRB5_Exp0);
	}
	RegisterInDisablers(static_text,text_ctrl,button);
	if (m_is_numeric) DoBind<wxTextCtrl,int>(text_ctrl);
	else              DoBind<wxTextCtrl,wxString>(text_ctrl);
	return *m_sizer;
}

mxDialog::MainSizer::InnerSizer &mxDialog::MainSizer::InnerSizer::InnerText::EndText(wxTextCtrl *&text_ctrl) {
	wxWindow *parent = m_sizer->m_outher_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_inner_sizer; wxButton *button = nullptr;
	if (!m_is_numeric) text_ctrl = new wxTextCtrl(parent, m_id, m_value, wxDefaultPosition, wxDefaultSize, (m_readonly?wxTE_READONLY:0));
	else text_ctrl = new wxTextCtrl(parent, m_id, m_value, wxDefaultPosition, wxDefaultSize,  wxTE_RIGHT|(m_readonly?wxTE_READONLY:0), wxTextValidator(wxFILTER_NUMERIC));
	wxStaticText *static_text = new wxStaticText(parent,wxID_ANY,m_label+":   ", wxDefaultPosition, wxDefaultSize, 0);
	sizer->Add(static_text,sizers->Center);
	sizer->Add(text_ctrl, sizers->Exp0);
	if (m_button_id!=mxID_NULL) {
		button = new wxButton(parent,m_button_id,m_button_text,wxDefaultPosition,m_button_text== _T("...")?wxSize(30,10):wxSize(-1,10));
		sizer->Add(button, sizers->Exp0_Right);
	}
	RegisterInDisablers(static_text,text_ctrl,button);
	if (m_is_numeric) DoBind<wxTextCtrl,int>(text_ctrl);
	else              DoBind<wxTextCtrl,wxString>(text_ctrl);
	return *m_sizer;
}

template<class TSizer>
void mxDialog::MainSizer::BaseCombo<TSizer>::FixValueAndSelection() {
	if (m_selection == -2) return;              // m_value already has the appropiate value
	else if (m_items.GetCount()==0)              { m_value = ""; m_selection = -1; }
	else if (m_selection==-1)                      m_value = m_items[m_selection=0];
	else if (m_selection<int(m_items.GetCount()))  m_value = m_items[m_selection];
	else if (m_selection>=int(m_items.GetCount())) m_value = m_items[m_selection=m_items.GetCount()-1];
}

mxDialog::MainSizer &mxDialog::MainSizer::MainCombo::EndCombo(wxComboBox *&combo_box) {
	wxWindow *parent = m_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_sizer;
	wxBoxSizer *innter_sizer = s_last_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *static_text = new wxStaticText(parent, wxID_ANY, m_label+_T(": "), wxDefaultPosition, wxDefaultSize, 0);
	FixValueAndSelection();
	combo_box = new wxComboBox(parent, m_id, m_value, wxDefaultPosition, 
							   wxDefaultSize, m_items, m_editable?0:wxCB_READONLY);
	if (m_selection>=0) combo_box->SetSelection(m_selection);
	innter_sizer->Add(static_text, sizers->Center);
	innter_sizer->Add(combo_box, sizers->Exp1);
	sizer->Add(innter_sizer, sizers->BA5_Exp0);
	RegisterInDisablers(static_text,combo_box);
	if (m_bind_by_pos) DoBind<wxComboBox,int>(combo_box);
	else               DoBind<wxComboBox,wxString>(combo_box);
	return *m_sizer;
}

void mxDialog::BaseSizer::SetAndFit() { 
	m_parent->SetSizerAndFit(m_sizer); 
}

void mxDialog::BaseSizer::Set() { 
	m_parent->SetSizer(m_sizer); 
}

mxDialog::MainSizer &mxDialog::MainSizer::InnerSizer::EndLine() {
	m_outher_sizer->m_sizer->Add(m_inner_sizer, sizers->BA5_Exp0);
	return *m_outher_sizer;
}

mxDialog::MainSizer::InnerSizer &mxDialog::MainSizer::InnerSizer::InnerLabel::EndLabel() {
	wxWindow *parent = m_sizer->m_outher_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_inner_sizer;
	wxStaticText *static_text = new wxStaticText(parent,m_id,m_label);
	sizer->Add(static_text,sizers->Center); 
	RegisterInDisablers(static_text);
	return *m_sizer;
}

mxDialog::MainSizer::InnerSizer &mxDialog::MainSizer::InnerSizer::InnerCombo::EndCombo(wxComboBox *&combo_box) {
	wxWindow *parent = m_sizer->m_outher_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_inner_sizer;
	wxStaticText *static_text = m_label.IsEmpty() ? nullptr : new wxStaticText(parent,wxID_ANY,m_label+": ");
	if (static_text) sizer->Add(static_text,sizers->Center);
	FixValueAndSelection();
	combo_box = new wxComboBox(parent, m_id, m_value,
								wxDefaultPosition, wxDefaultSize, m_items, m_editable?0:wxCB_READONLY);
	if (m_selection>=0) combo_box->SetSelection(m_selection);
	combo_box->SetMinSize(wxSize(50,-1));
	sizer->Add(combo_box,sizers->Exp1);
	RegisterInDisablers(static_text, combo_box);
	if (m_bind_by_pos) DoBind<wxComboBox,int>(combo_box);
	else               DoBind<wxComboBox,wxString>(combo_box);
	return *m_sizer;
}

mxDialog::MainSizer::InnerSizer &mxDialog::MainSizer::InnerSizer::InnerCheck::EndCheck(wxCheckBox *&check_box) {
	wxWindow *parent = m_sizer->m_outher_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_inner_sizer;
	check_box = new wxCheckBox(parent,m_id,m_label); 
	check_box->SetValue(m_value);
	sizer->Add(check_box, sizers->Center);
	RegisterInDisablers(check_box);
	DoBind<wxCheckBox,bool>(check_box);
	return *m_sizer;
}

mxDialog::MainSizer &mxDialog::MainSizer::MainCheck::EndCheck(wxCheckBox *&check_box) {
	wxWindow *parent = m_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_sizer;
	check_box = new wxCheckBox(parent, m_id, m_label);
	sizer->Add(check_box, sizers->BA5_Exp0);
	check_box->SetValue(m_value);
	RegisterInDisablers(check_box);
	DoBind<wxCheckBox,bool>(check_box);
	return *m_sizer;
}

mxDialog::MainSizer &mxDialog::MainSizer::MainLabel::EndLabel() {
	wxWindow *parent = m_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_sizer;
	wxStaticText *static_text = new wxStaticText(parent, m_id, m_label, wxDefaultPosition, wxDefaultSize, m_center?wxALIGN_CENTRE:0);
	sizer->Add( static_text, m_center?sizers->BA5_Center:sizers->BA5); 
	RegisterInDisablers(static_text);
	return *m_sizer;
}

mxDialog::MainSizer &mxDialog::MainSizer::MainButton::EndButton() {
	wxWindow *parent = m_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_sizer;
	sizer->Add(new wxButton(parent,m_id,m_label,wxDefaultPosition), m_expand?sizers->BA10_Exp0:sizers->BA10);
	return *m_sizer;
}

mxDialog::MainSizer::InnerSizer &mxDialog::MainSizer::InnerSizer::InnerButton::EndButton() {
	wxWindow *parent = m_sizer->m_outher_sizer->m_parent; wxBoxSizer *sizer = m_sizer->m_inner_sizer;
	sizer->Add(new wxButton(parent,m_id,m_label,wxDefaultPosition,m_label=="..."?wxSize(30,10):wxDefaultSize), sizers->Exp0);
	return *m_sizer;
}

mxDialog::MainSizer &mxDialog::MainSizer::BottomSizer::EndBottom(wxDialog *dialog) {
	wxBoxSizer *bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	if (m_help_id!=mxID_NULL) {
		wxBitmapButton *help_button = new wxBitmapButton(m_sizer->m_parent,m_help_id,*bitmaps->buttons.help);
		bottom_sizer->Add(help_button,sizers->BA5_Exp0);
	}
	bottom_sizer->AddStretchSpacer();
	if (m_cancel_id!=mxID_NULL) {
		wxButton *cancel_button = new mxBitmapButton(m_sizer->m_parent,m_cancel_id,bitmaps->buttons.cancel,LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
		if (dialog) dialog->SetEscapeId(wxID_CANCEL);
		bottom_sizer->Add(cancel_button,sizers->BA5);
	}
	if (m_extra_id!=mxID_NULL) {
		wxButton *extra_button = new mxBitmapButton(m_sizer->m_parent,m_extra_id,m_extra_icon,m_extra_label);
		bottom_sizer->Add(extra_button,sizers->BA5);
	}
	if (m_ok_id!=mxID_NULL) {
		wxButton *ok_button = new mxBitmapButton(m_sizer->m_parent,m_ok_id,bitmaps->buttons.ok,LANG(GENERAL_OK_BUTTON,"&Aceptar"));
		ok_button->SetDefault(); 
		bottom_sizer->Add(ok_button,sizers->BA5);
	}
	if (m_close_id!=mxID_NULL) {
		wxButton *close_button = new mxBitmapButton(m_sizer->m_parent,m_close_id,bitmaps->buttons.ok,LANG(GENERAL_CLOSE_BUTTON,"&Cerrar"));
		close_button->SetDefault(); if (dialog) dialog->SetEscapeId(m_close_id);
		bottom_sizer->Add(close_button,sizers->BA5);
	}
	m_sizer->m_sizer->Add(bottom_sizer,sizers->Exp0);
	return *m_sizer;
}

mxDialog::MainSizer::MainNotebook mxDialog::MainSizer::BeginNotebook() {
	return MainNotebook(this,new wxNotebook(m_parent,wxID_ANY));
}

wxNotebook *mxDialog::BaseNotebookAux::EndNotebook() {
	if (m_selection!=-1) m_notebook->SetSelection(m_selection);
	return m_notebook;
}

mxDialog::MainSizer &mxDialog::MainSizer::MainNotebook::EndNotebook() {
	BaseNotebook<MainNotebook>::EndNotebook();
	m_sizer->m_sizer->Add(m_notebook,sizers->Exp1);
	return *m_sizer;
}

void mxDialog::BaseNotebookAux::AddPageHelper(wxString name, wxWindow *page) {
	m_notebook->AddPage(page,name);
}

mxDialog::MainSizer &mxDialog::MainSizer::BeginSection(wxString label) {
	m_real_sizer = m_sizer;
	m_sizer = new wxStaticBoxSizer(wxVERTICAL,m_parent,label);
	m_real_sizer->Add(m_sizer,sizers->BA5_Exp0);
	return *this;
}

mxDialog::MainSizer &mxDialog::MainSizer::BeginSection(int margin) {
	wxBoxSizer *sizer_horiz = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sizer_vert = new wxBoxSizer(wxVERTICAL);
	sizer_horiz->AddSpacer(margin);
	sizer_horiz->Add(sizer_vert,sizers->Exp1);
	m_sizer->Add(sizer_horiz,sizers->Exp0);
	m_real_sizer = m_sizer; m_sizer = sizer_vert;
	return *this;
}

mxDialog::HorizontalSizer &mxDialog::HorizontalSizer::Add(MainSizer &sizer, int proportion) {
	m_sizer->Add(sizer.GetSizer(),wxSizerFlags().Proportion(proportion).Expand());
	return *this;
}

mxDialog::ReuseSizer::ReuseSizer(wxWindow *parent, wxBoxSizer *sizer) : mxDialog::MainSizer(parent,sizer) {}
mxDialog::CreateSizer::CreateSizer(wxWindow *parent) : mxDialog::MainSizer(parent) {}
mxDialog::CreatePanelAndSizer::CreatePanelAndSizer(wxWindow *parent) : mxDialog::MainSizer(new wxPanel(parent)) {}
mxDialog::FreeNotebook mxDialog::CreateNotebook(wxWindow *parent) { return FreeNotebook(new wxNotebook(parent,wxID_ANY)); }
mxDialog::CreateHorizontalSizer::CreateHorizontalSizer(wxWindow *parent) : mxDialog::HorizontalSizer(parent,new wxBoxSizer(wxHORIZONTAL)) {}


	
