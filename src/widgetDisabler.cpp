#include <wx/control.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include "widgetDisabler.h"
#include "mxUtils.h"
#include "mxCommonConfigControls.h"

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


