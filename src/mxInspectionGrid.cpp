#include <algorithm>
#include <wx/menu.h>
#include "Language.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxInspectionGrid.h"
#include "mxInspectionGridCellEditor.h"
#include "mxInspectionPrint.h"
#include "mxInspectionExplorerDialog.h"
#include "mxInspectionMatrix.h"
#include "mxInspectionsImprovingEditor.h"
#include "mxInspectionHistory.h"
#include "mxRealTimeInspectionEditor.h"
#include "mxMessageDialog.h"
#include "ProjectManager.h"
using namespace std;

#warning no se toma en cuenta config->Debug.use_colours_for_inspections
 
BEGIN_EVENT_TABLE(mxInspectionGrid, wxGrid)
	EVT_GRID_CELL_CHANGE(mxInspectionGrid::OnCellChange)
	EVT_GRID_CELL_LEFT_CLICK(mxInspectionGrid::OnClick)
	EVT_MENU(mxID_INSPECTION_FREEZE,mxInspectionGrid::OnFreeze)
	EVT_MENU(mxID_INSPECTION_UNFREEZE,mxInspectionGrid::OnUnFreeze)
	EVT_MENU(mxID_INSPECTION_BREAK,mxInspectionGrid::OnBreakClassOrArray)
	EVT_MENU(mxID_INSPECTION_RESCOPE,mxInspectionGrid::OnReScope)
	EVT_MENU(mxID_INSPECTION_SET_FRAMELESS,mxInspectionGrid::OnSetFrameless)
	EVT_MENU(mxID_INSPECTION_DUPLICATE,mxInspectionGrid::OnDuplicate)
	EVT_MENU(mxID_INSPECTION_FROM_CLIPBOARD,mxInspectionGrid::OnPasteFromClipboard)
	EVT_MENU(mxID_INSPECTION_FROM_SOURCE,mxInspectionGrid::OnCopyFromSelecction)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_HISTORY,mxInspectionGrid::OnShowInHistory)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_TEXT,mxInspectionGrid::OnShowInText)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_RTEDITOR,mxInspectionGrid::OnShowInRTEditor)
	EVT_MENU(mxID_INSPECTION_SET_WATCH_WRITE,mxInspectionGrid::OnSetWatch)
	EVT_MENU(mxID_INSPECTION_SET_WATCH_READ,mxInspectionGrid::OnSetWatch)
	EVT_MENU(mxID_INSPECTION_SET_WATCH_BOTH,mxInspectionGrid::OnSetWatch)
	EVT_MENU(mxID_INSPECTION_DEREF_PTR,mxInspectionGrid::OnDerefPtr)
	EVT_MENU(mxID_INSPECTION_SHOW_IN_TABLE,mxInspectionGrid::OnShowInTable)
	EVT_MENU(mxID_INSPECTION_EXPLORE,mxInspectionGrid::OnExploreExpression)
	EVT_MENU(mxID_INSPECTION_COPY_VALUE,mxInspectionGrid::OnCopyValue)
	EVT_MENU(mxID_INSPECTION_COPY_TYPE,mxInspectionGrid::OnCopyType)
	EVT_MENU(mxID_INSPECTION_COPY_EXPRESSION,mxInspectionGrid::OnCopyExpression)
	EVT_MENU(mxID_INSPECTION_COPY_ALL,mxInspectionGrid::OnCopyAll)
	EVT_MENU(mxID_INSPECTION_EXPLORE_ALL,mxInspectionGrid::OnExploreAll)
	EVT_MENU(mxID_INSPECTION_CLEAR_ALL,mxInspectionGrid::OnClearAll)
	EVT_MENU(mxID_INSPECTION_CLEAR_ONE,mxInspectionGrid::OnClearOne)
	EVT_MENU(mxID_INSPECTION_FORMAT_NAT,mxInspectionGrid::OnFormatNatural)
	EVT_MENU(mxID_INSPECTION_FORMAT_BIN,mxInspectionGrid::OnFormatBinary)
	EVT_MENU(mxID_INSPECTION_FORMAT_DEC,mxInspectionGrid::OnFormatDecimal)
	EVT_MENU(mxID_INSPECTION_FORMAT_OCT,mxInspectionGrid::OnFormatOctal)
	EVT_MENU(mxID_INSPECTION_FORMAT_HEX,mxInspectionGrid::OnFormatHexadecimal)
	
	EVT_MENU(mxID_INSPECTION_IMPR_ADD_GENERAL,mxInspectionGrid::OnRegisterNewImprovedExpressionGeneral)
	EVT_MENU(mxID_INSPECTION_IMPR_ADD_PROJECT,mxInspectionGrid::OnRegisterNewImprovedExpressionProject)
	EVT_MENU(mxID_INSPECTION_IMPR_EXPOSE,mxInspectionGrid::OnExposeImprovedExpression)
	EVT_MENU(mxID_INSPECTION_IMPR_DISCARD,mxInspectionGrid::OnDiscardImprovedExpression)
	EVT_MENU(mxID_INSPECTION_IMPR_CONF_GENERAL,mxInspectionGrid::OnInspectionsImprovingSettingsGeneral)
	EVT_MENU(mxID_INSPECTION_IMPR_CONF_PROJECT,mxInspectionGrid::OnInspectionsImprovingSettingsProject)
	EVT_MENU(mxID_INSPECTION_EDIT,mxInspectionGrid::OnEditFromKeyboard)
END_EVENT_TABLE()
	

struct mxIG_SideEffectUpdate {
	bool do_update;
	mxIG_SideEffectUpdate(mxInspectionGrid *grid) {
		do_update = grid && config->Debug.inspections_can_have_side_effects;
		if (do_update) grid->OnFullTableUpdateBegin();
	}
	~mxIG_SideEffectUpdate() {
		if (do_update) debug->UpdateInspections();;
	}
};
	
static struct mxIGStatusOpts {
	wxColour color;
	bool have_message;
	wxString message;
	bool editable_value;
	void Init(bool ev, const wxColour &c) { color=c; have_message=false; editable_value=ev; }
	void Init(bool ev, const wxColour &c, const wxString &m) { color=c; have_message=true; message=m; editable_value=ev; }
} s_mxig_status_opts[mxInspectionGrid::IGRS_COUNT];


