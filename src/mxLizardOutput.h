#ifndef MXLIZARDOUTPUT_H
#define MXLIZARDOUTPUT_H
#include <wx/dialog.h>
#include "mxPlainResultsTable.h"
class wxNotebook;

class mxLizardOutput : public wxDialog {
	wxNotebook *notebook;
	mxPlainResultsTable *functions_table;
	mxPlainResultsTable *files_table;
	void ParseResults(wxArrayString &results);
public:
	mxLizardOutput(wxWindow *parent, wxArrayString &results);
	DECLARE_EVENT_TABLE();
};

#endif

