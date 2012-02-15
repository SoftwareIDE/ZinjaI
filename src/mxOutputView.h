#ifndef MXOUTPUTVIEW_H
#define MXOUTPUTVIEW_H
#include <wx/dialog.h>
#include <wx/process.h>
#include <wx/timer.h>

class wxTextCtrl;
class wxProcess;
class wxTimer;
class wxStaticText;
class mxBitmapButton;
class wxFile;

class mxOutputView : public wxDialog {
private:
	wxTextCtrl *ctrl_std;
	wxTextCtrl *ctrl_err;
	wxStaticText *state;
	wxProcess *process;
	wxTimer *timer;
	int pid;
	wxString extra_command;
	wxString extra_button_text;
	mxBitmapButton *close_button;
	mxBitmapButton *extra_button;
	bool working;
	char mode;
	wxFile *textfile;
public:
	mxOutputView(wxString caption, wxString extra_button=_T(""),wxString run_on_finish=_T(""), char amode=' ');
	void Launch(wxString path, wxString command);
	void AppendError(wxString text);
	void AppendOutput(wxString text);
	void GetProcessOutput();
	void OnOkButton(wxCommandEvent &evt);
	void OnExtraButton(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnProcessTerminate(wxProcessEvent &evt);
	void OnTimer(wxTimerEvent &evt);
	~mxOutputView();
	DECLARE_EVENT_TABLE();
};

#endif

