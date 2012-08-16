#include "mxInspectionGrid.h"
#include "mxUtils.h"
#include "DebugManager.h"
#include <wx/choicdlg.h>
#include <wx/menu.h>
#include <wx/clipbrd.h>
#include "mxSource.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxInspectionExplorer.h"
#include "mxMessageDialog.h"
#include "mxInspectionMatrix.h"
#include "Language.h"
#include "mxTextDialog.h"
#include "mxInspectionPrint.h"

BEGIN_EVENT_TABLE(mxInspectionGrid, wxGrid)
	EVT_GRID_CELL_CHANGE(mxInspectionGrid::OnCellChange)
	EVT_KEY_DOWN(mxInspectionGrid::OnKey)
	EVT_GRID_CELL_LEFT_CLICK(mxInspectionGrid::OnClick)
	EVT_GRID_CELL_LEFT_DCLICK(mxInspectionGrid::OnDoubleClick)
	EVT_GRID_CELL_RIGHT_CLICK(mxInspectionGrid::OnRightClick)
	EVT_MENU(mxID_INSPECTION_FREEZE,mxInspectionGrid::OnFreeze)
	EVT_MENU(mxID_INSPECTION_BREAK,mxInspectionGrid::OnBreakClassOrArray)
	EVT_MENU(mxID_INSPECTION_RESCOPE,mxInspectionGrid::OnReScope)
	EVT_MENU(mxID_INSPECTION_DUPLICATE,mxInspectionGrid::OnDuplicate)
	EVT_MENU(mxID_INSPECTION_FROM_CLIPBOARD,mxInspectionGrid::OnPasteFromClipboard)
	EVT_MENU(mxID_INSPECTION_FROM_SOURCE,mxInspectionGrid::OnCopyFromSelecction)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_TEXT,mxInspectionGrid::OnShowInText)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_TABLE,mxInspectionGrid::OnShowInTable)
	EVT_MENU(mxID_INSPECTION_EXPLORE,mxInspectionGrid::OnExploreExpression)
	EVT_MENU(mxID_INSPECTION_COPY_DATA,mxInspectionGrid::OnCopyData)
	EVT_MENU(mxID_INSPECTION_COPY_EXPRESSION,mxInspectionGrid::OnCopyExpression)
	EVT_MENU(mxID_INSPECTION_EXPLORE_ALL,mxInspectionGrid::OnExploreAll)
	EVT_MENU(mxID_INSPECTION_CLEAR_ALL,mxInspectionGrid::OnClearAll)
	EVT_MENU(mxID_INSPECTION_CLEAR_ONE,mxInspectionGrid::OnClearOne)
	EVT_MENU(mxID_INSPECTION_WATCH_NO,mxInspectionGrid::OnWatchNo)
	EVT_MENU(mxID_INSPECTION_WATCH_RW,mxInspectionGrid::OnWatchReadWrite)
	EVT_MENU(mxID_INSPECTION_WATCH_READ,mxInspectionGrid::OnWatchRead)
	EVT_MENU(mxID_INSPECTION_WATCH_WRITE,mxInspectionGrid::OnWatchWrite)
	EVT_MENU(mxID_INSPECTION_FORMAT_NAT,mxInspectionGrid::OnFormatNatural)
	EVT_MENU(mxID_INSPECTION_FORMAT_BIN,mxInspectionGrid::OnFormatBinary)
	EVT_MENU(mxID_INSPECTION_FORMAT_DEC,mxInspectionGrid::OnFormatDecimal)
	EVT_MENU(mxID_INSPECTION_FORMAT_OCT,mxInspectionGrid::OnFormatOctal)
	EVT_MENU(mxID_INSPECTION_FORMAT_HEX,mxInspectionGrid::OnFormatHexadecimal)
	EVT_MENU(mxID_INSPECTION_SAVE_TABLE,mxInspectionGrid::OnSaveTable)
	EVT_MENU(mxID_INSPECTION_LOAD_TABLE,mxInspectionGrid::OnLoadTable)
	EVT_MENU(mxID_INSPECTION_MANAGE_TABLES,mxInspectionGrid::OnManageTables)
	EVT_MENU(mxID_INSPECTION_SET_FRAMELESS,mxInspectionGrid::OnSetFrameless)
	EVT_MENU(mxID_INSPECTION_SHOW_APPART,mxInspectionGrid::OnShowAppart)
	EVT_MENU(mxID_INSPECTION_COPY_ALL,mxInspectionGrid::OnCopyAll)
	EVT_SIZE(mxInspectionGrid::OnResize)
	EVT_GRID_COL_SIZE(mxInspectionGrid::OnColResize)
	EVT_GRID_LABEL_RIGHT_CLICK(mxInspectionGrid::OnLabelPopup)
	EVT_MENU_RANGE(mxID_COL_ID, mxID_COL_ID+IG_COLS_COUNT,mxInspectionGrid::OnShowHideCol)
