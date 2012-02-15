#ifndef MX_LIST_SHARED_WINDOW
#define MX_LIST_SHARED_WINDOW

#include <wx/dialog.h>
#include <wx/listctrl.h>

class wxTextCtrl;
class wxListBox;

class mxListSharedWindow : public wxDialog {
	
public:
	
	wxListBox *list;
	
	mxListSharedWindow(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	
	void OnStopSharingButton(wxCommandEvent &event);
	void OnHelpButton(wxCommandEvent &event);
	void OnCloseButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	
private:
	DECLARE_EVENT_TABLE()
	
};

#endif