mxInspectionGrid::mxInspectionGrid(wxWindow *parent) : mxGrid(parent,IG_COLS_COUNT) {
	
	full_table_update_began=false;
	
	BoolFlagGuard icce_guard(ignore_cell_change_event,true);
	
	last_return_had_shift_down = false;
	
	s_mxig_status_opts[IGRS_UNINIT].Init(false,wxColour(100,100,100),DebuggerInspection::GetUserStatusText(DIMSG_PENDING));
	s_mxig_status_opts[IGRS_OUT_OF_SCOPE].Init(false,wxColour(100,100,100),DebuggerInspection::GetUserStatusText(DIMSG_OUT_OF_SCOPE));
	s_mxig_status_opts[IGRS_IN_SCOPE].Init(true,wxColour(196,0,0));
	s_mxig_status_opts[IGRS_CHANGED].Init(true,wxColour(196,0,0));
	s_mxig_status_opts[IGRS_NORMAL].Init(true,wxColour(0,0,0));
	s_mxig_status_opts[IGRS_ERROR].Init(false,wxColour(196,0,0),DebuggerInspection::GetUserStatusText(DIMSG_ERROR));
	s_mxig_status_opts[IGRS_FREEZE].Init(false,wxColour(0,100,200));
	
	dragging_inspection=false;
//	ignore_changing=true;
//	created=false;
	
	mxGrid::InitColumn(IG_COL_LEVEL,LANG(INSPECTGRID_LEVEL,"Nivel"),8);
	mxGrid::InitColumn(IG_COL_EXPR,LANG(INSPECTGRID_EXPRESSION,"Expresión"),29);
	mxGrid::InitColumn(IG_COL_TYPE,LANG(INSPECTGRID_TYPE,"Tipo"),12);
	mxGrid::InitColumn(IG_COL_VALUE,LANG(INSPECTGRID_VALUE,"Valor"),27);
	mxGrid::DoCreate();
	InsertRows();
	
	mxGrid::SetRowSelectionMode();
	
	SetDropTarget(new mxInspectionDropTarget(this));
	SetColLabelSize(wxGRID_AUTOSIZE);
	
	wxAcceleratorEntry aentries[1];
	aentries[0].Set(0,WXK_F2,mxID_INSPECTION_EDIT);
	wxAcceleratorTable accel(1,aentries);
	SetAcceleratorTable(accel);
	
}

bool mxInspectionGrid::OnKey(int row, int col, int key, int modifiers) {
	if (key==WXK_DELETE) {
		vector<int> sel; int min=-1;
		if (mxGrid::GetSelectedRows(sel,true)==0) sel.push_back(GetGridCursorRow());
		for(unsigned int i=0;i<sel.size();i++) {
			if (sel[i]<0||sel[i]+1>=inspections.GetSize()) continue;
			if (min==-1||sel[i]<min) min=sel[i];
			DeleteInspection(sel[i],false);
		}
		if (min!=-1) min=GetGridCursorRow();
		if (min<0||min>inspections.GetSize()) min=0;
		Select(min);
		return true;
	} else if (key==WXK_INSERT) {
		InsertRows(row,1);
		mxGrid::Select(row,IG_COL_EXPR);
		return true;
	} else if (key==WXK_RETURN || key==WXK_NUMPAD_ENTER) {
		last_return_had_shift_down=modifiers&wxMOD_SHIFT;
		return false;
//	} else if (key==WXK_F2) {
//		if (mxGrid::SetGridCursor(row,IG_COL_EXPR)) {
//			EnableCellEditControl(true);
//			return true;
//		}
	}
	return false;
}

bool mxInspectionGrid::ModifyExpression (int row, const wxString & expression, bool is_frameless, bool do_update_cell) {
	BoolFlagGuard icce_guard(ignore_cell_change_event);
	if (row==-1) row = inspections.GetSize()-1;
	if (do_update_cell) mxGrid::SetCellValue(row,IG_COL_EXPR,expression);
	// caso especial, ayuda para las macros gdb
	if (expression==">help" || expression==">?") {
		wxMessageBox(debug->GetMacroOutput("help user-defined",true),"help user-defined");
		return false;
	} else if (expression.StartsWith(">? ") || expression.StartsWith(">help ")) {
		wxMessageBox(debug->GetMacroOutput(wxString("help ")+expression.AfterFirst(' '),true),expression.Mid(1));
		return false;
	}
	// caso normal, inspeccion
	if (row+1==inspections.GetSize()) InsertRows();
	if (!inspections[row].IsNull()) {
		if (inspections[row]->GetExpression()==expression) return false; // si en realidad no cambio
		DeleteInspection(row,true);
	}
	return CreateInspection(row,expression,is_frameless);
}

bool AuxShouldExpand(const wxString &expr, wxArrayString *arr=nullptr) {
	int l=expr.Len();
	for(int i=0;i<l-3;i++) { 
		if (expr[i]=='\'') { if (expr[++i]=='\\') i++; }
		else if (expr[i]=='\"') { i++; while (i<l && expr[i]!='\"') if (expr[i]=='\\') i++; }
		if (i && expr[i]=='.' && expr[i+1]=='.' && expr[i+2]=='.') {
			int i1=i, i2=i; 
			while (i1-1>0 && (expr[i1-1]>='0'&&expr[i1-1]<='9')) i1--;
			int i3=i+3, i4=i+3; 
			while (i4<l && (expr[i4]>='0'&&expr[i4]<='9')) i4++;
			if (i2!=i1 && i3!=i4) {
				long from; expr.Mid(i1,i2-i1+1).ToLong(&from);
				long to; expr.Mid(i3,i4-i3+1).ToLong(&to);
				if (to==from) return false;
				if (arr) {
					if (to<from) swap(to,from);
					for(int j=from;j<=to;j++) {
						wxString new_expr = expr.Mid(0,i1)<<j<<expr.Mid(i4);
						if (!AuxShouldExpand(new_expr,arr)) arr->Add(new_expr);
					}
				}
				return true;
			}
		}
	}
	return false;
}

void mxInspectionGrid::OnCellChange(wxGridEvent &event) {
	event.Skip();
	BoolFlagGuard icce_guard(ignore_cell_change_event);
	if (!icce_guard.IsOk()) return;
	int col = event.GetCol(), row = event.GetRow(); 
	wxString new_value = wxGrid::GetCellValue(event.GetRow(),event.GetCol());
	if (col==GetRealCol(IG_COL_EXPR)) {
		mxIG_SideEffectUpdate sda(this); // la expresion podría haber modificado algo, esto actualiza toda la tabla
		if ( AuxShouldExpand(new_value) ) {
			wxArrayString arr; AuxShouldExpand(new_value,&arr);
			if ( ModifyExpression(row,arr[0],last_return_had_shift_down,false) ) {
				InsertRows(row+1,arr.GetCount()-1);
				for(unsigned int i=1;i<arr.GetCount();i++)
					ModifyExpression(row+i,arr[i],last_return_had_shift_down,true);
				mxGrid::SetCellValue(row,IG_COL_EXPR,arr[0]); // for some reason, SetCellValue effect is dismissed at InsertRows
			}
		} else {
			if ( ModifyExpression(row,new_value,last_return_had_shift_down,false) ) Select(row+1);
		}
	} else if (col==GetRealCol(IG_COL_VALUE)) {
		if (row<0||row>inspections.GetSize()||inspections[row].IsNull()) return;
		if (inspections[row]->ModifyValue(new_value)) {
			mxGrid::SetCellValue(row,IG_COL_VALUE,inspections[row]->GetValue());
			DebuggerInspection::OnDebugPause(); // este cambio podría afectar a otras expresiones
		} else event.Veto();
	}
}

