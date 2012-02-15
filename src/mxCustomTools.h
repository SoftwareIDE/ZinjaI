#ifndef MXCUSTOMTOOLS_H
#define MXCUSTOMTOOLS_H
#include <wx/dialog.h>

class wxComboBox;
class wxTextCtrl;
class wxCheckBox;
class mxCustomTools : public wxDialog {
private:
	wxComboBox *the_combo;
	wxTextCtrl *command_ctrl, *name_ctrl, *workdir_ctrl;
	wxCheckBox *console_ctrl, *ontoolbar_ctrl;
	wxString commands[10],names[10];
	bool console[10], ontoolbar[10];
	static int prev_sel;
public:
	mxCustomTools(int cual=-1);
	void OnButtonOk(wxCommandEvent &event);
	void OnButtonTest(wxCommandEvent &event);
	void OnButtonCancel(wxCommandEvent &event);
	void OnButtonHelp(wxCommandEvent &event);
	void OnCommandPopup(wxCommandEvent &event);
	void OnWorkdirPopup(wxCommandEvent &event);
	void OnPopup(wxCommandEvent &evt);
	void OnComboChange(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

