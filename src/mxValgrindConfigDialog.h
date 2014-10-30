#ifndef MXVALGRINDCONFIGDIALOG_H
#define MXVALGRINDCONFIGDIALOG_H
#include <wx/dialog.h>

class wxComboBox;
class wxTextCtrl;

class mxValgrindConfigDialog : public wxDialog {
private:
	wxComboBox *cmb_tool;
	wxTextCtrl *suppressions;
	wxTextCtrl *additional_args;
public:
	mxValgrindConfigDialog(wxWindow *parent);
	~mxValgrindConfigDialog();
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonHelp(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	wxString GetArgs();
	void SetArg(const wxString &arg, bool present);
	DECLARE_EVENT_TABLE();
};

extern mxValgrindConfigDialog *valgrind_config;

#endif

