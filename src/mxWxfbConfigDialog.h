#ifndef MXWXFBCONFIGDIALOG_H
#define MXWXFBCONFIGDIALOG_H
#include <wx/dialog.h>
#include "widgetDisabler.h"

class wxCheckBox;
	
class mxWxfbConfigDialog : public wxDialog {
private:
	wxCheckBox *m_activate_integration;
	wxCheckBox *m_autoupdate_project;
	wxCheckBox *m_update_class_list;
	wxCheckBox *m_update_methods;
	wxCheckBox *m_dont_show_base_classes_in_goto;
	wxCheckBox *m_set_wxfb_sources_as_readonly;
	widgetDisabler m_disabler;
public:
	mxWxfbConfigDialog();
	~mxWxfbConfigDialog();
	void OnClose(wxCloseEvent &event);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

