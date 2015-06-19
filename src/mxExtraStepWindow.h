#ifndef MXEXTRASTEPWINDOW_H
#define MXEXTRASTEPWINDOW_H
#include <wx/dialog.h>
#include "Cpp11.h"

class wxCheckBox;
class wxComboBox;
class wxTextCtrl;
class compile_extra_step;
class project_configuration;

class mxExtraStepWindow : public wxDialog {
private:
	compile_extra_step *step;
	wxComboBox *position;
	wxTextCtrl *name, *command, *output, *deps;
	wxCheckBox *check_rv, *hide_win, *delclean, *link_output;
	project_configuration *configuration;
public:
	static wxString new_name;
	mxExtraStepWindow(wxWindow *parent, project_configuration *conf, compile_extra_step *astep=nullptr);
	void OnClose(wxCloseEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnHelpButton(wxCommandEvent &evt);
	void OnCommandButton(wxCommandEvent &evt);
	void OnDepsButton(wxCommandEvent &evt);
	void OnOutputButton(wxCommandEvent &evt);
	void OnPopup(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

