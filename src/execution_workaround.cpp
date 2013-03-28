#include "execution_workaround.h"
#include <iostream>
#include <wx/app.h>
using namespace std;

volatile static int count=0;

static void ExecutionWAControl(bool starts) {
	if (starts) { 
		cerr<<"Enters, waiting: count="<<count<<endl;
		while (count) 
			cerr<<wxApp::GetInstance()->Yield(true)<<endl;
		count++;
		cerr<<"Enters, proceed: count="<<count<<endl;
	}
	else {
		count--;
		cerr<<"Exits: count="<<count<<endl;
	}
}

long mxExecute(wxString command, int sync, wxProcess *process) {
	ExecutionWAControl(true);
	long ret=wxExecute(command,sync,process);
	ExecutionWAControl(false);
	return ret;
}
long mxExecute(const wxString& command, wxArrayString& output, int flags) {
	ExecutionWAControl(true);
	long ret=wxExecute(command,output,flags);
	ExecutionWAControl(false);
	return ret;
}

long mxExecute(const wxString& command, wxArrayString& output, wxArrayString& errors, int flags) {
	ExecutionWAControl(true);
	long ret=wxExecute(command,output,errors,flags);
	ExecutionWAControl(false);
	return ret;
}


