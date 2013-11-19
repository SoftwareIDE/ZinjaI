#ifndef MXCUSTOMTOOLS_H
#define MXCUSTOMTOOLS_H
#include <wx/dialog.h>

#define MAX_PROJECT_CUSTOM_TOOLS 5
#define MAX_CUSTOM_TOOLS 10
struct cfgCustomTool {
	wxString name;
	wxString command;
	wxString workdir;
	bool console;
	bool on_toolbar;
	cfgCustomTool():console(false),on_toolbar(false){}
};

class wxComboBox;
class wxTextCtrl;
class wxCheckBox;
class mxCustomTools : public wxDialog {
private:
	wxComboBox *the_combo;
	wxTextCtrl *command_ctrl, *name_ctrl, *workdir_ctrl;
	wxCheckBox *console_ctrl, *ontoolbar_ctrl;
	cfgCustomTool *tools, *orig;
	int tool_count;
	bool for_project;
	static int prev_sel;
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

