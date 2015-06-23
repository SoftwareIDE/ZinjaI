#ifndef MXGPROFOUTPUT_H
#define MXGPROFOUTPUT_H
#include <wx/dialog.h>
#include "GprofData.h"
#include <wx/grid.h>
#include "raii.h"

class wxComboBox;
class wxTextCtrl;
class wxNotebook;
class mxPlainResultsTable;

class mxGprofOutput : public wxDialog {
private:
	BoolFlag mask_combo_events;
	GprofData data;
	mxPlainResultsTable *grid_table, *grid_graph;
	wxComboBox *combo;
	wxNotebook *notebook;
public:
	mxGprofOutput(wxWindow *parent, wxString fname);
	void OnComboChange(wxCommandEvent &event);
	void ShowFunction(wxString func);
	void ShowFunction(int i);
	DECLARE_EVENT_TABLE();
};

#endif

