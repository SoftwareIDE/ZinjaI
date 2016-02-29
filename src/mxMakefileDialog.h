#ifndef MX_MAKEFILE_DIALOG_H
#define MX_MAKEFILE_DIALOG_H

#include "mxCommonConfigControls.h"

class wxCheckBox;
class wxComboBox;
class wxTextCtrl;

class mxMakefileDialog : public mxDialog {
private:
	wxComboBox *configuration_name;
	wxComboBox *split_type;
	wxTextCtrl *file_path;
	wxTextCtrl *mingw_dir;
	wxCheckBox *expand_comas;
	wxCheckBox *cmake_style;
public:
	mxMakefileDialog(wxWindow* parent);
	void OnOkButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnExploreButton(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

