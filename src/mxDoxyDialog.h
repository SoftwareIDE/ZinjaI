#ifndef MXDOXYDIALOG_H
#define MXDOXYDIALOG_H

#include "mxCommonConfigControls.h"

class wxPanel;
class wxTextCtrl;
class wxCheckBox;
class wxComboBox;
class wxNotebook;
class doxygen_configuration;

class mxDoxyDialog : public mxDialog {
private:
	doxygen_configuration *m_doxygen_config;
	wxTextCtrl *name_ctrl, *version_ctrl, *destdir_ctrl, *extra_files_ctrl, *exclude_files_ctrl, *base_path_ctrl;
	wxCheckBox *html_ctrl, *html_searchengine_ctrl, *use_in_quickhelp_ctrl, *html_navtree_ctrl, *latex_ctrl, *save_ctrl, *hideundocs_ctrl, *preprocess_ctrl, *static_ctrl, *private_ctrl;
	wxComboBox *base_files_ctrl, *lang_ctrl;
	wxTextCtrl *extra_conf;
public:
	mxDoxyDialog(wxWindow *parent);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnDestDirButton(wxCommandEvent &evt);
	void OnBaseDirButton(wxCommandEvent &evt);
	wxPanel *CreateGeneralPanel(wxNotebook *notebook);
	wxPanel *CreateMorePanel (wxNotebook *notebook);
	wxPanel *CreateExtraPanel (wxNotebook *notebook);
	DECLARE_EVENT_TABLE();
};

#endif