bool mxInspectionGrid::OnCellDoubleClick(int row, int col) {
	if (inspections[row].IsNull()) return false;
	if (col==GetRealCol(IG_COL_VALUE)) {
		if (inspections[row]->IsCompound()) 
			BreakCompoundInspection(row);
	} else if (col==GetRealCol(IG_COL_LEVEL)) {
		ChangeFrameless(row,!inspections[row]->IsFrameless(),true);
	} else if (col==GetRealCol(IG_COL_EXPR)) {
		if (!inspections[row].expression_renderer->HasIcon()) return false;
		if (last_event_x>mxGrid::GetColLeft(IG_COL_EXPR)+20) return false;
		ExposeImprovedExpression(row); return true;
	}
	return true;
}

bool mxInspectionGrid::OnCellClick(int row, int col) {
	if (col==mxGrid::GetRealCol(IG_COL_EXPR) && inspections[row].expression_renderer->HasIcon()) {
		if (last_event_x<mxGrid::GetColLeft(IG_COL_EXPR)+20) {
			if (GetGridCursorCol()==col&&GetGridCursorRow()==row) return true;
		}
	}
	return false;
}

static wxString Shorten(wxString str) {
	str.Replace("\t","   ",true);
	if (str.Len()>15) str=str.Mid(0,15)+"...";
	return str;
}

void mxInspectionGrid::OnCellPopupMenu(int row, int col) {
	// ensure that clicked cell is selected, so generated events will use that one
	// selection policy is: if theres a multiple selection, keep, else select only clicked cell
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	// ensure clicked row is selected... if it is, use current selection, if not make it current selection
	if (find(sel.begin(),sel.end(),row)==sel.end()) { sel.clear(); mxGrid::Select(row); sel.push_back(row); }
	
	bool there_are_inspections = inspections.GetSize()>1;
	bool sel_is_single = sel.size()==1; // hay una sola inspeccion seleccionada
	bool sel_is_empty = true; // la seleccion solo tiene filas vacias
	bool sel_is_vo = sel_is_single && !inspections[row].IsNull() && inspections[row]->GetDbiType()==DIT_VARIABLE_OBJECT; // la seleccionada corresponde a una variable_object
	DebuggerInspection *di = (sel_is_single)?inspections[row].di:nullptr; // puntero a la seleccionada si es unica y valida
	bool sel_has_vo = sel_is_vo; // si hay al menos una variable object seleccionada
	bool sel_has_frozen = false, sel_has_unfrozen=false; // si hay inspecciones congeladas y descongeladas
	bool sel_has_improved = false;
	for(unsigned int i=0;i<sel.size();i++) {
		if (sel[i]>=inspections.GetSize() || inspections[sel[i]].IsNull()) continue;
		sel_is_empty = false;
		DebuggerInspection *di = inspections[sel[i]].di;
		if (di->GetDbiType()==DIT_VARIABLE_OBJECT) { sel_has_vo=true; if (!di->GetHelperExpression().IsEmpty()) sel_has_improved=true; }
		if (inspections[sel[i]].is_frozen) sel_has_frozen=true; else sel_has_unfrozen=true; 
	}
	bool there_are_sources = main_window->notebook_sources->GetPageCount()!=0;
	mxSource *current_source = there_are_sources?(mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection())):nullptr;

	wxMenu menu; 
	if (sel_is_vo && di->IsClass()) menu.Append(mxID_INSPECTION_BREAK,LANG(INSPECTGRID_POPUP_SPLIT_CLASS,"&Separar clase en atributos"));
	if (sel_is_vo && di->IsArray()) menu.Append(mxID_INSPECTION_BREAK,LANG(INSPECTGRID_POPUP_SPLIT_ARRAY,"&Separar arreglo en elementos"));
	if (sel_is_vo && di->IsPointer()) menu.Append(mxID_INSPECTION_DEREF_PTR,LANG(INSPECTGRID_POPUP_DEREF_PTR,"Desreferenciar puntero"));
	wxMenu *extern_v = new wxMenu;
		if (!sel_is_empty && (!sel_is_vo || !di->IsSimpleType())) extern_v->Append(mxID_INSPECTION_SHOW_IN_TABLE,LANG(INSPECTGRID_POPUP_SHOW_IN_TABLE,"Mostrar en &tabla separada..."));
		if (!sel_is_empty) extern_v->Append(mxID_INSPECTION_SHOW_IN_TEXT,LANG(INSPECTGRID_POPUP_SHOW_IN_TEXT,"Mostrar en &ventana separada..."));
		if (!sel_is_empty) extern_v->Append(mxID_INSPECTION_SHOW_IN_HISTORY,LANG(INSPECTGRID_POPUP_SHOW_IN_HISTORY,"Generar historial de valores..."));
		if (debug->IsDebugging() && debug->IsPaused() && !sel_is_empty && sel_is_vo) extern_v->Append(mxID_INSPECTION_SHOW_IN_RTEDITOR,LANG(INSPECTGRID_POPUP_SHOW_IN_RTEDITOR,"Editar durante la ejecución..."));
		if (sel_is_vo) extern_v->Append(mxID_INSPECTION_EXPLORE,LANG(INSPECTGRID_POPUP_EXPLORE,"&Explorar datos..."));
	if (extern_v->GetMenuItemCount()) menu.AppendSubMenu(extern_v,LANG(INSPECTGRID_EXTERN_VISUALIZATION,"Otras &visualizaciones")); else delete extern_v;
	
	if (!sel_is_empty) {
		wxMenu *watch = new wxMenu;
		watch->Append(mxID_INSPECTION_SET_WATCH_READ,LANG(WATCHPOINT_READ,"Lectura"));
		watch->Append(mxID_INSPECTION_SET_WATCH_WRITE,LANG(WATCHPOINT_WRITE,"Escritura"));
		watch->Append(mxID_INSPECTION_SET_WATCH_BOTH,LANG(WATCHPOINT_BOTH,"Lectura/Escritura"));
		menu.AppendSubMenu(watch,LANG(INSPECTGRID_POPUP_SET_READ,"Agregar como watchpoint"));
	}
	
	if (there_are_inspections) menu.Append(mxID_INSPECTION_EXPLORE_ALL,LANG(INSPECTGRID_POPUP_EXPLORE_ALL,"Explorar &todos los datos"));
	if (sel_has_vo && !(sel_is_vo && !di->IsFrameless())) menu.Append(mxID_INSPECTION_RESCOPE,LANG(INSPECTGRID_POPUP_SET_CURRENT_FRAME,"Evaluar en el &ambito actual"));
	if (sel_has_vo && !(sel_is_vo && di->IsFrameless())) menu.Append(mxID_INSPECTION_SET_FRAMELESS,wxString(LANG(INSPECTGRID_POPUP_SET_NO_FRAME,"&Independizar del ambito"))+"\tCtrl+I");
	if (!sel_is_empty) menu.Append(mxID_INSPECTION_DUPLICATE,wxString(LANG(INSPECTGRID_POPUP_DUPLICATE_EXPRESSION,"Duplicar Inspeccion"))+"\tCtrl+L");
	wxMenu *copy_menu = new wxMenu;
	if (!sel_is_empty) copy_menu->Append(mxID_INSPECTION_COPY_EXPRESSION,wxString(LANG(INSPECTGRID_POPUP_COPY_EXPRESSION,"Copiar &Expresion"))+"\tCtrl+C");
	if (!sel_is_empty) copy_menu->Append(mxID_INSPECTION_COPY_TYPE,LANG(INSPECTGRID_POPUP_COPY_TYPE,"Copiar &Tipo"));
	if (!sel_is_empty) copy_menu->Append(mxID_INSPECTION_COPY_VALUE,LANG(INSPECTGRID_POPUP_COPY_DATA,"Copiar &Valor"));
	if (there_are_inspections) copy_menu->Append(mxID_INSPECTION_COPY_ALL,LANG(INSPECTGRID_POPUP_COPY_ALL,"&Copiar Toda la Tabla"));
	if (copy_menu->GetMenuItemCount()) menu.AppendSubMenu(copy_menu,LANG(INSPECTGRID_POPUP_COPY,"Copiar")); else delete copy_menu;
	
	if (sel_has_unfrozen) menu.Append(mxID_INSPECTION_FREEZE,wxString(LANG(INSPECTGRID_POPUP_FREEZE_VALUE,"Co&ngelar Valor"))+"\tCtrl+B");
	if (sel_has_frozen) menu.Append(mxID_INSPECTION_UNFREEZE,wxString(LANG(INSPECTGRID_POPUP_UNFREEZE_VALUE,"Desco&ngelar Valor"))+"\tCtrl+B");
	if (current_source) {
		int s = current_source->GetSelectionStart(), e = current_source->GetSelectionEnd();
		if (s!=e && current_source->LineFromPosition(s)==current_source->LineFromPosition(e))
			menu.Append(mxID_INSPECTION_FROM_SOURCE,LANG1(INSPECTGRID_POPUP_COPY_FROM_SELECTION,"Insertar Expresion Desde la &Seleccion (\"<{1}>\")",Shorten(current_source->GetTextRange(s,e))));
	}
	wxString clip_text = mxUT::GetClipboardText();
	if (clip_text.Contains('\n')) 
		menu.Append(mxID_INSPECTION_FROM_CLIPBOARD,wxString(LANG(INSPECTGRID_POPUP_COPY_FROM_CLIPBOARD_MULTIPLE,"Insertar Expresiones Desde el &Portapapeles"))+"\tCtrl+V");
	else if (!clip_text.IsEmpty())
		menu.Append(mxID_INSPECTION_FROM_CLIPBOARD,LANG1(INSPECTGRID_POPUP_COPY_FROM_CLIPBOARD_SINGLE,"Pegar Expresion Desde el &Portapapeles (\"<{1}>\")",Shorten(clip_text))+"\tCtrl+V");
	if (!sel_is_empty) menu.Append(mxID_INSPECTION_CLEAR_ONE,wxString(LANG(INSPECTGRID_POPUP_DELETE,"Eliminar Inspeccion"))+"\tSupr");
	if (sel_is_vo && di->IsSimpleType()) {
		wxMenu *submenu = new wxMenu;
		submenu->Append(mxID_INSPECTION_FORMAT_NAT,LANG(INSPECTGRID_FORMAT_NATURAL,"natural"));
		submenu->Append(mxID_INSPECTION_FORMAT_BIN,LANG(INSPECTGRID_FORMAT_BINARY,"binario"));
		submenu->Append(mxID_INSPECTION_FORMAT_OCT,LANG(INSPECTGRID_FORMAT_OCTAL,"octal"));
		submenu->Append(mxID_INSPECTION_FORMAT_DEC,LANG(INSPECTGRID_FORMAT_DECIMAL,"decimal"));
		submenu->Append(mxID_INSPECTION_FORMAT_HEX,LANG(INSPECTGRID_FORMAT_HEXADECIMAL,"hexadecimal"));
		menu.AppendSubMenu(submenu,LANG(INSPECTGRID_FORMAT,"Formato"));
	}
