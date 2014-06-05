#include <wx/textctrl.h>
#include <wx/sizer.h>
#include "mxGdbCommandsPanel.h"
#include "mxMainWindow.h"
#include "DebugManager.h"
#include "mxUtils.h"
#include "mxSizers.h"
#include "mxMessageDialog.h"

BEGIN_EVENT_TABLE(mxGdbCommandsPanel,wxPanel)
	EVT_TEXT_ENTER(wxID_FIND,mxGdbCommandsPanel::OnInput)
END_EVENT_TABLE()

mxGdbCommandsPanel::mxGdbCommandsPanel():wxPanel(main_window) {
	input = new wxTextCtrl(this,wxID_FIND,"",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	output = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(input,sizers->Exp0);
	sizer->Add(output,sizers->Exp1);
	SetSizer(sizer);
	SetSize(wxSize(400,200));
	input->SetFocus();
}


void mxGdbCommandsPanel::OnInput (wxCommandEvent & event) {
	if (!debug->debugging) { mxMessageDialog("No puede enviar comandos mientras no haya una sesión de depuración en progreso.","Error",mxMD_ERROR).ShowModal(); return; }
	if (debug->waiting) { mxMessageDialog("No puede enviar comandos sin antes interrumpir/pausar la ejecución.","Error",mxMD_ERROR).ShowModal(); return; }
	wxString cmd=input->GetValue(); input->SetSelection(0,cmd.Len());
	if (!cmd.Len()) return;
	if (input_history.GetCount() && input_history.Last()!=cmd) input_history.Add(cmd);
	AppendText(wxString("> ")+cmd+"\n");
	wxString ans = debug->SendCommand(cmd);
	if (ans.Len() && !ans.Last()=='\n') ans<<"\n";
	
	wxString msg;
	do {
		wxString line=ans.BeforeFirst('\n');
		ans=ans.AfterFirst('\n');
		if (line.StartsWith("~") || line.StartsWith("&")) {
			wxString ue=utils->UnEscapeString(line.Mid(1));
			if (ue.Len() && ue.Last()=='\n') ue.RemoveLast();
			msg<<"   "<<line[0]<<" "<<ue<<"\n";
		} else if (line.StartsWith("^")) {
			if (line.StartsWith("^error")) msg<<"^error: "<<debug->GetValueFromAns(ans,"msg",true,true)<<"\n";
			else msg<<"   ^ "<<line.Mid(1)<<"\n";
		} else if (line.StartsWith("=")) {
			msg<<"   = "<<line.Mid(1)<<"\n";
		} else if (line.StartsWith("&")) {
		} else // creo que no llega nunca aca, no hay caso no contemplado arriba, o si?
			msg<<"   "<<line.Mid(1)<<"\n";
	} while(ans.size());
	
	AppendText(msg);
}

void mxGdbCommandsPanel::AppendText (const wxString & str) {
	output->AppendText(str);
}

