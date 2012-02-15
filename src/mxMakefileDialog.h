#ifndef MX_MAKEFILE_DIALOG_H
#define MX_MAKEFILE_DIALOG_H

#include <wx/combobox.h>
#include <wx/dialog.h>

class wxCheckBox;
class wxComboBox;
class wxTextCtrl;

class mxMakefileDialog : public wxDialog {
private:
	wxComboBox *configuration_name;
	wxComboBox *split_type;
	wxTextCtrl *file_path;
	wxTextCtrl *mingw_dir;
	wxCheckBox *expand_comas;
public:
	mxMakefileDialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	void OnOkButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnExploreButton(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

