#ifndef MXBREAKOPTIONS_H
#define MXBREAKOPTIONS_H
#include <wx/dialog.h>

class mxSource;
class wxCheckBox;
class wxTextCtrl;
class BreakPointInfo;
struct file_item;

class mxBreakOptions : public wxDialog {
private:
	BreakPointInfo *bpi;
	wxCheckBox *break_check, *enable_check, *once_check;
	wxTextCtrl *ignore_text,*cond_text,*count_text;
public:
	mxBreakOptions(BreakPointInfo *_bpi);
	void OnClose(wxCloseEvent &event);
	void OnOkButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnBreakpointCheck(wxCommandEvent &etv);
	DECLARE_EVENT_TABLE();
};

#endif

