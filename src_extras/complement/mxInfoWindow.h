#ifndef MXINFOWINDOW_H
#define MXINFOWINDOW_H
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include "ComplementArchive.h"

class mxInfoWindow : public wxFrame {
private:
	complement_info info;
	wxString file,dest; int step;
	wxTextCtrl *text;
	wxButton *but_ok;
	wxButton *but_cancel;
	friend bool callback_info(wxString message);
protected:
public:
	mxInfoWindow(wxString _dest, wxString _file);
	void OnButtonOk(wxCommandEvent &evt);
	void OnButtonCancel(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void Notify(const wxString &message);
	DECLARE_EVENT_TABLE();
};

#endif

