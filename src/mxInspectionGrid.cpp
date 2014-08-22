//#include <wx/wx.h> // for wxGetSingleChoice, for some reasom <wx/choicdlg.h> doesn't work????
//#include <wx/choicdlg.h>
#include <wx/menu.h>
#include <wx/clipbrd.h>
#include "mxInspectionGrid.h"
#include "Language.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxInspectionGridCellEditor.h"
//#include "mxUtils.h"
//#include "DebugManager.h"
//#include "mxSource.h"
//#include "mxInspectionExplorer.h"
//#include "mxMessageDialog.h"
//#include "mxInspectionMatrix.h"
//#include "mxTextDialog.h"
//#include "mxInspectionPrint.h"
using namespace std;
 
BEGIN_EVENT_TABLE(mxInspectionGrid, wxGrid)
	EVT_GRID_CELL_CHANGE(mxInspectionGrid::OnCellChange)
	EVT_KEY_DOWN(mxInspectionGrid::OnKey)
//	EVT_GRID_CELL_LEFT_CLICK(mxInspectionGrid::OnClick)
//	EVT_GRID_CELL_LEFT_DCLICK(mxInspectionGrid::OnDoubleClick)
//	EVT_MENU(mxID_INSPECTION_FREEZE,mxInspectionGrid::OnFreeze)
	EVT_MENU(mxID_INSPECTION_BREAK,mxInspectionGrid::OnBreakClassOrArray)
	EVT_MENU(mxID_INSPECTION_RESCOPE,mxInspectionGrid::OnReScope)
	EVT_MENU(mxID_INSPECTION_SET_FRAMELESS,mxInspectionGrid::OnSetFrameless)
	EVT_MENU(mxID_INSPECTION_DUPLICATE,mxInspectionGrid::OnDuplicate)
//	EVT_MENU(mxID_INSPECTION_FROM_CLIPBOARD,mxInspectionGrid::OnPasteFromClipboard)
//	EVT_MENU(mxID_INSPECTION_FROM_SOURCE,mxInspectionGrid::OnCopyFromSelecction)
//	EVT_MENU(mxID_INSPECTION_SHOW_IN_TEXT,mxInspectionGrid::OnShowInText)
//	EVT_MENU(mxID_INSPECTION_SHOW_IN_TABLE,mxInspectionGrid::OnShowInTable)
//	EVT_MENU(mxID_INSPECTION_EXPLORE,mxInspectionGrid::OnExploreExpression)
//	EVT_MENU(mxID_INSPECTION_COPY_DATA,mxInspectionGrid::OnCopyData)
//	EVT_MENU(mxID_INSPECTION_COPY_EXPRESSION,mxInspectionGrid::OnCopyExpression)
//	EVT_MENU(mxID_INSPECTION_EXPLORE_ALL,mxInspectionGrid::OnExploreAll)
//	EVT_MENU(mxID_INSPECTION_CLEAR_ALL,mxInspectionGrid::OnClearAll)
//	EVT_MENU(mxID_INSPECTION_CLEAR_ONE,mxInspectionGrid::OnClearOne)
//	EVT_MENU(mxID_INSPECTION_WATCH_NO,mxInspectionGrid::OnWatchNo)
//	EVT_MENU(mxID_INSPECTION_WATCH_RW,mxInspectionGrid::OnWatchReadWrite)
//	EVT_MENU(mxID_INSPECTION_WATCH_READ,mxInspectionGrid::OnWatchRead)
//	EVT_MENU(mxID_INSPECTION_WATCH_WRITE,mxInspectionGrid::OnWatchWrite)
//	EVT_MENU(mxID_INSPECTION_FORMAT_NAT,mxInspectionGrid::OnFormatNatural)
//	EVT_MENU(mxID_INSPECTION_FORMAT_BIN,mxInspectionGrid::OnFormatBinary)
//	EVT_MENU(mxID_INSPECTION_FORMAT_DEC,mxInspectionGrid::OnFormatDecimal)
//	EVT_MENU(mxID_INSPECTION_FORMAT_OCT,mxInspectionGrid::OnFormatOctal)
//	EVT_MENU(mxID_INSPECTION_FORMAT_HEX,mxInspectionGrid::OnFormatHexadecimal)
//	EVT_MENU(mxID_INSPECTION_SAVE_TABLE,mxInspectionGrid::OnSaveTable)
//	EVT_MENU(mxID_INSPECTION_LOAD_TABLE,mxInspectionGrid::OnLoadTable)
//	EVT_MENU(mxID_INSPECTION_MANAGE_TABLES,mxInspectionGrid::OnManageTables)
//	EVT_MENU(mxID_INSPECTION_SHOW_APPART,mxInspectionGrid::OnShowAppart)
//	EVT_MENU(mxID_INSPECTION_COPY_ALL,mxInspectionGrid::OnCopyAll)
//	EVT_SIZE(mxInspectionGrid::OnResize)
//	EVT_GRID_COL_SIZE(mxInspectionGrid::OnColResize)
//	EVT_GRID_LABEL_RIGHT_CLICK(mxInspectionGrid::OnLabelPopup)
//	EVT_MENU_RANGE(mxID_COL_ID, mxID_COL_ID+IG_COLS_COUNT,mxInspectionGrid::OnShowHideCol)
////	EVT_GRID_SELECT_CELL(mxInspectionGrid::OnSelectCell)
END_EVENT_TABLE()
//	
	
class FlagGuard {
	bool *flag;
public:
	FlagGuard(bool &f, bool force):flag(&f) { *flag=true; }
	FlagGuard(bool &f):flag(&f) { if (*flag) { flag=NULL; } else *flag=true; }
	bool IsOk() { return flag!=NULL; }
	void Release() { if (flag) *flag=false; flag=NULL; }
	~FlagGuard() { if (flag) *flag=false; }
};

static struct mxIGStatusOpts {
	wxColour color;
	bool have_message;
	wxString message;
	bool editable_value;
	void Init(bool ev, const wxColour &c) { color=c; have_message=false; editable_value=ev; }
	void Init(bool ev, const wxColour &c, const wxString &m) { color=c; have_message=true; message=m; editable_value=ev; }
} mxig_status_opts[mxInspectionGrid::IGRS_COUNT];

