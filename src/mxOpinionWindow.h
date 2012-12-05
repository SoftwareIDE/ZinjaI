#ifndef MXOPINIONWINDOW_H
#define MXOPINIONWINDOW_H
#include <wx/dialog.h>
#include <wx/socket.h>

class wxTextCtrl;
class mxOSD;
class mxBitmapButton;

class mxOpinionWindow : public wxDialog {
private:
	long port;
	wxString proxy;
	wxTextCtrl *text;
	wxSocketClient *the_socket;
	mxOSD *osd;
	mxBitmapButton *send_button;
public:
	mxOpinionWindow(wxWindow *parent);
	void OnSocketEvent(wxSocketEvent &event);
	void OnClose(wxCloseEvent &evt);
	void OnProxyButton(wxCommandEvent &evt);
	void OnOkButton(wxCommandEvent &evt);
	void OnCloseButton(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

