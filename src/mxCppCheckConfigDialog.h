#ifndef MXCPPCHECKCONFIGDIALOG_H
#define MXCPPCHECKCONFIGDIALOG_H

#include "mxCommonConfigControls.h"

class wxListBox;
class wxPanel;
class wxCheckBox;
class wxTextCtrl;
class wxNotebook;
class cppcheck_configuration;

class mxCppCheckConfigDialog : public mxDialog {
private:
	wxListBox *sources_in, *sources_out;
	wxCheckBox *copy_from_config;
	wxTextCtrl *config_d;
	wxTextCtrl *config_u;
	wxTextCtrl *style;
	wxTextCtrl *platform;
	wxTextCtrl *standard;
	wxTextCtrl *suppress_file;
	wxTextCtrl *suppress_ids;
	wxCheckBox *inline_suppr;
	wxCheckBox *save_in_project;
	wxCheckBox *exclude_headers;
	wxTextCtrl *additional_files;
	cppcheck_configuration *ccc;
protected:
public:
	mxCppCheckConfigDialog(wxWindow *parent);
	wxPanel *CreateGeneralPanel(wxNotebook *notebook);
	wxPanel *CreateFilesPanel(wxNotebook *notebook);
	
	void OnCheckCopyConfig(wxCommandEvent &evt);
	void OnButtonConfigD(wxCommandEvent &evt);
	void OnButtonConfigU(wxCommandEvent &evt);
	void OnButtonStyle(wxCommandEvent &evt);
	void OnButtonPlatform(wxCommandEvent &evt);
	void OnButtonStandard(wxCommandEvent &evt);
	void OnButtonSuppressFile(wxCommandEvent &evt);
	void OnButtonSuppressIds(wxCommandEvent &evt);
	void OnButtonIncludeFile(wxCommandEvent &evt);
	void OnButtonExcludeFile(wxCommandEvent &evt);
	void OnButtonAdditionalFiles(wxCommandEvent &evt);
	void OnButtonExcludeHeaders(wxCommandEvent &evt);
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonHelp(wxCommandEvent &evt);

	DECLARE_EVENT_TABLE();
};

#endif