mxInspectionGrid::mxInspectionGrid(wxWindow *parent) : mxGrid(parent,IG_COLS_COUNT) {
	
	FlagGuard icce_guard(ignore_cell_change_event,true);
	
	last_return_had_shift_down = mask_cell_change_event = false;
	
	mxig_status_opts[IGRS_UNINIT].Init(false,wxColour(100,100,100),"<<< evaluación pendiente >>>"); /// @todo: traducir
	mxig_status_opts[IGRS_OUT_OF_SCOPE].Init(false,wxColour(100,100,100),LANG(INSPECTGRID_OUT_OF_SCOPE,"<<< fuera de ámbito >>>"));  /// @todo: traducir
	mxig_status_opts[IGRS_IN_SCOPE].Init(true,wxColour(196,0,0));
	mxig_status_opts[IGRS_CHANGED].Init(true,wxColour(196,0,0));
	mxig_status_opts[IGRS_NORMAL].Init(true,wxColour(0,0,0));
	mxig_status_opts[IGRS_ERROR].Init(false,wxColour(196,0,0),"<<< Error >>>");
	mxig_status_opts[IGRS_FREEZE].Init(false,wxColour(0,100,200));
	
//	can_drop=true;
//	ignore_changing=true;
//	created=false;
	
	mxGrid::InitColumn(IG_COL_LEVEL,LANG(INSPECTGRID_LEVEL,"Nivel"),8);
	mxGrid::InitColumn(IG_COL_EXPR,LANG(INSPECTGRID_EXPRESSION,"Expresion"),29);
	mxGrid::InitColumn(IG_COL_TYPE,LANG(INSPECTGRID_TYPE,"Tipo"),12);
	mxGrid::InitColumn(IG_COL_VALUE,LANG(INSPECTGRID_VALUE,"Valor"),27);
	mxGrid::DoCreate();
	InsertRows();
	
	mxGrid::SetRowSelectionMode();
	
//	EnableDragRowSize(false);
	
//	// la ayuda de wx dice que SetColMinimalAcceptableWidth(muy bajo) podría penalizar la performance cuando busca en que celda
//	// clickeo el usuario segun las coordenadas de pantalla, pero asumo que como hay muy pocas columnas no debería notarse
//	SetColMinimalAcceptableWidth(1);
//	for (int i=0;i<IG_COLS_COUNT;i++)
//		SetColMinimalWidth(i,GetColMinimalAcceptableWidth());
//	
//	cols_sizes[IG_COL_LEVEL]=8;
//	cols_sizes[IG_COL_EXPR]=29;
//	cols_sizes[IG_COL_TYPE]=12;
//	cols_sizes[IG_COL_VALUE]=27;
////	cols_sizes[IG_COL_FORMAT]=11;
////	cols_sizes[IG_COL_WATCH]=12;
//	old_size=1; for(int i=0;i<IG_COLS_COUNT;i++) old_size+=cols_sizes[i];
//	cols_visibles=config->Cols.inspections_grid;
////	cols_marginal=0;
//	for (int i=0;i<IG_COLS_COUNT;i++)
//		if (!cols_visibles[i]) {
//			old_size-=int(cols_sizes[i]);
//			cols_sizes[i]=-cols_sizes[i];
//			SetColSize(i,GetColMinimalWidth(i));
////			cols_marginal-=GetColMinimalWidth(i);
//		}
////	wxGridCellAttr *attr_format = new wxGridCellAttr;
////	wxGridCellAttr *attr_watch = new wxGridCellAttr;
////	attr_watch->SetEditor(new wxGridCellChoiceEditor(WXSIZEOF(inspect_watch_options),inspect_watch_options));
////	attr_format->SetEditor(new wxGridCellChoiceEditor(WXSIZEOF(inspect_format_options),inspect_format_options));
////	SetColAttr(IG_COL_WATCH,attr_watch);
////	SetColAttr(IG_COL_FORMAT,attr_format);
//	AddRow();
//	SetDropTarget(new mxInspectionDropTarget(this));
//	created=true;
//	ignore_changing=false;
	SetColLabelSize(wxGRID_AUTOSIZE);
	
}



void mxInspectionGrid::OnKey(wxKeyEvent &event) {
	int key = event.GetKeyCode();
	if (key==WXK_DELETE) {
		vector<int> sel; int min=-1;
		if (mxGrid::GetSelectedRows(sel,true)==0) sel.push_back(GetGridCursorRow());
		for(unsigned int i=0;i<sel.size();i++) {
			if (sel[i]<0||sel[i]+1>=inspections.GetSize()) continue;
			if (sel[i]<min) min=sel[i];
			DeleteInspection(sel[i],false);
		}
		if (min!=-1) min=GetGridCursorRow();
		if (min<0||min>inspections.GetSize()) min=0;
		Select(min);
	} else if (key==WXK_DOWN) {
		int r = GetGridCursorRow();
		if (r+1!=GetNumberRows()) mxGrid::Select(r+1);
	} else if (key==WXK_UP) {
		int r = GetGridCursorRow();
		if (r) Select(r-1);
	} else if (event.GetKeyCode()==WXK_INSERT) {
		int row = GetGridCursorRow();
		InsertRows(row,1);
		mxGrid::Select(row,IG_COL_EXPR);
	} else if (key==WXK_RETURN) {
		last_return_had_shift_down=event.ShiftDown();
		event.Skip(); 
	} else
		event.Skip();
}

void mxInspectionGrid::OnCellChange(wxGridEvent &event) {
	event.Skip();
	FlagGuard icce_guard(ignore_cell_change_event);
	if (!icce_guard.IsOk()) return;
	wxString new_value = wxGrid::GetCellValue(event.GetRow(),event.GetCol());
	if (event.GetCol()==GetRealCol(IG_COL_EXPR)) {
		// caso especial, ayuda para las macros gdb
		if (new_value.StartsWith(">?")) new_value=">help "+new_value.Mid(2);
		if (new_value.Trim()==">help") { 
			wxMessageBox(debug->GetMacroOutput("help user-defined",true),"help user-defined");
			return;
		} else if (new_value.StartsWith(">help")) {
			wxMessageBox(debug->GetMacroOutput(new_value.Mid(1),true),new_value.Mid(1));
			return;
		}
		// caso normal, inspeccion
		int row = event.GetRow();
		if (row+1==inspections.GetSize()) InsertRows();
		if (!inspections[row].IsNull()) {
			if (inspections[row]->GetExpression()==new_value) return; // si en realidad no cambio
			DeleteInspection(row,true);
		}
		OnFullTableUpdateBegin();
		if (CreateInspection(row,new_value,last_return_had_shift_down)) Select(row+1);
		DebuggerInspection::UpdateAll(); // la expresion podría haber modificado algo
		OnFullTableUpdateEnd();
	} else {
		if (event.GetCol()==GetRealCol(IG_COL_VALUE)) {
			int row = event.GetRow(); 
			if (row<0||row>inspections.GetSize()||inspections[row].IsNull()) return;
			OnFullTableUpdateBegin();
			if (inspections[row]->ModifyValue(new_value)) {
				mxGrid::SetCellValue(row,IG_COL_VALUE,inspections[row]->GetValue());
				event.Skip(); 
				DebuggerInspection::UpdateAll(); // la expresion podría haber modificado algo
			} else event.Veto();
			OnFullTableUpdateEnd();
		}
	}
//	if (!debug->debugging) {
//		if (event.GetCol()==IG_COL_EXPR) {
//			wxString value = GetCellValue(event.GetRow(),event.GetCol());
//			if (!debug->OffLineInspectionModify(event.GetRow(),value)) {
//				mxMessageDialog(main_window,LANG(INSPECTGRID_ERROR_OFFLINE_MODIFY,"Error al modificar la inspeccion."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
//				event.Veto();
//			}
//			return;
//		}
//		mxMessageDialog(main_window,LANG(INSPECTGRID_NO_DEBUGGER_RUNNING,"Actualmente no se esta depurando ningun programa.\nPara comenzar la depuracion utilice la opcion \"Iniciar\" del menu \"Depuracion\"."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
//		event.Veto();
//		return;
//	} else if (debug->running) {
//		mxMessageDialog(main_window,LANG(INSPECTGRID_CANT_MODIFY_WHILE_RUNNING,"Debe interrumpir la ejecucion para poder modificar las inspecciones."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
//		event.Veto();
//		return;
//	}
//	if (ignore_changing) { 
//		event.Veto();
//		return;
//	}
//	ignore_changing=true;
////	if (event.GetCol()==IG_COL_WATCH) {
////		wxString value = GetCellValue(event.GetRow(),event.GetCol());
////		if (!debug->ModifyInspectionWatch(event.GetRow(),LANG(INSPECTGRID_WATCH_WRITE,"lectura"),value.Contains(LANG(INSPECTGRID_WATCH_WRITE,"escritura"))))
////			SetCellValue(event.GetRow(),event.GetCol(),LANG(INSPECTGRID_WATCH_WRITE,"no"));
////	} else 
//	if (event.GetCol()==IG_COL_EXPR) {
//		wxString expr = GetCellValue(event.GetRow(),event.GetCol());
//		if (event.ShiftDown()) {
//		} else {
//			if (!ModifyExpresion(event.GetRow(),expr))
//				event.Veto();
//		}
//	} else if (event.GetCol()==IG_COL_VALUE) {
//		int num = selected_row = event.GetRow();
//		wxString nexpr = GetCellValue(num,event.GetCol());
//		if (debug->inspections[num].frameless) { // si no tiene scope, primero hay que hacerlo (para que sea un vo)
//			wxCommandEvent cevt;
//			OnReScope(cevt);
//			if (!debug->inspections[num].on_scope) {
//				OnSetFrameless(cevt);
//				event.Veto();
//				mxMessageDialog(main_window,LANG(INSPECTGRID_COULDNOT_RESCOPE_FOR_MODIFY,"La expresion no pudo ser evaluada en el ambito actual para su modificacion."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
//				ignore_changing=false;
//				return;
//			}
//		}
//		if (!debug->ModifyInspectionValue(num,nexpr)) {
//			event.Veto();
//			mxMessageDialog(main_window,LANG(INSPECTGRID_NO_MODIFIABLE_EXPRESSION,"La expresion no puede ser modificada."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
//		} else {
//			SetCellValue(num,event.GetCol(),nexpr);
//		}
////	} else if (event.GetCol()==IG_COL_FORMAT) {
////		wxString format=GetCellValue(event.GetRow(),event.GetCol());
////		if (format==LANG(INSPECTGRID_FORMAT_NATURAL,"natural")) format=LANG(INSPECTGRID_FORMAT_NATURAL,"natural");
////		if (format==LANG(INSPECTGRID_FORMAT_OCTAL,"octal")) format=LANG(INSPECTGRID_FORMAT_OCTAL,"octal");
////		if (format==LANG(INSPECTGRID_FORMAT_DECIMAL,"decimal")) format=LANG(INSPECTGRID_FORMAT_DECIMAL,"decimal");
////		if (format==LANG(INSPECTGRID_FORMAT_BINARY,"binario")) format=LANG(INSPECTGRID_FORMAT_BINARY,"binary");
////		if (format==LANG(INSPECTGRID_FORMAT_HEXADECIMAL,"hexadecimal")) format=LANG(INSPECTGRID_FORMAT_HEXADECIMAL,"hexadecimal");
////		debug->ModifyInspectionFormat(event.GetRow(),format);
//	}
//	ignore_changing=false;
}

