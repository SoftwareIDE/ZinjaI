#include "mxBreakList.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include <wx/grid.h>
#include "DebugManager.h"
#include "mxBreakOptions.h"
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
	grid->SetColLabelValue(BL_COL_NUM,LANG(BREAKLIST_NUMBER,"Numero"));
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
	
	old_size=100;
	cols_sizes[BL_COL_NUM]=7;
	cols_sizes[BL_COL_TYPE]=8;
	cols_sizes[BL_COL_WHY]=47;
	cols_sizes[BL_COL_ENABLE]=10;
//	cols_sizes[BL_COL_DISP]=10;
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
	debug->UpdateBreakList(grid);
	AddErrorBreaks();
	if (grid->GetNumberRows())
		grid->SelectRow(0);
	
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
	int r = evt.GetRow();
	long num, line;
	wxString snum(grid->GetCellValue(r,BL_COL_NUM));
	mxSource *source=NULL;
	file_item *file=NULL;
	break_line_item *bitem=NULL;
	if (snum.Len()) {
		snum.ToLong(&num);
		debug->FindBreakInfoFromNumber(num,bitem,source,file);
	} else {
		if (grid->GetCellValue(r,BL_COL_TYPE)==_T("bkpt")) {
			wxString where = grid->GetCellValue(r,BL_COL_WHY);
			int p = where.Find(wxString(_T(": "))<<LANG(BREAKLIST_LINE,"linea")<<_T(" "));
			wxString fname=where.Mid(0,p);
			where.Mid(p+3+wxString(LANG(BREAKLIST_LINE,"linea")).Len()).ToLong(&line);
			debug->FindBreakInfoFromData(fname,line,bitem,source,file);
		}
	}
//	if (!bitem) return;
	if (evt.GetCol()==BL_COL_WHY) {
		if (!source) source = main_window->OpenFile(DIR_PLUS_FILE(project->path,file->name),!project);
		else main_window->notebook_sources->SetSelection(main_window->notebook_sources->GetPageIndex(source));
		Close();
		if (source && source!=external_source) {
			source->MarkError(bitem->line);
			source->SetFocus();
		}
		return;
	} else {
		if (source) {
			if (bitem)
				new mxBreakOptions(source->sin_titulo?source->temp_filename.GetFullPath():source->source_filename.GetFullPath(),source->MarkerLineFromHandle(bitem->handle),source,bitem);
			else
				new mxBreakOptions(source->sin_titulo?source->temp_filename.GetFullPath():source->source_filename.GetFullPath(),line,source);
			grid->DeleteRows(0,grid->GetNumberRows());
			debug->UpdateBreakList(grid);
			AddErrorBreaks();
		} else if (file) {
			new mxBreakOptions(DIR_PLUS_FILE(project->path,file->name),bitem->line,file,bitem);
			grid->DeleteRows(0,grid->GetNumberRows());
			debug->UpdateBreakList(grid);
			AddErrorBreaks();
		}
	}
	grid->SetGridCursor(r,evt.GetCol());
	grid->SelectRow(r);
	grid->MakeCellVisible(r,evt.GetCol());
}

