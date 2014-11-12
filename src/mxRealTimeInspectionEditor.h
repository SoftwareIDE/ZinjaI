#ifndef MXREALTIMEINSPECTIONEDITOR_H
#define MXREALTIMEINSPECTIONEDITOR_H
#include <wx/frame.h>
#include "Inspection.h"

class wxTextCtrl;
class wxButton;
class wxFlexGridSizer;
class wxStaticText;

class mxRealTimeInspectionEditor : public wxFrame, public myDIEventHandler, public myDIGlobalEventHandler {
	bool mask_events;
	wxFlexGridSizer *sizer;
	struct AuxRTIE {
		DebuggerInspection *di;
		int level;
		wxStaticText *label;
		wxTextCtrl *text;
		wxButton *button;
	};
	SingleList<AuxRTIE> inspections;
public:
	mxRealTimeInspectionEditor(const wxString &inspection);
	~mxRealTimeInspectionEditor();
	
	void Add(int pos, int lev, DebuggerInspection *di);
	void Break(int num);
	void Resize();
	
	// eventos myDIGlobalEventHandler
	void OnDebugStop() { Destroy(); }
	// eventos myDIEventHandler
	void OnDIError(DebuggerInspection *di);
	void OnDIValueChanged(DebuggerInspection *di);
	void OnDIOutOfScope(DebuggerInspection *di);
	
	// eventos propios
//	void OnUpdateValues(wxCommandEvent &evt);
	void OnButton(wxCommandEvent &evt);
	void OnText(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

