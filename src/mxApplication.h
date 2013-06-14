#ifndef MX_APPLICATION_H
#define MX_APPLICATION_H

#include <wx/app.h>

class mxApplication : public wxApp {
public:
	virtual bool OnInit();
};

extern mxApplication *app;

#endif