bool mxInspectionGrid::OnCellDoubleClick(int row, int col) {
	if (inspections[row].IsNull()) return false;
	if (col==GetRealCol(IG_COL_VALUE)) {
		if (inspections[row]->IsCompound()) 
			BreakCompoundInspection(row);
	} else if (col==GetRealCol(IG_COL_LEVEL)) {
		ChangeFrameless(row,!inspections[row]->IsFrameless(),true);
	}
	return true;
}
//	if (event.GetCol()==IG_COL_VALUE) {
//		debug->BreakCompoundInspection(event.GetRow());
//	} else if (event.GetCol()==IG_COL_LEVEL) {
//		SelectRow(selected_row=event.GetRow());
//		if (selected_row<debug->inspections_count) {
//			wxCommandEvent evt;
//			if (debug->inspections[selected_row].frameless)
//				OnReScope(evt);
//			else if (debug->inspections[selected_row].is_vo)
//				OnSetFrameless(evt);
//		}
////	} else if (event.GetCol()==IG_COL_WATCH) {
////		if (GetCellValue(selected_row,IG_COL_WATCH).Len()) {
////			SelectRow(selected_row=event.GetRow());
////			if (debug->inspections[selected_row].is_vo && !debug->inspections[selected_row].frameless) {
////				wxMenu menu;
////				menu.Append(mxID_INSPECTION_WATCH_NO,LANG(INSPECTGRID_WATCH_NO,"no"));
////				menu.Append(mxID_INSPECTION_WATCH_RW,LANG(INSPECTGRID_WATCH_READ_WRITE,"lectura y escritura"));
////				menu.Append(mxID_INSPECTION_WATCH_READ,LANG(INSPECTGRID_WATCH_READ,"lectura"));
////				menu.Append(mxID_INSPECTION_WATCH_WRITE,LANG(INSPECTGRID_WATCH_WRITE,"escritura"));
////				PopupMenu(&menu);
////			}
////		}
////	}
////	else if (event.GetCol()==IG_COL_FORMAT) {
////		if (GetCellValue(selected_row,IG_COL_FORMAT).Len() && GetCellValue(selected_row,IG_COL_FORMAT)!=LANG(INSPECTGRID_TYPE_STRUCT,"estructura") && GetCellValue(selected_row,IG_COL_FORMAT)!=LANG(INSPECTGRID_TYPE_ARRAY,"arreglo") ) {
////			SelectRow(selected_row=event.GetRow());
////			wxMenu menu;
////			menu.Append(mxID_INSPECTION_FORMAT_NAT,LANG(INSPECTGRID_FORMAT_NATURAL,"natural"));
////			menu.Append(mxID_INSPECTION_FORMAT_BIN,LANG(INSPECTGRID_FORMAT_BINARY,"binario"));
////			menu.Append(mxID_INSPECTION_FORMAT_OCT,LANG(INSPECTGRID_FORMAT_OCTAL,"octal"));
////			menu.Append(mxID_INSPECTION_FORMAT_DEC,LANG(INSPECTGRID_FORMAT_DECIMAL,"decimal"));
////			menu.Append(mxID_INSPECTION_FORMAT_HEX,LANG(INSPECTGRID_FORMAT_HEXADECIMAL,"hexadecimal"));
////			PopupMenu(&menu);
////		}
//	}
////	if (event.GetCol()==IG_COL_VALUE && event.GetRow()<debug->inspections_count)
////		new mxInspectionExplorer(main_window,debug->inspections[event.GetRow()].expr,debug->inspections[event.GetRow()].expr);
//}
//
//void mxInspectionGrid::OnLabelPopup(wxGridEvent &event) {
//	wxMenu menu;
//	menu.AppendCheckItem(mxID_COL_ID+IG_COL_LEVEL,LANG(INSPECTGRID_SHRINK_LEVEL,"Encoger  Columna \"Nivel\""))->Check(!cols_visibles[IG_COL_LEVEL]);
//	menu.AppendCheckItem(mxID_COL_ID+IG_COL_EXPR,LANG(INSPECTGRID_SHRINK_EXPRESSION,"Encoger  Columna \"Expresion\""))->Check(!cols_visibles[IG_COL_EXPR]);
//	menu.AppendCheckItem(mxID_COL_ID+IG_COL_TYPE,LANG(INSPECTGRID_SHRINK_TYPE,"Encoger  Columna \"Tipo\""))->Check(!cols_visibles[IG_COL_TYPE]);
//	menu.AppendCheckItem(mxID_COL_ID+IG_COL_VALUE,LANG(INSPECTGRID_SHRINK_VALUE,"Encoger  Columna \"Valor\""))->Check(!cols_visibles[IG_COL_VALUE]);
////	menu.AppendCheckItem(mxID_COL_ID+IG_COL_FORMAT,LANG(INSPECTGRID_SHRINK_FORMAT,"Encoger Columna \"Formato\""))->Check(!cols_visibles[IG_COL_FORMAT]);
////	menu.AppendCheckItem(mxID_COL_ID+IG_COL_WATCH,LANG(INSPECTGRID_SHRINK_WATCH,"Encoger Columna \"WatchPoint\""))->Check(!cols_visibles[IG_COL_WATCH]);
//	PopupMenu(&menu);
//}
//
//void mxInspectionGrid::OnShowHideCol(wxCommandEvent &evt) {
//	int cn=evt.GetId()-mxID_COL_ID;
//	float sum=0, fs=cols_sizes[cn];
//	int mw=cols_visibles[cn]?GetColMinimalWidth(cn):-GetColMinimalWidth(cn);
//	for (int i=0;i<IG_COLS_COUNT;i++)
//		if (cols_visibles[i] && i!=cn) 
//			sum+=cols_sizes[i];
//	cols_visibles[cn]=!cols_visibles[cn];
//	cols_sizes[cn]=-cols_sizes[cn];
//	if (!cols_visibles[cn])
//		SetColSize(cn,mw);
//	else
//		SetColSize(cn,int(cols_sizes[cn]));
////	cols_marginal+=mw;
//	fs-=mw;
//	for (int i=0;i<IG_COLS_COUNT;i++)
//		if (cols_visibles[i] && i!=cn) {
//			cols_sizes[i]+=cols_sizes[i]/sum*fs;
//			SetColSize(i,int(cols_sizes[i]));
//		}
//	Refresh();
//}
//

