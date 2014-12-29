#include "execution_workaround.h"
#include <iostream>
#include <wx/app.h>
using namespace std;

static bool someone_is_running=false;

long mxExecute(wxString command, int sync, wxProcess *process) {
#ifdef _ZINJAI_DEBUG
	cerr<<"execution_workaround: Enters, command="<<command<<endl;
	if (someone_is_running) cerr<<"execution_workaround: COLLISION!"<<endl;
#endif
	if (someone_is_running) return 0;
	someone_is_running=true;
	long ret=wxExecute(command,sync,process);
	someone_is_running=false;	
#ifdef _ZINJAI_DEBUG
	cerr<<"execution_workaround: Exits, command="<<command<<endl;
	cerr<<"execution_workaround: Exits, retval="<<ret<<endl;
#endif
	return ret;
}
long mxExecute(const wxString& command, wxArrayString& output, int flags) {
#ifdef _ZINJAI_DEBUG
	cerr<<"execution_workaround: Enters, command="<<command<<endl;
	if (someone_is_running) cerr<<"execution_workaround: COLLISION!"<<endl;
#endif
	if (someone_is_running) return 0;
	someone_is_running=true;
	long ret=wxExecute(command,output,flags);
	someone_is_running=false;
#ifdef _ZINJAI_DEBUG
	cerr<<"execution_workaround: Exits, command="<<command<<endl;
	cerr<<"execution_workaround: Exits, retval="<<ret<<endl;
#endif
	return ret;
}

long mxExecute(const wxString& command, wxArrayString& output, wxArrayString& errors, int flags) {
#ifdef _ZINJAI_DEBUG
	cerr<<"execution_workaround: Enters, command="<<command<<endl;
	if (someone_is_running) cerr<<"execution_workaround: COLLISION!"<<endl;
#else
	if (someone_is_running) return 0;
#endif
	someone_is_running=true;
	long ret=wxExecute(command,output,errors,flags);
	someone_is_running=false;
#ifdef _ZINJAI_DEBUG
	cerr<<"execution_workaround: Exits, command="<<command<<endl;
	cerr<<"execution_workaround: Exits, retval="<<ret<<endl;
#endif
	return ret;
}


