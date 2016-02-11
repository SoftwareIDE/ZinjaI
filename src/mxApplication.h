#ifndef MX_APPLICATION_H
#define MX_APPLICATION_H

#include <wx/app.h>

#ifdef _TIMED_INIT
extern wxLongLong start_time;
extern wxLongLong aux_start_time;
#define SHOW_MILLIS(a) \
	cerr<<"TIMED INIT: "<<((aux_start_time=wxGetLocalTimeMillis())-start_time).ToLong()<<"   "<<a<<endl;\
	start_time=aux_start_time
#else
#define SHOW_MILLIS(a)
#endif

class mxApplication : public wxApp {
	void SelectLanguage();
	bool InitSingleton(const wxString &cmd_path);
	void ShowSplash();
	void LoadFilesOrWelcomePanel(const wxString &cmd_path);
public:
	virtual bool OnInit();
};

extern mxApplication *g_application;

#endif
