#ifndef MXINSPECTIONGRID_H
#define MXINSPECTIONGRID_H
#include <wx/dnd.h>
#include "mxGrid.h"
#include "SingleList.h"
#include "Inspection.h"
#include "mxInspectionsPanel.h"
#include "Cpp11.h"
#include "raii.h"

enum {IG_COL_LEVEL=0,IG_COL_EXPR,IG_COL_TYPE,IG_COL_VALUE,IG_COLS_COUNT};


class mxInspectionGrid;

class mxInspectionDropTarget : public wxDropTarget {
	mxInspectionGrid *grid;
	wxTextDataObject *data;
public:
	mxInspectionDropTarget(mxInspectionGrid *agrid);
	bool OnDrop(wxCoord x, wxCoord y);
	wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
	wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
//	void OnLeave();
//	wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def);
};


/**
* @brief Representa a la grilla del panel de inspecciones
**/
class mxInspectionGrid : public mxGrid, public myDIEventHandler, public myDIGlobalEventHandler, public mxInspectionsPanelTab {
	BoolFlag ignore_cell_change_event;
	bool full_table_update_began; ///< para poder llamar dos veces a begin sin que la segunda haga nada (la primera desde aca, la segunda desde DebuggerInspection::UpdateAll)
public:
	enum { IGRS_UNINIT, IGRS_OUT_OF_SCOPE, IGRS_IN_SCOPE, IGRS_NORMAL, IGRS_CHANGED, IGRS_ERROR, IGRS_FREEZE, IGRS_COUNT };

	struct InspectionGridRow {
		DebuggerInspection *di;
		mxGridCellRenderer *expression_renderer, *value_renderer;
		// estado de la fila en la grilla, para no invocar metodos de grilla si no cambio nada... 
		int status; long frame_level; bool on_thread, is_frozen; // frame_id==-1 para las frameless
		InspectionGridRow(DebuggerInspection *_di=nullptr) : di(_di),expression_renderer(nullptr),value_renderer(nullptr),status(IGRS_UNINIT),frame_level(-2),on_thread(true),is_frozen(false) {}
		bool operator==(const InspectionGridRow &o) { return di==o.di; }
		void operator=(DebuggerInspection *_di) { 
			Reset(); 
			mxGridCellRenderer *expr=expression_renderer, *valr=value_renderer;
			di=_di;
			expression_renderer=expr; value_renderer=valr; /*no cambiar los renderer*/
		}
		void CopyStatus(const InspectionGridRow &o) {
			if (o.expression_renderer->HasIcon()) expression_renderer->SetIconPlus(); else expression_renderer->SetIconNull();
			if (o.value_renderer->HasIcon()) value_renderer->SetIconPlus(); else value_renderer->SetIconNull();
			status=o.status; // tal vez no deber�a copiar status ???
			frame_level=o.frame_level; on_thread=o.on_thread; is_frozen=o.is_frozen;
		}
		void Swap(InspectionGridRow &o) {
			DebuggerInspection *o_di=o.di; o.di=di; di=o_di;
//			int o_status=o.status; o.status=status; status=o_status; // no copiar, porque esta asociado a la visualizacion de su columna y no a la inspeccion que contiene
			long o_frame_level=o.frame_level; o.frame_level=frame_level; frame_level=o_frame_level;
			bool o_on_thread=o.on_thread; o.on_thread=on_thread; on_thread=o_on_thread;
			bool o_is_frozen=o.is_frozen; o.is_frozen=is_frozen; is_frozen=o_is_frozen;
			if (o.expression_renderer->HasIcon()!=expression_renderer->HasIcon()) {
				if (o.expression_renderer->HasIcon()) expression_renderer->SetIconPlus(); 
				else expression_renderer->SetIconNull();
				mxGridCellRenderer *o_expression_renderer=o.expression_renderer;
				o.expression_renderer=expression_renderer; expression_renderer=o_expression_renderer;
			}
			if (o.value_renderer->HasIcon()!=value_renderer->HasIcon()) {
				if (o.value_renderer->HasIcon()) value_renderer->SetIconPlus(); 
				else value_renderer->SetIconNull();
				mxGridCellRenderer *o_value_renderer=o.value_renderer;
				o.value_renderer=value_renderer; value_renderer=o_value_renderer;
			}
			
		}
		int GetVisibleStatus() { if (is_frozen) return IGRS_FREEZE; else return status; }
		DebuggerInspection *operator->() { return di; }
		bool IsNull() { return di==nullptr; }
		void Reset() { is_frozen=false; di=nullptr; status=IGRS_UNINIT; frame_level=-2; on_thread=true; /*no cambiar los renderer*/ }
	};
	
private:
	SingleList<InspectionGridRow> inspections;
	
