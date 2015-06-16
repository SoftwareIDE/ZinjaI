#ifndef MXGOTOLISTDIALOG_H
#define MXGOTOLISTDIALOG_H
#include <wx/dialog.h>
#include <wx/timer.h>
#include "Cpp11.h"

class wxListBox;
class wxTextCtrl;
class wxButton;
class wxCheckBox;
class wxStaticText;

class mxGotoListDialog : public wxDialog {
protected:
	wxStaticText *label_ctrl;
	wxTextCtrl *text_ctrl;
	wxListBox *list_ctrl;
	wxTimer *timer;
	wxButton *goto_button;
	wxButton *cancel_button;
	wxCheckBox *case_sensitive;
public:
	mxGotoListDialog(wxWindow* parent, wxString window_title, wxString input_label, wxString extra_button_label="");
	virtual void OnExtraButton() = 0;
	/// perform search, return -1 if nothing found, selection index else
	virtual int OnSearch(wxString key, bool case_sensitive) = 0;
	virtual void OnGoto(int pos, wxString key) = 0;
	void SetInputValue(wxString key);
	void HideInput();
	void GotoNow();
protected:
	void SearchNow();
	void SetExtraButtonAccelerator(int flags, char key);
	void OnGotoFileButton(wxCommandEvent &event);
	void OnGotoButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnTimerInput(wxTimerEvent &event);
	void OnTextChange(wxCommandEvent &event);
	void OnCharHook (wxKeyEvent &event);
	void OnCaseCheck(wxCommandEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

