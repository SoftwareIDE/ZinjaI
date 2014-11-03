#ifndef MX_OPEN_SHARED_WINDOW
#define MX_OPEN_SHARED_WINDOW

#include <wx/dialog.h>

class wxTextCtrl;
class wxListBox;

class mxOpenSharedWindow : public wxDialog {

	wxTextCtrl *hostname;
	wxListBox *clients_list;
	wxListBox *files_list;

public:
	mxOpenSharedWindow(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	void OnClientList(wxCommandEvent &event);
	void OnGetSourceButton(wxCommandEvent &event);
	void OnGetListButton(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnCloseButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnCharHook(wxKeyEvent &event);
	void AddClient(const wxString &name, const wxString &ip);
	
private:
	DECLARE_EVENT_TABLE()

};

#endif
