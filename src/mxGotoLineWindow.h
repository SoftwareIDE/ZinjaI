#ifndef MX_GOTO_LINE_WINDOW_H
#define MX_GOTO_LINE_WINDOW_H

#include <wx/dialog.h>
class mxSource;
class wxTextCtrl;
class wxSlider;

class mxGotoLineWindow : public wxDialog {

public:

 	mxSource *source;
	wxTextCtrl *text_ctrl;
	wxSlider *slider;

	mxGotoLineWindow(mxSource *a_source, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);

	void OnGotoButton(wxCommandEvent &event);
	void OnCancelButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnScroll(wxScrollEvent &event);
	void OnTextChange(wxCommandEvent &event);

private:
	DECLARE_EVENT_TABLE()

};

#endif
