#ifndef MXGPROFOUTPUT_H
#define MXGPROFOUTPUT_H
#include <wx/dialog.h>
#include "GprofData.h"
#include <wx/grid.h>

class wxComboBox;
class wxTextCtrl;

class mxGprofOutput : public wxDialog {
private:
	GprofData data;
	wxGrid *grid_table,*grid_graph;
	wxTextCtrl *search_text;
	wxComboBox *combo;
public:
	mxGprofOutput(wxWindow *parent, wxString fname);
	void FillTable(const wxString &pattern="");
	void FillGraph();
	void OnClickTableLabel(wxGridEvent &event);
	void OnClickTableCell(wxGridEvent &event);
	void OnSearchTextChange(wxCommandEvent &event);
	void OnComboChange(wxCommandEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

