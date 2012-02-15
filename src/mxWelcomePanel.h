#ifndef MXWELCOMEPANEL_H
#define MXWELCOMEPANEL_H
#include <wx/html/htmlwin.h>
#include <wx/textfile.h>

class mxWelcomePanel : public wxHtmlWindow {
private:
	wxTextFile file;
public:
	mxWelcomePanel(wxWindow *parent);
	~mxWelcomePanel();
	void Reload();
	void OnLinkClicked (wxHtmlLinkEvent &event);
	DECLARE_EVENT_TABLE();
};

extern mxWelcomePanel *welcome_panel;

#endif

