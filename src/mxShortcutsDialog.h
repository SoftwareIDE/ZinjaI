#ifndef MXSHORTCUTSDIALOG_H
#define MXSHORTCUTSDIALOG_H
#include <wx/dialog.h>
#include <wx/timer.h>
#include "SingleList.h"
#include "MenusAndToolsConfig.h"

class wxStaticText;
class wxTextCtrl;
class wxSizer;
class wxButton;

class mxShortcutsDialog : public wxDialog {
	wxSizer *sizer;
	struct entry {
		int id;
		MenusAndToolsConfig::myMenuItem *menu_item;
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
	DECLARE_EVENT_TABLE();
};

#endif

