#ifndef MX_APPLICATION_H
#define MX_APPLICATION_H

#include <wx/app.h>

extern bool spanish;

class mxApplication : public wxApp {
public:
	virtual bool OnInit();
};

#endif
