#ifndef MXINSPECTIONGRID_H
#define MXINSPECTIONGRID_H
#include "mxGrid.h"

//#include <wx/dc.h>
//#include <wx/dnd.h>
#include "SingleList.h"
#include "Inspection.h"
//
enum {IG_COL_LEVEL=0,IG_COL_EXPR,IG_COL_TYPE,IG_COL_VALUE,IG_COLS_COUNT};
//
//
//class mxInspectionGrid;
//
//class mxInspectionDropTarget : public wxDropTarget {
//	mxInspectionGrid *grid;
//	wxTextDataObject *data;
//public:
//	mxInspectionDropTarget(mxInspectionGrid *agrid);
//	bool OnDrop(wxCoord x, wxCoord y);
//	wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
//	wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
////	void OnLeave();
////	wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def);
//};
//

/**
* @brief Representa a la grilla del panel de inspecciones
**/
class mxInspectionGrid : public mxGrid, public myDIEventHandler {
//	bool can_drop; // para evitar hacer drag y drop sobre si misma
//	bool created;
//	int selected_row;
	bool ignore_cell_change_event;
//	int old_size;
//	float cols_sizes[IG_COLS_COUNT];
//	bool *cols_visibles;
////	int cols_marginal;
public:
	enum { IGRS_UNINIT, IGRS_OUT_OF_SCOPE, IGRS_IN_SCOPE, IGRS_NORMAL, IGRS_CHANGED, IGRS_ERROR, IGRS_FREEZE, IGRS_UNFREEZE, IGRS_COUNT };
private:
	struct InspectionGridRow {
		DebuggerInspection *di;
		mxGridCellRenderer *expression_renderer, *value_renderer;
		// estado de la fila en la grilla, para no invocar metodos de grilla si no cambio nada... 
		int status; long frame_level; bool on_thread, is_frozen; // frame_id==-1 para las frameless
		InspectionGridRow(DebuggerInspection *_di=NULL) : di(_di),expression_renderer(NULL),value_renderer(NULL),status(IGRS_UNINIT),frame_level(-2),on_thread(true),is_frozen(false) {}
		bool operator==(const InspectionGridRow &o) { return di==o.di; }
		void operator=(DebuggerInspection *_di) { Reset(); di=_di; }
		DebuggerInspection *operator->() { return di; }
		bool IsNull() { return di==NULL; }
		void Reset() { is_frozen=false; di=NULL; status=IGRS_UNINIT; frame_level=-2; on_thread=true; /*no cambiar los renderer*/ }
	};
	SingleList<InspectionGridRow> inspections;
	
	int current_row;
	bool SetCurrentRow(DebuggerInspection* di) {
		current_row = inspections.Find(di); 
		return current_row!=inspections.NotFound();
	}
	
	bool mask_cell_change_event;
	bool last_return_had_shift_down;
	
public:
	mxInspectionGrid(wxWindow *parent);
//	void AddRow(int cant=1);
//	void AppendInspections(wxArrayString &vars);
//	~mxInspectionGrid();
	void OnFullTableUpdateBegin();
	void OnFullTableUpdateEnd();
	void OnKey(wxKeyEvent &event);
//	void OnSelectCell(wxGridEvent &event);
	void OnCellChange(wxGridEvent &event);
//	void OnDoubleClick(wxGridEvent &event);
//	void OnLabelPopup(wxGridEvent &event);
//	void OnRightClick(wxGridEvent &event);
	void OnFreeze(wxCommandEvent &evt);
	void OnUnFreeze(wxCommandEvent &evt);
	void SetFreezed(int row, bool freezed);
	void OnPasteFromClipboard(wxCommandEvent &evt);
	void OnCopyFromSelecction(wxCommandEvent &evt);
	void OnBreakClassOrArray(wxCommandEvent &evt);
	void OnDuplicate(wxCommandEvent &evt);
	void OnReScope(wxCommandEvent &evt);
	void OnSetFrameless(wxCommandEvent &evt);
//	void OnExploreExpression(wxCommandEvent &evt);
//	void OnExploreAll(wxCommandEvent &evt);
//	void OnShowInText(wxCommandEvent &evt);
//	void OnShowInTable(wxCommandEvent &evt);
	void OnCopyData(wxCommandEvent &evt);
	void OnCopyExpression(wxCommandEvent &evt);
	void OnCopyAll(wxCommandEvent &evt);
	void OnClearAll(wxCommandEvent &evt);
	void OnClearOne(wxCommandEvent &evt);
//	void OnWatchNo(wxCommandEvent &evt);
//	void OnWatchRead(wxCommandEvent &evt);
//	void OnWatchWrite(wxCommandEvent &evt);
//	void OnWatchReadWrite(wxCommandEvent &evt);
	void OnFormatNatural(wxCommandEvent &evt);
	void OnFormatDecimal(wxCommandEvent &evt);
	void OnFormatOctal(wxCommandEvent &evt);
	void OnFormatHexadecimal(wxCommandEvent &evt);
	void OnFormatBinary(wxCommandEvent &evt);
	void SetFormat(int format);
//	int ShouldDivide(int row, wxString expr, bool frameless=false);
//	void OnResize(wxSizeEvent &evt);
//	void OnColResize(wxGridSizeEvent &evt);
//	void OnShowHideCol(wxCommandEvent &evt);
//	void OnSaveTable(wxCommandEvent &evt);
//	void OnLoadTable(wxCommandEvent &evt);
//	void OnManageTables(wxCommandEvent &evt);
//	void OnShowAppart(wxCommandEvent &evt);
////	void OnCellEditorShown(wxGridEvent &evt);
////	void OnCellEditorHidden(wxGridEvent &evt);
//	void HightlightChange(int r);
//	void HightlightDisable(int r);
//	void HightlightNone(int r);
//	void HightlightFreeze(int r);
//	void ResetChangeHightlights();
//	bool ModifyExpresion(int r, wxString expr);
//	bool CanDrop();
//	
	void InsertRows(int pos=-1, int cant=1);
	void OnRedirectedEditEvent(wxCommandEvent &event);
	
	void ModifyInspectionExpression(int row, const wxString &expression, bool is_frameless);
	
	bool TryToSimplify(int row);
	
	// eventos generados por mxGrid
	bool OnCellClick(int row, int col);
	void OnCellPopupMenu(int row, int col);
	bool OnCellDoubleClick(int row, int col);
//	void OnLabelPopupMenu(int col);

	// eventos generados por DebuggerInspection
	void OnDICreated(DebuggerInspection *di);
	void OnDIError(DebuggerInspection *di);
	void OnDIValueChanged(DebuggerInspection *di);
	void OnDIOutOfScope(DebuggerInspection *di);
	void OnDIInScope(DebuggerInspection *di);
	void OnDINewType(DebuggerInspection *di);
	
	bool ValidInspection(int r);
	void BreakCompoundInspection(int r);
	void ChangeFrameless(int r, bool frameless, bool full_table_update=true);
	void ExposeImprovedExpression(int r);
	
	bool TryToImproveExpression(const wxString &pattern, wxString type, wxString &new_expr, const wxString &expr);

	void UpdateLevelColumn(int r);
	void UpdateValueColumn(int r);
	void UpdateTypeColumn(int r);
	void SetRowStatus(int r, int status);
	void DeleteInspection(int r, bool for_reuse);
	bool CreateInspection(int r, const wxString &expression, bool frameless);
	DECLARE_EVENT_TABLE();
	
};

#endif
