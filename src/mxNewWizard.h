#ifndef MX_TIPS_WINDOW_H
#define MX_TIPS_WINDOW_H

#include <wx/dialog.h>
#include <wx/arrstr.h>

class mxBitmapButton;
class wxBoxSizer;
class wxPanel;
class mxMainWindow;
class wxButton;
class wxRadioBox;
class wxCheckBox;
class wxComboBox;
class wxListBox;
class wxTextCtrl;
class wxStaticText;

class mxNewWizard : public wxDialog {
public:

	bool only_for_project; ///< indica si el cuadro se abrio para la opcion "Nuevo..." (general) o "Nuevo Proyecto..."
	bool project_internal_folder_change; ///< para evitar mover el radiobutton al cambiar el texto de la carpeta de proyecto por un click en el mismo radio button
	
	int size_w, size_h;
	
	wxRadioBox *onproject_radio;
	wxStaticText *onproject_label;
	wxStaticText *onproject_inherit_label;
	wxStaticText *onproject_inherit_include;
	wxTextCtrl *onproject_name;
	wxCheckBox *onproject_const_def;
	wxCheckBox *onproject_const_copy;
	wxCheckBox *onproject_dest;
	wxComboBox *onproject_inherit_visibility[5];
	wxTextCtrl *onproject_inherit_class[5];
	
	wxRadioBox *start_radio;
	wxCheckBox *templates_check;
	wxListBox *templates_list;
	int templates_default;
	wxArrayString file_templates;	

	wxCheckBox *wizard_cstring;
	wxCheckBox *wizard_cmath;
	wxCheckBox *wizard_rand;
	wxCheckBox *wizard_fstream;
	wxCheckBox *wizard_iomanip;
	wxCheckBox *wizard_algorithm;
	wxCheckBox *wizard_vector;
	wxCheckBox *wizard_list;
	wxCheckBox *wizard_map;
	wxCheckBox *wizard_set;
	wxCheckBox *wizard_queue;
	wxCheckBox *wizard_deque;
	wxCheckBox *wizard_stack;

	wxTextCtrl *project_full_path;
	wxTextCtrl *project_name;
	wxRadioBox *project_folder_radio;
	wxTextCtrl *project_folder_path;
	wxCheckBox *project_check;
	wxCheckBox *project_current_files;
	wxCheckBox *project_dir_files;
	wxCheckBox *project_folder_create;
	wxListBox *project_list;
	wxArrayString project_templates;	
	int project_default;
	
	wxBoxSizer *sizer_for_panel;
	wxPanel *panel;
	mxBitmapButton *nextButton;
	mxBitmapButton *cancelButton;
 
	wxPanel *panel_onproject;
	void CreatePanelOnProject();
	void ShowPanelOnProject();
	wxPanel *panel_start;
	void CreatePanelStart();
	void ShowPanelStart(bool show=false);
	wxPanel *panel_templates;
	void CreatePanelTemplates();
	void ShowPanelTemplates();
	wxPanel *panel_project_1;
	void CreatePanelProject1();
	void ShowPanelProject1();
	wxPanel *panel_project_2;
	void CreatePanelProject2();
	void ShowPanelProject2();
	wxPanel *panel_wizard_1;
	void CreatePanelWizard1();
	void ShowPanelWizard1();
	wxPanel *panel_wizard_2;
	void CreatePanelWizard2();
	void ShowPanelWizard2();

	mxNewWizard(mxMainWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);

	void OnButtonNext(wxCommandEvent &event);
	void OnButtonCancel(wxCommandEvent &event);
	void OnButtonHelp(wxCommandEvent &event);
	void OnButtonFolder(wxCommandEvent &event);
	void OnStartRadio(wxCommandEvent &event);
	void OnProjectRadio(wxCommandEvent &event);
	void OnProjectPathRadio(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void RunWizard(wxString how=_T("simple"));
	
	void ProjectCreate();
	void OnProjectCreate();
	void WizardCreate();
	
	void OnProjectFolderCheck(wxCommandEvent &evt);
	void OnProjectNameChange(wxCommandEvent &evt);
	void OnProjectFolderChange(wxCommandEvent &evt);
	void UpdateProjectFullPath();
	
	void OnProjectFilesOpenCheck(wxCommandEvent &evt);
	void OnProjectFilesDirCheck(wxCommandEvent &evt);
	
	void OnButtonNewFilePath(wxCommandEvent &evt);

private:
	DECLARE_EVENT_TABLE()

};

extern mxNewWizard *wizard;

#endif
