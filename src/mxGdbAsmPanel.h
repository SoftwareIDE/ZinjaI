#ifndef MXGDBASMPANEL_H
#define MXGDBASMPANEL_H
#include <wx/panel.h>
#include "Inspection.h"

class mxStyledOutput;
class wxCheckBox;

class mxGdbAsmPanel : public wxPanel, public myDIGlobalEventHandler {
	mxStyledOutput *m_code;
	wxCheckBox *m_asm_step_mode;
public:
	mxGdbAsmPanel(wxWindow *parent);
	void Update();
	void OnDebugPausePost() override { Update(); }
	void OnCheckStepMove(wxCommandEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

