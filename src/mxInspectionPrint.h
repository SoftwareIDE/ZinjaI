#ifndef MXINSPECTIONPRINT_H
#define MXINSPECTIONPRINT_H
#include "Inspection.h"

class wxTextCtrl;
class wxStaticText;

class mxInspectionPrint : public wxPanel, public myDIEventHandler/*, public myDIGlobalEventHandler*/ {
	wxTextCtrl *value;
	wxStaticText *type;
	bool is_macro;
	DebuggerInspection *di;
public:
	mxInspectionPrint(wxString expression, bool frameless);
	// eventos globales de DebuggerInspection
//	void OnDebugPausePre();
	// eventos de una instancia individual de DebuggerInspection
	void OnDICreated(DebuggerInspection *di);
	void OnDIError(DebuggerInspection *di);
	void OnDIValueChanged(DebuggerInspection *di);
	void OnDIOutOfScope(DebuggerInspection *di);
	void OnDIInScope(DebuggerInspection *di);
	void OnDINewType(DebuggerInspection *di);
//	DECLARE_EVENT_TABLE();
};

#endif

