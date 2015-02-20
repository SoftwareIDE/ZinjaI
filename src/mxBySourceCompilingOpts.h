#ifndef MXBYSOURCECOMPILINGOPTS_H
#define MXBYSOURCECOMPILINGOPTS_H
#include <wx/dialog.h>

class wxListBox;
class wxTextCtrl;
class wxCheckBox;
class project_file_item;

class mxBySourceCompilingOpts : public wxDialog {
private:
	wxListBox *list;
	wxCheckBox *fp_extra;
	wxCheckBox *fp_macros;
	wxCheckBox *fp_includes;
	wxCheckBox *fp_std;
	wxCheckBox *fp_warnings;
	wxCheckBox *fp_optimizations;
	wxCheckBox *apply_to_all;
	wxTextCtrl *additional_args;
public:
	mxBySourceCompilingOpts(wxWindow *parent, project_file_item *item);
protected:
	DECLARE_EVENT_TABLE();
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnButtonHelp(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
};

#endif

