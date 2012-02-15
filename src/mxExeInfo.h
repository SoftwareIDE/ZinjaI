#ifndef MXEXEINFO_H
#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/timer.h>
#define MXEXEINFO_H

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
public:
	mxExeInfo(mxSource *src);
	wxPanel *CreateGeneralPanel (wxNotebook *notebook);
	wxPanel *CreateDependPanel (wxNotebook *notebook);
	~mxExeInfo();
	void OnClose(wxCloseEvent &event);
	void OnCloseButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnStripButton(wxCommandEvent &evt);
	void OnTimer(wxTimerEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

