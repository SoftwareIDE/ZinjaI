#ifndef MXBREAKLIST_H
#define MXBREAKLIST_H
#include <wx/dialog.h>
#include <wx/grid.h>

class wxGrid;

enum {BL_COL_NUM=0,BL_COL_TYPE,BL_COL_WHY,BL_COL_ENABLE,/*BL_COL_DISP,*/BL_COL_HIT,BL_COL_COND,BL_COLS_COUNT};

class mxBreakList : public wxDialog {
private:
	wxGrid *grid;
	int old_size;
	float cols_sizes[BL_COLS_COUNT];
public:
	mxBreakList();
	void UpdateList(bool clear_first=true);
	void OnResize(wxSizeEvent &evt);
	void OnColResize(wxGridSizeEvent &evt);
	void OnClose(wxCloseEvent &evt);
	void OnDoubleClick(wxGridEvent &evt);
	void OnCharHook(wxKeyEvent &evt);
	void OnDeleteAllButton(wxCommandEvent &evt);
	void OnDeleteButton(wxCommandEvent &evt);
	void OnGotoButton(wxCommandEvent &evt);
	void OnEditButton(wxCommandEvent &evt);
	void AddErrorBreaks();
	DECLARE_EVENT_TABLE();
};

#endif

