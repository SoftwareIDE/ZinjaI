#ifndef MXARGUMENTSDIALOG_H
#define MXARGUMENTSDIALOG_H

#define AD_CANCEL 0
#define AD_EMPTY 1
#define AD_ARGS 2
#define AD_REMEMBER 4
#include "mxCommonPopup.h"
#include "mxCommonConfigControls.h"

class wxComboBox;
class wxCheckBox;

class mxArgumentsDialog : public mxDialog {
	_use_common_popup;
private:
	static wxArrayString list_for_combo_args;
	static wxArrayString list_for_combo_work;
	wxComboBox *combo_args;
	wxComboBox *combo_work;
	wxCheckBox *check;
//	static wxString last_dir;
	wxString m_base_path;
public:
	static wxString last_workdir;
	static wxString last_arguments;
	mxArgumentsDialog(wxWindow *parent, const wxString &base_path, const wxString &def_args, const wxString &def_dir);
	void OnYesButton(wxCommandEvent &evt);
	void OnNoButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnWorkdirButton(wxCommandEvent &evt);
	void OnArgsButton(wxCommandEvent &evt);
	void OnCharHook(wxKeyEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

