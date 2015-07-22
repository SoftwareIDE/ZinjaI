#ifndef MXGDBCOMMANDSPANEL_H
#define MXGDBCOMMANDSPANEL_H
#include <wx/panel.h>
#include <wx/stc/stc.h>

class wxTextCtrl;
class wxTimer;
class mxStyledOutput;

class mxGdbCommandsPanel : public wxPanel {
private:
	wxTextCtrl *input;
	mxStyledOutput *output;
public:
	void OnInput(wxCommandEvent &event);
	void SetFocusToInput();
	mxGdbCommandsPanel();
	DECLARE_EVENT_TABLE();
};

#endif

