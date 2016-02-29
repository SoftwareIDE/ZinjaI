#ifndef MXVALGRINDCONFIGDIALOG_H
#define MXVALGRINDCONFIGDIALOG_H

#include "mxCommonConfigControls.h"

class wxComboBox;
class wxTextCtrl;

class mxValgrindConfigDialog : public mxDialog {
private:
	wxComboBox *cmb_tool;
	wxTextCtrl *suppressions;
	wxTextCtrl *additional_args;
public:
	mxValgrindConfigDialog(wxWindow *parent);
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonHelp(wxCommandEvent &evt);
	wxString GetArgs();
	void SetArg(const wxString &arg, bool present);
	DECLARE_EVENT_TABLE();
};

#endif

