#ifndef MX_SHARE_SOURCE_WINDOW
#define MX_SHARE_SOURCE_WINDOW

#include <wx/dialog.h>

class wxTextCtrl;
class wxComboBox;
class wxCheckBox;
class mxSource;

class mxShareSourceWindow : public wxDialog {

public:

	wxComboBox *source_name;
	mxSource *source_ctrl;
	wxCheckBox *freeze_check;

	mxShareSourceWindow(mxSource *source, wxString name, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);

	void OnOkButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);

private:
	DECLARE_EVENT_TABLE()

};

#endif