	int current_row;
	bool SetCurrentRow(DebuggerInspection* di) {
		current_row = inspections.Find(di); 
		return current_row!=inspections.NotFound();
	}
	
	bool last_return_had_shift_down;
	
public:
	mxInspectionGrid(wxWindow *parent);
	~mxInspectionGrid();
	void OnFullTableUpdateBegin();
	void OnFullTableUpdateEnd();
	void OnCellChange(wxGridEvent &event);
	void OnEditFromKeyboard(wxCommandEvent &event); // F2 key
	void OnFreeze(wxCommandEvent &evt);
	void OnUnFreeze(wxCommandEvent &evt);
	void SetFreezed(int row, bool freezed);
	void OnPasteFromClipboard(wxCommandEvent &evt);
	void OnCopyFromSelecction(wxCommandEvent &evt);
	void OnBreakClassOrArray(wxCommandEvent &evt);
	void OnDuplicate(wxCommandEvent &evt);
	void OnReScope(wxCommandEvent &evt);
	void OnSetFrameless(wxCommandEvent &evt);
	void OnExploreExpression(wxCommandEvent &evt);
	void OnExploreAll(wxCommandEvent &evt);
	void OnShowInText(wxCommandEvent &evt);
	void OnShowInRTEditor(wxCommandEvent &evt);
	void OnShowInTable(wxCommandEvent &evt);
	void OnShowInHistory(wxCommandEvent &evt);
	void OnSetWatch(wxCommandEvent &evt);
	void OnDerefPtr(wxCommandEvent &evt);
	void OnCopyType(wxCommandEvent &evt);
	void OnCopyValue(wxCommandEvent &evt);
	void OnCopyExpression(wxCommandEvent &evt);
	void OnCopyAll(wxCommandEvent &evt);
	void OnClearAll(wxCommandEvent &evt);
	void ClearAll();
	void OnClearOne(wxCommandEvent &evt);
	void OnFormatNatural(wxCommandEvent &evt);
	void OnFormatDecimal(wxCommandEvent &evt);
	void OnFormatOctal(wxCommandEvent &evt);
	void OnFormatHexadecimal(wxCommandEvent &evt);
	void OnFormatBinary(wxCommandEvent &evt);
	void SetFormat(int format);
	
	// drag/drop
	bool dragging_inspection; // para evitar hacer drag y drop sobre si misma
	void OnClick(wxGridEvent &evt);
	bool IsDraggingAnInspection();
	void SwapInspections(int r1, int r2);
	
	void InsertRows(int pos=-1, int cant=1);
	void OnRedirectedEditEvent(wxCommandEvent &event);
	
	bool ModifyExpression(int row, const wxString &expression, bool is_frameless, bool do_update_cell=true);
	
	// eventos generados por mxGrid
	bool OnCellClick(int row, int col);
	void OnCellPopupMenu(int row, int col);
	bool OnCellDoubleClick(int row, int col);
	void OnColumnHideOrUnhide(int c, bool visible);
	bool OnKey(int row, int col, int key, int modifiers);
	
	bool CanHideColumn(int col) { return col==IG_COL_LEVEL||col==IG_COL_TYPE; }
	
	// eventos generados por DebuggerInspection globalmente
//	void OnDebugStart();
	void OnDebugPausePre();
	void OnDebugPausePost();
//	void OnDebugStop();
	// eventos generados por inspecciones individuales DebuggerInspection 
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
	void DiscardImprovedExpression(int r);
	
	void OnExposeImprovedExpression(wxCommandEvent &event);
	void OnDiscardImprovedExpression(wxCommandEvent &event);
	void OnRegisterNewImprovedExpression(bool for_project);
	void OnRegisterNewImprovedExpressionGeneral(wxCommandEvent &event);
	void OnRegisterNewImprovedExpressionProject(wxCommandEvent &event);
	void OnInspectionsImprovingSettingsGeneral(wxCommandEvent &event);
	void OnInspectionsImprovingSettingsProject(wxCommandEvent &event);
	
	void UpdateLevelColumn(int r);
	void UpdateValueColumn(int r);
	void UpdateTypeColumn(int r);
	void UpdateExpressionColumn(int r, bool only_icon=false);
	void SetRowStatus(int r, int status, bool dummy_force);
	void SetRowStatus(int r, int status);
	void DeleteInspection(int r, bool for_reuse);
	bool CreateInspection(int r, const wxString &expression, bool frameless, bool set_expr=false);
	
	
	void GetInspectionsList(wxArrayString &expressions);
	void SetInspectionsList(const wxArrayString &expressions);
		
	DECLARE_EVENT_TABLE();
	
};

#endif
