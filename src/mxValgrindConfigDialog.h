#ifndef MXVALGRINDCONFIGDIALOG_H
#define MXVALGRINDCONFIGDIALOG_H
#include <wx/dialog.h>

class wxComboBox;

class mxValgrindConfigDialog : public wxDialog {
private:
	wxComboBox *cmb_tool;
public:
	mxValgrindConfigDialog(wxWindow *parent);
	~mxValgrindConfigDialog();
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonHelp(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	wxString GetArgs();
	DECLARE_EVENT_TABLE();
};

extern mxValgrindConfigDialog *valgrind_config;

#endif

