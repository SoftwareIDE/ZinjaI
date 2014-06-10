#ifndef MXGDBCOMMANDSPANEL_H
#define MXGDBCOMMANDSPANEL_H
#include <wx/panel.h>

class wxTextCtrl;
class wxTimer;

class mxGdbCommandsPanel : public wxPanel {
private:
	wxTextCtrl *input,*output;
	void AppendText(const wxString &str);
public:
	void OnInput(wxCommandEvent &event);
	mxGdbCommandsPanel();
	DECLARE_EVENT_TABLE();
};

#endif

