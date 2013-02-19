#ifndef MXGPROFOUTPUT_H
#define MXGPROFOUTPUT_H
#include <wx/dialog.h>
#include "GprofData.h"
#include <wx/grid.h>

class wxComboBox;

class mxGprofOutput : public wxDialog {
private:
	GprofData data;
	wxGrid *grid_table,*grid_graph;
	wxComboBox *combo;
public:
	mxGprofOutput(wxWindow *parent, wxString fname);
	void FillTable();
	void FillGraph();
	void OnClickTable(wxGridEvent &event);
	void OnComboChange(wxCommandEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