void mxBreakList::AddErrorBreaks() {
	int sc = main_window->notebook_sources->GetPageCount();
	break_line_item *bitem;
	bool force = !debug->debugging || debug->running;
	int cont = grid->GetNumberRows();
	if (sc) {
		mxSource *source;
		for (int i=0;i<sc;i++) {
			source = (mxSource*)(main_window->notebook_sources->GetPage(i));
			if (source->next_source_with_same_file!=source) { // si hay vista duplicadas, evita repetir tomando solo el menor
				mxSource *min=source,*aux=source->next_source_with_same_file;
				while (aux && aux!=source) { 
					if (aux<min) min=aux; 
					aux=aux->next_source_with_same_file;
				}
				if (min!=source) continue;
			}
			bitem = source->first_break_item;
			while (bitem) {
				if (force || bitem->error) {
					grid->AppendRows(1);
					for (int i=0;i<BL_COLS_COUNT;i++)
						grid->SetReadOnly(cont,i,true);
					grid->SetCellValue(cont,BL_COL_TYPE,_T("bkpt"));
					if (bitem->error) {
						if (bitem->valid_cond)
							grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_ERROR_LOCATION,"error (ubicacion)"));
						else
							grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_ERROR_CONDITION,"error (condicion)"));
					} else if (bitem->enabled) {
						if (bitem->only_once)
							grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_ONCE,"una vez"));
						else
							grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_ENABLED,"habilitado"));
					} else
						grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_DISABLED,"deshabilitado"));
					wxString fname = source->sin_titulo?source->temp_filename.GetFullPath():source->source_filename.GetFullPath();
					grid->SetCellValue(cont,BL_COL_WHY,fname<<_T(": ")<<LANG(BREAKLIS_LINE,"linea")<<_T(" ")<<source->MarkerLineFromHandle(bitem->handle));
					grid->SetCellValue(cont,BL_COL_COND,bitem->cond);
					grid->SetCellValue(cont,BL_COL_HIT,_T(""));
					cont++;
				}
				bitem = bitem->next;
			}
			if (project) project->GetSourceExtras(source);
		}
	}

	if (project) {
		file_item *file = project->first_source;
		bool header_done=false;
		while (file) {
			if (!main_window->IsOpen(file->item)) {
				bitem = file->breakpoints;
				while (bitem) {
					if ((force || bitem->error)) {
						grid->AppendRows(1);
						for (int i=0;i<BL_COLS_COUNT;i++)
							grid->SetReadOnly(cont,i,true);
						grid->SetCellValue(cont,BL_COL_TYPE,_T("bkpt"));
						if (bitem->error) {
							if (bitem->valid_cond)
								grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_ERROR_LOCATION,"error (ubicacion)"));
							else
								grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_ERROR_CONDITION,"error (condicion)"));
						} else if (bitem->enabled) {
							if (bitem->only_once)
								grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_ONCE,"una vez"));
							else
								grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_ENABLED,"habilitado"));
						} else
							grid->SetCellValue(cont,BL_COL_ENABLE,LANG(BREAKLIST_DISABLED,"deshabilitado"));
						wxString fname = DIR_PLUS_FILE(project->path,file->name);
						grid->SetCellValue(cont,BL_COL_WHY,fname<<_T(": ")<<LANG(BREAKLIST_LINE,"linea")<<_T(" ")<<bitem->line);
						grid->SetCellValue(cont,BL_COL_COND,bitem->cond);
						grid->SetCellValue(cont,BL_COL_HIT,_T(""));
						cont++;
					}
					bitem = bitem->next;
				}
			}
			file = file->next;
			if (!file && !header_done) {
				file = project->first_header;
				header_done=true;
			}
		}
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

void mxBreakList::OnDeleteButton(wxCommandEvent &evt) {
	if (debug->running && debug->waiting) return;
	for (int r=int(grid->GetNumberRows())-1;r>=0;r--) {
		bool selected=false;
		for (unsigned int j=0;j<BL_COLS_COUNT;j++)
			if (grid->IsInSelection(r,j)) {
				selected=true; break;
			}
		if (selected) {
			long num;
			wxString snum(grid->GetCellValue(r,BL_COL_NUM));
			mxSource *source=NULL;
			file_item *file=NULL;
			break_line_item *bitem=NULL;
			if (snum.Len()) {
				snum.ToLong(&num);
				debug->FindBreakInfoFromNumber(num,bitem,source,file);
			} else {
				if (grid->GetCellValue(r,BL_COL_TYPE)==_T("bkpt")) {
					wxString where = grid->GetCellValue(r,BL_COL_WHY);
					int p = where.Find(wxString(_T(": "))<<LANG(BREAKLIST_LINE,"linea")<<_T(" "));
					wxString fname=where.Mid(0,p);
					long line;
					where.Mid(p+3+wxString(LANG(BREAKLIST_LINE,"linea")).Len()).ToLong(&line);
					debug->FindBreakInfoFromData(fname,line,bitem,source,file);
				}
			}
			if (bitem) {
				if (debug->debugging && !bitem->error) debug->DeleteBreakPoint(bitem->num);
				if (bitem->next) bitem->next->prev=bitem->prev;
				if (bitem->prev) bitem->prev->next=bitem->next;
				else if (source && source->first_break_item==bitem)
					source->first_break_item=bitem->next;
				else if (file && file->breakpoints==bitem)
					file->breakpoints=bitem->next;
				if (source) source->MarkerDeleteHandle(bitem->handle);
				delete bitem;
				grid->DeleteRows(r);
			}
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
