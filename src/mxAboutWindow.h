#ifndef MX_ABOUT_WINDOW_H
#define MX_ABOUT_WINDOW_H

#include <wx/dialog.h>
#include <wx/html/htmlwin.h>

class wxHtmlWindow;

class mxAboutWindow : public wxDialog {

public:
	mxAboutWindow(wxWindow* parent);
	void OnCloseButton(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnLink (wxHtmlLinkEvent &event);
	wxHtmlWindow *html;

private:
	DECLARE_EVENT_TABLE()

};

#endif
