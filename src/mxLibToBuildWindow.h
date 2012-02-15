#ifndef MXLIBTOBUILDWINDOW_H
#define MXLIBTOBUILDWINDOW_H
#include <wx/dialog.h>

class project_configuration;
class project_library;
class wxTextCtrl;
class wxComboBox;
class wxListBox;
class wxCheckBox;
class mxProjectConfigWindow;

class mxLibToBuildWindow : public wxDialog {
private:
	project_configuration *configuration;
	project_library *lib;
	wxTextCtrl *name;
	wxTextCtrl *path;
	wxTextCtrl *filename;
	wxTextCtrl *extra_link;
	wxListBox *sources_in;
	wxListBox *sources_out;
	wxCheckBox *default_lib;
	wxComboBox *type;
	bool constructed;
	mxProjectConfigWindow *parent;
public:
	static wxString new_name;
	mxLibToBuildWindow(mxProjectConfigWindow *aparent, project_configuration *conf, project_library *alib=NULL);
	~mxLibToBuildWindow();
	void OnClose(wxCloseEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnInButton(wxCommandEvent &evt);
	void OnOutButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnCombo(wxCommandEvent &evt);
	void SetFName();
	DECLARE_EVENT_TABLE();
};

#endif

