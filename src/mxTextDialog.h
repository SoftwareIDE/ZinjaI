#ifndef MXTEXTDIALOG_H
#define MXTEXTDIALOG_H
#include <wx/dialog.h>
#include <wx/textctrl.h>

class mxTextDialog : public wxDialog {
private:
	wxString answer;
	wxTextCtrl *text;
public:
	mxTextDialog(wxWindow *parent, wxString title, wxString message, wxString value);
	wxString GetAnswer();
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnCharHook(wxKeyEvent &event);
	DECLARE_EVENT_TABLE();
};

wxString mxGetTextFromUser(wxString message, wxString title, wxString value, wxWindow *parent);
bool mxGetTextFromUser(wxString &ans, wxString message, wxString title, wxString value, wxWindow *parent);

#endif