//	EVT_GRID_SELECT_CELL(mxInspectionGrid::OnSelectCell)
END_EVENT_TABLE()

		
//static const wxString inspect_watch_options[] = {
//	_T("no"),
//	_T("lectura y escritura"),
//	_T("lectura"),
//	_T("escritura")
// };
//
//static const wxString inspect_format_options[] = {
//	_T("natural"),
//	_T("binary"),
//	_T("octal"),
//	_T("decimal"),
//	_T("hexadecimal")
// };

	
mxInspectionGrid::mxInspectionGrid(wxWindow *parent, wxWindowID id ):wxGrid(parent,id, wxDefaultPosition,wxSize(400,300),wxWANTS_CHARS) {
	
	disable_colour = wxColour(100,100,100);
	default_colour = wxColour(0,0,0);
	change_colour = wxColour(196,0,0);
	special_colour = wxColour(0,150,0);
	freeze_colour = wxColour(0,100,200);
	
	can_drop=true;
	ignore_changing=true;
	created=false;
	CreateGrid(0,IG_COLS_COUNT);
	SetColLabelValue(IG_COL_LEVEL,LANG(INSPECTGRID_LEVEL,"Nivel"));
	SetColLabelValue(IG_COL_EXPR,LANG(INSPECTGRID_EXPRESSION,"Expresion"));
	SetColLabelValue(IG_COL_TYPE,LANG(INSPECTGRID_TYPE,"Tipo"));
	SetColLabelValue(IG_COL_VALUE,LANG(INSPECTGRID_VALUE,"Valor"));
	SetColLabelValue(IG_COL_FORMAT,LANG(INSPECTGRID_FORMAT,"Formato"));
	SetColLabelValue(IG_COL_WATCH,LANG(INSPECTGRID_WATCH,"WatchPoint"));
	SetColLabelAlignment(wxALIGN_CENTRE,wxALIGN_CENTRE);
	SetRowLabelSize(0);
	SetSelectionMode(wxGrid::wxGridSelectRows);
	SetCellHighlightPenWidth(0);
	EnableDragRowSize(false);	
	
	// la ayuda de wx dice que SetColMinimalAcceptableWidth(muy bajo) podría penalizar la performance cuando busca en que celda
	// clickeo el usuario segun las coordenadas de pantalla, pero asumo que como hay muy pocas columnas no debería notarse
	SetColMinimalAcceptableWidth(1);
	for (int i=0;i<IG_COLS_COUNT;i++)
		SetColMinimalWidth(i,GetColMinimalAcceptableWidth());
	
	old_size=100;
	cols_sizes[IG_COL_LEVEL]=8;
	cols_sizes[IG_COL_EXPR]=29;
	cols_sizes[IG_COL_TYPE]=12;
	cols_sizes[IG_COL_VALUE]=27;
	cols_sizes[IG_COL_FORMAT]=11;
	cols_sizes[IG_COL_WATCH]=12;
	cols_visibles=config->Cols.inspections_grid;
//	cols_marginal=0;
	for (int i=0;i<IG_COLS_COUNT;i++)
		if (!cols_visibles[i]) {
			old_size-=int(cols_sizes[i]);
			cols_sizes[i]=-cols_sizes[i];
			SetColSize(i,GetColMinimalWidth(i));
//			cols_marginal-=GetColMinimalWidth(i);
		}
//	wxGridCellAttr *attr_format = new wxGridCellAttr;
//	wxGridCellAttr *attr_watch = new wxGridCellAttr;
//	attr_watch->SetEditor(new wxGridCellChoiceEditor(WXSIZEOF(inspect_watch_options),inspect_watch_options));
//	attr_format->SetEditor(new wxGridCellChoiceEditor(WXSIZEOF(inspect_format_options),inspect_format_options));
//	SetColAttr(IG_COL_WATCH,attr_watch);
//	SetColAttr(IG_COL_FORMAT,attr_format);
	AddRow();
	SetDropTarget(new mxInspectionDropTarget(this));
	created=true;
	ignore_changing=false;
}

mxInspectionGrid::~mxInspectionGrid() {
}

void mxInspectionGrid::OnKey(wxKeyEvent &event) {
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
	} else if (event.GetKeyCode()==WXK_DELETE) {
//		wxGridCellCoordsArray sel = GetSelectedCells();
//		DEBUG_INFO(sel.Count());
//		wxArrayInt a = GetSelectedRows();
//		for (int i=a.Count()-1;i>=0;i--) {
//			if (a[i]+1!=GetNumberRows() && debug->DeleteInspection(a[i]))
//				DeleteRows(a[i]);
//		}
		if (r+1!=GetNumberRows() &&
			( (debug->debugging && debug->DeleteInspection(r)) || 
			(!debug->debugging && debug->OffLineInspectionDelete(r)) ) )
				DeleteRows(r);
		SetGridCursor(r,0);
	} else if (event.GetKeyCode()==WXK_INSERT) {
		SetGridCursor(GetNumberRows()-1,0);
		SelectRow(GetGridCursorRow());
	} else
		event.Skip();
}

void mxInspectionGrid::AddRow(int cant) {
	int n = GetNumberRows();
	AppendRows(cant);
	for (int i=0;i<cant;i++) {
		SetReadOnly(n+i,IG_COL_LEVEL);
		SetReadOnly(n+i,IG_COL_FORMAT);
		SetReadOnly(n+i,IG_COL_TYPE);
		SetReadOnly(n+i,IG_COL_VALUE);
		SetReadOnly(n+i,IG_COL_WATCH);
	}
}