void mxInspectionGrid::OnCellPopupMenu(int row, int col) {
	// ensure that clicked cell is selected, so generated events will use that one
	// selection policy is: if theres a multiple selection, keep, else select only clicked cell
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	if (sel.size()==0) sel.push_back(row); 
	else if (sel.size()==1 && sel[0]!=row) { sel[0]=row; mxGrid::Select(row,col); }
	bool there_are_inspections = inspections.GetSize()>1;
	bool sel_is_single = sel.size()==1; // hay una sola inspeccion seleccionada
	bool sel_is_last = sel_is_single && row+1==inspections.GetSize(); // la seleccionada es la ultima de la tabla (en blanco, invalida)
	bool sel_is_vo = sel_is_single && !sel_is_last && inspections[row]->GetDbiType()==DIT_VARIABLE_OBJECT; // la seleccionada corresponde a una variable_object
	DebuggerInspection *di = (sel_is_single && !sel_is_last)?inspections[row].di:NULL; // puntero a la seleccionada si es unica y valida
	bool sel_has_vo = sel_is_vo; // si hay al menos una variable object seleccionada
	bool sel_has_frozen = false, sel_has_unfrozen=false; // si hay inspecciones congeladas y descongeladas
	if (!sel_has_vo && !sel_is_single) {
		for(unsigned int i=0;i<sel.size();i++) {
			if (sel[i]>=inspections.GetSize()) continue;
			DebuggerInspection *di = inspections[sel[i]].di;
			if (di->GetDbiType()==DIT_VARIABLE_OBJECT) { sel_has_vo=true; }
			if (di->IsFrozen()) sel_has_frozen=true; else sel_has_unfrozen=true; 
		}
	}
	bool there_are_sources = main_window->notebook_sources->GetPageCount()!=0;
	mxSource *current_source = there_are_sources?(mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection())):NULL;

#warning restablecer funcionalidad de todo lo que este comentado
	wxMenu menu; 
	if (sel_is_vo && di->IsClass()) menu.Append(mxID_INSPECTION_BREAK,LANG(INSPECTGRID_POPUP_SPLIT_CLASS,"&Separar clase en atributos"));
	if (sel_is_vo && di->IsArray()) menu.Append(mxID_INSPECTION_BREAK,LANG(INSPECTGRID_POPUP_SPLIT_ARRAY,"&Separar arreglo en elementos"));
//	wxMenu *extern_v = new wxMenu;
//		if (!sel_is_last && (!sel_is_vo || !di->IsSimpleType())) extern_v->Append(mxID_INSPECTION_SHOW_IN_TABLE,LANG(INSPECTGRID_POPUP_SHOW_IN_TABLE,"Mostrar en &tabla separada..."));
//		if (sel_is_single && !sel_is_last) extern_v->Append(mxID_INSPECTION_SHOW_IN_TEXT,LANG(INSPECTGRID_POPUP_SHOW_IN_TEXT,"Mostrar en &ventana separada..."));
//		if (sel_is_vo) extern_v->Append(mxID_INSPECTION_EXPLORE,LANG(INSPECTGRID_POPUP_EXPLORE,"&Explorar datos..."));
//	if (extern_v->GetMenuItemCount()) menu.AppendSubMenu(extern_v,LANG(INSPECTGRID_EXTERN_VISUALIZATION,"Otras &visualizaciones")); else delete extern_v;
//	if (inspections.GetSize()) menu.Append(mxID_INSPECTION_EXPLORE_ALL,LANG(INSPECTGRID_POPUP_EXPLORE_ALL,"Explorar &todos los datos"));
	if (sel_has_vo && !(sel_is_vo && !di->IsFrameless())) menu.Append(mxID_INSPECTION_RESCOPE,LANG(INSPECTGRID_POPUP_SET_CURRENT_FRAME,"Evaluar en el &ambito actual"));
	if (sel_has_vo && !(sel_is_vo && di->IsFrameless())) menu.Append(mxID_INSPECTION_SET_FRAMELESS,wxString(LANG(INSPECTGRID_POPUP_SET_NO_FRAME,"&Independizar del ambito"))+"\tCtrl+I");
	if (!sel_is_last) menu.Append(mxID_INSPECTION_DUPLICATE,wxString(LANG(INSPECTGRID_POPUP_DUPLICATE_EXPRESSION,"Duplicar Inspeccion"))+"\tCtrl+L");
//	if (!sel_is_last) menu.Append(mxID_INSPECTION_COPY_EXPRESSION,wxString(LANG(INSPECTGRID_POPUP_COPY_EXPRESSION,"Copiar E&xpresion"))+"\tCtrl+C");
//	if (!sel_is_last) menu.Append(mxID_INSPECTION_COPY_DATA,LANG(INSPECTGRID_POPUP_COPY_DATA,"&Copiar Valor"));
//	if (sel_has_unfrozen) menu.Append(mxID_INSPECTION_FREEZE,wxString(LANG(INSPECTGRID_POPUP_FREEZE_VALUE,"Co&ngelar Valor"))+"\tCtrl+B");
//	if (sel_has_frozen) menu.Append(mxID_INSPECTION_FREEZE,wxString(LANG(INSPECTGRID_POPUP_UNFREEZE_VALUE,"Desco&ngelar Valor"))+"\tCtrl+B");
//	if (there_are_inspections) menu.Append(mxID_INSPECTION_COPY_ALL,LANG(INSPECTGRID_POPUP_COPY_ALL,"Copiar Toda la Ta&bla"));
//	if (current_source) {
//		int s = current_source->GetSelectionStart(), e = current_source->GetSelectionEnd();
//		if (s!=e && current_source->LineFromPosition(s)==current_source->LineFromPosition(e))
//			menu.Append(mxID_INSPECTION_FROM_SOURCE,LANG1(INSPECTGRID_POPUP_COPY_FROM_SELECTION,"Insertar Expresion Desde la &Seleccion <{1}>",current_source->GetTextRange(s,e)));
//	}
//	if (wxTheClipboard->Open()) {
//		wxTextDataObject clip_data;
//		if (wxTheClipboard->GetData(clip_data) {
//			wxString clip_text = clip_data.GetText();
//			if (clip_text.Contains('\n')) 
//				menu.Append(mxID_INSPECTION_FROM_CLIPBOARD,wxString(LANG(INSPECTGRID_POPUP_COPY_FROM_CLIPBOARD_MULTIPLE,"Insertar Expresiones Desde el &Portapapeles"))+"\tCtrl+V");
//			else if (!clip_text.IsEmpty())
//				menu.Append(mxID_INSPECTION_FROM_CLIPBOARD,LANG1(INSPECTGRID_POPUP_COPY_FROM_CLIPBOARD_SINGLE,"Pegar Expresion Desde el &Portapapeles (<{1}>)",(<{1}>)",clip_text)+"\tCtrl+V");
//		wxTheClipboard->Close();
//	}
//	if (!sel_is_last) menu.Append(mxID_INSPECTION_CLEAR_ONE,wxString(LANG(INSPECTGRID_POPUP_DELETE,"Eliminar Inspeccion"))+"\tSupr");
//	if (sel_is_vo && di->IsSimpleType()) {
//		wxMenu *submenu= new wxMenu; wxMenuItem *it[4];
//		it[0] = submenu->AppendRadioItem(mxID_INSPECTION_WATCH_NO,LANG(INSPECTGRID_WATCH_NO,"no"));
//		it[1] = submenu->AppendRadioItem(mxID_INSPECTION_WATCH_READ,LANG(INSPECTGRID_WATCH_READ,"lectura"));
//		it[2] = submenu->AppendRadioItem(mxID_INSPECTION_WATCH_WRITE,LANG(INSPECTGRID_WATCH_WRITE,"escritura"));
//		it[3] = submenu->AppendRadioItem(mxID_INSPECTION_WATCH_RW,LANG(INSPECTGRID_WATCH_READ_WRITE,"lectura y escritura"));
//		it[(debug->inspections[selected_row].watch_read?1:0)+(debug->inspections[selected_row].watch_write?2:0)]->Check(true);
//		menu.AppendSubMenu(submenu,LANG(INSPECTGRID_WATCH,"WatchPoint"));
//	}
//	if (sel_is_vo && di->IsSimpleType()) {
//		wxMenu *submenu = new wxMenu;
//		submenu->Append(mxID_INSPECTION_FORMAT_NAT,LANG(INSPECTGRID_FORMAT_NATURAL,"natural"));
//		submenu->Append(mxID_INSPECTION_FORMAT_BIN,LANG(INSPECTGRID_FORMAT_BINARY,"binario"));
//		submenu->Append(mxID_INSPECTION_FORMAT_OCT,LANG(INSPECTGRID_FORMAT_OCTAL,"octal"));
//		submenu->Append(mxID_INSPECTION_FORMAT_DEC,LANG(INSPECTGRID_FORMAT_DECIMAL,"decimal"));
//		submenu->Append(mxID_INSPECTION_FORMAT_HEX,LANG(INSPECTGRID_FORMAT_HEXADECIMAL,"hexadecimal"));
//		menu.AppendSubMenu(submenu,LANG(INSPECTGRID_FORMAT,"Formato"));
//	}
//	if (there_are_inspections) {
//		menu.AppendSeparator();
//		menu.Append(mxID_INSPECTION_CLEAR_ALL,LANG(INSPECTGRID_POPUP_CLEAN_TABLE,"&Limpiar Tabla de Inspeccion"));
//		menu.Append(mxID_INSPECTION_SAVE_TABLE,LANG(INSPECTGRID_POPUP_SAVE_TABLE,"&Guardar Lista de Inspecciones..."));
//	}
//	if (debug->inspections_tables.size()) {
//		menu.Append(mxID_INSPECTION_LOAD_TABLE,LANG(INSPECTGRID_POPUP_LOAD_TABLE,"Ca&rgar Lista de Inspecciones..."));
//		menu.Append(mxID_INSPECTION_MANAGE_TABLES,LANG(INSPECTGRID_POPUP_MANAGE_TABLES,"Administrar Listas de Inspecciones..."));
//	}
	PopupMenu(&menu);
}

