#include "mxGrid.h"

mxGrid::mxGrid(int number_of_cols) {
	if (number_of_cols) cols.Reserve(number_of_cols);
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

void mxGrid::AddColumn (wxString name, int width, bool visible) {
	mxGridCol gc;
	gc.name=name;
	gc.width=width;
	gc.orig_pos=cols.GetSize();
	gc.real_pos=visible?1:-1;
	cols.Add(gc);
}


void mxGrid::DoCreate ( ) {
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
	OnCellDoubleClick(event.GetRow(),event.GetCol());
}

void mxGrid::OnRightClick (wxGridEvent & event) {
	OnCellPopupMenu(event.GetRow(),event.GetCol());
}

void mxGrid::OnLabelPopup (wxGridEvent & event) {
	OnLabelPopupMenu(event.GetCol());
}