void mxInspectionGrid::OnCellChange(wxGridEvent &event) {
	if (!debug->debugging) {
		if (event.GetCol()==IG_COL_EXPR) {
			wxString value = GetCellValue(event.GetRow(),event.GetCol());
			if (!debug->OffLineInspectionModify(event.GetRow(),value)) {
				mxMessageDialog(main_window,LANG(INSPECTGRID_ERROR_OFFLINE_MODIFY,"Error al modificar la inspeccion."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
				event.Veto();
			}
			return;
		}
		mxMessageDialog(main_window,LANG(INSPECTGRID_NO_DEBUGGER_RUNNING,"Actualmente no se esta depurando ningun programa.\nPara comenzar la depuracion utilice la opcion \"Iniciar\" del menu \"Depuracion\"."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
		event.Veto();
		return;
	} else if (debug->running) {
		mxMessageDialog(main_window,LANG(INSPECTGRID_CANT_MODIFY_WHILE_RUNNING,"Debe interrumpir la ejecucion para poder modificar las inspecciones."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
		event.Veto();
		return;
	}
	if (ignore_changing) { 
		event.Veto();
		return;
	}
	ignore_changing=true;
	if (event.GetCol()==IG_COL_WATCH) {
		wxString value = GetCellValue(event.GetRow(),event.GetCol());
		if (!debug->ModifyInspectionWatch(event.GetRow(),LANG(INSPECTGRID_WATCH_WRITE,"lectura"),value.Contains(LANG(INSPECTGRID_WATCH_WRITE,"escritura"))))
			SetCellValue(event.GetRow(),event.GetCol(),LANG(INSPECTGRID_WATCH_WRITE,"no"));
	} else if (event.GetCol()==IG_COL_EXPR) {
		wxString expr = GetCellValue(event.GetRow(),event.GetCol());
		if (event.ShiftDown()) {
		} else {
			if (!ModifyExpresion(event.GetRow(),expr))
				event.Veto();
		}
	} else if (event.GetCol()==IG_COL_VALUE) {
		int num = selected_row = event.GetRow();
		wxString nexpr = GetCellValue(num,event.GetCol());
		if (debug->inspections[num].frameless) { // si no tiene scope, primero hay que hacerlo (para que sea un vo)
			wxCommandEvent cevt;
			OnReScope(cevt);
			if (!debug->inspections[num].on_scope) {
				OnSetFrameless(cevt);
				event.Veto();
				mxMessageDialog(main_window,LANG(INSPECTGRID_COULDNOT_RESCOPE_FOR_MODIFY,"La expresion no pudo ser evaluada en el ambito actual para su modificacion."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
				ignore_changing=false;
				return;
			}
		}
		if (!debug->ModifyInspectionValue(num,nexpr)) {
			event.Veto();
			mxMessageDialog(main_window,LANG(INSPECTGRID_NO_MODIFIABLE_EXPRESSION,"La expresion no puede ser modificada."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
		} else {
			SetCellValue(num,event.GetCol(),nexpr);
		}
	} else if (event.GetCol()==IG_COL_FORMAT) {
		wxString format=GetCellValue(event.GetRow(),event.GetCol());
		if (format==LANG(INSPECTGRID_FORMAT_NATURAL,"natural")) format=LANG(INSPECTGRID_FORMAT_NATURAL,"natural");
		if (format==LANG(INSPECTGRID_FORMAT_OCTAL,"octal")) format=LANG(INSPECTGRID_FORMAT_OCTAL,"octal");
		if (format==LANG(INSPECTGRID_FORMAT_DECIMAL,"decimal")) format=LANG(INSPECTGRID_FORMAT_DECIMAL,"decimal");
		if (format==LANG(INSPECTGRID_FORMAT_BINARY,"binario")) format=LANG(INSPECTGRID_FORMAT_BINARY,"binary");
		if (format==LANG(INSPECTGRID_FORMAT_HEXADECIMAL,"hexadecimal")) format=LANG(INSPECTGRID_FORMAT_HEXADECIMAL,"hexadecimal");
		debug->ModifyInspectionFormat(event.GetRow(),format);
	}
	ignore_changing=false;
}

void mxInspectionGrid::OnDoubleClick(wxGridEvent &event) {
	if (event.GetCol()==IG_COL_VALUE) {
		debug->BreakCompoundInspection(event.GetRow());
	} else if (event.GetCol()==IG_COL_LEVEL) {
		SelectRow(selected_row=event.GetRow());
		if (selected_row<debug->inspections_count) {
			wxCommandEvent evt;
			if (debug->inspections[selected_row].frameless)
				OnReScope(evt);
			else if (debug->inspections[selected_row].is_vo)
				OnSetFrameless(evt);
		}
	} else if (event.GetCol()==IG_COL_WATCH) {
		if (GetCellValue(selected_row,IG_COL_WATCH).Len()) {
			SelectRow(selected_row=event.GetRow());
			if (debug->inspections[selected_row].is_vo && !debug->inspections[selected_row].frameless) {
				wxMenu menu;
				menu.Append(mxID_INSPECTION_WATCH_NO,LANG(INSPECTGRID_WATCH_NO,"no"));
				menu.Append(mxID_INSPECTION_WATCH_RW,LANG(INSPECTGRID_WATCH_READ_WRITE,"lectura y escritura"));
				menu.Append(mxID_INSPECTION_WATCH_READ,LANG(INSPECTGRID_WATCH_READ,"lectura"));
				menu.Append(mxID_INSPECTION_WATCH_WRITE,LANG(INSPECTGRID_WATCH_WRITE,"escritura"));
				PopupMenu(&menu);
			}
		}
	}
	else if (event.GetCol()==IG_COL_FORMAT) {
		if (GetCellValue(selected_row,IG_COL_FORMAT).Len() && GetCellValue(selected_row,IG_COL_FORMAT)!=LANG(INSPECTGRID_TYPE_STRUCT,"estructura") && GetCellValue(selected_row,IG_COL_FORMAT)!=LANG(INSPECTGRID_TYPE_ARRAY,"arreglo") ) {
			SelectRow(selected_row=event.GetRow());
			wxMenu menu;
			menu.Append(mxID_INSPECTION_FORMAT_NAT,LANG(INSPECTGRID_FORMAT_NATURAL,"natural"));
			menu.Append(mxID_INSPECTION_FORMAT_BIN,LANG(INSPECTGRID_FORMAT_BINARY,"binario"));
			menu.Append(mxID_INSPECTION_FORMAT_OCT,LANG(INSPECTGRID_FORMAT_OCTAL,"octal"));
			menu.Append(mxID_INSPECTION_FORMAT_DEC,LANG(INSPECTGRID_FORMAT_DECIMAL,"decimal"));
			menu.Append(mxID_INSPECTION_FORMAT_HEX,LANG(INSPECTGRID_FORMAT_HEXADECIMAL,"hexadecimal"));
			PopupMenu(&menu);
		}
	}
//	if (event.GetCol()==IG_COL_VALUE && event.GetRow()<debug->inspections_count)
//		new mxInspectionExplorer(main_window,debug->inspections[event.GetRow()].expr,debug->inspections[event.GetRow()].expr);
}

void mxInspectionGrid::OnLabelPopup(wxGridEvent &event) {
	wxMenu menu;
	menu.AppendCheckItem(mxID_COL_ID+IG_COL_LEVEL,LANG(INSPECTGRID_SHRINK_LEVEL,"Encoger  Columna \"Nivel\""))->Check(!cols_visibles[IG_COL_LEVEL]);
	menu.AppendCheckItem(mxID_COL_ID+IG_COL_EXPR,LANG(INSPECTGRID_SHRINK_EXPRESSION,"Encoger  Columna \"Expresion\""))->Check(!cols_visibles[IG_COL_EXPR]);
	menu.AppendCheckItem(mxID_COL_ID+IG_COL_TYPE,LANG(INSPECTGRID_SHRINK_TYPE,"Encoger  Columna \"Tipo\""))->Check(!cols_visibles[IG_COL_TYPE]);
	menu.AppendCheckItem(mxID_COL_ID+IG_COL_VALUE,LANG(INSPECTGRID_SHRINK_VALUE,"Encoger  Columna \"Valor\""))->Check(!cols_visibles[IG_COL_VALUE]);
	menu.AppendCheckItem(mxID_COL_ID+IG_COL_FORMAT,LANG(INSPECTGRID_SHRINK_FORMAT,"Encoger Columna \"Formato\""))->Check(!cols_visibles[IG_COL_FORMAT]);
	menu.AppendCheckItem(mxID_COL_ID+IG_COL_WATCH,LANG(INSPECTGRID_SHRINK_WATCH,"Encoger Columna \"WatchPoint\""))->Check(!cols_visibles[IG_COL_WATCH]);
	PopupMenu(&menu);
}

void mxInspectionGrid::OnShowHideCol(wxCommandEvent &evt) {
	int cn=evt.GetId()-mxID_COL_ID;
	float sum=0, fs=cols_sizes[cn];
	int mw=cols_visibles[cn]?GetColMinimalWidth(cn):-GetColMinimalWidth(cn);
	for (int i=0;i<IG_COLS_COUNT;i++)
		if (cols_visibles[i] && i!=cn) 
			sum+=cols_sizes[i];
	cols_visibles[cn]=!cols_visibles[cn];
	cols_sizes[cn]=-cols_sizes[cn];
	if (!cols_visibles[cn])
		SetColSize(cn,mw);
	else
		SetColSize(cn,int(cols_sizes[cn]));
//	cols_marginal+=mw;
	fs-=mw;
	for (int i=0;i<IG_COLS_COUNT;i++)
		if (cols_visibles[i] && i!=cn) {
			cols_sizes[i]+=cols_sizes[i]/sum*fs;
			SetColSize(i,int(cols_sizes[i]));
		}
	Refresh();
}

void mxInspectionGrid::OnRightClick(wxGridEvent &event) {
	selected_row = event.GetRow();
	this->SetGridCursor(selected_row,event.GetCol());
	this->SelectRow(selected_row);
	wxMenu menu; //(selected_row<debug->inspections_count?debug->inspections[selected_row].expr:wxString(_T("<inspecciones>")));
//	menu.AppendSeparator();
	if (debug->debugging) {
		inspectinfo &ii=debug->inspections[selected_row];
		if (selected_row<debug->inspections_count && ii.on_scope && !ii.frameless) {
			if (ii.is_class&&ii.is_vo) menu.Append(mxID_INSPECTION_BREAK,LANG(INSPECTGRID_POPUP_SPLIT_CLASS,"&Separar clase en atributos"));
			if (ii.is_array&&ii.is_vo) menu.Append(mxID_INSPECTION_BREAK,LANG(INSPECTGRID_POPUP_SPLIT_ARRAY,"&Separar arreglo en elementos"));
			menu.Append(mxID_INSPECTION_SHOW_IN_TABLE,LANG(INSPECTGRID_POPUP_SHOW_IN_TABLE,"&Mostrar en tabla separada"));
			menu.Append(mxID_INSPECTION_SHOW_IN_TEXT,LANG(INSPECTGRID_POPUP_SHOW_IN_TEXT,"&Mostrar en ventana separada"));
			if (ii.is_vo) menu.Append(mxID_INSPECTION_EXPLORE,LANG(INSPECTGRID_POPUP_EXPLORE,"&Explorar datos"));
			menu.Append(mxID_INSPECTION_RESCOPE,LANG(INSPECTGRID_POPUP_SET_CURRENT_FRAME,"Evaluar en el &ambito actual"));
			menu.Append(mxID_INSPECTION_SET_FRAMELESS,LANG(INSPECTGRID_POPUP_SET_NO_FRAME,"&Independizar del ambito"));
		} else if (selected_row<debug->inspections_count) {
			menu.Append(mxID_INSPECTION_SHOW_IN_TEXT,LANG(INSPECTGRID_POPUP_SHOW_IN_TEXT,"&Mostrar en ventana separada"));
			menu.Append(mxID_INSPECTION_RESCOPE,LANG(INSPECTGRID_POPUP_SET_CURRENT_FRAME,"Evaluar en el &ambito actual"));
			if (!ii.frameless) menu.Append(mxID_INSPECTION_SET_FRAMELESS,LANG(INSPECTGRID_POPUP_SET_NO_FRAME,"&Independizar del ambito"));
		}
		if (debug->inspections_count)
			menu.Append(mxID_INSPECTION_EXPLORE_ALL,LANG(INSPECTGRID_POPUP_EXPLORE_ALL,"Explorar &todos los datos"));
	}
	if (selected_row<debug->inspections_count) {
		if (debug->debugging) menu.Append(mxID_INSPECTION_DUPLICATE,LANG(INSPECTGRID_POPUP_DUPLICATE_EXPRESSION,"Duplicar Inspeccion"));
		menu.Append(mxID_INSPECTION_COPY_EXPRESSION,LANG(INSPECTGRID_POPUP_COPY_EXPRESSION,"Copiar E&xpresion"));
		menu.Append(mxID_INSPECTION_COPY_DATA,LANG(INSPECTGRID_POPUP_COPY_DATA,"&Copiar Valor"));
		if (debug->inspections[selected_row].freezed)
			menu.Append(mxID_INSPECTION_FREEZE,LANG(INSPECTGRID_POPUP_UNFREEZE_VALUE,"Desco&ngelar Valor"));
		else
			menu.Append(mxID_INSPECTION_FREEZE,LANG(INSPECTGRID_POPUP_FREEZE_VALUE,"Co&ngelar Valor"));
	}
	if (debug->inspections_count)
		menu.Append(mxID_INSPECTION_COPY_ALL,LANG(INSPECTGRID_POPUP_COPY_ALL,"Copiar Toda la Ta&bla"));
	if (main_window->notebook_sources->GetPageCount()>0) {
		mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
		int s = source->GetSelectionStart(), e = source->GetSelectionEnd();
		if (s!=e && source->LineFromPosition(s)==source->LineFromPosition(e))
			menu.Append(mxID_INSPECTION_FROM_SOURCE,LANG(INSPECTGRID_POPUP_COPY_FROM_SELECTION,"Insertar Expresion Desde la &Seleccion"));
	}
	if (wxTheClipboard->Open()) {
		wxTextDataObject clip_data;
		if (wxTheClipboard->GetData(clip_data)) {
			if (clip_data.GetText().Find('\n')==wxNOT_FOUND)
				menu.Append(mxID_INSPECTION_FROM_CLIPBOARD,LANG(INSPECTGRID_POPUP_COPY_FROM_CLIPBOARD,"Pegar Expresion Desde el &Portapapeles"));
		}
		wxTheClipboard->Close();
	}
	if (debug->inspections_count) {
		menu.AppendSeparator();
		if (selected_row<debug->inspections_count)
			menu.Append(mxID_INSPECTION_CLEAR_ONE,LANG(INSPECTGRID_POPUP_DELETE,"Eliminar Inspeccion"));
		menu.Append(mxID_INSPECTION_CLEAR_ALL,LANG(INSPECTGRID_POPUP_CLEAN_TABLE,"&Limpiar Tabla de Inspeccion"));
		menu.Append(mxID_INSPECTION_SAVE_TABLE,LANG(INSPECTGRID_POPUP_SAVE_TABLE,"&Guardar Lista de Inspecciones..."));
	}
	if (debug->inspections_tables.size()) {
		menu.Append(mxID_INSPECTION_LOAD_TABLE,LANG(INSPECTGRID_POPUP_LOAD_TABLE,"Ca&rgar Lista de Inspecciones..."));
		menu.Append(mxID_INSPECTION_MANAGE_TABLES,LANG(INSPECTGRID_POPUP_MANAGE_TABLES,"Administrar Listas de Inspecciones..."));
	}
	
	PopupMenu(&menu);
}

void mxInspectionGrid::OnBreakClassOrArray(wxCommandEvent &evt) {
	debug->BreakCompoundInspection(selected_row);
}

void mxInspectionGrid::OnPasteFromClipboard(wxCommandEvent &evt) {
	if (!wxTheClipboard->Open()) return;
	wxTextDataObject clip_data;
	if (wxTheClipboard->GetData(clip_data)) {
		wxString expr = clip_data.GetText();
		if (expr.Len()) ModifyExpresion(selected_row,expr);
	}
	wxTheClipboard->Close();
}

void mxInspectionGrid::OnCopyFromSelecction(wxCommandEvent &evt) {
	if (main_window->notebook_sources->GetPageCount()>0) {
		mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
		int s = source->GetSelectionStart(), e = source->GetSelectionEnd();
		wxString expr = source->GetTextRange(s,e);
		if (expr.Len()) ModifyExpresion(selected_row,expr);
	}
}

bool mxInspectionGrid::ModifyExpresion(int row, wxString expr) {
	ignore_changing=true;
	while (expr.Len()) {
		if (expr.StartsWith(">?")) expr=">help "+expr.Mid(2);
		if (expr.Trim()==">help") { 
			wxMessageBox(debug->GetMacroOutput("help user-defined",true),"help user-defined");
			return false;
		} else if (expr.StartsWith(">help")) {
			wxMessageBox(debug->GetMacroOutput(expr.Mid(1),true),expr.Mid(1));
			return false;
		}
		int sd=ShouldDivide(row,expr);
		if (sd==0) {
			if (debug->ModifyInspection(row,expr)) {
				SetCellValue(row,IG_COL_EXPR,expr);
				if (row==GetNumberRows()-2) {
					SelectRow(row+1);
					SetGridCursor(row+1,IG_COL_EXPR);
//					} else {
//						SelectRow(row);
//						SetGridCursor(row,IG_COL_EXPR);
				}
				break;
			}
			if (!debug->debugging) {
				mxMessageDialog(main_window,LANG(INSPECTGRID_DEBUGGING_HAS_ENDED,"La sesion de depuracion ha finalizado."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
				return ignore_changing=false;
			} else
				if (debug->last_error==_T("<killed>")) {
					mxMessageDialog(main_window,LANG(INSPECTGRID_DEBUGGER_ERROR,"Ha ocurrido un error en el depurador."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
					return ignore_changing=false;
				} else if (debug->last_error.Len()==0 || debug->last_error.Find(_T("unable to create variable object"))!=wxNOT_FOUND)
					expr = mxGetTextFromUser(LANG(INSPECTGRID_INCORRECT_EXPRESSION,"La expresion ingresada no es correcta."),LANG(INSPECTGRID_MODIFY_CAPTION,"Modificar Expresion"),expr,main_window);
				else
					expr = mxGetTextFromUser(wxString(LANG(INSPECTGRID_CANT_EVALUATE,"No se puede evaluar la expresion:\n"))<<debug->last_error,LANG(INSPECTGRID_MODIFY_CAPTION,"Modificar Expresion"),expr,main_window);
		} else if (sd==1) break;
		if (!expr.Len()) return ignore_changing=false;
	}
	ignore_changing=false;
	return true;
}

void mxInspectionGrid::OnExploreExpression(wxCommandEvent &evt) {
	if (selected_row<=debug->inspections_count)
		new mxInspectionExplorer(debug->inspections[selected_row].frame,debug->inspections[selected_row].expr);
}

void mxInspectionGrid::OnCopyData(wxCommandEvent &evt) {
	if (!wxTheClipboard->Open()) return;
	wxTextDataObject clip_data;
	if (selected_row<=debug->inspections_count)
		wxTheClipboard->SetData(new wxTextDataObject(GetCellValue(selected_row,IG_COL_VALUE)));
	wxTheClipboard->Close();
}

void mxInspectionGrid::OnCopyExpression(wxCommandEvent &evt) {
	if (!wxTheClipboard->Open()) return;
	wxTextDataObject clip_data;
	if (selected_row<=debug->inspections_count)
		wxTheClipboard->SetData(new wxTextDataObject(GetCellValue(selected_row,IG_COL_EXPR)));
	wxTheClipboard->Close();
}

void mxInspectionGrid::OnClearAll(wxCommandEvent &evt) {
	if (debug->debugging)
		debug->ClearInspections();
	else {
		if (debug->OffLineInspectionDelete())
			DeleteRows(0,GetNumberRows()-1);
		else
			mxMessageDialog(main_window,LANG(INSPECTGRID_ERROR_OFFLINE_DELETE,"Error al eliminar inspecciones."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
	}	
}

void mxInspectionGrid::OnClearOne(wxCommandEvent &evt) {
	if (debug->debugging) {
		if (selected_row+1!=GetNumberRows() && debug->DeleteInspection(selected_row))
			DeleteRows(selected_row);
		SetGridCursor(selected_row,0);
	} else {
		if (debug->OffLineInspectionDelete(selected_row))
			DeleteRows(selected_row,1);
		else
			mxMessageDialog(main_window,LANG(INSPECTGRID_ERROR_OFFLINE_DELETE,"Error al eliminar inspecciones."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
	}
}

void mxInspectionGrid::OnExploreAll(wxCommandEvent &event) {
	wxArrayString array,frames;
	for (int i=0;i<debug->inspections_count;i++) {
		if (!debug->inspections[i].is_vo) continue;
		array.Add(debug->inspections[i].expr);
		frames.Add(debug->inspections[i].frame);
	}
	new mxInspectionExplorer(LANG(INSPECTEXPLORER_CAPTION,"Explorador de Inspecciones"),frames,array);
}

void mxInspectionGrid::AppendInspections(wxArrayString &vars) {
	int r = GetNumberRows()-1;
	for (unsigned int i=0;i<vars.GetCount();i++) {
		if (debug->ModifyInspection(r,vars[i]))
			SetCellValue(r++,IG_COL_EXPR,vars[i]);
	}
}


/**
* @retval 0   expresion unica que no se divide
* @retval 1   expresion que se divide pero incorrecta
* @retval 2   expresion que se dividio correctamente
**/
int mxInspectionGrid::ShouldDivide(int row, wxString expr, bool frameless) {
	int c = 0, i, l = expr.Len();
	bool comillas = false;
	for (i=0; i<l; i++) {
		if (expr[i]=='\"') comillas = !comillas;
		else if (!comillas && expr[i]=='.') {
			c++;
			if (c==3) 
				break;
		} else 
			c = 0;
	}
	if (i!=l) {
		int t1=i-3, f1=i-3, t2=i+1, f2=i+1;
		long n1=0, n2=0;
		while (f1>0 && expr[f1]>='0' && expr[f1]<='9')
			f1--;
		l--;
		while (t2<l && expr[t2]>='0' && expr[t2]<='9')
			t2++;
		expr.SubString(f1+1,t1).ToLong(&n1);
		expr.SubString(f2,t2).ToLong(&n2);
		if (n2<n1) swap(n2,n1);
		if (n2>=n1) {
			wxString p1 = expr.Mid(0,f1+1), p2 = expr.Mid(t2);
			if (debug->ModifyInspection(row,wxString(p1)<<n1<<p2)) {
				SetCellValue(row,IG_COL_EXPR,wxString(p1)<<n1<<p2);
				if (n2-n1)
					debug->MakeRoomForInspections(row+1,n2-n1);
				for (i = n1+1; i<=n2; i++) {
					row++;
					SetCellValue(row,IG_COL_EXPR,wxString(p1)<<i<<p2);
//					if (frameless)
//						debug->ModifyInspectionFrameless(row,wxString(p1)<<i<<p2,true);
//					else
						debug->ModifyInspection(row,wxString(p1)<<i<<p2,true);
				}
			} else return false;
		}
		return true;
	}
	return false;
}

void mxInspectionGrid::OnWatchNo(wxCommandEvent &evt) {
	SetCellValue(selected_row,IG_COL_WATCH,LANG(INSPECTGRID_WATCH_NO,"no"));
	wxGridEvent event(wxID_ANY,0,this,selected_row,IG_COL_WATCH);
	OnCellChange(event);
}
void mxInspectionGrid::OnWatchReadWrite(wxCommandEvent &evt) {
	SetCellValue(selected_row,IG_COL_WATCH,LANG(INSPECTGRID_WATCH_READ_WRITE,"lectura y escritura"));
	wxGridEvent event(wxID_ANY,0,this,selected_row,IG_COL_WATCH);
	OnCellChange(event);
}
void mxInspectionGrid::OnWatchRead(wxCommandEvent &evt) {
	SetCellValue(selected_row,IG_COL_WATCH,LANG(INSPECTGRID_WATCH_READ,"lectura"));
	wxGridEvent event(wxID_ANY,0,this,selected_row,IG_COL_WATCH);
	OnCellChange(event);
}
void mxInspectionGrid::OnWatchWrite(wxCommandEvent &evt) {
	SetCellValue(selected_row,IG_COL_WATCH,LANG(INSPECTGRID_WATCH_WRITE,"escritura"));
	wxGridEvent event(wxID_ANY,0,this,selected_row,IG_COL_WATCH);
	OnCellChange(event);
}

void mxInspectionGrid::OnFormatNatural(wxCommandEvent &evt) {
	SetCellValue(selected_row,IG_COL_FORMAT,LANG(INSPECTGRID_FORMAT_NATURAL,"natural"));
	wxGridEvent event(wxID_ANY,0,this,selected_row,IG_COL_FORMAT);
	OnCellChange(event);
}
void mxInspectionGrid::OnFormatDecimal(wxCommandEvent &evt) {
	SetCellValue(selected_row,IG_COL_FORMAT,LANG(INSPECTGRID_FORMAT_DECIMAL,"decimal"));
	wxGridEvent event(wxID_ANY,0,this,selected_row,IG_COL_FORMAT);
	OnCellChange(event);
}
void mxInspectionGrid::OnFormatOctal(wxCommandEvent &evt) {
	SetCellValue(selected_row,IG_COL_FORMAT,LANG(INSPECTGRID_FORMAT_OCTAL,"octal"));
	wxGridEvent event(wxID_ANY,0,this,selected_row,IG_COL_FORMAT);
	OnCellChange(event);
}
void mxInspectionGrid::OnFormatHexadecimal(wxCommandEvent &evt) {
	SetCellValue(selected_row,IG_COL_FORMAT,LANG(INSPECTGRID_FORMAT_HEXADECIMAL,"hexadecimal"));
	wxGridEvent event(wxID_ANY,0,this,selected_row,IG_COL_FORMAT);
	OnCellChange(event);
}
void mxInspectionGrid::OnFormatBinary(wxCommandEvent &evt) {
	SetCellValue(selected_row,IG_COL_FORMAT,LANG(INSPECTGRID_FORMAT_BINARY,"binario"));
	wxGridEvent event(wxID_ANY,0,this,selected_row,IG_COL_FORMAT);
	OnCellChange(event);
}

void mxInspectionGrid::OnResize(wxSizeEvent &evt) {
	int w; // la segunda condicion es para evitar problemas en windows al agrandar una columna
	if (created && (w=evt.GetSize().GetWidth())!=old_size) {
		int mw=0; // para contar los pixeles que no cambian por las colapsadas
		for (int i=0;i<IG_COLS_COUNT;i++)
			if (!cols_visibles[i])
				mw+=GetColMinimalWidth(i);
		w-=mw;
		double p = double(w)/old_size;
		for (int i=0;i<IG_COLS_COUNT;i++)
			cols_sizes[i]*=p;
		old_size=w;
		for (int i=0;i<IG_COLS_COUNT;i++)
			if (cols_visibles[i])
				SetColSize(i,int(cols_sizes[i]));
	}
	evt.Skip();
}

void mxInspectionGrid::OnColResize(wxGridSizeEvent &evt) {
	if (!created) return;
	if (old_size) {
		int i=evt.GetRowOrCol();
		cols_sizes[i]=GetColSize(i);
		cols_visibles[i]=GetColMinimalWidth(i)!=cols_sizes[i];
	}
}

void mxInspectionGrid::OnShowInTable(wxCommandEvent &evt) {
	if (selected_row<=debug->inspections_count)
		new mxInspectionMatrix(selected_row);
}

void mxInspectionGrid::OnShowInText(wxCommandEvent &evt) {
	if (selected_row<=debug->inspections_count)
		new mxInspectionPrint(debug->inspections[selected_row].frame,debug->inspections[selected_row].expr,debug->inspections[selected_row].frameless);
}

void mxInspectionGrid::OnReScope(wxCommandEvent &event) {
	if (selected_row>=debug->inspections_count) return;
	wxString expr = GetCellValue(selected_row,IG_COL_EXPR);
	if (!debug->debugging) {
		mxMessageDialog(main_window,LANG(INSPECTGRID_DEBUGGING_HAS_ENDED,"La sesion de depuracion ha finalizado."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
		return;
	} else if (!debug->ModifyInspection(selected_row,expr,true)) {
		debug->inspections[selected_row].on_scope=false;
		SetCellValue(selected_row,IG_COL_EXPR,expr);
		main_window->inspection_ctrl->SetCellRenderer(selected_row,IG_COL_VALUE,new wxGridCellStringRenderer());
		HightlightDisable(selected_row);
		SetCellValue(selected_row,IG_COL_VALUE,LANG(INSPECTGRID_OUT_OF_SCOPE,"<<< Fuera de Scope >>>"));
	}
}

void mxInspectionGrid::OnSelectCell(wxGridEvent &event) {
	if (created) { // porque esto no anda?
		if (event.GetCol()==IG_COL_LEVEL || (event.GetRow()==debug->inspections_count && event.GetCol()!=IG_COL_EXPR)) {
			SetGridCursor(IG_COL_EXPR,event.GetRow());
		} else
			event.Skip();
	}
}

/**
* @brief Duplica una inspeccion de la grilla, colocando la copia justo debajo del original
**/
void mxInspectionGrid::OnDuplicate(wxCommandEvent &evt) {
	if (selected_row>=debug->inspections_count) return;
	if (debug->debugging) debug->DuplicateInspection(selected_row);
}

void mxInspectionGrid::OnSaveTable(wxCommandEvent &evt) {
	wxString name = mxGetTextFromUser(LANG(INSPECTGRID_TABLE_NAME,"Nombre de la lista:"),_T("Guardar Lista"),_T(""),this);
	if (name.Len()) debug->SaveInspectionsTable(name);
}

void mxInspectionGrid::OnLoadTable(wxCommandEvent &evt) {
	wxArrayString as;
	for (unsigned int i=0;i<debug->inspections_tables.size();i++)
		as.Add(debug->inspections_tables[i]->name);
	wxString name = wxGetSingleChoice(LANG(INSPECTGRID_LOAD_TABLE,"Cargar Lista:"),LANG(INSPECTGRID_INSPECTIONS_TABLE,"Lista de Inspecciones"),as);
	if (name.Len()) debug->LoadInspectionsTable(name);
}

void mxInspectionGrid::OnManageTables(wxCommandEvent &evt) {
	wxArrayString as;
	for (unsigned int i=0;i<debug->inspections_tables.size();i++)
		as.Add(debug->inspections_tables[i]->name);
	wxString name = wxGetSingleChoice(LANG(INSPECTGRID_DELETE_TABLE,"Eliminar Lista:"),LANG(INSPECTGRID_INSPECTIONS_TABLE,"Lista de Inspecciones"),as);
	if (name.Len() && mxMD_YES==mxMessageDialog(main_window,wxString(LANG(INSPECTGRID_CONFIRM_DELETE_TABLE_PRE,"Desea eliminar la lista \""))<<name<<LANG(INSPECTGRID_CONFIRM_DELETE_TABLE_POST,"\"?"),LANG(GENERAL_CONFIRM,"Confirmacion"),mxMD_YES_NO).ShowModal())
		debug->DeleteInspectionsTable(name);	
}

void mxInspectionGrid::OnSetFrameless(wxCommandEvent &evt) {
	debug->SetFramelessInspection(selected_row);
}

void mxInspectionGrid::OnShowAppart(wxCommandEvent &evt) {
	
}

void mxInspectionGrid::HightlightNone(int r) {
	SetCellTextColour(r,IG_COL_VALUE,default_colour);
}

void mxInspectionGrid::HightlightFreeze(int r) {
	SetCellTextColour(r,IG_COL_VALUE,freeze_colour);
}

void mxInspectionGrid::HightlightChange(int r) {
	SetCellTextColour(r,IG_COL_VALUE,change_colour);
}

void mxInspectionGrid::HightlightDisable(int r) {
	SetCellTextColour(r,IG_COL_VALUE,disable_colour);
}

void mxInspectionGrid::HightlightSpecial(int r) {
	SetCellTextColour(r,IG_COL_VALUE,special_colour);
}

void mxInspectionGrid::ResetChangeHightlights() {
	int i,r=GetNumberRows()-1;
	if (config->Debug.select_modified_inspections) {
		for (i=0;i<r;i++) {
			if (debug->inspections[i].freezed)
				SetCellTextColour(i,IG_COL_VALUE,freeze_colour);
			else if (debug->inspections[i].is_vo && !debug->inspections[i].on_scope)
				SetCellTextColour(i,IG_COL_VALUE,disable_colour);
			else
				SetCellTextColour(i,IG_COL_VALUE,default_colour);
		}
	} else {
		for (i=0;i<r;i++)
			SetCellTextColour(i,IG_COL_VALUE,default_colour);
	}
}

void mxInspectionGrid::OnFreeze(wxCommandEvent &evt) {
	if (selected_row>=debug->inspections_count) return;
	debug->ToggleInspectionFreeze(selected_row);
}

void mxInspectionGrid::OnCopyAll(wxCommandEvent &evt) {
	wxString text;
	unsigned int c[4]={IG_COL_LEVEL,IG_COL_EXPR,IG_COL_TYPE,IG_COL_VALUE}, w[3]={0,0,0};
	bool formatted_copy=true;
	if (formatted_copy) {
		for (int i=0;i<debug->inspections_count;i++) {
			for (int j=0;j<3;j++) {
				wxString sw; sw<<GetCellValue(i,c[j]);
				if (sw.Len()>w[j]) w[j]=sw.Len();
			}
		}
		for (int i=0;i<debug->inspections_count;i++) {
			for (int j=0;j<3;j++)
				text<<GetCellValue(i,c[j])<<wxString(' ',w[j]+3-GetCellValue(i,c[j]).Len());
			text<<GetCellValue(i,c[3])<<"\n";
		}
	} else {
		for (int i=0;i<debug->inspections_count;i++) {
			text<<GetCellValue(i,IG_COL_LEVEL)<<"\t";
			text<<GetCellValue(i,IG_COL_EXPR)<<"\t";
			text<<GetCellValue(i,IG_COL_TYPE)<<"\t";
			text<<GetCellValue(i,IG_COL_VALUE)<<"\n";
		}
	}
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData( new wxTextDataObject(text) );
		wxTheClipboard->Close();
	}
}

wxDragResult mxInspectionDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def) {
	if (!grid->CanDrop()) return wxDragCancel;
	int ux,uy,vx,vy;
	grid->GetScrollPixelsPerUnit(&ux,&uy);
	grid->GetViewStart(&vx,&vy);
	int r=grid->YToRow(y+vy*uy-grid->GetColLabelSize());
	if (r!=wxNOT_FOUND) grid->SelectRow(r);
	return wxDragCopy;
}


mxInspectionDropTarget::mxInspectionDropTarget(mxInspectionGrid *agrid) {
	grid=agrid; 
	SetDataObject(data=new wxTextDataObject());
}

bool mxInspectionDropTarget::OnDrop(wxCoord x, wxCoord y) {
	return true;
}

wxDragResult mxInspectionDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult ref) {
	if (!grid->CanDrop()) return wxDragCancel;
	int ux,uy,vx,vy;
	grid->GetScrollPixelsPerUnit(&ux,&uy);
	grid->GetViewStart(&vx,&vy);
	int r=grid->YToRow(y+vy*uy-grid->GetColLabelSize());
	if (r==wxNOT_FOUND) return wxDragCancel;
	GetData(); grid->SelectRow(r);
	wxString str=data->GetText();
	if (!str.size()) return wxDragCancel;
	str.Replace("\n"," ");
	str.Replace("\r"," ");
	grid->ModifyExpresion(r,str);
	return wxDragCopy;
}

bool mxInspectionGrid::CanDrop() {
	return can_drop;
}

void mxInspectionGrid::OnClick(wxGridEvent &event) {
	if (event.AltDown()) {
		SelectRow(event.GetRow());
		can_drop=false;
		wxTextDataObject my_data(GetCellValue(event.GetRow(),event.GetCol()));
		wxDropSource dragSource(this);
		dragSource.SetData(my_data);
		wxDragResult result = dragSource.DoDragDrop(wxDrag_AllowMove|wxDrag_DefaultMove);
		can_drop=true;
	} else event.Skip();
}