void mxInspectionGrid::OnBreakClassOrArray(wxCommandEvent &evt) {
	BreakCompoundInspection(GetGridCursorRow());
}

//void mxInspectionGrid::OnPasteFromClipboard(wxCommandEvent &evt) {
//	if (!wxTheClipboard->Open()) return;
//	wxTextDataObject clip_data;
//	if (wxTheClipboard->GetData(clip_data)) {
//		wxString expr = clip_data.GetText();
//		if (expr.Len()) ModifyExpresion(selected_row,expr);
//	}
//	wxTheClipboard->Close();
//}
//
//void mxInspectionGrid::OnCopyFromSelecction(wxCommandEvent &evt) {
//	if (main_window->notebook_sources->GetPageCount()>0) {
//		mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
//		int s = source->GetSelectionStart(), e = source->GetSelectionEnd();
//		wxString expr = source->GetTextRange(s,e);
//		if (expr.Len()) ModifyExpresion(selected_row,expr);
//	}
//}
//
//bool mxInspectionGrid::ModifyExpresion(int row, wxString expr) {
//	ignore_changing=true;
//	while (expr.Len()) {
//		if (expr.StartsWith(">?")) expr=">help "+expr.Mid(2);
//		if (expr.Trim()==">help") { 
//			wxMessageBox(debug->GetMacroOutput("help user-defined",true),"help user-defined");
//			return false;
//		} else if (expr.StartsWith(">help")) {
//			wxMessageBox(debug->GetMacroOutput(expr.Mid(1),true),expr.Mid(1));
//			return false;
//		}
//		int sd=ShouldDivide(row,expr);
//		if (sd==0) {
//			if (debug->ModifyInspection(row,expr)) {
////				SetCellValue(row,IG_COL_EXPR,expr);
//				if (row==GetNumberRows()-2) {
//					SelectRow(row+1);
//					SetGridCursor(row+1,IG_COL_EXPR);
////					} else {
////						SelectRow(row);
////						SetGridCursor(row,IG_COL_EXPR);
//				}
//				break;
//			}
//			if (!debug->debugging) {
//				mxMessageDialog(main_window,LANG(INSPECTGRID_DEBUGGING_HAS_ENDED,"La sesion de depuracion ha finalizado."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
//				return ignore_changing=false;
//			} else
//				if (debug->last_error==_T("<killed>")) {
//					mxMessageDialog(main_window,LANG(INSPECTGRID_DEBUGGER_ERROR,"Ha ocurrido un error en el depurador."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
//					return ignore_changing=false;
//				} else {
//					ignore_changing=false;
//					return true;
//				}
//		} else if (sd==1) break;
//		if (!expr.Len()) return ignore_changing=false;
//	}
//	ignore_changing=false;
//	return true;
//}
//
//void mxInspectionGrid::OnExploreExpression(wxCommandEvent &evt) {
//	if (selected_row<=debug->inspections_count)
//		new mxInspectionExplorer(debug->inspections[selected_row].frame,debug->inspections[selected_row].expr);
//}
//
//void mxInspectionGrid::OnCopyData(wxCommandEvent &evt) {
//	if (!wxTheClipboard->Open()) return;
//	wxTextDataObject clip_data;
//	if (selected_row<=debug->inspections_count)
//		wxTheClipboard->SetData(new wxTextDataObject(GetCellValue(selected_row,IG_COL_VALUE)));
//	wxTheClipboard->Close();
//}
//
//void mxInspectionGrid::OnCopyExpression(wxCommandEvent &evt) {
//	if (!wxTheClipboard->Open()) return;
//	wxTextDataObject clip_data;
//	if (selected_row<=debug->inspections_count)
//		wxTheClipboard->SetData(new wxTextDataObject(GetCellValue(selected_row,IG_COL_EXPR)));
//	wxTheClipboard->Close();
//}
//
//void mxInspectionGrid::OnClearAll(wxCommandEvent &evt) {
//	if (debug->debugging)
//		debug->ClearInspections();
//	else {
//		if (debug->OffLineInspectionDelete())
//			DeleteRows(0,GetNumberRows()-1);
//		else
//			mxMessageDialog(main_window,LANG(INSPECTGRID_ERROR_OFFLINE_DELETE,"Error al eliminar inspecciones."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
//	}	
//}
//
//void mxInspectionGrid::OnClearOne(wxCommandEvent &evt) {
//	if (debug->debugging) {
//		if (selected_row+1!=GetNumberRows() && debug->DeleteInspection(selected_row))
//			DeleteRows(selected_row);
//		SetGridCursor(selected_row,0);
//	} else {
//		if (debug->OffLineInspectionDelete(selected_row))
//			DeleteRows(selected_row,1);
//		else
//			mxMessageDialog(main_window,LANG(INSPECTGRID_ERROR_OFFLINE_DELETE,"Error al eliminar inspecciones."),LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();
//	}
//}
//
//void mxInspectionGrid::OnExploreAll(wxCommandEvent &event) {
//	wxArrayString array,frames;
//	for (int i=0;i<debug->inspections_count;i++) {
//		if (!debug->inspections[i].is_vo) continue;
//		array.Add(debug->inspections[i].expr);
//		frames.Add(debug->inspections[i].frame);
//	}
//	new mxInspectionExplorer(LANG(INSPECTEXPLORER_CAPTION,"Explorador de Inspecciones"),frames,array);
//}
//
//void mxInspectionGrid::AppendInspections(wxArrayString &vars) {
//	int r = GetNumberRows()-1;
//	for (unsigned int i=0;i<vars.GetCount();i++) {
//		if (debug->ModifyInspection(r,vars[i]))
//			SetCellValue(r++,IG_COL_EXPR,vars[i]);
//	}
//}
//
//
///**
//* @retval 0   expresion unica que no se divide
//* @retval 1   expresion que se divide pero incorrecta
//* @retval 2   expresion que se dividio correctamente
//**/
//int mxInspectionGrid::ShouldDivide(int row, wxString expr, bool frameless) {
//	int c = 0, i, l = expr.Len();
//	bool comillas = false;
//	for (i=0; i<l; i++) {
//		if (expr[i]=='\"') comillas = !comillas;
//		else if (!comillas && expr[i]=='.') {
//			c++;
//			if (c==3) 
//				break;
//		} else 
//			c = 0;
//	}
//	if (i!=l) {
//		int t1=i-3, f1=i-3, t2=i+1, f2=i+1;
//		long n1=0, n2=0;
//		while (f1>0 && expr[f1]>='0' && expr[f1]<='9')
//			f1--;
//		l--;
//		while (t2<l && expr[t2]>='0' && expr[t2]<='9')
//			t2++;
//		expr.SubString(f1+1,t1).ToLong(&n1);
//		expr.SubString(f2,t2).ToLong(&n2);
//		if (n2<n1) swap(n2,n1);
//		if (n2>=n1) {
//			wxString p1 = expr.Mid(0,f1+1), p2 = expr.Mid(t2);
//			if (debug->ModifyInspection(row,wxString(p1)<<n1<<p2)) {
//				SetCellValue(row,IG_COL_EXPR,wxString(p1)<<n1<<p2);
//				if (n2-n1)
//					debug->MakeRoomForInspections(row+1,n2-n1);
//				for (i = n1+1; i<=n2; i++) {
//					row++;
//					SetCellValue(row,IG_COL_EXPR,wxString(p1)<<i<<p2);
////					if (frameless)
////						debug->ModifyInspectionFrameless(row,wxString(p1)<<i<<p2,true);
////					else
//						debug->ModifyInspection(row,wxString(p1)<<i<<p2,true);
//				}
//			} else return false;
//		}
//		return true;
//	}
//	return false;
//}
//
//void mxInspectionGrid::OnWatchNo(wxCommandEvent &evt) {
//	debug->ModifyInspectionWatch(selected_row,false,false);
//}
//void mxInspectionGrid::OnWatchReadWrite(wxCommandEvent &evt) {
//	debug->ModifyInspectionWatch(selected_row,true,true);
//}
//void mxInspectionGrid::OnWatchRead(wxCommandEvent &evt) {
//	debug->ModifyInspectionWatch(selected_row,true,false);
//}
//void mxInspectionGrid::OnWatchWrite(wxCommandEvent &evt) {
//	debug->ModifyInspectionWatch(selected_row,false,true);
//}
//
//void mxInspectionGrid::OnFormatNatural(wxCommandEvent &evt) {
//	debug->ModifyInspectionFormat(selected_row,"natural");
//}
//
//void mxInspectionGrid::OnFormatDecimal(wxCommandEvent &evt) {
//	debug->ModifyInspectionFormat(selected_row,"decimal");
//}
//void mxInspectionGrid::OnFormatOctal(wxCommandEvent &evt) {
//	debug->ModifyInspectionFormat(selected_row,"octal");
//}
//void mxInspectionGrid::OnFormatHexadecimal(wxCommandEvent &evt) {
//	debug->ModifyInspectionFormat(selected_row,"hexadecimal");
//}
//
//void mxInspectionGrid::OnFormatBinary(wxCommandEvent &evt) {
//	debug->ModifyInspectionFormat(selected_row,"binary");
//}
//
//void mxInspectionGrid::OnResize(wxSizeEvent &evt) {
//	int w; // la segunda condicion es para evitar problemas en windows al agrandar una columna
//	if (created && (w=evt.GetSize().GetWidth())!=old_size) {
//		int mw=0; // para contar los pixeles que no cambian por las colapsadas
//		for (int i=0;i<IG_COLS_COUNT;i++)
//			if (!cols_visibles[i])
//				mw+=GetColMinimalWidth(i);
//		w-=mw;
//		double p = double(w)/old_size;
//		for (int i=0;i<IG_COLS_COUNT;i++)
//			cols_sizes[i]*=p;
//		old_size=w;
//		for (int i=0;i<IG_COLS_COUNT;i++)
//			if (cols_visibles[i])
//				SetColSize(i,int(cols_sizes[i]));
//	}
//	evt.Skip();
//}
//
//void mxInspectionGrid::OnColResize(wxGridSizeEvent &evt) {
//	if (!created) return;
//	if (old_size) {
//		int i=evt.GetRowOrCol();
//		cols_sizes[i]=GetColSize(i);
//		cols_visibles[i]=GetColMinimalWidth(i)!=cols_sizes[i];
//	}
//}
//
//void mxInspectionGrid::OnShowInTable(wxCommandEvent &evt) {
//	if (selected_row<=debug->inspections_count)
//		new mxInspectionMatrix(selected_row);
//}
//
//void mxInspectionGrid::OnShowInText(wxCommandEvent &evt) {
//	if (selected_row<=debug->inspections_count)
//		new mxInspectionPrint(debug->inspections[selected_row].frame,debug->inspections[selected_row].expr,debug->inspections[selected_row].frameless);
//}
//

