#include <wx/control.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include "widgetDisabler.h"
#include "mxUtils.h"
#include "mxCommonConfigControls.h"
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>

widgetDisabler::widgetDisabler() {
	m_first = nullptr;
}

widgetDisabler::~widgetDisabler() {
	while (m_first) {
		node *next = m_first->next;
		delete m_first;
		m_first = next;
	}
}

void widgetDisabler::Add (wxControl * _control) {
	node *n = new node;
	n->next = m_first;
	n->control = _control;
	m_first = n;
}

void widgetDisabler::operator+=(wxControl * _control) {
	node *n = new node;
	n->next = m_first;
	n->control = _control;
	m_first = n;
}

void widgetDisabler::EnableAll (bool _enable) {
	node *n = m_first;
	while (n) {
		n->control->Enable(_enable);
		n = n->next;
	}
}

void widgetDisabler::DisableAll ( ) {
	node *n = m_first;
	while (n) {
		n->control->Disable();
		n=n->next;
	}
}

void widgetBinder::ToWidgets ( ) { Transfer(true); }

void widgetBinder::FromWidgets ( ) { Transfer(false); }

void widgetBinder::Transfer (bool to_widget) {
	{
		vector< BinderAux<wxTextCtrl,wxString> >::iterator it = m_text_string.begin(), end = m_text_string.end();
		if (to_widget) for(;it!=end;++it) { it->control->SetValue(*(it->value)); }
		else           for(;it!=end;++it) { *(it->value) = it->control->GetValue(); }
	}
	{
		vector< BinderAux<wxTextCtrl,int> >::iterator it = m_text_int.begin(), end = m_text_int.end();
		if (to_widget) for(;it!=end;++it) { it->control->SetValue(wxString()<<(*(it->value))); }
		else           for(;it!=end;++it) { long l; if (it->control->GetValue().ToLong(&l)) *(it->value) = l; }
	}
	{
		vector< BinderAux<wxComboBox,wxString> >::iterator it = m_combo_string.begin(), end = m_combo_string.end();
		if (to_widget) for(;it!=end;++it) { it->control->SetValue(*(it->value)); }
		else           for(;it!=end;++it) { *(it->value) = it->control->GetValue(); }
	}
	{
		vector< BinderAux<wxComboBox,int> >::iterator it = m_combo_int.begin(), end = m_combo_int.end();
		if (to_widget) for(;it!=end;++it) { it->control->SetSelection(*(it->value)); }
		else           for(;it!=end;++it) { *(it->value) = it->control->GetSelection(); }
	}
	{
		vector< BinderAux<wxCheckBox,bool> >::iterator it = m_check_bool.begin(), end = m_check_bool.end();
		if (to_widget) for(;it!=end;++it) { it->control->SetValue(*(it->value)); }
		else           for(;it!=end;++it) { *(it->value) = it->control->GetValue(); }
	}
}