//	if (sel_is_vo) {
	wxMenu *imprv_submenu = new wxMenu;
	if (sel_has_improved) imprv_submenu->Append(mxID_INSPECTION_IMPR_EXPOSE,LANG(INSPECTGRID_IMPRV_SHOW_IMPROVE,"Mostrar expresión mejorada"));
	if (sel_has_improved) imprv_submenu->Append(mxID_INSPECTION_IMPR_DISCARD,LANG(INSPECTGRID_IMPRV_DONT_IMPROVE,"No mejorar expresión automáticamente"));
	if (sel_is_single && sel_is_vo) imprv_submenu->Append(mxID_INSPECTION_IMPR_ADD_GENERAL,LANG(INSPECTGRID_IMPRV_ADD_FOR_THIS_TYPE_GENERAL,"Agregar mejora para este tipo (generales)..."));
	if (project) if (sel_is_single && sel_is_vo) imprv_submenu->Append(mxID_INSPECTION_IMPR_ADD_PROJECT,LANG(INSPECTGRID_IMPRV_ADD_FOR_THIS_TYPE_PROJECT,"Agregar mejora para este tipo (proyecto)..."));
	imprv_submenu->Append(mxID_INSPECTION_IMPR_CONF_GENERAL,LANG(INSPECTGRID_IMPRV_CONF_GENERAL,"Configurar (generales)..."));
	if (project) imprv_submenu->Append(mxID_INSPECTION_IMPR_CONF_PROJECT,LANG(INSPECTGRID_IMPRV_CONF_PROYECT,"Configurar (proyecto)..."));
	menu.AppendSubMenu(imprv_submenu,LANG(INSPECTGRID_IMPRV_SUBMENU,"Mejoras automáticas según tipo"));
//	}
	if (there_are_inspections) {
		menu.AppendSeparator();
		menu.Append(mxID_INSPECTION_CLEAR_ALL,LANG(INSPECTGRID_POPUP_CLEAN_TABLE,"&Limpiar Tabla de Inspecciones"));
	}
	PopupMenu(&menu);
}

void mxInspectionGrid::OnBreakClassOrArray(wxCommandEvent &evt) {
	BreakCompoundInspection(GetGridCursorRow());
}

void mxInspectionGrid::OnPasteFromClipboard(wxCommandEvent &evt) {
	wxString data = mxUT::GetClipboardText();
	data<<"\n"; data.Replace("\r","",true);
	int row=GetGridCursorRow();
	while (data.Contains('\n')) {
		wxString expr = data.BeforeFirst('\n');
		data = data.AfterFirst('\n');
		if (expr.IsEmpty()) continue;
		InsertRows(row,1);
		CreateInspection(row,expr,false,true);
		row++;
	}
}

void mxInspectionGrid::OnCopyFromSelecction(wxCommandEvent &evt) {
	if (main_window->notebook_sources->GetPageCount()>0) {
		mxSource *source = (mxSource*)(main_window->notebook_sources->GetPage(main_window->notebook_sources->GetSelection()));
		int s = source->GetSelectionStart(), e = source->GetSelectionEnd();
		wxString expr = source->GetTextRange(s,e);
		if (expr.IsEmpty()) return;
		int row=GetGridCursorRow();
		InsertRows(row,1);
		CreateInspection(row,expr,false,true);
	}
}

void mxInspectionGrid::OnCopyValue(wxCommandEvent &evt) {
	mxGrid::CopyToClipboard(true,IG_COL_VALUE);
}

