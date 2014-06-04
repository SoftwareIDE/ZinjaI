#ifndef MXARGUMENTSDIALOG_H
#define MXARGUMENTSDIALOG_H
#include <wx/dialog.h>

#define AD_EMPTY 1
#define AD_ARGS 2
#define AD_CANCEL 4
#define AD_REMEMBER 8

class wxComboBox;
class wxCheckBox;

class mxArgumentsDialog : public wxDialog {
private:
	static wxArrayString list_for_combo_args;
	static wxArrayString list_for_combo_work;
	wxComboBox *combo_args;
	wxComboBox *combo_work;
	wxCheckBox *check;
	static wxString last_dir;
public:
	static wxString last_workdir;
	static wxString last_arguments;
	mxArgumentsDialog(wxWindow *parent, wxString def_args);
	void OnYesButton(wxCommandEvent &evt);
	void OnNoButton(wxCommandEvent &evt);
	void OnCancelButton(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnWorkdirButton(wxCommandEvent &evt);
	void OnArgsButton(wxCommandEvent &evt);
	void OnArgsAddFile(wxCommandEvent &evt);
	void OnArgsReplaceFile(wxCommandEvent &evt);
	void OnArgsAddDir(wxCommandEvent &evt);
	void OnArgsReplaceDir(wxCommandEvent &evt);
	void OnArgsEditList(wxCommandEvent &evt);
	void OnArgsEditText(wxCommandEvent &evt);
	void OnCharHook(wxKeyEvent &evt);
	~mxArgumentsDialog();
	DECLARE_EVENT_TABLE();
};

#endif

