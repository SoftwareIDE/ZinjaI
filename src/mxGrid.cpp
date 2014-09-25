#include "mxGrid.h"
#include <algorithm>
#include <wx/menu.h>
#include "ids.h"
#include "Language.h"
#include "mxUtils.h"
#include <wx/msgdlg.h>
using namespace std;

mxGrid::mxGrid(wxWindow *parent, int number_of_cols, wxWindowID id, wxSize sz) : wxGrid(parent,id,wxDefaultPosition,sz,wxWANTS_CHARS), cols(number_of_cols) {
	created=false;
	CreateGrid(0,number_of_cols);
	SetColLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	SetRowLabelSize(0);
}

void mxGrid::SetColumnVisible (int c, bool visible) {
	if ( (cols[c].real_pos!=-1)==visible ) return;
	wxGrid::Freeze();
	if (visible) {
		// find its real col position
		int p=0; 
		for(int i=0;i<c;i++) if (cols[i].real_pos!=-1) p++;
		cols[c].real_pos=p;
		// insert the column
		InsertCols(p,1,true);
		wxGrid::SetColSize(p,cols[c].width);
		// update the following column's real positions
		for(int i=c+1;i<cols.GetSize();i++)
			if (cols[i].real_pos!=-1) cols[i].real_pos++;
		// fill cells in that column
	} else {
		DeleteCols(cols[c].real_pos,1,false);
		cols[c].real_pos=-1;
		for(int i=c+1;i<cols.GetSize();i++)
			if (cols[i].real_pos!=-1) cols[i].real_pos--;
	}
	for(int i=0;i<cols.GetSize();i++) 
		if (cols[i].real_pos!=-1)
			SetColLabelValue(cols[i].real_pos,cols[i].name);
	RecalcColumns(GetSize().GetWidth());
	if (created) OnColumnHideOrUnhide(c,visible);
	wxGrid::Thaw();
}

bool mxGrid::IsColumnVisible (int c) {
	return cols[c].real_pos!=-1;
}

void mxGrid::RecalcColumns(int new_w) {
	if (new_w==0) return;
	BeginBatch();
	float old_w=0;
	for(int i=0;i<cols.GetSize();i++) 
		if (cols[i].real_pos!=-1) old_w+=cols[i].width;
	float ratio=new_w/old_w;
	for(int i=0;i<cols.GetSize();i++) {
		cols[i].width*=ratio;
		if (cols[i].real_pos!=-1) {
			SetColSize(cols[i].real_pos,int(cols[i].width));
		}
	}
	EndBatch();
	Refresh();
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
	Connect(wxEVT_GRID_COL_SIZE,wxGridSizeEventHandler(mxGrid::OnColResize),NULL,this);
	Connect(wxEVT_GRID_CELL_LEFT_CLICK,wxGridEventHandler(mxGrid::OnLeftClick),NULL,this);
	Connect(wxEVT_GRID_CELL_LEFT_DCLICK,wxGridEventHandler(mxGrid::OnDblClick),NULL,this);
	Connect(wxEVT_GRID_CELL_RIGHT_CLICK,wxGridEventHandler(mxGrid::OnRightClick),NULL,this);
	Connect(wxEVT_GRID_LABEL_RIGHT_CLICK,wxGridEventHandler(mxGrid::OnLabelPopup),NULL,this);
	Connect(wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(mxGrid::OnShowHideCol),NULL,this);
	Connect(wxEVT_KEY_DOWN,wxKeyEventHandler(mxGrid::OnKey),NULL,this);
	created=true;
}

void mxGrid::OnColResize (wxGridSizeEvent & event) {
	cols[GetRealCol(event.GetRowOrCol())].width=GetColSize(event.GetRowOrCol());
	event.Skip();
}
void mxGrid::OnResize (wxSizeEvent & event) {
	cerr<<"OnResize: "<<endl;
	RecalcColumns(event.GetSize().GetWidth());
	event.Skip();
}

void mxGrid::OnDblClick (wxGridEvent & event) {
	last_event_x=event.GetPosition().x; last_event_y=event.GetPosition().y;
	if (!OnCellDoubleClick(event.GetRow(),event.GetCol())) event.Skip();
}

void mxGrid::OnRightClick (wxGridEvent & event) {
	last_event_x=event.GetPosition().x; last_event_y=event.GetPosition().y;
	OnCellPopupMenu(event.GetRow(),event.GetCol());
}

void mxGrid::OnLeftClick (wxGridEvent & event) {
	last_event_x=event.GetPosition().x; last_event_y=event.GetPosition().y;
	if (!OnCellClick(event.GetRow(),event.GetCol())) event.Skip();
}

void mxGrid::OnLabelPopup (wxGridEvent & event) {
	wxMenu menu;
	for(int i=0;i<cols.GetSize();i++)
		if (CanHideColumn(i))
			menu.AppendCheckItem(mxID_COL_ID+i,LANG1(MXGRID_SHOW_COL,"Mostrar Columna \"<{1}>\"",cols[i].name))
				->Check(mxGrid::IsColumnVisible(i));
	PopupMenu(&menu);
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

void mxGrid::OnShowHideCol(wxCommandEvent &evt) {
	int cn=evt.GetId()-mxID_COL_ID;
	if (cn<0||cn>=cols.GetSize()) { evt.Skip(); return; }
	mxGrid::SetColumnVisible(cn,!mxGrid::IsColumnVisible(cn));
	Refresh();
}

void mxGrid::OnKey (wxKeyEvent & event) {
	int key = event.GetKeyCode();
	int r = GetGridCursorRow();
	if (key==WXK_DOWN) {
		if (r+1!=GetNumberRows()) mxGrid::Select(r+1);
	} else if (event.GetKeyCode()==WXK_UP) {
		if (r) mxGrid::Select(r-1);
	} else if (!OnKey(r,GetGridCursorCol(),key,event.GetModifiers()))
		event.Skip();
}

