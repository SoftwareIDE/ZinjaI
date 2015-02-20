#ifndef MX_PROJECT_CONFIG_WINDOW_H
#define MX_PROJECT_CONFIG_WINDOW_H

#include <wx/dialog.h>
#include <wx/listctrl.h>
#include "widgetDisabler.h"

class project_configuration;
class wxCheckBox;
class wxComboBox;
class wxTextCtrl;
class wxNotebook;
class wxPanel;
class wxListBox;

class mxProjectConfigWindow : public wxDialog {
	
	friend class mxLibToBuildWindow;
	
	static int last_page_index;
	wxNotebook *notebook;
	bool discard;
	
	wxTextCtrl *text_for_edit;
	bool comma_splits_for_edit;
	wxString last_dir;

	wxComboBox *configuration_name;
	project_configuration *configuration;

	// pestana general
	wxTextCtrl *general_working_folder;
	wxCheckBox *general_always_ask_args;
	wxTextCtrl *general_args;
	wxComboBox *general_exec_method;
	wxTextCtrl *general_exec_script;
	wxComboBox *general_wait_for_key;
	
	wxTextCtrl *general_env_vars;
	wxTextCtrl *general_output_file;
	wxTextCtrl *linking_icon;
	wxTextCtrl *linking_manifest;
	
	// pestana compilacion
	wxTextCtrl *compiling_macros;
	wxTextCtrl *compiling_extra_options;
	wxTextCtrl *compiling_headers_dirs;
	wxComboBox *compiling_warnings_level;
	wxCheckBox *compiling_pedantic;
	wxComboBox *compiling_debug_level;
	wxComboBox *compiling_optimization_level;
	wxComboBox *compiling_std_c;
	wxComboBox *compiling_std_cpp;
	wxTextCtrl *compiling_temp_folder;

	// pestana enlazado
	//wxTextCtrl *linking_resource_file;
	wxTextCtrl *linking_extra_options;
	wxTextCtrl *linking_libraries_dirs;
	wxTextCtrl *linking_libraries;
	wxComboBox *linking_strip_executable;
	wxCheckBox *linking_force_relink;
	wxCheckBox *linking_console_program;
	
	// pestaña pasos
	wxListBox *steps_list;
	wxComboBox *toolchains_combo;
	
	// pestaña bibliotecas
	wxListBox *libtobuild_list;
	wxCheckBox *libtobuild_noexec;
	
	widgetDisabler wx_noscript;
	widgetDisabler wx_noexe;
	widgetDisabler wx_extern;

public:

	mxProjectConfigWindow(wxWindow* parent);
	wxPanel *CreateQuickHelpPanel (wxNotebook *notebook);
	wxPanel *CreateGeneralPanel (wxNotebook *notebook);
	wxPanel *CreateCompilingPanel (wxNotebook *notebook);
	wxPanel *CreateLinkingPanel (wxNotebook *notebook);
	wxPanel *CreateStepsPanel (wxNotebook *notebook);
	wxPanel *CreateLibsPanel (wxNotebook *notebook);
	void OnManifestDirButton(wxCommandEvent &event);
	void OnIconDirButton(wxCommandEvent &event);
	void OnEnvVarsButton(wxCommandEvent &event);
	void OnTempDirButton(wxCommandEvent &event);
	void OnWorkingDirButton(wxCommandEvent &event);
	void OnAddConfigButton(wxCommandEvent &event);
	void OnSelectConfigButton(wxCommandEvent &event);
	void OnRemoveConfigButton(wxCommandEvent &event);
	void OnRenameConfigButton(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnOkButton(wxCommandEvent &event);
	bool SaveValues();
	void DiscardChanges();
	void OnCancelButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void LoadValues();
	void OnSelectConfigInCombo(wxCommandEvent &event);
	void OnExecutionMethodButton(wxCommandEvent &evt);
	void OnComboExecutionScript(wxCommandEvent &evt);
	void OnArgsAddFile(wxCommandEvent &evt);
	void OnArgsAddDir(wxCommandEvent &evt);
	void OnArgsReplaceFile(wxCommandEvent &evt);
	void OnArgsReplaceDir(wxCommandEvent &evt);
	void OnArgsEditList(wxCommandEvent &evt);
	void OnArgsEditText(wxCommandEvent &evt);
	void OnGeneralArgsButton(wxCommandEvent &evt);
	void OnCompilingExtraOptionsButton(wxCommandEvent &evt);
	void OnCompilingHeadersDirsButton(wxCommandEvent &evt);
	void OnCompilingMacrosButton(wxCommandEvent &evt);
	void OnLinkingExtraOptionsButton(wxCommandEvent &evt);
	void OnLinkingLibrariesButton(wxCommandEvent &evt);
	void OnLinkingLibrariesDirsButton(wxCommandEvent &evt);
	void OnGeneralExePathButton(wxCommandEvent &evt);
	void OnStepsUp(wxCommandEvent &evt);
	void OnStepsDown(wxCommandEvent &evt);
	void OnStepsAdd(wxCommandEvent &evt);
	void OnStepsDel(wxCommandEvent &evt);
	void OnStepsRun(wxCommandEvent &evt);
	void OnStepsEdit(wxCommandEvent &evt);
	void OnToolchainOptionsButton(wxCommandEvent &evt);
	void OnComboToolchainChange(wxCommandEvent &evt);
//	void OnImportLibsButton(wxCommandEvent &evt);
	void OnLibsAdd(wxCommandEvent &evt);
	void OnLibsEdit(wxCommandEvent &evt);
	void OnLibsDel(wxCommandEvent &evt);
	void ReloadSteps(wxString selection="");
	void ReloadLibs(wxString selection="");
	void OnLibsNoExe(wxCommandEvent &evt);
	
	void SelectCustomStep(const wxString &custom_step_name);
private:
	DECLARE_EVENT_TABLE()
};


#endif