//void mxInspectionGrid::OnSelectCell(wxGridEvent &event) {
//	if (created) { // porque esto no anda?
//		if (event.GetCol()==IG_COL_LEVEL || (event.GetRow()==debug->inspections_count && event.GetCol()!=IG_COL_EXPR)) {
//			SetGridCursor(IG_COL_EXPR,event.GetRow());
//		} else
//			event.Skip();
//	}
//}
//
/**
* @brief Duplica una inspeccion de la grilla, colocando la copia justo debajo del original
**/
void mxInspectionGrid::OnDuplicate(wxCommandEvent &evt) {
	DebugManager::TemporaryScopeChange scope;
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(int i=0;i<sel.size();i++) {
		int si = sel[i];
		wxString expr = inspections[si]->GetExpression();
		bool is_frameless = inspections[si]->IsFrameless();
		if (!is_frameless) scope.ChangeTo(inspections[si]->GetFrameID(),inspections[si]->GetThreadID());
		InsertRows(si,1);
		mxGrid::SetCellValue(si,IG_COL_EXPR,expr);
		CreateInspection(si,expr,is_frameless);
	}
//	DebuggerInspection::UpdateAll(); // la expresion podría haber modificado algo
}
//
//void mxInspectionGrid::OnSaveTable(wxCommandEvent &evt) {
//	wxString name = mxGetTextFromUser(LANG(INSPECTGRID_TABLE_NAME,"Nombre de la lista:"),_T("Guardar Lista"),"",this);
//	if (name.Len()) debug->SaveInspectionsTable(name);
//}
//
//void mxInspectionGrid::OnLoadTable(wxCommandEvent &evt) {
//	wxArrayString as;
//	for (unsigned int i=0;i<debug->inspections_tables.size();i++)
//		as.Add(debug->inspections_tables[i]->name);
//	wxString name = wxGetSingleChoice(LANG(INSPECTGRID_LOAD_TABLE,"Cargar Lista:"),LANG(INSPECTGRID_INSPECTIONS_TABLE,"Lista de Inspecciones"),as);
//	if (name.Len()) debug->LoadInspectionsTable(name);
//}
//
//void mxInspectionGrid::OnManageTables(wxCommandEvent &evt) {
//	wxArrayString as;
//	for (unsigned int i=0;i<debug->inspections_tables.size();i++)
//		as.Add(debug->inspections_tables[i]->name);
//	wxString name = wxGetSingleChoice(LANG(INSPECTGRID_DELETE_TABLE,"Eliminar Lista:"),LANG(INSPECTGRID_INSPECTIONS_TABLE,"Lista de Inspecciones"),as);
//	if (name.Len() && mxMD_YES==mxMessageDialog(main_window,LANG1(INSPECTGRID_CONFIRM_DELETE_TABLE,"Desea eliminar la lista \"<{1}>\"?",name),LANG(GENERAL_CONFIRM,"Confirmacion"),mxMD_YES_NO).ShowModal())
//		debug->DeleteInspectionsTable(name);	
//}
//
//
//void mxInspectionGrid::OnShowAppart(wxCommandEvent &evt) {
//	
//}
//
//void mxInspectionGrid::ResetChangeHightlights() {
//	int i,r=GetNumberRows()-1;
//	if (config->Debug.select_modified_inspections) {
//		for (i=0;i<r;i++) {
//			if (debug->inspections[i].freezed)
//				SetCellTextColour(i,IG_COL_VALUE,freeze_colour);
//			else if (debug->inspections[i].is_vo && !debug->inspections[i].on_scope)
//				SetCellTextColour(i,IG_COL_VALUE,disable_colour);
//			else
//				SetCellTextColour(i,IG_COL_VALUE,default_colour);
//		}
//	} else {
//		for (i=0;i<r;i++)
//			SetCellTextColour(i,IG_COL_VALUE,default_colour);
//	}
//}
//
//void mxInspectionGrid::OnFreeze(wxCommandEvent &evt) {
//	if (selected_row>=debug->inspections_count) return;
//	debug->ToggleInspectionFreeze(selected_row);
//}
//
//void mxInspectionGrid::OnCopyAll(wxCommandEvent &evt) {
//	wxString text;
//	bool formatted_copy=true;
//	if (formatted_copy) {
//		unsigned int c[4]={IG_COL_LEVEL,IG_COL_EXPR,IG_COL_TYPE,IG_COL_VALUE}, w[3]={0,0,0};;
//		for (int i=0;i<debug->inspections_count;i++) {
//			for (int j=0;j<3;j++) {
//				wxString sw; sw<<GetCellValue(i,c[j]);
//				if (sw.Len()>w[j]) w[j]=sw.Len();
//			}
//		}
//		for (int i=0;i<debug->inspections_count;i++) {
//			for (int j=0;j<3;j++)
//				text<<GetCellValue(i,c[j])<<wxString(' ',w[j]+3-GetCellValue(i,c[j]).Len());
//			text<<GetCellValue(i,c[3])<<"\n";
//		}
//	} else {
//		for (int i=0;i<debug->inspections_count;i++) {
//			text<<GetCellValue(i,IG_COL_LEVEL)<<"\t";
//			text<<GetCellValue(i,IG_COL_EXPR)<<"\t";
//			text<<GetCellValue(i,IG_COL_TYPE)<<"\t";
//			text<<GetCellValue(i,IG_COL_VALUE)<<"\n";
//		}
//	}
//	if (wxTheClipboard->Open()) {
//		wxTheClipboard->SetData( new wxTextDataObject(text) );
//		wxTheClipboard->Close();
//	}
//}
//
//wxDragResult mxInspectionDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def) {
//	if (!grid->CanDrop()) return wxDragCancel;
//	int ux,uy,vx,vy;
//	grid->GetScrollPixelsPerUnit(&ux,&uy);
//	grid->GetViewStart(&vx,&vy);
//	int r=grid->YToRow(y+vy*uy-grid->GetColLabelSize());
//	if (r!=wxNOT_FOUND) grid->SelectRow(r);
//	return wxDragCopy;
//}
//
//
//mxInspectionDropTarget::mxInspectionDropTarget(mxInspectionGrid *agrid) {
//	grid=agrid; 
//	SetDataObject(data=new wxTextDataObject());
//}
//
//bool mxInspectionDropTarget::OnDrop(wxCoord x, wxCoord y) {
//	return true;
//}
//
//wxDragResult mxInspectionDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult ref) {
//	if (!grid->CanDrop()) return wxDragCancel;
//	int ux,uy,vx,vy;
//	grid->GetScrollPixelsPerUnit(&ux,&uy);
//	grid->GetViewStart(&vx,&vy);
//	int r=grid->YToRow(y+vy*uy-grid->GetColLabelSize());
//	if (r==wxNOT_FOUND) return wxDragCancel;
//	GetData(); grid->SelectRow(r);
//	wxString str=data->GetText();
//	if (!str.size()) return wxDragCancel;
//	str.Replace("\n"," ");
//	str.Replace("\r"," ");
//	grid->ModifyExpresion(r,str);
//	return wxDragCopy;
//}
//
//bool mxInspectionGrid::CanDrop() {
//	return can_drop;
//}
//
//void mxInspectionGrid::OnClick(wxGridEvent &event) {
//	if (event.AltDown()) {
//		SelectRow(event.GetRow());
//		can_drop=false;
//		wxTextDataObject my_data(GetCellValue(event.GetRow(),event.GetCol()));
//		wxDropSource dragSource(this);
//		dragSource.SetData(my_data);
//		dragSource.DoDragDrop(wxDrag_AllowMove|wxDrag_DefaultMove);
//		can_drop=true;
//	} else event.Skip();
//}
//
void mxInspectionGrid::OnRedirectedEditEvent (wxCommandEvent & event) {
//	selected_row=GetGridCursorRow();
//	switch (event.GetId()) {
//	case wxID_COPY: 
//		OnCopyExpression(event); 
//		break;
//	case wxID_PASTE:
//		OnPasteFromClipboard(event);
//		break;
//	case wxID_CUT:
//		OnCopyExpression(event);
//		OnClearOne(event);
//		break;
//	case mxID_EDIT_DUPLICATE_LINES:
//		OnDuplicate(event);
//		break;
//	case mxID_EDIT_DELETE_LINES:
//		OnClearOne(event);
//		break;
//	case mxID_EDIT_MARK_LINES:
//		OnFreeze(event);
//		break;
//	case mxID_EDIT_INDENT:
//		if (!debug->debugging || selected_row>=debug->inspections_count) return;
//		if (debug->inspections[selected_row].frameless) OnReScope(event); else OnSetFrameless(event);
//		break;
//	default:
//		event.Skip();
//	}
}

