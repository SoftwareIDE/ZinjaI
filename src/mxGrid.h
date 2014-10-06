#ifndef MXGRID_H
#define MXGRID_H
#include <wx/grid.h>
#include <vector>
#include "SingleList.h"
#include <wx/dc.h>
using namespace std;

class mxGridCellRenderer : public wxGridCellStringRenderer {
	bool show_icon;
public:
	mxGridCellRenderer():show_icon(false) {}
	void SetIconNull() { show_icon=false; }
	void SetIconPlus() { show_icon=true; }
	virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
		if (!show_icon) {
			wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
		} else {
			wxRect rect2=rect; 
			rect2.x+=20; rect2.width-=20;
			wxGridCellStringRenderer::Draw(grid, attr, dc, rect2, row, col, isSelected);
			rect2.x-=20; rect2.width=20;
			dc.DrawRectangle(rect2);
			dc.SetPen(*wxMEDIUM_GREY_PEN);
			rect2.x+=3;
			rect2.y+=(rect2.height-14)/2;
			rect2.width=rect2.height=13;
			dc.DrawRectangle(rect2);
			dc.DrawLine(rect2.x+2,rect2.y+rect2.height/2,rect2.x+rect2.width-2,rect2.y+rect2.height/2);
			dc.DrawLine(rect2.x+rect2.width/2,rect2.y+2,rect2.x+rect2.width/2,rect2.y+rect2.height-2);
		}
	}
	bool HasIcon() { return show_icon; }
};

class mxGrid : public wxGrid {
private:
	bool created; ///< for masking some events until DoCreate
	struct mxGridCol {
		wxString name; ///< column label
		int orig_pos; ///< column original position (as hardcoded in inheritance constructor, assuming all columns visible)
		int real_pos; ///< column real position (may change when user hides other columns), or -1 if is hidden
		float width; ///< column width in pixels, for reescaling
	};
	SingleList<mxGridCol> cols;
	void RecalcColumns(int new_w);
	
	void OnResize(wxSizeEvent &event);
	void OnColResize(wxGridSizeEvent &evt);
	void OnLeftClick(wxGridEvent &event);
	void OnRightClick(wxGridEvent &event);
	void OnDblClick(wxGridEvent &event);
	void OnLabelPopup(wxGridEvent &event);
	void OnShowHideCol(wxCommandEvent &evt);
	void OnKeyDown(wxKeyEvent &event);
protected:
	void InitColumn(int col_idx, wxString name, int width/*, bool visible=true*/); 
	void DoCreate(); 
	int last_event_x, last_event_y;
public:
	mxGrid(wxWindow *parent, int number_of_cols, wxWindowID id=wxID_ANY, wxSize sz=wxDefaultSize);
	
	void SetRowSelectionMode();
		
	void SetColumnVisible(int c, bool visible);
	bool IsColumnVisible(int c);
	
	void Select(int row, int col=-1);
	int GetSelectedRows(vector<int> &rows, bool inverted=false);
	
		
	/// @brief callback that will be called when user unhides a column so child class can update its content
	virtual void OnColumnHideOrUnhide(int col, bool visible) {}
	virtual bool CanHideColumn(int col) { return true; }
	
	/// @brief cell right click event
	virtual void OnCellPopupMenu(int row, int col) {}
	/// @brief cell left click event
	virtual bool OnCellClick(int row, int col) { return false; }
	/// @brief cell double left click event
	virtual bool OnCellDoubleClick(int row, int col) { return false; }
	/// @brief key pressed
	virtual bool OnKey(int row, int col, int key, int modifiers) { return false; }
	
	int GetRealCol(int col) { return cols[col].real_pos; }
	
	// wrappers for wxGrid methods, that considers column hidding
	void SetCellValue(int r, int c, const wxString &value) {
		int rc=cols[c].real_pos;
		if (rc!=-1) wxGrid::SetCellValue(r,rc,value);
	}
	bool SetGridCursor (int r, int c) {
		int rc=cols[c].real_pos;
		if (rc==-1) return false;
		wxGrid::SetGridCursor(r,rc);
		return true;
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
	void SetCellRenderer(int r, int c, wxGridCellRenderer *renderer) {
		if (cols[c].real_pos==-1) return;
		wxGrid::SetCellRenderer(r,cols[c].real_pos,renderer);
	}
	int GetColLeft(int c) {
		return wxGrid::GetColLeft(cols[c].real_pos);
	}
	
	// other useful grid functions
	void CopyToClipboard(bool only_selected, int col=-1);
//	~mxGrid();
};

#endif

