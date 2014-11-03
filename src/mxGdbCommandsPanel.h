#ifndef MXGDBCOMMANDSPANEL_H
#define MXGDBCOMMANDSPANEL_H
#include <wx/panel.h>
#include <wx/stc/stc.h>

class wxTextCtrl;
class wxTimer;
class wxStyledTextCtrl;

class mxGdbCommandsPanel : public wxPanel {
private:
	wxTextCtrl *input;
	wxStyledTextCtrl *output;
	void AppendText(const wxString &str);
public:
	void OnInput(wxCommandEvent &event);
	void OnMarginClick(wxStyledTextEvent &e);
	mxGdbCommandsPanel();
	DECLARE_EVENT_TABLE();
};

#endif

