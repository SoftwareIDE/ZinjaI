#ifndef MXSHORTCUTSDIALOG_H
#define MXSHORTCUTSDIALOG_H
#include <wx/dialog.h>
#include "SingleList.h"
#include <wx/timer.h>

class wxStaticText;
class wxTextCtrl;
class wxSizer;
class wxButton;

class mxShortcutsDialog : public wxDialog {
	wxSizer *sizer;
	struct entry {
		int id;
		wxString *real_value;
		wxString search_text;
		wxStaticText *label;
		wxTextCtrl *text;
		wxButton *button;
	};
	SingleList<entry> actions;
	wxTextCtrl *filter;
	wxTimer timer;
public:
	void OnTimer(wxTimerEvent &evt);
	mxShortcutsDialog(wxWindow *parent);
	void OnFilter(wxCommandEvent &evt);
	void OnGrabButton(wxCommandEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	~mxShortcutsDialog();
	DECLARE_EVENT_TABLE();
};

#endif

