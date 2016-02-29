#ifndef MXCUSTOMTOOLS_H
#define MXCUSTOMTOOLS_H
#include <wx/dialog.h>
#include "mxCommonPopup.h"
#include "mxCommonConfigControls.h"

class CustomToolsPack;

class wxComboBox;
class wxTextCtrl;
class wxCheckBox;
class mxCustomTools : public mxDialog {
private:
	_use_common_popup;
	wxComboBox *the_combo, *pre_action_ctrl, *post_action_ctrl, *output_mode_ctrl;
	wxTextCtrl *command_ctrl, *name_ctrl, *workdir_ctrl;
	wxCheckBox *ontoolbar_ctrl, *async_exec_ctrl;
	CustomToolsPack *m_tools;
	int m_tool_count;
	bool m_for_project;
	int m_prev_sel;
	void ToolToDialog(int i);
	void DialogToTool(int i);
public:
	mxCustomTools(bool for_project, int cual);
	~mxCustomTools();
	void OnButtonOk(wxCommandEvent &event);
	void OnButtonTest(wxCommandEvent &event);
	void OnButtonCancel(wxCommandEvent &event);
	void OnButtonHelp(wxCommandEvent &event);
	void OnCommandPopup(wxCommandEvent &event);
	void OnWorkdirPopup(wxCommandEvent &event);
	void OnComboChange(wxCommandEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

