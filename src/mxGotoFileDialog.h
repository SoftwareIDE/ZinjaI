#ifndef MXGOTOFILEDIALOG_H
#define MXGOTOFILEDIALOG_H
#include <wx/dialog.h>
#include <wx/timer.h>

class wxListBox;
class wxTextCtrl;
class wxCheckBox;
class wxButton;

class mxGotoFileDialog: public wxDialog {
private:
	wxTextCtrl *text_ctrl;
	wxListBox *list_ctrl;
	wxCheckBox *case_sensitive;
	wxTimer *timer;
	wxButton *goto_button;
	wxButton *cancel_button;
public:
	mxGotoFileDialog(wxString text, wxWindow* parent);
	~mxGotoFileDialog();
	void OnGotoButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnTimerInput(wxTimerEvent &event);
	void OnTextChange(wxCommandEvent &event);
	void OnCaseCheck(wxCommandEvent &event);
	void OnCharHook (wxKeyEvent &event);
private:
	DECLARE_EVENT_TABLE();
};

#endif
