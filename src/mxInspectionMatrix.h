#ifndef MXINSPECTIONMATRIX_H
#define MXINSPECTIONMATRIX_H

#include <wx/panel.h>
#include "Inspection.h"
//#include <wx/grid.h>
//#include "mxExternInspection.h"
//
class wxGrid;
class DebuggerInspection;
//class wxCheckBox;
//class wxBoxSizer;
//

class mxInspectionMatrix : public wxPanel, public myDIEventHandler {
//private:
//	wxBoxSizer *mySizer;
	bool created;
	wxCheckBox *adapt;
	bool do_adapt;
//	bool is_class, is_cstring, dont_adapt, is_vo;
	int cols,old_size,rows;
	DebuggerInspection *di;
	wxGrid *grid;
//	int *cols_sizes;
public:
	mxInspectionMatrix(const wxString &expression, bool is_frameless);
	~mxInspectionMatrix();
	
	// eventos de una instancia individual de DebuggerInspection
	void OnDICreated(DebuggerInspection *di);
	void OnDIError(DebuggerInspection *di);
	void OnDIValueChanged(DebuggerInspection *di);
	void OnDIOutOfScope(DebuggerInspection *di);
	void OnDIInScope(DebuggerInspection *di);
	void OnDINewType(DebuggerInspection *di);
	
	void SetMatrixSize(int w, int h);
	void UpdateMatrix();
	
//	bool SetOutdated();
	void OnAdaptCheck(wxCommandEvent &evt);
	void OnResize(wxSizeEvent &evt);
//	void OnColResize(wxGridSizeEvent &evt);
//	void OnChar(wxKeyEvent &event);
////	void OnClose(wxCloseEvent &evt);
//	void AutoSizeRowLabel();
//	~mxInspectionMatrix();
	
	void CalculateColumnSizes();
	DECLARE_EVENT_TABLE();
};

#endif

