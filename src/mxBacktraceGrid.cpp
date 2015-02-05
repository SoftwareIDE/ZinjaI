#include "mxBacktraceGrid.h"
#include "DebugManager.h"
#include "mxSource.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxInspectionGrid.h"
#include "Language.h"
#include "mxMessageDialog.h"

BEGIN_EVENT_TABLE(mxBacktraceGrid, wxGrid)
//	EVT_KEY_DOWN(mxBacktraceGrid::OnKey)
//	EVT_GRID_CELL_LEFT_DCLICK(mxBacktraceGrid::OnDblClick)
//	EVT_GRID_CELL_RIGHT_CLICK(mxBacktraceGrid::OnRightClick)
	EVT_MENU(mxID_BACKTRACE_GOTO_POS,mxBacktraceGrid::OnGotoPos)
	EVT_MENU(mxID_BACKTRACE_INSPECT_ARGS,mxBacktraceGrid::OnInspectArgs)
	EVT_MENU(mxID_BACKTRACE_INSPECT_LOCALS,mxBacktraceGrid::OnInspectLocals)
	EVT_MENU(mxID_BACKTRACE_EXPLORE_ARGS,mxBacktraceGrid::OnExploreArgs)
//	EVT_MENU(mxID_BACKTRACE_ADD_FUNCTION_TO_BLACKLIST,mxBacktraceGrid::OnAddFunctionToBlackList)
	EVT_MENU(mxID_BACKTRACE_ADD_FILE_TO_BLACKLIST,mxBacktraceGrid::OnAddFileToBlackList)
END_EVENT_TABLE()
	

mxBacktraceGrid::mxBacktraceGrid(wxWindow *parent):mxGrid(parent,BG_COLS_COUNT,wxID_ANY,wxSize(400,300)) {
	mxGrid::InitColumn(BG_COL_LEVEL,LANG(BACKTRACE_LEVEL,"Nivel"),10);
	mxGrid::InitColumn(BG_COL_FUNCTION,LANG(BACKTRACE_FUNCTION,"Función"),19);
	mxGrid::InitColumn(BG_COL_FILE,LANG(BACKTRACE_FILE,"Archivo"),21);
	mxGrid::InitColumn(BG_COL_LINE,LANG(BACKTRACE_LINE,"Linea"),10);
	mxGrid::InitColumn(BG_COL_ARGS,LANG(BACKTRACE_ARGS,"Argumentos"),36);
	mxGrid::DoCreate();
	InsertRows(0,BACKTRACE_SIZE); entries.Resize(BACKTRACE_SIZE);
	mxGrid::SetRowSelectionMode();
	EnableEditing(false);
	EnableDragRowSize(false);
	SetColLabelSize(wxGRID_AUTOSIZE);
}

//mxBacktraceGrid::~mxBacktraceGrid() {
//	
//}

bool mxBacktraceGrid::OnCellDoubleClick(int row, int col) {
	SelectFrame(row); return true;
}

