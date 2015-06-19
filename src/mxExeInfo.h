#ifndef MXEXEINFO_H
#define MXEXEINFO_H
#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/timer.h>

class mxSource;
class wxNotebook;
class wxPanel;
class wxTextCtrl;
class wxTimer;

class mxExeInfo : public wxDialog {
private:
	mxSource *source;
	wxFileName fname;
	wxTextCtrl *text_size, *text_type, *text_time, *ldd_ctrl;
	wxTimer *wait_for_parser;
	void UpdateTypeAndDeps();
public:
	mxExeInfo(wxWindow *parent, mxSource *src);
	wxPanel *CreateGeneralPanel (wxNotebook *notebook);
	wxPanel *CreateDependPanel (wxNotebook *notebook);
	void OnClose(wxCloseEvent &event);
	void OnCloseButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnStripButton(wxCommandEvent &evt);
	void OnTimer(wxTimerEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

