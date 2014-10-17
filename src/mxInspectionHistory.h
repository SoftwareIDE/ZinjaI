#ifndef MXINSPECTIONHISTORY_H
#define MXINSPECTIONHISTORY_H
#include <wx/listbox.h>
//#include <wx/panel.h>
#include "Inspection.h"

//class wxListBox;

class mxInspectionHistory : public wxListBox, public myDIEventHandler, public myDIGlobalEventHandler {
private:
	wxListBox *lista;
	DebuggerInspection *di;
	wxString last_value;
	int log_mode;
public:
	mxInspectionHistory(wxString expression, bool is_frameless);
	~mxInspectionHistory();
	void OnNewValue(const wxString &new_value, bool force=false);
	void OnPopupMenu(wxMouseEvent &evt);
	void OnLogNone(wxCommandEvent &evt);
	void OnLogChange(wxCommandEvent &evt);
	void OnLogAll(wxCommandEvent &evt);
	void OnClearLog(wxCommandEvent &evt);
	void OnCopy(wxCommandEvent &evt);
	void OnSelectAll(wxCommandEvent &evt);
	// eventos de una instancia individual de DebuggerInspection
	void OnDICreated(DebuggerInspection *di);
	void OnDIError(DebuggerInspection *di);
	void OnDIValueChanged(DebuggerInspection *di);
	void OnDIOutOfScope(DebuggerInspection *di);
	void OnDIInScope(DebuggerInspection *di);
	void OnDINewType(DebuggerInspection *di);
	// eventos globales de DebuggerInspection
	void OnDebugPausePost() /*override*/;
	DECLARE_EVENT_TABLE();
};

#endif

