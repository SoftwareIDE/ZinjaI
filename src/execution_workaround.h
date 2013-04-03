#ifndef EXECUTION_WORKAROUND_H
#define EXECUTION_WORKAROUND_H
#include <wx/utils.h>

long mxExecute(wxString command, int sync, wxProcess *process=NULL);

long mxExecute(const wxString& command, wxArrayString& output, int flags = 0);

long mxExecute(const wxString& command, wxArrayString& output, wxArrayString& errors, int flags = 0);

#endif
