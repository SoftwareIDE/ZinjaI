#ifndef MXGRID_H
#define MXGRID_H
#include <wx/grid.h>
#include "SingleList.h"

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
	void SetCellValue(int r, int c, const wxString &value) {
		int rc=cols[c].real_pos;
		if (rc!=-1) wxGrid::SetCellValue(r,rc,value);
	}
	wxString GetCellValue(int r, int c) {
		int rc=cols[c].real_pos;
		if (rc==-1) return "";
		return wxGrid::GetCellValue(r,rc);
	}
	void SetColumnVisible(int c, bool visible);
	bool IsColumnVisible(int c);
	
	/// @brief callback that will be called when user unhides a column so child class can update its content
	virtual void OnColumnUnhide(int c) {}
	
	virtual void OnCellPopupMenu(int row, int col) {}
	virtual void OnCellDoubleClick(int row, int col) {}
	virtual void OnLabelPopupMenu(int col) {}
//	~mxGrid();
};

#endif

