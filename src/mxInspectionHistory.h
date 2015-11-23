#ifndef MXINSPECTIONHISTORY_H
#define MXINSPECTIONHISTORY_H
#include <wx/listbox.h>
#include "Inspection.h"

class mxInspectionHistory : public wxListBox, public myDIEventHandler, public myDIGlobalEventHandler {
private:
	DebuggerInspection *di;
	wxString last_value;
	enum { LM_Never, LM_Change, LM_Always };
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
	void OnDebugPausePost() override;
	DECLARE_EVENT_TABLE();
};

#endif

