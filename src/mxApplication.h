#ifndef MX_APPLICATION_H
#define MX_APPLICATION_H

#include <wx/app.h>

class mxApplication : public wxApp {
	void SelectLanguage();
	bool InitSingleton(const wxString &cmd_path);
	void ShowSplash();
	void LoadFilesOrWelcomePanel(const wxString &cmd_path);
public:
	virtual bool OnInit();
};

extern mxApplication *app;

#endif
