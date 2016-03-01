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
	
static bool dissasembly_flavor_setted = false;

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
	
	if (!dissasembly_flavor_setted) {
		debug->SendCommand("set dissasembly-flavor intel");
		dissasembly_flavor_setted = true;
	}
	
	wxString pc = debug->InspectExpression("$pc").BeforeFirst(' ');
	AddressRange::addr_t addr = AddressRange::Parse(pc);
	if (m_current_range.Contains(addr)) { // si la función ya esta siendo mostrada
		if (m_marker_handle!=-1) 
			m_code->MarkerDeleteHandle(m_marker_handle);
		m_marker_handle = -1;
	} else {
		map<AddressRange,wxString>::iterator it = m_cache.find(AddressRange(addr));
		if (it==m_cache.end()) { // si es la primera vez que se analiza esa función
			wxString ans = debug->SendCommand("disassemble /m");
			AddressRange range = ParseCode(ans);
			it = m_cache.insert(pair<AddressRange,wxString>(range,ans)).first;
		} else { // si ya estaba desensamblada
			ParseCode(it->second);
		}
	}
	map<AddressRange::addr_t,int>::iterator it = m_addr_to_line.find(addr);
	if (it!=m_addr_to_line.end()) {
		m_marker_handle = m_code->MarkerAdd(it->second,m_code->next_available_marker);
		m_code->EnsureVisibleEnforcePolicy(it->second);
	}
}

void mxGdbAsmPanel::OnCheckStepMove (wxCommandEvent & evt) {
	debug->SetStepMode(m_asm_step_mode->GetValue());
}

mxGdbAsmPanel::AddressRange mxGdbAsmPanel::ParseCode (wxString ans) {
	m_code->Clear(); 
	m_marker_handle = -1;
	m_addr_to_line.clear();
	AddressRange range;
	do {
		wxString line = ans.BeforeFirst('\n');
		ans = ans.AfterFirst('\n');
		if (line.StartsWith("~")) {
			// arreglar el formato
			line = mxUT::UnEscapeString(line.Mid(1));
			if (line.Last()=='\n') line.RemoveLast();
			// evitar lineas en blanco
			if (line.IsEmpty()) continue;
			// quitar el indicador de la instruccion actual
			if (line.StartsWith("=>")) { line[0]=' '; line[1]=' '; }
			// determinar si es linea de código máquina, 
			bool inner = false;
			int i1=0, l=line.Len();
			while(i1<l&&line[i1]==' ')++i1;
			if (i1+2<l && line[i1]=='0' && line[i1+1]=='x') {
				// y extrar la dirección para definir el rango de direcciones de la función
				int i2 = i1; inner = true;
				while(i2<l&&line[i2]!=' ')++i2;
				range.m_to = AddressRange::Parse(line.Mid(i1,i2-i1));
				m_addr_to_line[range.m_to] = m_code->GetLineCount()-1;
				if (range.m_from==-1) range.m_from = range.m_to;
			}
			// mostrar la linea
			m_code->AppendLine(line,!inner);
		}
	} while(ans.size());
	return range;
}

void mxGdbAsmPanel::OnDebugStart ( ) {
	m_cache.clear(); m_addr_to_line.clear(); dissasembly_flavor_setted = false;
}

