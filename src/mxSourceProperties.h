#ifndef MXSOURCEPROPERTIES_H
#define MXSOURCEPROPERTIES_H
#include <wx/dialog.h>
#include <wx/timer.h>
class mxSource;
class wxTextCtrl;
class wxTimer;

class mxSourceProperties : public wxDialog {
private:
	wxTextCtrl *text_type, *text_deps;
	wxString fname;
public:
	mxSourceProperties(wxString fname,mxSource *src=nullptr);
	void OnOkButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnTimer(wxTimerEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