void mxInspectionGrid::InsertRows(int pos, int cant) {
	if (pos==-1) { 
		pos=wxGrid::GetNumberRows(); 
		wxGrid::AppendRows(cant,false);
		for(unsigned int i=0;i<cant;i++) 
			inspections.Add(NULL);
	} else {
		wxGrid::InsertRows(pos,cant,false);
		inspections.MakeRoomForMultipleInsert(pos,cant);
	}
	for(int i=pos;i<pos+cant;i++) {
		mxGrid::SetReadOnly(i,IG_COL_LEVEL,true);
		mxGrid::SetCellEditor(i,IG_COL_EXPR,new gdbInspCtrl);
		mxGrid::SetReadOnly(i,IG_COL_TYPE,true);
		mxGrid::SetReadOnly(i,IG_COL_VALUE,true);
		mxGrid::SetCellRenderer(i,IG_COL_VALUE,inspections[i].renderer = new mxGridCellRenderer());
	}
}


// eventos de DebuggerInspection

void mxInspectionGrid::OnDIError(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
	SetRowStatus(current_row,IGRS_ERROR);
}

void mxInspectionGrid::OnDICreated(DebuggerInspection *di) {
	OnDINewType(di);
	UpdateLevelColumn(current_row);
}

void mxInspectionGrid::OnDIValueChanged(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
	UpdateValueColumn(current_row);
	SetRowStatus(current_row,IGRS_CHANGED);
}

void mxInspectionGrid::OnDINewType(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
	TryToSimplify(current_row);
	UpdateTypeColumn(current_row);
	UpdateValueColumn(current_row);
	SetRowStatus(current_row,IGRS_CHANGED);
}

void mxInspectionGrid::OnDIInScope(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
	UpdateValueColumn(current_row);
	SetRowStatus(current_row,IGRS_IN_SCOPE);
}

void mxInspectionGrid::OnDIOutOfScope(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
	SetRowStatus(current_row,IGRS_OUT_OF_SCOPE);
}

