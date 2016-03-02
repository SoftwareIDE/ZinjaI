#ifndef MX_PREFERENCE_WINDOW_H
#define MX_PREFERENCE_WINDOW_H

#include <wx/dialog.h>
#include "widgetDisabler.h"

#ifndef __WIN32__
#include "mxCommonConfigControls.h"
struct LinuxTerminalInfo {
	wxString name; ///< nombre para mostrar en el cuadro de preferencias
	wxString test_command; ///< comando para probar si existe (usualmente ejecutable+" --version")
	wxString extra_test; ///< extra string to find in test_command's output
	wxString run_command; ///< comando para setear en config->Files.terminal_command
	bool warning; ///< indica si es problematica y se debe recomendar utilizar otra.
	LinuxTerminalInfo() {}
	LinuxTerminalInfo(wxString _name, wxString _test_command, wxString _run_command, bool _warning=false, wxString _extra_test="")
		:name(_name),test_command(_test_command),extra_test(_extra_test),run_command(_run_command),warning(_warning) {}
	bool Test();
	static int count;
	static LinuxTerminalInfo *list;
	static void Initialize();
};
#endif

#ifdef __WIN32__
# define mxBookCtrl wxTreebook
#else
# define mxBookCtrl wxListbook
#endif

class wxCheckBox;
class wxComboBox;
class wxTextCtrl;
class wxAuiNotebook;
class wxNotebook;
class mxBookCtrl;
class wxListBox;
class wxPanel;
class wxCheckListBox;
class mxToolbarEditor;
class wxStaticBitmap;

class mxPreferenceWindow : public mxDialog {
	
	widgetBinder m_binder;
	
	bool ignore_styles_changes;

	wxCheckBox *toolbars_wich_file;
	wxCheckBox *toolbars_wich_find;
	wxCheckBox *toolbars_wich_project;
	wxCheckBox *toolbars_wich_edit;
	wxCheckBox *toolbars_wich_view;
	wxCheckBox *toolbars_wich_run;
	wxCheckBox *toolbars_wich_debug;
	wxCheckBox *toolbars_wich_misc;
	wxCheckBox *toolbars_wich_tools;
	
	wxComboBox *toolbars_side_file;
//	wxComboBox *toolbars_side_project;
	wxComboBox *toolbars_side_edit;
	wxComboBox *toolbars_side_view;
	wxComboBox *toolbars_side_run;
	wxComboBox *toolbars_side_debug;
	wxComboBox *toolbars_side_misc;
	wxComboBox *toolbars_side_tools;
	mxToolbarEditor *toolbar_editor_file;
	mxToolbarEditor *toolbar_editor_edit;
	mxToolbarEditor *toolbar_editor_tools;
	mxToolbarEditor *toolbar_editor_misc;
	mxToolbarEditor *toolbar_editor_debug;
	mxToolbarEditor *toolbar_editor_run;
	mxToolbarEditor *toolbar_editor_view;
	
#ifdef __linux__
	wxCheckBox *init_disable_ubuntu_tweaks;
#endif
	wxTextCtrl *files_autocode;
	wxTextCtrl *files_temp_dir;
	wxComboBox *files_toolchain;
	wxTextCtrl *files_explorer_command;
	wxTextCtrl *files_terminal_command;
	wxTextCtrl *files_img_viewer_command;
	wxTextCtrl *files_xdot_command;
	wxTextCtrl *files_debugger_command;
	wxTextCtrl *files_project_folder;
	wxTextCtrl *files_wxfb_command;
	wxTextCtrl *files_valgrind_command;
	wxTextCtrl *files_cppcheck_command;
	wxTextCtrl *files_doxygen_command;
	wxTextCtrl *files_browser_command;
	wxTextCtrl *help_wxhelp_index;
	wxCheckListBox *help_autocomp_indexes;
	wxTextCtrl *source_edgeColumnPos;
	wxCheckBox *source_edgeColumnCheck;
	wxCheckBox *source_whiteSpace;
	wxCheckBox *source_toolTips;
	wxTextCtrl *styles_font_size;
	wxComboBox *styles_font_name;
	wxCheckBox *styles_dark;
	wxTextCtrl *files_default_template;
	wxCheckBox *debug_compile_again;
#ifdef __WIN32__
	wxCheckBox *debug_no_debug_heap;
#endif
	wxTextCtrl *debug_macros_file;
//#ifdef __WIN32__
//	wxTextCtrl *files_mingw_dir;
//#endif
	
	wxComboBox *toolbar_icon_size;
	
	wxArrayString skin_paths;
	wxTextCtrl *skin_text;
	wxListBox *skin_list;
	wxStaticBitmap *skin_image;
	wxPanel *panel_toolbars;
	
	wxComboBox *simple_default_cpp_template;
	wxComboBox *simple_default_c_template;
	
