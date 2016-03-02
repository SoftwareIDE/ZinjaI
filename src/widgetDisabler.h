#ifndef WIDGETDISABLER_H
#define WIDGETDISABLER_H

#include <vector>
#include <wx/string.h>
#include "widgetDisabler.h"
#include "Cpp11.h"
class wxControl;
class wxTextCtrl;
class wxComboBox;
class wxCheckBox;

/**
* This class stores a list of control that can be enabled/disabled
* easily all at once, for using in configuration dialogs when lot
* of items depends on some other item.
*
* used in mxProjectConfigWindow
**/
class widgetDisabler {
private:
	struct node {
		wxControl *control;
		node *next;
	} *m_first;
public:
	widgetDisabler();
	void Add(wxControl *_control);
	void operator+=(wxControl *_control);
	void EnableAll(bool _enable=true);
	void DisableAll();
	~widgetDisabler();
};

class widgetBinder {
	template<typename wxCtrl_t, typename data_t>
	struct BinderAux { 
		wxCtrl_t *control; data_t *value; 
		BinderAux(wxCtrl_t *c, data_t *v) : control(c), value(v) {}
		BinderAux() : control(nullptr), value(nullptr) {}
	};
	std::vector< BinderAux<wxTextCtrl,wxString> > m_text_string;
	std::vector< BinderAux<wxTextCtrl,int> > m_text_int;
	std::vector< BinderAux<wxCheckBox,bool> > m_check_bool;
	std::vector< BinderAux<wxComboBox,int> > m_combo_int;
	std::vector< BinderAux<wxComboBox,wxString> > m_combo_string;
	template<typename Vec, typename Ctrl, typename Val>
	void Add(Vec &m, Ctrl *c, Val *v) { m.push_back(BinderAux<Ctrl,Val>(c,v)); }
	void Transfer(bool to_widget);
public:
	void Add(wxTextCtrl *control, wxString &value) { Add(m_text_string,control,&value); }
	void Add(wxTextCtrl *control, int &value) { Add(m_text_int,control,&value); }
	void Add(wxComboBox *control, int &value) { Add(m_combo_int,control,&value); }
	void Add(wxCheckBox *control, bool &value) { Add(m_check_bool,control,&value); }
	void Add(wxComboBox *control, wxString &value) { Add(m_combo_string,control,&value); }
	void ToWidgets();
	void FromWidgets();
};

#endif

