#ifndef MXBREAKOPTIONS_H
#define MXBREAKOPTIONS_H
#include <wx/dialog.h>

class mxSource;
class wxCheckBox;
class wxTextCtrl;
struct file_item;
struct break_line_item;

class mxBreakOptions : public wxDialog {
private:
	break_line_item *bitem;
	file_item *file;
	wxString filename;
	mxSource *source;
	int line;
	wxCheckBox *break_check, *enable_check, *once_check;
	wxTextCtrl *ignore_text,*cond_text,*count_text;
public:
	mxBreakOptions(wxString filename, int line, mxSource *source, break_line_item *item=NULL);
	mxBreakOptions(wxString filename, int line, file_item *file, break_line_item *item=NULL);
	void OnClose(wxCloseEvent &event);
	void OnOkButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnBreakpointCheck(wxCommandEvent &etv);
	void CommonConstructorStuff();
	DECLARE_EVENT_TABLE();
};

#endif

