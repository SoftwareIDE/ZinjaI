#ifndef MXCUSTOMTOOLS_H
#define MXCUSTOMTOOLS_H
#include <wx/dialog.h>

class CustomToolsPack;

class wxComboBox;
class wxTextCtrl;
class wxCheckBox;
class mxCustomTools : public wxDialog {
private:
	wxComboBox *the_combo, *pre_action_ctrl, *post_action_ctrl, *output_mode_ctrl;
	wxTextCtrl *command_ctrl, *name_ctrl, *workdir_ctrl;
	wxCheckBox *ontoolbar_ctrl, *async_exec_ctrl;
	CustomToolsPack *tools;
	int tool_count;
	bool for_project;
	static int prev_sel;
	void ToolToDialog(int i);
	void DialogToTool(int i);
public:
	mxCustomTools(bool for_project, int cual);
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

