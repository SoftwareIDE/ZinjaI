#include <wx/checkbox.h>
#include <wx/sizer.h>
#include "mxGdbAsmPanel.h"
#include "Cpp11.h"
#include "mxSizers.h"
#include "mxColoursEditor.h"
#include "mxStyledOutput.h"

BEGIN_EVENT_TABLE(mxGdbAsmPanel,wxPanel)
	EVT_CHECKBOX(wxID_ANY,mxGdbAsmPanel::OnCheckStepMove)
END_EVENT_TABLE()

mxGdbAsmPanel::mxGdbAsmPanel (wxWindow * parent) : wxPanel(parent) {
#ifdef __WIN32__
	SetBackgroundColour(wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ));
#endif
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	m_code = new mxStyledOutput(this,true,false);
	m_code->MarkerDefine(m_code->next_available_marker,wxSTC_MARK_SHORTARROW, g_ctheme->DEFAULT_FORE, "Z GREEN");
	m_code->SetZoom(-1);
	sizer->Add(m_code,sizers->Exp1);
	m_asm_step_mode = new wxCheckBox(this,wxID_ANY,LANG(GDBASM_STEP_MODE,"Avanzar paso a paso por instrucción de máquina (para Step In y Step Over)"));
	sizer->Add(m_asm_step_mode,sizers->Exp0);
	SetSizer(sizer);
	Update();
}

void mxGdbAsmPanel::Update ( ) {
	m_asm_step_mode->SetValue(debug->IsAsmStepModeOn());
	if (!debug->CanTalkToGDB()) return;
	m_code->Clear(); 
	wxString ans = debug->SendCommand("disassemble /m");
	int current_line = 0;
	do {
		wxString line = ans.BeforeFirst('\n');
		ans = ans.AfterFirst('\n');
		if (line.StartsWith("~")) {
			line = mxUT::UnEscapeString(line.Mid(1));
			if (line.Last()=='\n') line.RemoveLast();
			if (line.IsEmpty()) continue;
			if (line.StartsWith("=>")) { 
				current_line = m_code->GetLineCount()-1;
				line[0]=' '; line[1]=' '; 
			}
			bool inner = line.StartsWith("   0x");
			m_code->AppendLine(line,!inner);
		}
	} while(ans.size());
	if (current_line) {
		m_code->MarkerAdd(current_line,m_code->next_available_marker);
		m_code->EnsureVisibleEnforcePolicy(current_line);
	}
}

void mxGdbAsmPanel::OnCheckStepMove (wxCommandEvent & evt) {
	debug->SetStepMode(m_asm_step_mode->GetValue());
}
