#include "mxGrid.h"
#include <algorithm>
using namespace std;

mxGrid::mxGrid(wxWindow *parent, int number_of_cols, wxWindowID id) : wxGrid(parent,id,wxDefaultPosition,wxSize(400,300),wxWANTS_CHARS), cols(number_of_cols) {
	CreateGrid(0,number_of_cols);
	SetColLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	SetRowLabelSize(0);
}

void mxGrid::SetColumnVisible (int c, bool visible) {
	if ( (cols[c].real_pos!=-1)==visible ) return;
	if (visible) {
		int p=0; 
		for(int i=0;i<p;i++)
			if (cols[i].real_pos!=-1) p++;
		for(int i=p+1;i<cols.GetSize();i++)
			if (cols[i].real_pos!=-1) cols[i].real_pos++;
		InsertCols(p);
		OnColumnUnhide(c);
	} else {
		DeleteCols(cols[c].real_pos);
		cols[c].real_pos=-1;
	}
}

bool mxGrid::IsColumnVisible (int c) {
	return cols[c].real_pos!=-1;
}

void mxGrid::RecalcColumns(int new_w) {
	if (new_w==0) return;
	float old_w=0;
	for(int i=0;i<cols.GetSize();i++) 
		if (cols[i].real_pos!=-1) old_w+=cols[i].width;
	float ratio=new_w/old_w;
	for(int i=0;i<cols.GetSize();i++) {
		if (cols[i].real_pos!=-1) {
			cols[i].width*=ratio;
			SetColSize(cols[i].real_pos,int(cols[i].width));
		}
	}
}

void mxGrid::InitColumn (int col_idx, wxString name, int width/*, bool visible*/) {
	mxGridCol &gc=cols[col_idx];
	gc.name =name;
	gc.width = width;
	gc.orig_pos = col_idx;
	gc.real_pos = col_idx;
//	gc.real_pos = visible?1:-1;
	SetColLabelValue(col_idx,name);
	SetColSize(col_idx,width);
}


void mxGrid::DoCreate ( ) {
	RecalcColumns(GetSize().GetWidth());
	Connect(wxEVT_SIZE,wxSizeEventHandler(mxGrid::OnResize),NULL,this);
//	Connect(wxEVT_GRID_COL_SIZE,wxGridEventHandler(mxGrid::OnColResize),NULL,this);
	Connect(wxEVT_GRID_CELL_LEFT_DCLICK,wxGridEventHandler(mxGrid::OnDblClick),NULL,this);
	Connect(wxEVT_GRID_CELL_RIGHT_CLICK,wxGridEventHandler(mxGrid::OnRightClick),NULL,this);
	Connect(wxEVT_GRID_LABEL_RIGHT_CLICK,wxGridEventHandler(mxGrid::OnLabelPopup),NULL,this);
}

void mxGrid::OnResize (wxSizeEvent & event) {
	RecalcColumns(event.GetSize().GetWidth());
	event.Skip();
}

void mxGrid::OnDblClick (wxGridEvent & event) {
	if (!OnCellDoubleClick(event.GetRow(),event.GetCol())) event.Skip();
}

void mxGrid::OnRightClick (wxGridEvent & event) {
	OnCellPopupMenu(event.GetRow(),event.GetCol());
}

void mxGrid::OnLabelPopup (wxGridEvent & event) {
	OnLabelPopupMenu(event.GetCol());
}

void mxGrid::SetRowSelectionMode ( ) {
	SetSelectionMode(wxGrid::wxGridSelectRows);
	SetCellHighlightPenWidth(0);
}

void mxGrid::Select(int row, int col) {
	if (col==-1) col=GetGridCursorCol();
	SelectRow(row);
	SetGridCursor(row,col);
	MakeCellVisible(row,col);
}

int mxGrid::GetSelectedRows (vector<int> & rows, bool inverted) {
	wxArrayInt sr = wxGrid::GetSelectedRows();
	for(unsigned int i=0;i<sr.GetCount();i++) rows.push_back(sr[i]);
	wxGridCellCoordsArray c = wxGrid::GetSelectedCells();
	for(unsigned int i=0;i<c.GetCount();i++) rows.push_back(c[i].GetRow());
	wxGridCellCoordsArray tl = GetSelectionBlockTopLeft();
	wxGridCellCoordsArray br = wxGrid::GetSelectionBlockBottomRight();
	for(unsigned int i=0;i<tl.GetCount();i++) 
		for(int j=tl[i].GetRow();j<=br[i].GetRow();j++) rows.push_back(j);
	sort(rows.begin(),rows.end());
	rows.erase(unique(rows.begin(),rows.end()),rows.end());
	if (inverted) reverse(rows.begin(),rows.end());
	return rows.size();
}

