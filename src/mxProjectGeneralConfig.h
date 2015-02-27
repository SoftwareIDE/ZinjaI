#ifndef MX_PROJECT_GENERAL_CONFIG_H
#define MX_PROJECT_GENERAL_CONFIG_H
#include <wx/dialog.h>

class wxTextCtrl;
class wxCheckBox;
class wxPanel;
class wxNotebook;

class mxProjectGeneralConfig : public wxDialog {
private:
	wxTextCtrl *project_name;
	wxTextCtrl *project_autocomp;
	wxTextCtrl *project_debug_macros;
	wxTextCtrl *project_autocodes;
	wxCheckBox *custom_tab;
	wxTextCtrl *tab_width;
	wxCheckBox *tab_use_spaces;
//	wxCheckBox *use_wxfb;
public:
	mxProjectGeneralConfig();
	~mxProjectGeneralConfig();
	wxPanel *CreateTabGeneral(wxNotebook *notebook);
	wxPanel *CreateTabAdvanced(wxNotebook *notebook);
	void OnClose(wxCloseEvent &event);
	void OnOkButton(wxCommandEvent &evt);
	void OnDoxygenConfigButton(wxCommandEvent &evt);
	void OnCompileConfigButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnDebugMacrosOpen(wxCommandEvent &evt);
	void OnDebugMacrosEdit(wxCommandEvent &evt);
	void OnDebugMacros(wxCommandEvent &evt);
	void OnAutocodesOpen(wxCommandEvent &evt);
	void OnAutocodesEdit(wxCommandEvent &evt);
	void OnAutocodes(wxCommandEvent &evt);
	void OnIndexesButton(wxCommandEvent &evt);
	void OnCustomTabs(wxCommandEvent &evt);
	void OnCustomToolsConfig(wxCommandEvent &evt);
	void OnWxfbConfig(wxCommandEvent &evt);
	void OnCppCheckConfig(wxCommandEvent &evt);
	void OnBySrcCompilingOts(wxCommandEvent &evt);
	void OnDrawGraph(wxCommandEvent &evt);
	void OnStatistics(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

