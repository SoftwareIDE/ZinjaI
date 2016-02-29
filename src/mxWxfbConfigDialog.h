#ifndef MXWXFBCONFIGDIALOG_H
#define MXWXFBCONFIGDIALOG_H

#include "widgetDisabler.h"
#include "mxCommonConfigControls.h"

class wxCheckBox;
class wxfb_configuration;
	
class mxWxfbConfigDialog : public mxDialog {
private:
	wxfb_configuration *conf;
	wxCheckBox *m_activate_integration;
	wxCheckBox *m_autoupdate_projects;
	wxCheckBox *m_update_class_list;
	wxCheckBox *m_update_methods;
	wxCheckBox *m_dont_show_base_classes_in_goto;
	wxCheckBox *m_set_wxfb_sources_as_readonly;
	widgetDisabler m_disabler;
public:
	mxWxfbConfigDialog();
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnActivateClick(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

