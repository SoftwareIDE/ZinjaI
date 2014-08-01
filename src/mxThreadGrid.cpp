#include "mxThreadGrid.h"
#include "DebugManager.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxThreadGrid, wxGrid)
	EVT_KEY_DOWN(mxThreadGrid::OnKey)
	EVT_GRID_CELL_LEFT_DCLICK(mxThreadGrid::OnDblClick)
//	EVT_GRID_CELL_RIGHT_CLICK(mxThreadGrid::OnRightClick)
//	EVT_MENU(mxID_Thread_GOTO_POS,mxThreadGrid::OnGotoPos)
	EVT_SIZE(mxThreadGrid::OnResize)
	EVT_GRID_COL_SIZE(mxThreadGrid::OnColResize)
//	EVT_GRID_LABEL_RIGHT_CLICK(mxThreadGrid::OnLabelPopup)
//	EVT_MENU_RANGE(mxID_COL_ID, mxID_COL_ID+TG_COLS_COUNT,mxThreadGrid::OnShowHideCol)
END_EVENT_TABLE()
	
	
mxThreadGrid::mxThreadGrid(wxWindow *parent, wxWindowID id ):wxGrid(parent,id, wxDefaultPosition,wxSize(400,300)) {
	created=false;
	CreateGrid(1,TG_COLS_COUNT);
	SetColLabelValue(TG_COL_ID,LANG(THREADS_ID,"Id"));
	SetColLabelValue(TG_COL_FUNCTION,LANG(THREADS_FUNCTION,"Funcion"));
	SetColLabelValue(TG_COL_FILE,LANG(THREADS_FILE,"Archivo"));
	SetColLabelValue(TG_COL_LINE,LANG(THREADS_LINE,"Linea"));
	SetRowLabelSize(0);
	EnableEditing(false);
	EnableDragRowSize(false);
//	SetColLabelSize(wxGrid::wxGridAutoSize);
	SetColLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	SetSelectionMode(wxGrid::wxGridSelectRows);
	SetCellHighlightPenWidth(0);
	old_size=62;
	cols_sizes[TG_COL_ID]=5;
	cols_sizes[TG_COL_FUNCTION]=20;
	cols_sizes[TG_COL_FILE]=30;
	cols_sizes[TG_COL_LINE]=7;
//		cols_visibles=config->Cols.Thread_grid;
//		for (int i=0;i<TG_COLS_COUNT;i++)
//			if (!cols_visibles[i]) {
//				old_size-=int(cols_sizes[i]);
//				cols_sizes[i]=-cols_sizes[i];
//				SetColSize(i,GetColMinimalWidth(i));
//			}
	cols_marginal=0;
	created=true;
}

void mxThreadGrid::OnDblClick(wxGridEvent &event) {
	long thread_id; 
	if (debug->CanTalkToGDB() && GetCellValue(event.GetRow(),TG_COL_ID).ToLong(&thread_id))
		debug->SelectThread(thread_id);
}

//void mxThreadGrid::OnGotoPos(wxCommandEvent &event) {
//	if (debug->running||debug->waiting) return;
//	int r = selected_row;
//	long line;
//	GetCellValue(r,TG_COL_LINE).ToLong(&line);
//	wxString file = GetCellValue(r,TG_COL_FILE);
//	if (file.Len()) {
//		if (r) {
//			debug->MarkCurrentPoint(file,line,mxSTC_MARK_FUNCCALL);
//		} else {
//			debug->MarkCurrentPoint(file,line,mxSTC_MARK_EXECPOINT);
//		}
//		debug->SelectThread(GetCellValue(r,TG_COL_ID));
//	}
// }

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
		wxGridEvent event(wxID_ANY,wxEVT_GRID_CELL_LEFT_DCLICK,this,GetGridCursorRow());
		OnDblClick(event);
	} else
		event.Skip();
}

//void mxThreadGrid::OnRightClick(wxGridEvent &event) {
//	selected_row = event.GetRow();
//	if (!GetCellValue(selected_row,TG_COL_LEVEL).Len()) {
//		wxMenu menu; 
//		menu.Append(mxID_Thread_UPDATE,"Actualizar");
//		PopupMenu(&menu);
//		return;
//	}
//	wxMenu menu; 
//	if (GetCellValue(selected_row,TG_COL_LINE).Len()) {
//		menu.Append(mxID_Thread_GOTO_POS,wxString("Ir a ")+GetCellValue(selected_row,TG_COL_FILE)+" : "+GetCellValue(selected_row,TG_COL_LINE));
//		menu.Append(mxID_Thread_ADD_TO_BLACKLIST,"Evitar detenerse este fuente (para step in)");
//	}
//	this->SetGridCursor(selected_row,event.GetCol());
//	if (GetCellValue(selected_row,TG_COL_ARGS).Len()) {
//		menu.Append(mxID_Thread_INSPECT_ARGS,"Inspeccionar Argumentos");
//		menu.Append(mxID_Thread_EXPLORE_ARGS,"Explorar Argumentos");
//	}
//	menu.Append(mxID_Thread_INSPECT_LOCALS,"Inspeccionar Variables Locales");
//	menu.Append(mxID_Thread_EXPLORE_LOCALS,"Explorar Variables Locales");
//	menu.AppendSeparator();
//	menu.Append(mxID_Thread_UPDATE,"Actualizar");
//	PopupMenu(&menu);
// }

void mxThreadGrid::OnResize(wxSizeEvent &evt) {
	int w;
	if (created && (w=evt.GetSize().GetWidth())!=old_size) {
		double p = double(w)/old_size;
		for (int i=0;i<TG_COLS_COUNT;i++)
			cols_sizes[i]*=p;
		old_size=w;
		for (int i=0;i<TG_COLS_COUNT;i++)
//			if (cols_visibles[i])
			SetColSize(i,int(cols_sizes[i]));
	}
	evt.Skip();
}

void mxThreadGrid::OnColResize(wxGridSizeEvent &evt) {
	if (!created) return;
	if (old_size) {
		int i=evt.GetRowOrCol();
		cols_sizes[i]=GetColSize(i);
//		cols_visibles[i]=GetColMinimalWidth(i)!=cols_sizes[i];
	}
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
