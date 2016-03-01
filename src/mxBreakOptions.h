#ifndef MXBREAKOPTIONS_H
#define MXBREAKOPTIONS_H

#include <wx/dialog.h>
#include "mxCommonConfigControls.h"

class mxSource;
class wxCheckBox;
class wxTextCtrl;
class wxComboBox;
class BreakPointInfo;
struct file_item;

class mxBreakOptions : public mxDialog {
private:
	BreakPointInfo *m_bpi;
	wxCheckBox *break_check, *enable_check;
	wxTextCtrl *ignore_text,*cond_text,*count_text, *annotation_text;
	wxComboBox *action;
public:
	mxBreakOptions(BreakPointInfo *bpi);
	void OnOkButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnBreakpointCheck(wxCommandEvent &etv);
	DECLARE_EVENT_TABLE();
};

#endif

