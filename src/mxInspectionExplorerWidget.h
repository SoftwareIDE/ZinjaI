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
			wxString retval = di->GetExpression();
			if (!is_root && retval[0]=='.') retval.erase(0,1);
			if (!is_open) retval<<": "<<di->GetValue();
			return retval;
		}
	};
	

	
	SingleList<mxIEWAux> inspections;
	int AddItem (wxTreeItemId *parent, DebuggerInspection *di, bool is_root=false);
public:
	
	mxInspectionExplorerWidget(wxWindow *parent, const wxString &expression="", bool frameless=false);
	~mxInspectionExplorerWidget();
	void AddExpression(wxString expression, bool frameless);
	
	class EventListener {
	public:
		virtual void OnSize()=0;
		virtual void OnType(const wxString &type)=0;
	} * event_listener;
	void SetEventListener(EventListener *s);
//	void Expand(int pos);
	
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
	
	wxString GetRootType();
	
	DECLARE_EVENT_TABLE();
};

#endif