	wxArrayString temp_debug_blacklist;
	
	widgetDisabler wx_toolbars_widgets;

public:

	mxBookCtrl *notebook;

	mxPreferenceWindow(wxWindow* parent);
	
	wxPanel *CreateQuickHelpPanel (mxBookCtrl *notebook);
	
	wxPanel *CreateGeneralPanel (mxBookCtrl *notebook);
	
	wxNotebook *CreateCompilePanel (mxBookCtrl *notebook);
	wxPanel *CreateCompilePanelSimple (wxNotebook *notebook);
	wxPanel *CreateCompilePanelProject (wxNotebook*notebook);
	
	wxPanel *CreateStylePanel (mxBookCtrl *notebook);
	
	wxNotebook *CreateWritingPanels (mxBookCtrl *notebook);
	wxPanel *CreateWritingPanel1 (wxNotebook *notebook);
	wxPanel *CreateWritingPanel2 (wxNotebook *notebook);
	
	wxNotebook *CreatePathsPanels (mxBookCtrl *notebook);
	wxPanel *CreatePathsPanel1 (wxNotebook *notebook);
	wxPanel *CreatePathsPanel2 (wxNotebook *notebook);
	wxPanel *CreateToolbarsPanel (mxBookCtrl *notebook);
	wxPanel *CreateSkinPanel (mxBookCtrl *notebook);
	wxNotebook *CreateDebugPanels (mxBookCtrl *notebook);
	wxPanel *CreateDebugPanel1 (wxNotebook *notebook);
	wxPanel *CreateDebugPanel2 (wxNotebook*notebook);
	
	void OnWxHelpButton(wxCommandEvent &event);
	void OnSkinList(wxCommandEvent &event);
	void OnSkinButton(wxCommandEvent &event);
	void OnProjectButton(wxCommandEvent &event);
	void OnTempButton(wxCommandEvent &event);
#ifdef __WIN32__
//	void OnMingwButton(wxCommandEvent &event);
#else
	void OnExplorerButton(wxCommandEvent &event);
	void OnExplorerNautilus(wxCommandEvent &event);
	void OnExplorerDolphin(wxCommandEvent &event);
	void OnExplorerThunar(wxCommandEvent &event);
	void OnExplorerKonqueror(wxCommandEvent &event);
	void OnTerminalButton(wxCommandEvent &event);
	void OnSelectTerminal(wxCommandEvent &event);
#endif
	void OnOkButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnClearSubcmdCache(wxCommandEvent &event);
	void OnImproveInspectionsByTypeButton(wxCommandEvent &event);
	void OnColoursButton(wxCommandEvent &event);
	void OnMaxJobsButton(wxCommandEvent &event);
	void OnGdbButton(wxCommandEvent &event);
	void OnImgBrowserButton(wxCommandEvent &event);
	void OnXdotButton(wxCommandEvent &event);
	void OnBrowserButton(wxCommandEvent &event);
	void OnValgrindButton(wxCommandEvent &event);
	void OnCppCheckButton(wxCommandEvent &event);
	void OnWxfbButton(wxCommandEvent &event);
	void OnDoxygenButton(wxCommandEvent &event);
	void OnDebugMacrosOpen(wxCommandEvent &event);
	void OnDebugMacrosEdit(wxCommandEvent &event);
	void OnDebugMacrosButton(wxCommandEvent &event);
	void OnAutocodesOpen(wxCommandEvent &event);
	void OnAutocodesEdit(wxCommandEvent &event);
	void OnAutocodesButton(wxCommandEvent &event);
	void OnDebugBlacklistButton(wxCommandEvent &event);
	void OnToolbarsCommon(mxToolbarEditor *wx_toolbar, int tb_id);
	void OnToolbarsFile(wxCommandEvent &evt);
	void OnToolbarsView(wxCommandEvent &evt);
	void OnToolbarsEdit(wxCommandEvent &evt);
	void OnToolbarsMisc(wxCommandEvent &evt);
	void OnToolbarsTools(wxCommandEvent &evt);
	void OnToolbarsDebug(wxCommandEvent &evt);
	void OnToolbarsRun(wxCommandEvent &evt);
	void OnToolbarsReset(wxCommandEvent &evt);
	void OnXdgButton(wxCommandEvent &evt);
	void OnToolchainButton(wxCommandEvent &evt);
	void SetToolbarPage(const wxString &edit_one="");
	static mxPreferenceWindow *ShowUp();
	static void Delete(); ///< to force reloading of some things that Reset won't redo, such as the list of autocompletion indexes
	void ResetChanges();
	void OnFontChange(wxCommandEvent &evt);
	void OnCustomizeShortcuts(wxCommandEvent &evt);
	void EnableOrDisableControls();
private:
	DECLARE_EVENT_TABLE()

};

#endif