void mxInspectionGrid::OnCopyType(wxCommandEvent &evt) {
	mxGrid::CopyToClipboard(true,IG_COL_TYPE);
}

void mxInspectionGrid::OnCopyExpression(wxCommandEvent &evt) {
	mxGrid::CopyToClipboard(true,IG_COL_EXPR);
}

void mxInspectionGrid::OnClearAll(wxCommandEvent &evt) {
	ClearAll();
}


void mxInspectionGrid::OnClearOne(wxCommandEvent &evt) {
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	if (sel.empty()) return; int min=sel.back();
	for(unsigned int i=0;i<sel.size();i++) DeleteInspection(sel[i],false);
	mxGrid::Select(min);
}


static void auxOnExplore(DebugManager::TemporaryScopeChange &scope, mxInspectionGrid::InspectionGridRow &row, mxInspectionExplorerDialog *dialog=nullptr) {
	if (row.IsNull()) return;
	DebuggerInspection *di = row.di;
	scope.ChangeIfNeeded(di);
	if (dialog) dialog->AddExpression(di->GetExpression(),di->IsFrameless());
	else new mxInspectionExplorerDialog(di->GetExpression(),di->IsFrameless());
}

void mxInspectionGrid::OnExploreAll(wxCommandEvent &event) {
	DebugManager::TemporaryScopeChange scope;
	mxInspectionExplorerDialog *dialog = new mxInspectionExplorerDialog();
	for(int i=0;i<inspections.GetSize();i++)
		auxOnExplore(scope,inspections[i],dialog);
}

void mxInspectionGrid::OnExploreExpression(wxCommandEvent &evt) {
	DebugManager::TemporaryScopeChange scope;
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	if (sel.size()==1) {
		auxOnExplore(scope,inspections[sel[0]]);
	} else {
		mxInspectionExplorerDialog *dialog = new mxInspectionExplorerDialog();
		for(unsigned int i=0;i<sel.size();i++)
			auxOnExplore(scope,inspections[sel[i]],dialog);
	}
}

void mxInspectionGrid::SetFormat(int format) {
	vector<int> sel; mxGrid::GetSelectedRows(sel,false);
	for(unsigned int i=0;i<sel.size();i++) 
		if (inspections[i]->IsSimpleType()) {
			inspections[i]->SetFormat((GDB_VO_FORMAT)format);
			UpdateValueColumn(i);
		}
}
void mxInspectionGrid::OnFormatNatural(wxCommandEvent &evt) {
	SetFormat(GVF_NATURAL);
}

void mxInspectionGrid::OnFormatDecimal(wxCommandEvent &evt) {
	SetFormat(GVF_DECIMAL);
}
void mxInspectionGrid::OnFormatOctal(wxCommandEvent &evt) {
	SetFormat(GVF_OCTAL);
}
void mxInspectionGrid::OnFormatHexadecimal(wxCommandEvent &evt) {
	SetFormat(GVF_HEXADECIMAL);
}

void mxInspectionGrid::OnFormatBinary(wxCommandEvent &evt) {
	SetFormat(GVF_BINARY);
}

void mxInspectionGrid::OnShowInTable(wxCommandEvent &evt) {
	DebugManager::TemporaryScopeChange scope;
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++) {
		if (inspections[sel[i]].IsNull()) continue;
		DebuggerInspection *di = inspections[sel[i]].di;
		scope.ChangeIfNeeded(di);
		new mxInspectionMatrix(di->GetExpression(),di->IsFrameless());
	}
}

void mxInspectionGrid::OnShowInText(wxCommandEvent &evt) {
	DebugManager::TemporaryScopeChange scope;
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++) {
		if (inspections[sel[i]].IsNull()) continue;
		DebuggerInspection *di = inspections[sel[i]].di;
		scope.ChangeIfNeeded(di);
		new mxInspectionPrint(di->GetExpression(),di->IsFrameless());
	}
}

/**
* @brief Duplica una inspeccion de la grilla, colocando la copia justo debajo del original
**/
void mxInspectionGrid::OnDuplicate(wxCommandEvent &evt) {
	DebugManager::TemporaryScopeChange scope;
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++) {
		if (inspections[sel[i]].IsNull()) continue;
		DebuggerInspection *di = inspections[sel[i]].di;
		scope.ChangeIfNeeded(di);
		InsertRows(sel[i]+1,1);
		CreateInspection(sel[i]+1,di->GetExpression(),di->IsFrameless(),true);
	}
//	DebuggerInspection::OnDebugPause(); // la expresion podría haber modificado algo
}

void mxInspectionGrid::OnFreeze(wxCommandEvent &evt) {
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	for(unsigned int i=0;i<sel.size();i++) SetFreezed(sel[i],true);
}

void mxInspectionGrid::OnUnFreeze(wxCommandEvent &evt) {
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	for(unsigned int i=0;i<sel.size();i++) SetFreezed(sel[i],false);
}

void mxInspectionGrid::SetFreezed(int row, bool freeze) {
	if (inspections[row].IsNull()) return;
	if (freeze) {
		SetRowStatus(row,IGRS_FREEZE);
		inspections[row].is_frozen=true; // after SetRowStatus (SetRowStatus only changes what user sees if inspection is not frozen)
	} else {
		inspections[row].is_frozen=false; // before SetRowStatus (SetRowStatus only changes what user sees if inspection is not frozen)
		int real_status = inspections[row].status;
		inspections[row].status=IGRS_FREEZE; // to force SetRowStatus apply the real status
		SetRowStatus(row,real_status);
		UpdateValueColumn(row); 
		UpdateTypeColumn(row); 
		UpdateExpressionColumn(row,true); 
//		if (inspections[row]->IsFrameless()) TryToSimplify(row); // type can change when a frameless inspection is frozen
	}
}

void mxInspectionGrid::OnCopyAll(wxCommandEvent &evt) {
	mxGrid::CopyToClipboard(false);
}
//
wxDragResult mxInspectionDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def) {
	int ux,uy,vx,vy;
	grid->GetScrollPixelsPerUnit(&ux,&uy);
	grid->GetViewStart(&vx,&vy);
	int r = grid->YToRow(y+vy*uy-grid->GetColLabelSize());
	if (!grid->IsDraggingAnInspection() && r!=wxNOT_FOUND) grid->SelectRow(r);
	return grid->IsDraggingAnInspection()?wxDragMove:wxDragCopy;
}


mxInspectionDropTarget::mxInspectionDropTarget(mxInspectionGrid *agrid) {
	grid=agrid;
	SetDataObject(data=new wxTextDataObject());
}

bool mxInspectionDropTarget::OnDrop(wxCoord x, wxCoord y) {
	return true;
}

wxDragResult mxInspectionDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult ref) {
	int ux,uy,vx,vy;
	grid->GetScrollPixelsPerUnit(&ux,&uy);
	grid->GetViewStart(&vx,&vy);
	int r=grid->YToRow(y+vy*uy-grid->GetColLabelSize());
	if (r==wxNOT_FOUND) return wxDragCancel;
	if (grid->IsDraggingAnInspection()) {
		grid->SwapInspections(grid->GetGridCursorRow(),r);
		return wxDragMove;
	} else {
		GetData(); grid->SelectRow(r);
		wxString str=data->GetText();
		if (!str.size()) return wxDragCancel;
		str.Replace("\n"," ");
		str.Replace("\r"," ");
		grid->InsertRows(r,1);
		grid->CreateInspection(r,str,false,true);
		grid->Select(r);
		return wxDragCopy;
	}
}

