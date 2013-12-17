#include "mxBreakList.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include <wx/grid.h>
#include "DebugManager.h"
#include "mxBreakOptions.h"
#include "BreakPointInfo.h"
#include "DebugManager.h"
#include "mxSource.h"
#include "ProjectManager.h"
#include "ids.h"
#include "mxSizers.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxBreakList, wxDialog)
	EVT_SIZE(mxBreakList::OnResize)
	EVT_GRID_COL_SIZE(mxBreakList::OnColResize)
	EVT_GRID_CELL_LEFT_DCLICK(mxBreakList::OnDoubleClick)
	EVT_CLOSE(mxBreakList::OnClose)
	EVT_KEY_DOWN(mxBreakList::OnCharHook)
	EVT_BUTTON(mxID_BREAK_LIST_DELETE_ALL,mxBreakList::OnDeleteAllButton)
	EVT_BUTTON(mxID_BREAK_LIST_DELETE,mxBreakList::OnDeleteButton)
	EVT_BUTTON(mxID_BREAK_LIST_GOTO,mxBreakList::OnGotoButton)
	EVT_BUTTON(mxID_BREAK_LIST_EDIT,mxBreakList::OnEditButton)
END_EVENT_TABLE()

mxBreakList::mxBreakList() : wxDialog(main_window, wxID_ANY, LANG(BREAKLIST_CAPTION,"Lista de Break/Watch points"), wxDefaultPosition, wxSize(600,400) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {
	old_size=0;
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	grid = new wxGrid(this,wxID_ANY);
	grid->CreateGrid(0,BL_COLS_COUNT);
//	grid->SetColLabelValue(BL_COL_NUM,LANG(BREAKLIST_NUMBER,"Numero"));
	grid->SetColLabelValue(BL_COL_TYPE,LANG(BREAKLIST_TYPE,"Tipo"));
	grid->SetColLabelValue(BL_COL_WHY,LANG(BREAKLIST_LOCATION_EXPRESSION,"Ubicacion/Expresion"));
	grid->SetColLabelValue(BL_COL_ENABLE,LANG(BREAKLIST_STATE,"Estado"));
	grid->SetColLabelValue(BL_COL_COND,LANG(BREAKLIST_CONDITION,"Condicion"));
	grid->SetColLabelValue(BL_COL_HIT,LANG(BREAKLIST_COUNTER,"Conteo"));
	grid->SetColLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	grid->SetRowLabelSize(0);
	grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid->SetCellHighlightPenWidth(0);
	grid->EnableDragRowSize(false);	
	
	old_size=100-7;
//	cols_sizes[BL_COL_NUM]=7;
	cols_sizes[BL_COL_TYPE]=8;
	cols_sizes[BL_COL_WHY]=47;
	cols_sizes[BL_COL_ENABLE]=10;
	cols_sizes[BL_COL_HIT]=7;
	cols_sizes[BL_COL_COND]=20;
	
	wxButton *close_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.ok,LANG(BREAKLIST_CLOSE,"&Cerrar"));
	SetEscapeId(wxID_CANCEL);
	wxButton *goto_button = new mxBitmapButton (this,mxID_BREAK_LIST_GOTO,bitmaps->buttons.next,LANG(BREAKLIST_GO_TO,"&Ir A..."));
	wxButton *edit_button = new mxBitmapButton (this,mxID_BREAK_LIST_EDIT,bitmaps->buttons.ok,LANG(BREAKLIST_EDIT,"&Editar..."));
	edit_button->SetDefault();
	wxButton *delete_button = new mxBitmapButton (this,mxID_BREAK_LIST_DELETE,bitmaps->buttons.cancel,LANG(BREAKLIST_DELETE,"E&liminar"));
	wxButton *delete_all_button = new mxBitmapButton (this,mxID_BREAK_LIST_DELETE_ALL,bitmaps->buttons.cancel,LANG(BREAKLIST_CLEAN,"&Limpiar"));
	buttonSizer->Add(delete_button,sizers->BA5);
	buttonSizer->Add(delete_all_button,sizers->BA5);
	buttonSizer->AddStretchSpacer();
	buttonSizer->Add(goto_button,sizers->BA5);
	buttonSizer->Add(edit_button,sizers->BA5);
	buttonSizer->Add(close_button,sizers->BA5);
	
	mySizer->Add(grid,sizers->Exp1);
	mySizer->Add(buttonSizer,sizers->Exp0);
	
	SetSizer(mySizer);
	
	wxSizeEvent evt(this->GetSize());
	OnResize(evt);
	
	PopulateGrid();
	
	if (grid->GetNumberRows()) grid->SelectRow(0);
	
	grid->SetFocus();
	Show();
	
}

