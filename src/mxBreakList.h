#ifndef MXBREAKLIST_H
#define MXBREAKLIST_H
#include <wx/dialog.h>
#include <wx/grid.h>
#include <vector>
using namespace std;

class wxGrid;

enum {/*BL_COL_NUM=0,*/BL_COL_TYPE=0,BL_COL_WHY,BL_COL_ENABLE,/*BL_COL_DISP,*/BL_COL_HIT,BL_COL_COND,BL_COL_ANNOTATION,BL_COLS_COUNT};

class mxBreakList : public wxDialog {
private:
	wxGrid *grid;
	int old_size;
	float cols_sizes[BL_COLS_COUNT];
	vector<int> ids; ///< zinjai id for every breakpoint in the table, or -1 for watchpoints
	friend class DebugManager;
public:
	mxBreakList();
	void PopulateGrid();
	void OnResize(wxSizeEvent &evt);
	void OnColResize(wxGridSizeEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnDoubleClick(wxGridEvent &evt);
	void OnCharHook(wxKeyEvent &evt);
	void OnDeleteAllButton(wxCommandEvent &evt);
	void OnDeleteButton(wxCommandEvent &evt);
	void RemoveBreakPoint(int _row);
	void OnGotoButton(wxCommandEvent &evt);
	void OnEditButton(wxCommandEvent &evt);
	int AppendRow(int _id);
	DECLARE_EVENT_TABLE();
};

#endif

