#ifndef MX_APPLICATION_H
#define MX_APPLICATION_H

#include <wx/app.h>
#include <wx/splash.h>
#include <wx/longlong.h>

#ifdef DEBUG
extern wxLongLong start_time;
extern wxLongLong aux_start_time;
#define SHOW_MILLIS(a) \
	cerr<<((aux_start_time=wxGetLocalTimeMillis())-start_time).ToLong()<<"   "<<a<<endl;\
	start_time=aux_start_time
#else
#define SHOW_MILLIS(a)
#endif

class mxApplication : public wxApp {
public:
	virtual bool OnInit();
};

extern mxApplication *app;

#endif
