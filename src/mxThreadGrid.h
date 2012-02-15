#ifndef MXTHREADGRID_H
#define MXTHREADGRID_H
#include <wx/grid.h>

enum {TG_COL_ID = 0, TG_COL_FUNCTION, TG_COL_FILE ,TG_COL_LINE, TG_COLS_COUNT};

class mxThreadGrid : public wxGrid {
private:
	DECLARE_EVENT_TABLE();
	int selected_row;
	int old_size;
	float cols_sizes[TG_COLS_COUNT];
//	bool *cols_visibles;
	int cols_marginal;
	bool created;
public:
	mxThreadGrid(wxWindow *parent, wxWindowID id);
//	void AddRow();
//	~mxThreadGrid();
	void OnKey(wxKeyEvent &event);
	void OnDblClick(wxGridEvent &event);
	void SetData(int row, wxString id, wxString func, wxString file, wxString line);
	void SetNumber(int nrows);
//	void OnRightClick(wxGridEvent &event);
//	void OnExploreArgs(wxCommandEvent &event);
//	void OnExploreLocals(wxCommandEvent &event);
//	void OnInspectArgs(wxCommandEvent &event);
//	void OnInspectLocals(wxCommandEvent &event);
//	void OnGotoPos(wxCommandEvent &event);
//	void OnAddToBlackList(wxCommandEvent &event);
	void OnResize(wxSizeEvent &evt);
	void OnColResize(wxGridSizeEvent &evt);
//	void OnLabelPopup(wxGridEvent &evt);
//	void OnShowHideCol(wxCommandEvent &evt);
};

#endif