void mxBreakList::OnResize(wxSizeEvent &evt) {
	int w=evt.GetSize().GetWidth();
	if (w!=old_size) {
		double p = double(w)/old_size;
		for (int i=0;i<BL_COLS_COUNT;i++)
			cols_sizes[i]*=p;
		old_size=w;
		for (int i=0;i<BL_COLS_COUNT;i++)
			grid->SetColSize(i,int(cols_sizes[i]));
	}
	evt.Skip();
}

void mxBreakList::OnColResize(wxGridSizeEvent &evt) {
	if (old_size) cols_sizes[evt.GetRowOrCol()]=grid->GetColSize(evt.GetRowOrCol());
}

void mxBreakList::OnClose(wxCloseEvent &evt) {
	Destroy();
}

void mxBreakList::OnDoubleClick(wxGridEvent &evt) {
	int r = evt.GetRow(), c=evt.GetCol();
	grid->SetGridCursor(r,c); grid->SelectRow(r);
	if (ids[r]==-1) return;
	if (evt.GetCol()==BL_COL_WHY) { // doble click en la columna con la ubicacion, abre ese archivo y va a esa linea
		BreakPointInfo *bpi=BreakPointInfo::FindFromNumber(ids[r],false);
		if (bpi) {
			if (!bpi->source) main_window->OpenFile(bpi->fname,!project);
			else main_window->notebook_sources->SetSelection(main_window->notebook_sources->GetPageIndex(bpi->source));
			if (bpi->source) {
				Close();
				bpi->source->MarkError(bpi->line_number);
				bpi->source->SetFocus();
			}
		}
	} else { // doble click en otro lado, abre el cuadro para editar el breakpoint
		BreakPointInfo *bpi=BreakPointInfo::FindFromNumber(ids[r],false);
		if (bpi) { new mxBreakOptions(bpi); PopulateGrid(); }
	}
}

void mxBreakList::OnCharHook(wxKeyEvent &evt) {
	if (evt.GetKeyCode()==WXK_DOWN) {
		int r=grid->GetGridCursorRow();
		if (r+1!=grid->GetNumberRows()) {
			grid->SelectRow(r+1);
			grid->SetGridCursor(r+1,grid->GetGridCursorCol());
			grid->MakeCellVisible(r+1,grid->GetGridCursorCol());
		}
	} else if (evt.GetKeyCode()==WXK_UP) {
		int r=grid->GetGridCursorRow();
		if (r) {
			grid->SetGridCursor(r-1,grid->GetGridCursorCol());
			grid->SelectRow(r-1);
			grid->MakeCellVisible(r-1,grid->GetGridCursorCol());
		}
	} else if (evt.GetKeyCode()==WXK_DELETE || evt.GetKeyCode()==WXK_NUMPAD_DELETE) {
		wxCommandEvent evt;
		OnDeleteButton(evt);
	} else if (evt.GetKeyCode()==WXK_RETURN || evt.GetKeyCode()==WXK_NUMPAD_ENTER) {
		wxCommandEvent evt;
		OnEditButton(evt);
	} else if (evt.GetKeyCode()==WXK_ESCAPE) {
		Close();
	} else evt.Skip();
}
	
