#ifndef MXBACKTRACEGRID_H
#define MXBACKTRACEGRID_H
#include <wx/grid.h>

enum {BG_COL_LEVEL = 0, /*BG_COL_ADDRESS,*/ BG_COL_FUNCTION, BG_COL_FILE ,BG_COL_LINE, BG_COL_ARGS, BG_COLS_COUNT};

class mxBacktraceGrid : public wxGrid {
private:
	DECLARE_EVENT_TABLE();
	int selected_row;
	int old_size;
	float cols_sizes[BG_COLS_COUNT];
	bool *cols_visibles;
	int cols_marginal;
	bool created;
public:
	mxBacktraceGrid(wxWindow *parent, wxWindowID id);
	~mxBacktraceGrid();
	void OnKey(wxKeyEvent &event);
	void OnDblClick(wxGridEvent &event);
	void OnRightClick(wxGridEvent &event);
	void OnExploreArgs(wxCommandEvent &event);
	void OnExploreLocals(wxCommandEvent &event);
	void OnInspectArgs(wxCommandEvent &event);
	void OnInspectLocals(wxCommandEvent &event);
	void OnGotoPos(wxCommandEvent &event);
	void OnAddToBlackList(wxCommandEvent &event);
	void OnResize(wxSizeEvent &evt);
	void OnColResize(wxGridSizeEvent &evt);
	void OnLabelPopup(wxGridEvent &evt);
	void OnShowHideCol(wxCommandEvent &evt);
};

#endif

