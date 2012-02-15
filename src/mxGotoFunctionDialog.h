#ifndef MX_GOTO_FUNCTION_DIALOG_H
#define MX_GOTO_FUNCTION_DIALOG_H
#include <wx/dialog.h>
#include <wx/timer.h>

class wxListBox;
class wxTextCtrl;
class wxButton;
class wxCheckBox;

class mxGotoFunctionDialog: public wxDialog {
private:
	wxTextCtrl *text_ctrl;
	wxListBox *list_ctrl;
	wxTimer *timer;
	wxButton *goto_button;
	wxButton *cancel_button;
	wxCheckBox *case_sensitive;
	bool strict_compare;
public:
	mxGotoFunctionDialog(wxString text, wxWindow* parent, bool direct_goto=false);
	~mxGotoFunctionDialog();
	void OnGotoButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnTimerInput(wxTimerEvent &event);
	void OnTextChange(wxCommandEvent &event);
	void OnCharHook (wxKeyEvent &event);
	void OnCaseCheck(wxCommandEvent &event);
private:
	DECLARE_EVENT_TABLE();
};

#endif

