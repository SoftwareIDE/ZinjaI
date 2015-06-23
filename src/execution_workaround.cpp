#include "execution_workaround.h"
#include <iostream>
#include <wx/app.h>
#include "ConfigManager.h"
using namespace std;

static bool someone_is_running=false;

void fix_command_for_wxexecute(wxString &command) {
#ifndef __WIN32__
	// por alguna razon, con wx 2.8 no podemos tener mas de 127 argumentos (ver WXEXECUTE_NARGS en los fuentes de wx 2.8)
	// asi que hacemos un script con el comando y llamamos al script si ese es el caso
	int num_args=0, i=0,l=command.Len();
	while (++i<l) {
		if (command[i]=='\'') { while (++i<l && command[i]!='\''); }
		else if (command[i]=='\"') { while (++i<l && command[i]!='\'') if (command[i]=='\\') i++; }
		else if (command[i]=='\\') i++;
		else if (command[i]==' ') num_args++;
	}
	if (num_args>=127) {
		wxString aux_file(DIR_PLUS_FILE(config->temp_dir,"long_command"));
		wxTextFile file(aux_file); 
		file.Create();
		file.AddLine(command);
		file.Write();
		file.Close();
		command=wxString("sh ")+aux_file;
	}
#endif
}

long mxExecute(wxString command, int sync, wxProcess *process) {
	fix_command_for_wxexecute(command);
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
long mxExecute(wxString command, wxArrayString& output, int flags) {
	fix_command_for_wxexecute(command);
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

long mxExecute(wxString command, wxArrayString& output, wxArrayString& errors, int flags) {
	fix_command_for_wxexecute(command);
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


