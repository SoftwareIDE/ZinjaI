#ifndef MXBACKTRACEGRID_H
#define MXBACKTRACEGRID_H
#include "mxGrid.h"
#include "SingleList.h"

enum {BG_COL_LEVEL = 0, BG_COL_FUNCTION, BG_COL_FILE ,BG_COL_LINE, BG_COL_ARGS, BG_COLS_COUNT};

class mxBacktraceGrid : public mxGrid {
private:
	DECLARE_EVENT_TABLE();
	int selected_row;
	struct aux_bt_entry { wxString fname,line; };
	SingleList<aux_bt_entry> entries; ///< to remember file and line column for doble-click event even if the columns are hidden
public:
	mxBacktraceGrid(wxWindow *parent);
//	~mxBacktraceGrid();
//	void OnKey(wxKeyEvent &event);
//	void OnDblClick(wxGridEvent &event);
//	void OnRightClick(wxGridEvent &event);
	void OnExploreArgs(wxCommandEvent &event);
	void OnExploreLocals(wxCommandEvent &event);
	void OnInspectArgs(wxCommandEvent &event);
	void OnInspectLocals(wxCommandEvent &event);
	void OnGotoPos(wxCommandEvent &event);
	void SelectFrame(int r);
	void OnAddFunctionToBlackList(wxCommandEvent &event);
	void OnAddFileToBlackList(wxCommandEvent &event);
	void AddToBlackList(const wxString &type,const wxString &what);
	void SetCellValue(int r, int c, const wxString &value);
	void OnColumnHideOrUnhide(int col, bool visible);
	void OnCellPopupMenu(int row, int col);
//	bool OnCellClick(int row, int col);
	bool OnCellDoubleClick(int row, int col);
//	bool CanHideColumn(int col);
	bool OnKey(int row, int col, int key, int modifiers);
};

#endif

