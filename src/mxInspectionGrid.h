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
//class mxPlusCellRenderer : public wxGridCellStringRenderer {
//public:
//	virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected) {
//		wxRect rect2=rect; 
//		rect2.x+=20; rect2.width-=20;
//		wxGridCellStringRenderer::Draw(grid, attr, dc, rect2, row, col, isSelected);
//		rect2.x-=20; rect2.width=20;
//		dc.DrawRectangle(rect2);
//		dc.SetPen(*wxMEDIUM_GREY_PEN);
//		rect2.x+=3;
//		rect2.y+=(rect2.height-14)/2;
//		rect2.width=rect2.height=13;
//		dc.DrawRectangle(rect2);
//		dc.DrawLine(rect2.x+2,rect2.y+rect2.height/2,rect2.x+rect2.width-2,rect2.y+rect2.height/2);
//		dc.DrawLine(rect2.x+rect2.width/2,rect2.y+2,rect2.x+rect2.width/2,rect2.y+rect2.height-2);
//	}
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
	enum { IGRS_UNINIT, IGRS_OUT_OF_SCOPE, IGRS_IN_SCOPE, IGRS_NORMAL, IGRS_CHANGED, IGRS_ERROR, IGRS_FREEZE, IGRS_COUNT };
private:
	struct InspectionGridRow {
		DebuggerInspection *di;
		// estado de la fila en la grilla, para no invocar metodos de grilla si no cambio nada... 
		int status; long frame_level; bool on_thread; // frame_id==-1 para las frameless
		InspectionGridRow(DebuggerInspection *_di=NULL) : di(_di),status(IGRS_UNINIT),frame_level(-2),on_thread(true) {}
		bool operator==(const InspectionGridRow &o) { return di==o.di; }
		DebuggerInspection *operator->() { return di; }
		bool IsNull() { return di==NULL; }
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
//	void OnClick(wxGridEvent &event);
//	void OnRightClick(wxGridEvent &event);
//	void OnFreeze(wxCommandEvent &evt);
//	void OnPasteFromClipboard(wxCommandEvent &evt);
//	void OnCopyFromSelecction(wxCommandEvent &evt);
//	void OnBreakClassOrArray(wxCommandEvent &evt);
//	void OnDuplicate(wxCommandEvent &evt);
//	void OnReScope(wxCommandEvent &evt);
//	void OnExploreExpression(wxCommandEvent &evt);
//	void OnExploreAll(wxCommandEvent &evt);
//	void OnShowInText(wxCommandEvent &evt);
//	void OnShowInTable(wxCommandEvent &evt);
//	void OnCopyData(wxCommandEvent &evt);
//	void OnCopyExpression(wxCommandEvent &evt);
//	void OnClearAll(wxCommandEvent &evt);
//	void OnClearOne(wxCommandEvent &evt);
//	void OnWatchNo(wxCommandEvent &evt);
//	void OnWatchRead(wxCommandEvent &evt);
//	void OnWatchWrite(wxCommandEvent &evt);
//	void OnWatchReadWrite(wxCommandEvent &evt);
//	void OnFormatNatural(wxCommandEvent &evt);
//	void OnFormatDecimal(wxCommandEvent &evt);
//	void OnFormatOctal(wxCommandEvent &evt);
//	void OnFormatHexadecimal(wxCommandEvent &evt);
//	void OnFormatBinary(wxCommandEvent &evt);
//	int ShouldDivide(int row, wxString expr, bool frameless=false);
//	void OnResize(wxSizeEvent &evt);
//	void OnColResize(wxGridSizeEvent &evt);
//	void OnShowHideCol(wxCommandEvent &evt);
//	void OnSaveTable(wxCommandEvent &evt);
//	void OnLoadTable(wxCommandEvent &evt);
//	void OnManageTables(wxCommandEvent &evt);
//	void OnSetFrameless(wxCommandEvent &evt);
//	void OnShowAppart(wxCommandEvent &evt);
//	void OnCopyAll(wxCommandEvent &evt);
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
	
	// eventos generados por mxGrid
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

	void UpdateLevelColumn(int r);
	void SetRowStatus(int r, int status);
	void DeleteInspection(int r, bool for_reuse);
	bool CreateInspection(int r, const wxString &expression, bool frameless);
	DECLARE_EVENT_TABLE();
	
};

#endif
