#ifndef MXINSPECTIONEXPLORERWIDGET_H
#define MXINSPECTIONEXPLORERWIDGET_H
#include <wx/treectrl.h>
#include "Inspection.h"

class mxInspectionExplorerWidget : public wxTreeCtrl, public myDIEventHandler  {
	struct mxIEWAux {
		DebuggerInspection *di;
		wxTreeItemId item;
		bool is_root, is_open;
		mxIEWAux(DebuggerInspection *_di, const wxTreeItemId &_item, bool _root=false):di(_di), item(_item), is_root(_root), is_open(false) {}
		mxIEWAux(DebuggerInspection *_di=NULL):di(_di) {} // ctor solo para poder usar el Find de SingleList con un DebuggerInspection*
		mxIEWAux(const wxTreeItemId &_item):di(NULL),item(_item){} // ctor solo para poder usar el Find de SingleList con un wxTreeItemId
		bool operator==(const mxIEWAux &o) const { return o.di?(di==o.di):(item==o.item); } // para el Find de SingleList
		wxString MakeItemLabel() {
			wxString retval = di->GetShortExpression();
			if (!is_root && retval[0]=='.') retval.erase(0,1);
			if (!is_open) retval<<": "<<di->GetValue();
			return retval;
		}
		wxString MakeItemLabel(DEBUG_INSPECTION_MESSAGE err) { // para cuando hay errores
			wxString retval = di->GetShortExpression();
			retval<<": "<<DebuggerInspection::GetUserStatusText(err);
			return retval;
		}
	};
	
	bool hidden_root, auto_expand_roots;
	
	SingleList<mxIEWAux> inspections;
	int AddItem (wxTreeItemId *parent, DebuggerInspection *di, bool is_root=false);
public:
	
	mxInspectionExplorerWidget(wxWindow *parent, const wxString &expression="", bool frameless=false, bool expand_roots=true);
	~mxInspectionExplorerWidget();
	void AddExpression(wxString expression, bool frameless);
	
	class EventListener {
	public:
		virtual void OnSize()=0;
		virtual void OnType(const wxString &type)=0;
		virtual ~EventListener(){}
	} * event_listener;
	void SetEventListener(EventListener *s);
	
	/// Cuando una inspeccion deja de ser valida, esta función elimina las hijas del arbol y del sistema de depuracion
	void DeleteChildrenInspections(int pos, bool destroy_root=false);
	
	// eventos generados por DebuggerInspection
	void OnDICreated(DebuggerInspection *di);
	void OnDIError(DebuggerInspection *di);
	void OnDIValueChanged(DebuggerInspection *di);
	void OnDIOutOfScope(DebuggerInspection *di);
	void OnDIInScope(DebuggerInspection *di);
	void OnDINewType(DebuggerInspection *di);
	
	// eventos generados por el wxTreeCtrl
	void OnItemExpanding(wxTreeEvent &event);
	void OnItemExpanded(wxTreeEvent &event);
	void OnItemTooltip(wxTreeEvent &event);
	void OnItemPopup(wxTreeEvent &event);
	
	// popup menu
	void OnAddToInspectionsGrid(wxCommandEvent &evt);
	void OnShowInText(wxCommandEvent &evt);
	void OnShowInTable(wxCommandEvent &evt);
	void OnExploreExpression(wxCommandEvent &evt);
	void OnShowInHistory(wxCommandEvent &evt);
	void OnShowInRTEditor(wxCommandEvent &evt);
	void OnSetWatch(wxCommandEvent &evt);
	void OnCopyValue(wxCommandEvent &evt);
	void OnCopyType(wxCommandEvent &evt);
	void OnCopyExpression(wxCommandEvent &evt);
	
	wxString GetRootType();
	
	wxSize GetFullSize();
	
	DECLARE_EVENT_TABLE();
};

#endif