bool mxInspectionGrid::IsDraggingAnInspection() {
	return dragging_inspection;
}

void mxInspectionGrid::OnClick(wxGridEvent &event) {
	if (event.AltDown()) {
		mxGrid::Select(event.GetRow());
		dragging_inspection=true;
		wxTextDataObject my_data(wxGrid::GetCellValue(event.GetRow(),event.GetCol()));
		wxDropSource dragSource(this);
		dragSource.SetData(my_data);
		dragSource.DoDragDrop(wxDrag_AllowMove|wxDrag_DefaultMove);
		dragging_inspection=false;
	} else event.Skip();
}

void mxInspectionGrid::OnRedirectedEditEvent (wxCommandEvent & event) {
	switch (event.GetId()) {
	case wxID_COPY: 
		OnCopyExpression(event); 
		break;
	case wxID_PASTE:
		OnPasteFromClipboard(event);
		break;
	case wxID_CUT:
		OnCopyExpression(event);
		OnClearOne(event);
		break;
	case mxID_EDIT_DUPLICATE_LINES:
		OnDuplicate(event);
		break;
	case mxID_EDIT_DELETE_LINES:
		OnClearOne(event);
		break;
	case mxID_EDIT_COMMENT:
		OnFreeze(event);
		break;
	case mxID_EDIT_UNCOMMENT:
		OnUnFreeze(event); 
		break;
	case mxID_EDIT_MARK_LINES:
	case mxID_EDIT_INDENT:
		if (debug->CanTalkToGDB()) {
			vector<int> sels;
			if (!mxGrid::GetSelectedRows(sels,false)) return;
			if (inspections[sels[0]]->IsFrameless()) OnReScope(event); else OnSetFrameless(event);
		}
		break;
	case mxID_EDIT_SELECT_ALL:
		wxGrid::SelectAll();
		break;
	case mxID_EDIT_TOGGLE_LINES_UP: 
		{
			vector<int> sels;
			if (!mxGrid::GetSelectedRows(sels,false) || sels[0]==0) return;
			mxGrid::Select(sels.front()-1);
			for(unsigned int i=0;i<sels.size();i++) { SwapInspections(sels[i],sels[i]-1); wxGrid::SelectRow(sels[i]-1,true); }
		}
		break;
	case mxID_EDIT_TOGGLE_LINES_DOWN:
		{
			vector<int> sels;
			if (!mxGrid::GetSelectedRows(sels,true) || sels[0]+1==inspections.GetSize()) return;
			mxGrid::Select(sels.back()+1);
			for(unsigned int i=0;i<sels.size();i++) { SwapInspections(sels[i],sels[i]+1); wxGrid::SelectRow(sels[i]+1,true); }
		}
		break;
	default:
		event.Skip();
	}
}

void mxInspectionGrid::InsertRows(int pos, int cant) {
	if (pos==-1) {
		pos=wxGrid::GetNumberRows(); 
		wxGrid::AppendRows(cant,false);
		for(int i=0;i<cant;i++) 
			inspections.Add(nullptr);
	} else {
		wxGrid::InsertRows(pos,cant,false);
		inspections.MakeRoomForMultipleInsert(pos,cant);
		for(int i=pos;i<pos+cant;i++) inspections[i].Reset();
	}
	for(int i=pos;i<pos+cant;i++) {
		inspections[i].Reset();
		mxGrid::SetReadOnly(i,IG_COL_LEVEL,true);
		mxGrid::SetCellEditor(i,IG_COL_EXPR,new gdbInspCtrl);
		mxGrid::SetReadOnly(i,IG_COL_TYPE,true);
		mxGrid::SetReadOnly(i,IG_COL_VALUE,true);
		mxGrid::SetCellRenderer(i,IG_COL_EXPR,inspections[i].expression_renderer = new mxGridCellRenderer());
		mxGrid::SetCellRenderer(i,IG_COL_VALUE,inspections[i].value_renderer = new mxGridCellRenderer());
	}
}

// eventos de DebuggerInspection
void mxInspectionGrid::OnDIError(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
	SetRowStatus(current_row,IGRS_ERROR);
}

void mxInspectionGrid::OnDICreated(DebuggerInspection *di) {
	OnDINewType(di); if (current_row==-1) return;
	UpdateLevelColumn(current_row);
}

void mxInspectionGrid::OnDIValueChanged(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
	if (!inspections[current_row].is_frozen) UpdateValueColumn(current_row);
	SetRowStatus(current_row,IGRS_CHANGED);
}

void mxInspectionGrid::OnDINewType(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
//	if (!inspections[current_row].is_frozen) TryToSimplify(current_row);
	if (!inspections[current_row].is_frozen) {
		UpdateTypeColumn(current_row);
		UpdateValueColumn(current_row);
		UpdateExpressionColumn(current_row,true);
	}
	SetRowStatus(current_row,IGRS_CHANGED);
}

void mxInspectionGrid::OnDIInScope(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
//	TryToSimplify(current_row);
	UpdateValueColumn(current_row);
	SetRowStatus(current_row,IGRS_IN_SCOPE);
}

void mxInspectionGrid::OnDIOutOfScope(DebuggerInspection *di) {
	if (!SetCurrentRow(di)) return;
	SetRowStatus(current_row,IGRS_OUT_OF_SCOPE);
	if (!di->IsInScope()) UpdateLevelColumn(current_row);
}

void mxInspectionGrid::SetRowStatus (int r, int status) {
	int prev_status = inspections[r].status;
	if (prev_status==status) return;
	if (status!=IGRS_FREEZE) inspections[r].status=status;
	if (inspections[r].is_frozen) return;
	if (s_mxig_status_opts[prev_status].color!=s_mxig_status_opts[status].color)
		mxGrid::SetCellColour(r,IG_COL_VALUE,s_mxig_status_opts[status].color);
	if (s_mxig_status_opts[status].have_message) 
		mxGrid::SetCellValue(r,IG_COL_VALUE,s_mxig_status_opts[status].message);
	if (s_mxig_status_opts[prev_status].editable_value!=s_mxig_status_opts[status].editable_value)
		mxGrid::SetReadOnly(r,IG_COL_VALUE,!s_mxig_status_opts[status].editable_value);
}

void mxInspectionGrid::SetRowStatus (int r, int status, bool dummy_force) {
	if (status!=IGRS_FREEZE) inspections[r].status=status;
	mxGrid::SetCellColour(r,IG_COL_VALUE,s_mxig_status_opts[status].color);
	mxGrid::SetCellValue(r,IG_COL_VALUE,s_mxig_status_opts[status].message);
	mxGrid::SetReadOnly(r,IG_COL_VALUE,!s_mxig_status_opts[status].editable_value);
}