void mxBacktraceGrid::SelectFrame(int r) {
	if (debug->IsDebugging() && !debug->CanTalkToGDB()) return;
	long line;
	entries[r].line.ToLong(&line);
	wxString file = entries[r].fname;
	if (file.Len()) {
		if (!debug->MarkCurrentPoint(file,line,r?mxSTC_MARK_FUNCCALL:mxSTC_MARK_EXECPOINT))
			mxMessageDialog(main_window,wxString()<<LANG(MAINW_FILE_NOT_FOUND,"No se encontro el archivo:")<<"\n"<<file,LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		if (debug->CanTalkToGDB()) debug->SelectFrame(-1,r);
		debug->UpdateInspections();
	}
}

void mxBacktraceGrid::OnGotoPos(wxCommandEvent &event) {
	SelectFrame(selected_row);
}

bool mxBacktraceGrid::OnKey(int row, int col, int key, int modifiers) {
	if (key==WXK_RETURN || key==WXK_NUMPAD_ENTER) {
		OnCellDoubleClick(row,col);
		return true;
	} else 
		return false;
}

void mxBacktraceGrid::OnExploreArgs(wxCommandEvent &event) {
#warning reestablecer funcionalidad
//	int r = selected_row;
//	long line;
//	GetCellValue(r,BG_COL_LINE).ToLong(&line);
//	wxString file = GetCellValue(r,BG_COL_FILE);
//	if (file.Len()) {
//		if (r) {
//			debug->MarkCurrentPoint(file,line,mxSTC_MARK_FUNCCALL);
//		} else {
//			debug->MarkCurrentPoint(file,line,mxSTC_MARK_EXECPOINT);
//		}
//		debug->SelectFrame(-1,r);
//	}
//	wxArrayString vars;
//	if (debug->GetArgs(vars,GetCellValue(selected_row,BG_COL_LEVEL))) {
//		wxArrayString frames(vars.GetCount());
//		for (unsigned int i=0;i<frames.GetCount();i++)
//			frames[i]=debug->current_frame;
//		new mxInspectionExplorer(GetCellValue(selected_row,BG_COL_FUNCTION),vars,frames);
//	}
}

void mxBacktraceGrid::OnExploreLocals(wxCommandEvent &event) {
#warning restablecer funcionalidad
//	int r = selected_row;
//	long line;
//	GetCellValue(r,BG_COL_LINE).ToLong(&line);
//	wxString file = GetCellValue(r,BG_COL_FILE);
//	if (file.Len()) {
//		if (r) {
//			debug->MarkCurrentPoint(file,line,mxSTC_MARK_FUNCCALL);
//		} else {
//			debug->MarkCurrentPoint(file,line,mxSTC_MARK_EXECPOINT);
//		}
//		debug->SelectFrame(-1,r);
//	}
//	wxArrayString vars;
//	if (debug->GetLocals(vars,GetCellValue(selected_row,BG_COL_LEVEL))) {
//		wxArrayString frames(vars.GetCount());
//		for (unsigned int i=0;i<frames.GetCount();i++)
//			frames[i]=debug->current_frame;
//		new mxInspectionExplorer(GetCellValue(selected_row,BG_COL_FUNCTION),vars,frames);
//	}
}

void mxBacktraceGrid::OnInspectArgs(wxCommandEvent &event) {
	int r = selected_row;
	long line;
	GetCellValue(r,BG_COL_LINE).ToLong(&line);
	wxString file = GetCellValue(r,BG_COL_FILE);
	if (file.Len()) {
		if (r) {
			debug->MarkCurrentPoint(file,line,mxSTC_MARK_FUNCCALL);
		} else {
			debug->MarkCurrentPoint(file,line,mxSTC_MARK_EXECPOINT);
		}
		debug->SelectFrame(-1,r);
	}
	wxArrayString vars;
#warning reestablecer esto
//	if (debug->GetArgs(vars,GetCellValue(selected_row,BG_COL_LEVEL)))
//		main_window->inspection_ctrl->AppendInspections(vars);
}

void mxBacktraceGrid::OnInspectLocals(wxCommandEvent &event) {
	int r = selected_row;
	long line;
	GetCellValue(r,BG_COL_LINE).ToLong(&line);
	wxString file = GetCellValue(r,BG_COL_FILE);
	if (file.Len()) {
		if (r) {
			debug->MarkCurrentPoint(file,line,mxSTC_MARK_FUNCCALL);
		} else {
			debug->MarkCurrentPoint(file,line,mxSTC_MARK_EXECPOINT);
		}
		debug->SelectFrame(-1,r);
	}
	wxArrayString vars;
#warning reestablecer esto
//	if (debug->GetLocals(vars,GetCellValue(selected_row,BG_COL_LEVEL)))
//		main_window->inspection_ctrl->AppendInspections(vars);
}


void mxBacktraceGrid::OnCellPopupMenu(int row, int col) {
	selected_row = row;
	if (!GetCellValue(selected_row,BG_COL_LEVEL).Len()) {
		wxMenu menu; 
		menu.Append(mxID_BACKTRACE_UPDATE,LANG(BACKTRACE_UPDATE,"Actualizar"));
		PopupMenu(&menu);
		return;
	}
	wxMenu menu; 
	if (GetCellValue(selected_row,BG_COL_LINE).Len()) {
		menu.Append(mxID_BACKTRACE_GOTO_POS,wxString(LANG(BACKTRACE_GOTO_PRE,"Ir a "))+GetCellValue(selected_row,BG_COL_FILE)+LANG(BACKTRACE_GOTO_POST," : ")+GetCellValue(selected_row,BG_COL_LINE));
		if (!debug->IsDebugging() || debug->CanTalkToGDB()) {
			menu.Append(mxID_BACKTRACE_ADD_FILE_TO_BLACKLIST,LANG(BACKTRACE_BLACKLIST_THIS_FILE,"Evitar detenerse este fuente (para step in)"));
//			menu.Append(mxID_BACKTRACE_ADD_FUNCTION_TO_BLACKLIST,LANG(BACKTRACE_BLACKLIST_THIS_FUNCTION,"Evitar detenerse esta función (para step in)"));
		}
	}
	this->SetGridCursor(selected_row,col);
	if (GetCellValue(selected_row,BG_COL_ARGS).Len()) {
		menu.Append(mxID_BACKTRACE_INSPECT_ARGS,LANG(BACKTRACE_INSPECT_ARGS,"Inspeccionar Argumentos"));
		menu.Append(mxID_BACKTRACE_EXPLORE_ARGS,LANG(BACKTRACE_EXPLORE_ARGS,"Explorar Argumentos"));
	}
	menu.Append(mxID_BACKTRACE_INSPECT_LOCALS,LANG(BACKTRACE_INSPECT_LOCALS,"Inspeccionar Variables Locales"));
	menu.Append(mxID_BACKTRACE_EXPLORE_LOCALS,LANG(BACKTRACE_EXPLORE_LOCALS,"Explorar Variables Locales"));
	menu.AppendSeparator();
	menu.Append(mxID_BACKTRACE_UPDATE,LANG(BACKTRACE_UPDATE,"Actualizar"));
	PopupMenu(&menu);
}


void mxBacktraceGrid::OnAddFunctionToBlackList(wxCommandEvent &event) {
	AddToBlackList("function",GetCellValue(selected_row,BG_COL_FUNCTION));
}

void mxBacktraceGrid::OnAddFileToBlackList(wxCommandEvent &event) {
	wxString file = GetCellValue(selected_row,BG_COL_FILE);
}

void mxBacktraceGrid::AddToBlackList(const wxString &type, const wxString &what) {
	if (what.Len()) {
		config->Debug.use_blacklist=true;
		config->Debug.blacklist.Add(/*type+" "+mxUT::Quotize(*/what/*)*/);
		debug->SetBlacklist(true);
	}
}

void mxBacktraceGrid::OnColumnHideOrUnhide (int col, bool visible) {
	if (col==BG_COL_ARGS) debug->backtrace_shows_args=visible;
	if (visible) debug->UpdateBacktrace(false,false);
}

void mxBacktraceGrid::SetCellValue (int r, int c, const wxString & value) {
	if (c==BG_COL_FILE) entries[r].fname=value;
	else if (c==BG_COL_LINE) entries[r].line=value;
	mxGrid::SetCellValue(r,c,value);
}

