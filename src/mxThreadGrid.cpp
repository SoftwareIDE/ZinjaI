#include "mxThreadGrid.h"
#include "DebugManager.h"
#include "Language.h"

mxThreadGrid::mxThreadGrid(wxWindow *parent ):mxGrid(parent,TG_COLS_COUNT,wxID_ANY,wxSize(400,300)) {
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
		if (debug->SelectThread(thread_id)) {
			debug->UpdateBacktrace(false);
			debug->UpdateInspections();
		}
	}
	return true;
}

bool mxThreadGrid::OnKey(int row, int col, int key, int modifiers) {
	if (key==WXK_RETURN || key==WXK_NUMPAD_ENTER) {
		OnCellDoubleClick(row,col);
		return true;
	} else 
		return false;
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