void mxInspectionGrid::SetRowStatus (int r, int status) {
	int prev_status = inspections[r].status;
	if (prev_status==status) return;
	inspections[r].status=status;
	if (mxig_status_opts[prev_status].color!=mxig_status_opts[status].color)
		mxGrid::SetCellColour(r,IG_COL_VALUE,mxig_status_opts[status].color);
	if (mxig_status_opts[status].have_message) 
		mxGrid::SetCellValue(r,IG_COL_VALUE,mxig_status_opts[status].message);
	if (mxig_status_opts[prev_status].editable_value!=mxig_status_opts[status].editable_value)
		mxGrid::SetReadOnly(r,IG_COL_VALUE,!mxig_status_opts[status].editable_value);
}

void mxInspectionGrid::OnFullTableUpdateBegin( ) {
	BeginBatch();
	for(int i=0;i<inspections.GetSize();i++) {
		if (inspections[i].IsNull()) continue; ///< pasa cuando se crea una nuevo, se llama a este metodo antes de hacerlo, pero ya habiendo reservado el espacio en inspections
		UpdateLevelColumn(i);
		InspectionGridRow &di = inspections[i];
		if (di.status==IGRS_UNINIT || di->IsFrozen()) continue;
		if (di->RequiresManualUpdate()) {
			if (di->IsInScope()) {
				if (di->UpdateValue()) {
					UpdateValueColumn(current_row);
					SetRowStatus(i,IGRS_CHANGED);	
				} else
					SetRowStatus(i,IGRS_NORMAL);
			}
		} else if (di.status==IGRS_IN_SCOPE||di.status==IGRS_CHANGED) {
			SetRowStatus(i,IGRS_NORMAL);
		}
	}
}

void mxInspectionGrid::OnFullTableUpdateEnd ( ) {
	EndBatch();
}

void mxInspectionGrid::ModifyInspectionExpression (int row, const wxString & expression, bool is_frameless) {
	if (row==-1) row=inspections.GetSize()-1;
	last_return_had_shift_down = is_frameless;
	mxGrid::SetCellValue(row,IG_COL_EXPR,expression);
}

void mxInspectionGrid::UpdateLevelColumn (int r) {
	InspectionGridRow &di=inspections[r];
	if (di.IsNull()) { mxGrid::SetCellValue(r,IG_COL_LEVEL,""); return; }
	if (!debug->debugging) { mxGrid::SetCellValue(r,IG_COL_LEVEL,di->IsFrameless()?"*":"?"); return; }
	if (di->GetDbiType()==DIT_ERROR) { mxGrid::SetCellValue(r,IG_COL_LEVEL,"?"); return; }
	if (di->IsFrameless()) {
		if (di.frame_level!=-1) {
			di.frame_level=-1;
			mxGrid::SetCellValue(r,IG_COL_LEVEL,"*");
		}
	} else {
		if (debug->current_thread_id!=di->GetThreadID()) {
			if (di.on_thread) { 
				di.on_thread=false;
				mxGrid::SetCellValue(r,IG_COL_LEVEL,wxString("(t ")<<di->GetThreadID()<<")");
			}
		} else {
			long di_frame_level = debug->GetFrameLevel(di->GetFrameID());
			if (!di.on_thread || di.frame_level!=di_frame_level) {
				di.on_thread=true;
				di.frame_level=di_frame_level;
				mxGrid::SetCellValue(r,IG_COL_LEVEL,wxString()<<di_frame_level);
			}
		}
	}
}

bool mxInspectionGrid::CreateInspection (int r, const wxString &expression, bool frameless) {
	inspections[r] = DebuggerInspection::Create(expression,frameless,this,false);
	if (!inspections[r]->Init()) SetRowStatus(r,IGRS_UNINIT);
	else UpdateLevelColumn(r);
	return inspections[r]->GetDbiType()!=DIT_ERROR;
}

void mxInspectionGrid::DeleteInspection (int r, bool for_reuse) {
	if (inspections[r].di) inspections[r]->Destroy(); // si es una que ya existía
	if (for_reuse) {
		inspections[r].Reset(); // para que OnFullTableUpdateBegin no la considere más
	} else {
		inspections.Remove(r); // quitar de la lista propia de inspecciones
		DeleteRows(r,1); // eliminar fila de la tabla
	}
}

bool mxInspectionGrid::TryToSimplify (int row) {
	wxArrayString &from=config->Debug.inspection_improving_template_from;
	if (config->Debug.improve_inspections_by_type) {
		wxString mtype=inspections[row]->GetValueType();
		if (mtype.EndsWith(" &")) { mtype.RemoveLast(); mtype.RemoveLast(); }
		if (mtype.StartsWith("const ")) { mtype=mtype.Mid(6); }
		for(unsigned int i=0, n=from.GetCount(); i<n; i++) {
			if (from[i]==mtype) {
				wxString expr=config->Debug.inspection_improving_template_to[i];
				expr.Replace("${EXP}",inspections[row]->GetExpression(),true);
				inspections[row]->SetHelperInspection(expr);
				return true;
			}
		}
	}
	return false;
}

void mxInspectionGrid::UpdateValueColumn (int r) {
	mxGrid::SetCellValue(r,IG_COL_VALUE,inspections[r]->GetValue());
}

void mxInspectionGrid::UpdateTypeColumn (int r) {
	DebuggerInspection *di = inspections[r].di;
	mxGrid::SetCellValue(r,IG_COL_TYPE,di->GetValueType());
	if (di->GetDbiType()==DIT_VARIABLE_OBJECT) {
		if (di->IsCompound()) inspections[r].renderer->SetIconPlus();
		else inspections[r].renderer->SetIconNull();
		mxGrid::SetReadOnly(r,IG_COL_VALUE,true);
	} else {
		mxGrid::SetReadOnly(r,IG_COL_VALUE,true);
		inspections[r].renderer->SetIconNull();
	}
}

void mxInspectionGrid::BreakCompoundInspection (int r) {
	if (!ValidInspection(r)) return;
	InspectionGridRow old=inspections[r];
	if (!old->IsCompound()) return;
	SingleList<DebuggerInspection *> children;
	if (!old->Break(children,true,true,true)) return;
	// delete the old one
	wxString old_level = mxGrid::GetCellValue(r,IG_COL_LEVEL);
	DeleteInspection(r,true);
	// make room for new children
	if (children.GetSize()>1) InsertRows(r,children.GetSize()-1);
	// fill grid with new children
	for(int i=0;i<children.GetSize();i++) { 
		// copy status from original one, and assign new DebuggerInspection
		inspections[r+i]=old; 
		mxGrid::SetCellValue(r+i,IG_COL_LEVEL,old_level);
		inspections[r+i].di=children[i];
		// fill grid with new expressions
		mxGrid::SetCellValue(r+i,IG_COL_EXPR,children[i]->GetExpression());
		UpdateTypeColumn(r+i);
		UpdateValueColumn(r+i);
	}
}

void mxInspectionGrid::OnReScope(wxCommandEvent &event) {
	OnFullTableUpdateBegin();
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	for(int i=0;i<sel.size();i++) 
		ChangeFrameless(sel[i],false,false);
	DebuggerInspection::UpdateAll(); 
	OnFullTableUpdateEnd();
}

void mxInspectionGrid::OnSetFrameless (wxCommandEvent & evt) {
	OnFullTableUpdateBegin();
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	for(int i=0;i<sel.size();i++) 
		ChangeFrameless(sel[i],true,false);
	DebuggerInspection::UpdateAll(); 
	OnFullTableUpdateEnd();
}

bool mxInspectionGrid::ValidInspection (int r) {
	return r>=0&&r<inspections.GetSize()&&!inspections[r].IsNull();
}

void mxInspectionGrid::ChangeFrameless (int r, bool frameless, bool full_table_update) {
	if (!ValidInspection(r)) return;
	bool was_frameless = inspections[r]->IsFrameless();
	wxString old_expression = inspections[r]->GetExpression();
	// delete old inspection
	DeleteInspection(r,true);
	// create new one
	if (full_table_update) OnFullTableUpdateBegin();
	CreateInspection(r,old_expression,!was_frameless);
	if (full_table_update) DebuggerInspection::UpdateAll(); // la expresion podría haber modificado algo
	if (full_table_update) OnFullTableUpdateEnd();
}

