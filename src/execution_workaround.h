#ifndef EXECUTION_WORKAROUND_H
#define EXECUTION_WORKAROUND_H
#include <wx/utils.h>
#include "Cpp11.h"

long mxExecute(wxString command, int sync, wxProcess *process=nullptr);

long mxExecute(wxString command, wxArrayString& output, int flags = 0);

long mxExecute(wxString command, wxArrayString& output, wxArrayString& errors, int flags = 0);

void fix_command_for_wxexecute(wxString &command);

#endif
