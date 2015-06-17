#ifndef MXTHREADGRID_H
#define MXTHREADGRID_H
#include "mxGrid.h"

enum {TG_COL_ID = 0, TG_COL_FUNCTION, TG_COL_FILE ,TG_COL_LINE, TG_COLS_COUNT};

class mxThreadGrid : public mxGrid {
private:
	int selected_row;
public:
	mxThreadGrid(wxWindow *parent);
	void OnKey(wxKeyEvent &event);
	void SetData(int row, wxString id, wxString func, wxString file, wxString line);
	void SetNumber(int nrows);
	
	void OnColumnHideOrUnhide(int col, bool visible);
	bool OnCellDoubleClick(int row, int col);
	bool OnKey(int row, int col, int key, int modifiers);
};

#endif