void mxInspectionGrid::OnFullTableUpdateBegin( ) {
	if (full_table_update_began) return;
	full_table_update_began=true; BeginBatch();
	for(int i=0;i<inspections.GetSize();i++) {
		if (inspections[i].IsNull()) continue; ///< pasa cuando se crea una nuevo, se llama a este metodo antes de hacerlo, pero ya habiendo reservado el espacio en inspections
		UpdateLevelColumn(i);
		InspectionGridRow &di = inspections[i];
		if (di.status==IGRS_UNINIT || inspections[i].is_frozen) continue;
		// no es mas necesario preguntar por las manuales, avisa solo el DebuggerInspection
//		if (di->RequiresManualUpdate()) {
//			if (di->IsInScope()) {
//				if (di->UpdateValue()) {
//					UpdateValueColumn(current_row);
//					SetRowStatus(i,IGRS_CHANGED);	
//				} else
//					SetRowStatus(i,IGRS_NORMAL);
//			}
//		}
		else if (di.status==IGRS_IN_SCOPE||di.status==IGRS_CHANGED) {
			SetRowStatus(i,IGRS_NORMAL);
		}
	}
}

void mxInspectionGrid::OnFullTableUpdateEnd ( ) {
	EndBatch();
	full_table_update_began=false;
}


void mxInspectionGrid::UpdateLevelColumn (int r) {
	InspectionGridRow &di=inspections[r];
	if (di.IsNull()) { mxGrid::SetCellValue(r,IG_COL_LEVEL,""); return; }
	if (!di->IsInScope()) { mxGrid::SetCellValue(r,IG_COL_LEVEL,"-"); return; }
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

bool mxInspectionGrid::CreateInspection (int r, const wxString &expression, bool frameless, bool set_expr) {
	if (set_expr) mxGrid::SetCellValue(r,IG_COL_EXPR,expression);
	inspections[r] = DebuggerInspection::Create(expression,FlagIf(DIF_FRAMELESS,frameless)|DIF_AUTO_IMPROVE,this,false);
	if (!inspections[r]->Init()) SetRowStatus(r,IGRS_UNINIT);
	else UpdateLevelColumn(r);
	return inspections[r]->GetDbiType()!=DIT_ERROR;
}

void mxInspectionGrid::DeleteInspection (int r, bool for_reuse) {
	if (inspections[r].di) inspections[r]->Destroy(); // si es una que ya existía
	if (for_reuse) {
		inspections[r].Reset(); // para que OnFullTableUpdateBegin no la considere más
		inspections[r].expression_renderer->SetIconNull();
	} else {
		inspections.Remove(r); // quitar de la lista propia de inspecciones
		DeleteRows(r,1); 														// eliminar fila de la tabla
	}
}

void mxInspectionGrid::UpdateValueColumn (int r) {
	DebuggerInspection *di = inspections[r].di;
	mxGrid::SetCellValue(r,IG_COL_VALUE,di?di->GetValue():"");
}

void mxInspectionGrid::UpdateTypeColumn (int r) {
	DebuggerInspection *di = inspections[r].di;
	if (!di) { mxGrid::SetCellValue(r,IG_COL_TYPE,""); mxGrid::SetReadOnly(r,IG_COL_VALUE,true); return; }
	mxGrid::SetCellValue(r,IG_COL_TYPE,di->GetValueType());
	if (di->GetDbiType()==DIT_VARIABLE_OBJECT) {
		if (di->IsCompound()) inspections[r].value_renderer->SetIconPlus();
		else inspections[r].value_renderer->SetIconNull();
		mxGrid::SetReadOnly(r,IG_COL_VALUE,true);
	} else {
		mxGrid::SetReadOnly(r,IG_COL_VALUE,true);
		inspections[r].value_renderer->SetIconNull();
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
		inspections[r+i].CopyStatus(old);
		mxGrid::SetCellValue(r+i,IG_COL_LEVEL,old_level);
		inspections[r+i].di=children[i];
		// fill grid with new expressions
		mxGrid::SetCellValue(r+i,IG_COL_EXPR,children[i]->GetExpression());
		UpdateTypeColumn(r+i);
		UpdateValueColumn(r+i);
		UpdateExpressionColumn(r+i,true);
	}
}

void mxInspectionGrid::OnReScope(wxCommandEvent &event) {
	mxIG_SideEffectUpdate sda(this); // la expresion podría haber modificado algo, esto actualiza toda la tabla
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	for(unsigned int i=0;i<sel.size();i++) 
		ChangeFrameless(sel[i],false,false);
}

void mxInspectionGrid::OnSetFrameless (wxCommandEvent & evt) {
	mxIG_SideEffectUpdate sda(this); // la expresion podría haber modificado algo, esto actualiza toda la tabla
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	for(unsigned int i=0;i<sel.size();i++) 
		ChangeFrameless(sel[i],true,false);
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
	mxIG_SideEffectUpdate sda(full_table_update?this:nullptr); // la expresion podría haber modificado algo, esto actualiza toda la tabla
	CreateInspection(r,old_expression,!was_frameless);
}

void mxInspectionGrid::SwapInspections (int r1, int r2) {
	wxString old_value1 = inspections[r1].is_frozen?mxGrid::GetCellValue(r1,IG_COL_VALUE):"";
	wxString old_value2 = inspections[r2].is_frozen?mxGrid::GetCellValue(r2,IG_COL_VALUE):"";
	int status1 = inspections[r1].GetVisibleStatus(), status2 = inspections[r2].GetVisibleStatus();
	inspections[r1].Swap(inspections[r2]); 
	bool one_frozen = (status1==IGRS_FREEZE||status2==IGRS_FREEZE)&&status1!=status2;
	SetRowStatus(r1,status2,one_frozen); SetRowStatus(r2,status1,one_frozen);
	UpdateExpressionColumn(r1); UpdateLevelColumn(r1); UpdateTypeColumn(r1); 
	if (inspections[r1].is_frozen) mxGrid::SetCellValue(r1,IG_COL_VALUE,old_value2); else UpdateValueColumn(r1);
	UpdateExpressionColumn(r2); UpdateLevelColumn(r2); UpdateTypeColumn(r2);
	if (inspections[r2].is_frozen) mxGrid::SetCellValue(r2,IG_COL_VALUE,old_value1); else UpdateValueColumn(r2);
}

void mxInspectionGrid::UpdateExpressionColumn (int r, bool only_icon) {
	if (inspections[r].IsNull()) {
		if (!only_icon) mxGrid::SetCellValue(r,IG_COL_EXPR,"");
		inspections[r].expression_renderer->SetIconNull();
	} else {
		if (!only_icon) mxGrid::SetCellValue(r,IG_COL_EXPR,inspections[r]->GetExpression());
		if (inspections[r]->GetHelperExpression().IsEmpty()) {
			inspections[r].expression_renderer->SetIconNull();
		} else {
			inspections[r].expression_renderer->SetIconPlus();
		}
	}
}

void mxInspectionGrid::OnColumnHideOrUnhide (int c, bool visible) {
		for(int i=0;i<inspections.GetSize();i++) { 
			if (visible) {
				switch (c) {
					case IG_COL_LEVEL: UpdateLevelColumn(i); break;
					case IG_COL_EXPR: UpdateExpressionColumn(i); break;
					case IG_COL_TYPE: UpdateTypeColumn(i); break;
					case IG_COL_VALUE: UpdateValueColumn(i); break;
				}
			}
		}
}

void mxInspectionGrid::OnDebugPausePre ( ) {
	OnFullTableUpdateBegin();
}

void mxInspectionGrid::OnDebugPausePost ( ) {
	OnFullTableUpdateEnd();
}

mxInspectionGrid::~mxInspectionGrid ( ) {
	while(inspections.GetSize()) DeleteInspection(0,false);
}


void mxInspectionGrid::OnDiscardImprovedExpression (wxCommandEvent & event) {
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	for(unsigned int i=0;i<sel.size();i++) 
		DiscardImprovedExpression(sel[i]);
}

void mxInspectionGrid::DiscardImprovedExpression (int r) {
	if (inspections[r].IsNull()) return;
	inspections[r].di->DeleteHelperInspection();
	UpdateExpressionColumn(r,true);
}


void mxInspectionGrid::OnExposeImprovedExpression (wxCommandEvent & event) {
	mxIG_SideEffectUpdate sda(this); // la expresion podría haber modificado algo, esto actualiza toda la tabla
	vector<int> sel; mxGrid::GetSelectedRows(sel);
	for(unsigned int i=0;i<sel.size();i++) 
		ExposeImprovedExpression(sel[i]);
}

void mxInspectionGrid::ExposeImprovedExpression (int r) {
	if (inspections[r].IsNull()) return;
	BoolFlagGuard icce_guard(ignore_cell_change_event);
	DebuggerInspection *di = inspections[r].di;
	wxString new_expr = di->GetHelperExpression();
	if (!new_expr.Len()) return;
	DebugManager::TemporaryScopeChange scope;
	scope.ChangeIfNeeded(di);
	bool was_frameless=di->IsFrameless();
	di->Destroy();
	CreateInspection(r,new_expr,was_frameless);
	mxGrid::SetCellValue(r,IG_COL_EXPR,new_expr);
}

static void aux_show_inspections_improving_settings(bool for_project,const wxString &type="", const wxString &expr="") {
	mxInspectionsImprovingEditor(main_window,
		for_project?project->inspection_improving_template_from:config->Debug.inspection_improving_template_from,
		for_project?project->inspection_improving_template_to:config->Debug.inspection_improving_template_to,
		type,expr);
}

void mxInspectionGrid::OnRegisterNewImprovedExpressionProject (wxCommandEvent & event) {
	OnRegisterNewImprovedExpression(true);
}

void mxInspectionGrid::OnRegisterNewImprovedExpressionGeneral (wxCommandEvent & event) {
	OnRegisterNewImprovedExpression(false);
}

void mxInspectionGrid::OnRegisterNewImprovedExpression (bool for_project) {
	vector<int> sel; mxGrid::GetSelectedRows(sel); 
	if (sel.size()!=1) return;
	if (inspections[sel[0]].IsNull()) return;
	if (inspections[sel[0]]->GetDbiType()==DIT_GDB_COMMAND) return;
	if (inspections[sel[0]]->GetValueType().IsEmpty()) return;
	aux_show_inspections_improving_settings(for_project,
		inspections[sel[0]]->GetValueType(),inspections[sel[0]]->GetExpression());
}

void mxInspectionGrid::OnInspectionsImprovingSettingsGeneral (wxCommandEvent & event) {
	aux_show_inspections_improving_settings(false);
}

void mxInspectionGrid::OnInspectionsImprovingSettingsProject (wxCommandEvent & event) {
	aux_show_inspections_improving_settings(true);
}

void mxInspectionGrid::GetInspectionsList (wxArrayString &expressions) {
	for(int i=0;i<inspections.GetSize();i++) {
		if (!inspections[i].IsNull())
			expressions.Add(inspections[i]->GetExpression());
	}
}

void mxInspectionGrid::SetInspectionsList (const wxArrayString &expressions) {
	for(unsigned int i=0;i<expressions.GetCount();i++)
		ModifyExpression(-1,expressions[i],true,true);
}

void mxInspectionGrid::ClearAll ( ) {
	int n = inspections.GetSize()-1;
	for(int i=0;i<n;i++) 
		DeleteInspection(n-i-1,true);
	inspections.Remove(0,n);
	DeleteRows(0,n);
}

void mxInspectionGrid::OnShowInHistory (wxCommandEvent & evt) {
	DebugManager::TemporaryScopeChange scope;
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++) {
		if (inspections[sel[i]].IsNull()) continue;
		DebuggerInspection *di = inspections[sel[i]].di;
		scope.ChangeIfNeeded(di);
		new mxInspectionHistory(di->GetExpression(),di->IsFrameless());
	}
}

