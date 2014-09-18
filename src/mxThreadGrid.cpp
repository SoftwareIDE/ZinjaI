#include "mxThreadGrid.h"
#include "DebugManager.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxThreadGrid, wxGrid)
	EVT_KEY_DOWN(mxThreadGrid::OnKey)
//	EVT_GRID_CELL_LEFT_DCLICK(mxThreadGrid::OnDblClick)
//	EVT_GRID_CELL_RIGHT_CLICK(mxThreadGrid::OnRightClick)
//	EVT_MENU(mxID_Thread_GOTO_POS,mxThreadGrid::OnGotoPos)
END_EVENT_TABLE()
	
	
mxThreadGrid::mxThreadGrid(wxWindow *parent ):mxGrid(parent,TG_COLS_COUNT) {
	mxGrid::InitColumn(TG_COL_ID,LANG(THREADS_ID,"Id"),5);
	mxGrid::InitColumn(TG_COL_FUNCTION,LANG(THREADS_FUNCTION,"Funcion"),20);
	mxGrid::InitColumn(TG_COL_FILE,LANG(THREADS_FILE,"Archivo"),30);
	mxGrid::InitColumn(TG_COL_LINE,LANG(THREADS_LINE,"Linea"),7);
	mxGrid::DoCreate();
	InsertRows(0,BACKTRACE_SIZE);
	mxGrid::SetRowSelectionMode();
	EnableEditing(false);
	EnableDragRowSize(false);
	SetColLabelSize(wxGRID_AUTOSIZE);
}

bool mxThreadGrid::OnCellDoubleClick(int row, int col) {
	long thread_id; 
	if (debug->CanTalkToGDB() && GetCellValue(row,TG_COL_ID).ToLong(&thread_id)) {
		if (debug->SelectThread(thread_id)) debug->UpdateInspections();
	}
	return true;
}

void mxThreadGrid::OnKey(wxKeyEvent &event) {
	int r = GetGridCursorRow();
	if (event.GetKeyCode()==WXK_DOWN) {
		if (r+1!=GetNumberRows()) {
			SelectRow(r+1);
			SetGridCursor(r+1,GetGridCursorCol());
			MakeCellVisible(r+1,GetGridCursorCol());
		}
	} else if (event.GetKeyCode()==WXK_UP) {
		if (r) {
			SetGridCursor(r-1,GetGridCursorCol());
			SelectRow(r-1);
			MakeCellVisible(r-1,GetGridCursorCol());
		}
	} else if (event.GetKeyCode()==WXK_RETURN || event.GetKeyCode()==WXK_NUMPAD_ENTER) {
		OnCellDoubleClick(GetGridCursorRow(),0);
	} else
		event.Skip();
}

void mxThreadGrid::SetData(int row, wxString id, wxString func, wxString file, wxString line) {
	if (row==GetNumberRows()) InsertRows(GetNumberRows(),1);
	SetCellValue(row,TG_COL_ID,id);
	SetCellValue(row,TG_COL_FUNCTION,func);
	SetCellValue(row,TG_COL_FILE,file);
	SetCellValue(row,TG_COL_LINE,line);
}

void mxThreadGrid::SetNumber(int nrows) {
	if (GetNumberRows()>nrows) {
		DeleteRows(nrows,GetNumberRows()-nrows);
	} else if (GetNumberRows()<nrows) {
		InsertRows(GetNumberRows(),nrows-GetNumberRows());
	}
}

void mxThreadGrid::OnColumnHideOrUnhide (int col, bool visible) {
	if (visible) debug->UpdateThreads();
}

