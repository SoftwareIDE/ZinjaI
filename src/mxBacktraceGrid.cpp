#include "mxBacktraceGrid.h"
#include "DebugManager.h"
#include "mxSource.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxInspectionExplorer.h"
#include "mxInspectionGrid.h"
#include "Language.h"
#include "mxMessageDialog.h"

BEGIN_EVENT_TABLE(mxBacktraceGrid, wxGrid)
	EVT_KEY_DOWN(mxBacktraceGrid::OnKey)
	EVT_GRID_CELL_LEFT_DCLICK(mxBacktraceGrid::OnDblClick)
	EVT_GRID_CELL_RIGHT_CLICK(mxBacktraceGrid::OnRightClick)
	EVT_MENU(mxID_BACKTRACE_GOTO_POS,mxBacktraceGrid::OnGotoPos)
	EVT_MENU(mxID_BACKTRACE_INSPECT_ARGS,mxBacktraceGrid::OnInspectArgs)
	EVT_MENU(mxID_BACKTRACE_INSPECT_LOCALS,mxBacktraceGrid::OnInspectLocals)
	EVT_MENU(mxID_BACKTRACE_EXPLORE_ARGS,mxBacktraceGrid::OnExploreArgs)
	EVT_MENU(mxID_BACKTRACE_ADD_TO_BLACKLIST,mxBacktraceGrid::OnAddToBlackList)
	EVT_SIZE(mxBacktraceGrid::OnResize)
	EVT_GRID_COL_SIZE(mxBacktraceGrid::OnColResize)
	EVT_GRID_LABEL_RIGHT_CLICK(mxBacktraceGrid::OnLabelPopup)
	EVT_MENU_RANGE(mxID_COL_ID, mxID_COL_ID+BG_COLS_COUNT,mxBacktraceGrid::OnShowHideCol)
END_EVENT_TABLE()
	

mxBacktraceGrid::mxBacktraceGrid(wxWindow *parent, wxWindowID id ):wxGrid(parent,id, wxDefaultPosition,wxSize(400,300)) {
	created=false;
	CreateGrid(BACKTRACE_SIZE,BG_COLS_COUNT);
	SetColLabelValue(BG_COL_LEVEL,LANG(BACKTRACE_LEVEL,"Nivel"));
//	SetColLabelValue(BG_COL_ADDRESS,_T("Direccion"));
	SetColLabelValue(BG_COL_FUNCTION,LANG(BACKTRACE_FUNCTION,"Funcion"));
	SetColLabelValue(BG_COL_FILE,LANG(BACKTRACE_FILE,"Archivo"));
	SetColLabelValue(BG_COL_LINE,LANG(BACKTRACE_LINE,"Linea"));
	SetColLabelValue(BG_COL_ARGS,LANG(BACKTRACE_ARGS,"Argumentos"));
	SetRowLabelSize(0);
	EnableEditing(false);
	EnableDragRowSize(false);
	
	// la ayuda de wx dice que SetColMinimalAcceptableWidth(muy bajo) podría penalizar la performance cuando busca en que celda
	// clickeo el usuario segun las coordenadas de pantalla, pero asumo que como hay muy pocas columnas no debería notarse
	SetColMinimalAcceptableWidth(1);
	for (int i=0;i<BG_COLS_COUNT;i++)
		SetColMinimalWidth(i,GetColMinimalAcceptableWidth());
	
//	SetColLabelSize(wxGrid::wxGridAutoSize);
	SetColLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	SetSelectionMode(wxGrid::wxGridSelectRows);
	SetCellHighlightPenWidth(0);
	old_size=100;
	cols_sizes[BG_COL_LEVEL]=10;
	cols_sizes[BG_COL_FUNCTION]=19;
	cols_sizes[BG_COL_FILE]=21;
	cols_sizes[BG_COL_LINE]=10;
	cols_sizes[BG_COL_ARGS]=36;
	cols_visibles=config->Cols.backtrace_grid;
	for (int i=0;i<BG_COLS_COUNT;i++)
		if (!cols_visibles[i]) {
			old_size-=int(cols_sizes[i]);
			cols_sizes[i]=-cols_sizes[i];
			SetColSize(i,GetColMinimalWidth(i));
		}
	cols_marginal=0;
	created=true;
}

mxBacktraceGrid::~mxBacktraceGrid() {
	
}

void mxBacktraceGrid::OnDblClick(wxGridEvent &event) {
	SelectFrame(event.GetRow());
}

void mxBacktraceGrid::SelectFrame(int r) {
	if (debug->IsDebugging() && !debug->CanTalkToGDB()) return;
	long line;
	GetCellValue(r,BG_COL_LINE).ToLong(&line);
	wxString file = GetCellValue(r,BG_COL_FILE);
	if (file.Len()) {
		if (!debug->MarkCurrentPoint(file,line,r?mxSTC_MARK_FUNCCALL:mxSTC_MARK_EXECPOINT))
			mxMessageDialog(main_window,wxString()<<LANG(MAINW_FILE_NOT_FOUND,"No se encontro el archivo:")<<"\n"<<file,LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		if (debug->CanTalkToGDB()) debug->SelectFrame(-1,r+1);
		debug->UpdateInspections();
	}
}

void mxBacktraceGrid::OnGotoPos(wxCommandEvent &event) {
	SelectFrame(selected_row);
}

void mxBacktraceGrid::OnKey(wxKeyEvent &event) {
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
//		debug->SelectFrame(-1,r+1);
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
//		debug->SelectFrame(-1,r+1);
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
		debug->SelectFrame(-1,r+1);
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
		debug->SelectFrame(-1,r+1);
	}
	wxArrayString vars;
#warning reestablecer esto
//	if (debug->GetLocals(vars,GetCellValue(selected_row,BG_COL_LEVEL)))
//		main_window->inspection_ctrl->AppendInspections(vars);
}