void mxBreakList::OnDeleteAllButton(wxCommandEvent &evt) {
	grid->SelectAll();
	OnDeleteButton(evt);
}

void mxBreakList::RemoveBreakPoint(int _row, bool single) {
	BreakPointInfo *bpi=BreakPointInfo::FindFromNumber(ids[_row],false);
	if (!bpi) return;
	if (debug->debugging && ((!debug->waiting)||single)) {
		debug->DeleteBreakPoint(bpi);
	} else delete bpi;
	ids.erase(ids.begin()+_row);
	grid->DeleteRows(_row,1);
}

void mxBreakList::OnDeleteButton(wxCommandEvent &evt) {
	if (debug->running && debug->waiting) return;
	for (int r=int(grid->GetNumberRows())-1;r>=0;r--) {
		if (ids[r]==-1) continue;
		for (unsigned int j=0;j<BL_COLS_COUNT;j++)
			if (grid->IsInSelection(r,j)) {
				RemoveBreakPoint(r,false); 
				break;
			}
	}
}

void mxBreakList::OnEditButton(wxCommandEvent &evt) {
	if (debug->running && debug->waiting) return;
	int r = grid->GetGridCursorRow();
	if (r<0 || r>=grid->GetNumberRows()) return;
	wxGridEvent event(wxID_ANY,0,this,r,BL_COL_COND);
	OnDoubleClick(event);
}

void mxBreakList::OnGotoButton(wxCommandEvent &evt) {
	if (debug->running && debug->waiting) return;
	int r = grid->GetGridCursorRow();
	if (r<0 || r>=grid->GetNumberRows()) return;
	wxGridEvent event(wxID_ANY,0,this,r,BL_COL_WHY);
	OnDoubleClick(event);
}


void mxBreakList::PopulateGrid ( ) {
	grid->DeleteRows(0,grid->GetNumberRows()); ids.clear();
	
	bool ask_debug=debug->debugging && !debug->waiting;
	if (ask_debug) debug->PopulateBreakpointsList(this,true);
	
	GlobalListIterator<BreakPointInfo*> bpi=BreakPointInfo::GetGlobalIterator();
	while(bpi.IsValid()) {
		
		if (ask_debug && bpi->gdb_id!=-1) continue; // ya lo cargo debug
		
		bpi->UpdateLineNumber();
		
		int i=AppendRow(bpi->zinjai_id);
		
		grid->SetCellValue(i,BL_COL_TYPE,"bkpt");
		
		wxString where(bpi->fname);	where<<": "<<LANG(BREAKLIST_LINE,"linea")<<" "<<bpi->line_number+1;
		grid->SetCellValue(i,BL_COL_WHY,where);
		
		wxString status=LANG(BREAKLIST_ENABLED,"habilitado");
		if (bpi->action==BPA_STOP_ONCE) status=LANG(BREAKLIST_ONCE,"una vez");
		if (bpi->action==BPA_INSPECTIONS) status=LANG(BREAKLIST_INSPECTIONS,"inspecciones");
		if (bpi->gdb_status==BPS_ERROR_CONDITION) status=LANG(BREAKLIST_ERROR_CONDITION,"error (condicion)");
		if (bpi->gdb_status==BPS_ERROR_SETTING) status=LANG(BREAKLIST_ERROR_LOCATION,"error (ubicacion)");
		if (!bpi->enabled) status=LANG(BREAKLIST_DISABLED,"deshabilitado");
		grid->SetCellValue(i,BL_COL_ENABLE,status);
		
		grid->SetCellValue(i,BL_COL_COND,bpi->cond);
		
		grid->SetCellValue(i,BL_COL_HIT,"0");
		
		bpi.Next();
	}
}

int mxBreakList::AppendRow(int _id) {
	int n=grid->GetNumberRows(); grid->AppendRows(1);
	for (int i=0;i<BL_COLS_COUNT;i++) grid->SetReadOnly(n,i,true);
	ids.push_back(_id);
	return n;
}
