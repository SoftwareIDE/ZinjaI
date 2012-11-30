#ifndef MXINSPECTIONMATRIX_H
#define MXINSPECTIONMATRIX_H
#include <wx/grid.h>
#include "mxExternInspection.h"

class wxGrid;
class wxCheckBox;
class wxBoxSizer;

class mxInspectionMatrix : public mxExternInspection {
private:
	wxBoxSizer *mySizer;
	bool created;
	friend class DebugManager;
	wxCheckBox *adapt;
	bool is_class, is_cstring, dont_adapt, is_vo;
	int cols,old_size,rows;
	wxGrid *grid;
	int *cols_sizes;
public:
	mxInspectionMatrix(int inspection_num);
	void Update();
	bool SetOutdated();
	void OnAdaptCheck(wxCommandEvent &evt);
	void OnResize(wxSizeEvent &evt);
	void OnColResize(wxGridSizeEvent &evt);
//	void OnClose(wxCloseEvent &evt);
	void AutoSizeRowLabel();
	~mxInspectionMatrix();
	DECLARE_EVENT_TABLE();
};

#endif

