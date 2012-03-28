#include "Application.h"
#include "mxInfoWindow.h"
#include "ComplementArchive.h"
#include "mxCreateComplementWindow.h"
#include <iostream>
using namespace std;

bool spanish=false;

bool mxApplication::OnInit() {
	wxString zpath,fname;
	for(int i=1;i<argc;i++) { 
		wxString argvi(argv[i]);
		if (argvi.StartsWith("--lang=")) {
			spanish=argvi=="--lang=spanish";
		} else 
		if (zpath.Len()==0) zpath=argvi; else fname=argvi;
	}
#ifndef __WIN32__
	cerr<<(spanish?"\nNo cierre esta ventana.\n":"\nDo not close this window.\n");
#endif
	if (fname.Len())
		new mxInfoWindow(zpath,fname);
	else
		new mxCreateComplementWindow();
	return true;
}

IMPLEMENT_APP(mxApplication)

