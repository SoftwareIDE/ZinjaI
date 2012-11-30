#ifndef MXMESSAGEDIALOG_H
#define MXMESSAGEDIALOG_H

#define mxMD_ERROR 1
#define mxMD_QUESTION 2
#define mxMD_WARNING 4
#define mxMD_INFO 8
#define mxMD_OK 16
#define mxMD_CANCEL 32
#define mxMD_YES 64
#define mxMD_NO 128
#define mxMD_CHECKED 256
#define mxMD_OK_CANCEL mxMD_OK|mxMD_CANCEL
#define mxMD_YES_NO mxMD_YES|mxMD_NO
#define mxMD_YES_NO_CANCEL mxMD_YES|mxMD_NO|mxMD_CANCEL

#include <wx/dialog.h>

class wxCheckBox;

class mxMessageDialog : public wxDialog {
private:
	wxCheckBox *checkbox;
	int buttons;
public:
	mxMessageDialog(wxWindow *parent, wxString message, wxString title=_T("ZinjaI"), unsigned int style=mxMD_OK, wxString check=_T(""), bool bval=false);
	void OnCancelButton(wxCommandEvent &event);
	void OnOkButton(wxCommandEvent &event);
	void OnYesButton(wxCommandEvent &event);
	void OnNoButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnCharHook(wxKeyEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

