#ifndef MXOUTPUTVIEW_H
#define MXOUTPUTVIEW_H
#include <wx/dialog.h>
#include <wx/process.h>
#include <wx/timer.h>
#include "mxValgrindOuput.h"

class wxTextCtrl;
class wxProcess;
class wxTimer;
class wxStaticText;
class mxBitmapButton;
class wxFile;

enum mxOVmode {
	mxOV_EXTRA_NULL,
	mxOV_EXTRA_COMMAND,
	mxOV_EXTRA_URL,
};

class mxOutputView : public wxDialog {
private:
	wxTextCtrl *ctrl_std;
	wxTextCtrl *ctrl_err;
	wxStaticText *state;
	wxProcess *process;
	wxTimer *timer;
	int pid;
	mxOVmode extra_mode;
	wxString extra_command;
	wxString extra_label;
	mxBitmapButton *extra_button;
	mxBitmapButton *close_button;
	bool working;
	mxVOmode output_mode;
	wxString output_file;
	wxFile *textfile;
public:
	mxOutputView(wxString caption, mxOVmode extra_mode=mxOV_EXTRA_NULL, wxString extra_button="",wxString extra_command="", mxVOmode output_mode=mxVO_NULL, wxString output_file="");
	void Launch(wxString path, wxString command);
	void Launched(wxProcess *_process, int _pid);
	void AppendError(wxString text);
	void AppendOutput(wxString text);
	void GetProcessOutput();
	void OnOkButton(wxCommandEvent &evt);
	void OnExtraButton(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnProcessTerminateCommon(int exit_code);
	void OnProcessTerminate(wxProcessEvent &evt);
	void OnTimer(wxTimerEvent &evt);
	~mxOutputView();
	DECLARE_EVENT_TABLE();
};

#endif