void mxInspectionGrid::OnShowInRTEditor (wxCommandEvent & evt) {
	DebugManager::TemporaryScopeChange scope;
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++) {
		if (inspections[sel[i]].IsNull()) continue;
		if (inspections[sel[i]]->GetDbiType()==DIT_GDB_COMMAND) continue;
		DebuggerInspection *di = inspections[sel[i]].di;
		scope.ChangeIfNeeded(di);
		new mxRealTimeInspectionEditor(di->GetExpression());
	}
}

void mxInspectionGrid::OnSetWatch (wxCommandEvent & evt) {
	bool read = evt.GetId()!=mxID_INSPECTION_WATCH_WRITE;
	bool write = evt.GetId()!=mxID_INSPECTION_WATCH_READ;
	DebugManager::TemporaryScopeChange scope;
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++) {
		if (inspections[sel[i]].IsNull()) continue;
		if (inspections[sel[i]]->GetDbiType()==DIT_GDB_COMMAND) continue;
		wxString expr= inspections[sel[i]]->GetExpression();
		wxString num =debug->AddWatchPoint(expr,read,write);
		if (!num.IsEmpty())
			mxMessageDialog(main_window,LANG(INSPECTION_WATCH_ADDED_OK,"Watchpoint insertado correctamente."),num+": "+expr,mxMD_OK|mxMD_INFO).ShowModal();
		else
			mxMessageDialog(main_window,LANG(INSPECTION_WATCH_ADDED_ERROR,"Error al insertar watchpoint."),expr,mxMD_OK|mxMD_ERROR).ShowModal();
	}
}

void mxInspectionGrid::OnDerefPtr (wxCommandEvent & evt) {
	DebugManager::TemporaryScopeChange scope;
	vector<int> sel; mxGrid::GetSelectedRows(sel,true);
	for(unsigned int i=0;i<sel.size();i++) {
		if (inspections[sel[i]].IsNull()) continue;
		DebuggerInspection *di = inspections[sel[i]].di;
		if (di->IsPointer()) {
			ModifyExpression(sel[i],
				wxString("*")+DebuggerInspection::RewriteExpressionForBreaking(di->GetExpression()),
				di->IsFrameless(),true);
		}
	}
}

void mxInspectionGrid::OnEditFromKeyboard (wxCommandEvent & event) {
	if (mxGrid::SetGridCursor(GetGridCursorRow(),IG_COL_EXPR)) EnableCellEditControl(true);
}

