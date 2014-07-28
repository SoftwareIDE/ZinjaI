#ifndef MXGRID_H
#define MXGRID_H
#include <wx/grid.h>
#include <vector>
#include "SingleList.h"
using namespace std;

class mxGrid : public wxGrid {
private:
	struct mxGridCol {
		wxString name; ///< column label
		int orig_pos; ///< column original position (as hardcoded in inheritance constructor, assuming all columns visible)
		int real_pos; ///< column real position (may change when user hides other columns), or -1 if is hidden
		float width; ///< column width in pixels, for reescaling
	};
	SingleList<mxGridCol> cols;
	void RecalcColumns(int new_w);
	
	void OnResize(wxSizeEvent &event);
//	void OnColResize(wxGridEvent &evt);
	void OnDblClick(wxGridEvent &event);
	void OnRightClick(wxGridEvent &event);
	void OnLabelPopup(wxGridEvent &event);
	
protected:
	void InitColumn(int col_idx, wxString name, int width/*, bool visible=true*/); 
	void DoCreate(); 
public:
	mxGrid(wxWindow *parent, int number_of_cols, wxWindowID id=wxID_ANY);
	
	void SetRowSelectionMode();
		
	void SetColumnVisible(int c, bool visible);
	bool IsColumnVisible(int c);
	
	void Select(int row, int col=-1);
	int GetSelectedRows(vector<int> &rows, bool inverted=false);
	
		
	/// @brief callback that will be called when user unhides a column so child class can update its content
	virtual void OnColumnUnhide(int c) {}
	
	virtual void OnCellPopupMenu(int row, int col) {}
	virtual void OnCellDoubleClick(int row, int col) {}
	virtual void OnLabelPopupMenu(int col) {}
	int GetRealCol(int col) { return cols[col].real_pos; }
	
	// wrappers for wxGrid methods, that considers column hidding
	void SetCellValue(int r, int c, const wxString &value) {
		int rc=cols[c].real_pos;
		if (rc!=-1) wxGrid::SetCellValue(r,rc,value);
	}
	wxString GetCellValue(int r, int c) {
		int rc=cols[c].real_pos;
		if (rc==-1) return "";
		return wxGrid::GetCellValue(r,rc);
	}
	void SetReadOnly(int r, int c, bool is_read_only) {
		if (cols[c].real_pos==-1) return;
		wxGrid::SetReadOnly(r,cols[c].real_pos,is_read_only);
	}
	void SetCellColour(int r, int c, const wxColour &color) {
		if (cols[c].real_pos==-1) return;
		wxGrid::SetCellTextColour(r,cols[c].real_pos,color);
	}
	void SetCellEditor(int r, int c, wxGridCellEditor *editor) {
		if (cols[c].real_pos==-1) return;
		wxGrid::SetCellEditor(r,cols[c].real_pos,editor);
	}
//	~mxGrid();
};

#endif