void mxBacktraceGrid::OnRightClick(wxGridEvent &event) {
	selected_row = event.GetRow();
	if (!GetCellValue(selected_row,BG_COL_LEVEL).Len()) {
		wxMenu menu; 
		menu.Append(mxID_BACKTRACE_UPDATE,LANG(BACKTRACE_UPDATE,"Actualizar"));
		PopupMenu(&menu);
		return;
	}
	wxMenu menu; 
	if (GetCellValue(selected_row,BG_COL_LINE).Len()) {
		menu.Append(mxID_BACKTRACE_GOTO_POS,wxString(LANG(BACKTRACE_GOTO_PRE,"Ir a "))+GetCellValue(selected_row,BG_COL_FILE)+LANG(BACKTRACE_GOTO_POST," : ")+GetCellValue(selected_row,BG_COL_LINE));
		menu.Append(mxID_BACKTRACE_ADD_TO_BLACKLIST,LANG(BACKTRACE_BLACKLIST_THIS_ONE,"Evitar detenerse este fuente (para step in)"));
	}
	this->SetGridCursor(selected_row,event.GetCol());
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


void mxBacktraceGrid::OnResize(wxSizeEvent &evt) {
	int w;
	if (created && (w=evt.GetSize().GetWidth())!=old_size) {
		int mw=0; // para contar los pixeles que no cambian por las colapsadas
		for (int i=0;i<BG_COLS_COUNT;i++)
			if (!cols_visibles[i])
				mw+=GetColMinimalWidth(i);
		w-=mw;
		double p = double(w)/old_size;
		for (int i=0;i<BG_COLS_COUNT;i++)
			cols_sizes[i]*=p;
		old_size=w;
		for (int i=0;i<BG_COLS_COUNT;i++)
			if (cols_visibles[i])
				SetColSize(i,int(cols_sizes[i]));
	}
	evt.Skip();
}

void mxBacktraceGrid::OnColResize(wxGridSizeEvent &evt) {
	if (!created) return;
	if (old_size) {
		int i=evt.GetRowOrCol();
		cols_sizes[i]=GetColSize(i);
		cols_visibles[i]=GetColMinimalWidth(i)!=cols_sizes[i];
	}
}

void mxBacktraceGrid::OnAddToBlackList(wxCommandEvent &event) {
	int r = selected_row;
	wxString file = GetCellValue(r,BG_COL_FILE);
	if (file.Len()) {
		debug->black_list.Add(file);
		config->Debug.blacklist<<" \""<<file<<"\"";
	}
}

void mxBacktraceGrid::OnLabelPopup(wxGridEvent &event) {
	wxMenu menu;
	menu.AppendCheckItem(mxID_COL_ID+BG_COL_LEVEL,LANG(BACKTRACE_SHRINK_LEVEL,"Encoger  Columna \"Nivel\""))->Check(!cols_visibles[BG_COL_LEVEL]);
	menu.AppendCheckItem(mxID_COL_ID+BG_COL_FUNCTION,LANG(BACKTRACE_SHRINK_FUNCTION,"Encoger  Columna \"Funcion\""))->Check(!cols_visibles[BG_COL_FUNCTION]);
	menu.AppendCheckItem(mxID_COL_ID+BG_COL_FILE,LANG(BACKTRACE_SHRINK_FILE,"Encoger  Columna \"Archivo\""))->Check(!cols_visibles[BG_COL_FILE]);
	menu.AppendCheckItem(mxID_COL_ID+BG_COL_LINE,LANG(BACKTRACE_SHRINK_LINE,"Encoger  Columna \"Linea\""))->Check(!cols_visibles[BG_COL_LINE]);
	menu.AppendCheckItem(mxID_COL_ID+BG_COL_ARGS,LANG(BACKTRACE_SHRINK_ARGS,"Encoger Columna \"Argumentos\""))->Check(!cols_visibles[BG_COL_ARGS]);
	PopupMenu(&menu);
}

void mxBacktraceGrid::OnShowHideCol(wxCommandEvent &evt) {
	int cn=evt.GetId()-mxID_COL_ID;
	float sum=0, fs=cols_sizes[cn];
	int mw=cols_visibles[cn]?GetColMinimalWidth(cn):-GetColMinimalWidth(cn);
	for (int i=0;i<BG_COLS_COUNT;i++)
		if (cols_visibles[i] && i!=cn) 
			sum+=cols_sizes[i];
	cols_visibles[cn]=!cols_visibles[cn];
	cols_sizes[cn]=-cols_sizes[cn];
	if (!cols_visibles[cn])
		SetColSize(cn,mw);
	else
		SetColSize(cn,int(cols_sizes[cn]));
	cols_marginal+=mw;
	fs-=mw;
	for (int i=0;i<BG_COLS_COUNT;i++)
		if (cols_visibles[i] && i!=cn) {
			cols_sizes[i]+=cols_sizes[i]/sum*fs;
			SetColSize(i,int(cols_sizes[i]));
		}
	if (cn==BG_COL_ARGS) debug->SetBacktraceShowsArgs(cols_visibles[cn]);
	Refresh();
}

