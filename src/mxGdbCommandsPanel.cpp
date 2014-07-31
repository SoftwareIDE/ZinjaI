#include <wx/textctrl.h>
#include <wx/sizer.h>
#include "mxGdbCommandsPanel.h"
#include "mxMainWindow.h"
#include "DebugManager.h"
#include "mxUtils.h"
#include "mxSizers.h"
#include "mxMessageDialog.h"


class gdbTextCtrl:public wxTextCtrl {
	wxArrayString comp_options;
	unsigned int input_history_pos;
	wxArrayString input_history;
public:
	gdbTextCtrl(wxWindow *parent):wxTextCtrl(parent,wxID_FIND,"",wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER|wxTE_PROCESS_TAB) {
		input_history_pos=0;
	}
	void OnEnter(wxCommandEvent &event) {
		wxString cmd=GetValue(); if (cmd.IsEmpty()) return;
		if (!input_history.GetCount() || input_history.Last()!=cmd) input_history.Add(cmd);
		input_history_pos=input_history.GetCount();
		if (input_history_pos) --input_history_pos;
		event.Skip();
	}
	void OnChar(wxKeyEvent &evt) {
		if (evt.GetKeyCode()==WXK_TAB) {
			if (debug->CanTalkToGDB()) {
				comp_options.Clear();
				wxString ans = debug->SendCommand("complete ",GetValue());
				while (ans.Contains("\n")) {
					wxString line=ans.BeforeFirst('\n');
					ans=ans.AfterFirst('\n');
					if (line.StartsWith("~\"")) {
						wxString ue=mxUT::UnEscapeString(line.Mid(1));
						if (ue.Len() && ue.Last()=='\n') ue.RemoveLast();
						comp_options.Add(ue);
					}
				}
				if (!comp_options.GetCount()) return;
				if (comp_options.GetCount()==1) { SetText(comp_options[0]); return; }
				wxMenu menu("");
				for(unsigned int i=0;i<comp_options.GetCount();i++)
					menu.Append(wxID_HIGHEST+1000+i, comp_options[i]);
				wxPoint pos=GetPosition();
				pos.y+=GetSize().GetHeight();
				PopupMenu(&menu,pos);
				
			}
		} else if (evt.GetKeyCode()==WXK_UP) {
			if (input_history_pos>0) 
				SetText(input_history[--input_history_pos]);
		} else if (evt.GetKeyCode()==WXK_DOWN) {
			if (input_history_pos+1==input_history.GetCount())
				SetText("");
			else if (input_history_pos+1<input_history.GetCount()) 
				SetText(input_history[++input_history_pos]);
		} else {
			evt.Skip();
		}
	}
	void OnMenu(wxCommandEvent &evt) {
		SetText(comp_options[evt.GetId()-wxID_HIGHEST-1000]);
	}
	void SetText(const wxString &text) {
		SetValue(text); 
		SetSelection(text.Len(),text.Len());
	}
	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(gdbTextCtrl,wxTextCtrl)
	EVT_CHAR(gdbTextCtrl::OnChar)
	EVT_TEXT_ENTER(wxID_ANY,gdbTextCtrl::OnEnter)
	EVT_MENU_RANGE(wxID_HIGHEST+1000,wxID_HIGHEST+5000,gdbTextCtrl::OnMenu)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(mxGdbCommandsPanel,wxPanel)
	EVT_TEXT_ENTER(wxID_FIND,mxGdbCommandsPanel::OnInput)
END_EVENT_TABLE()

mxGdbCommandsPanel::mxGdbCommandsPanel():wxPanel(main_window) {
	input = new gdbTextCtrl(this);
	output = new wxTextCtrl(this,wxID_ANY,"",wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE|wxTE_READONLY);
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(input,sizers->Exp0);
	sizer->Add(output,sizers->Exp1);
	SetSizer(sizer);
	SetSize(wxSize(400,200));
	input->SetFocus();
}


void mxGdbCommandsPanel::OnInput (wxCommandEvent & event) {
	if (!debug->IsDebugging()) { mxMessageDialog("No puede enviar comandos mientras no haya una sesión de depuración en progreso.","Error",mxMD_ERROR).ShowModal(); return; }
	if (!debug->CanTalkToGDB()) { mxMessageDialog("No puede enviar comandos sin antes interrumpir/pausar la ejecución.","Error",mxMD_ERROR).ShowModal(); return; }
	wxString cmd=input->GetValue(); input->SetSelection(0,cmd.Len());
	if (!cmd.Len()) return;
	AppendText(wxString("> ")+cmd+"\n");
	wxString ans = debug->SendCommand(cmd);
	if (ans.Len() && !ans.Last()=='\n') ans<<"\n";
	
	wxString msg;
	do {
		wxString line=ans.BeforeFirst('\n');
		ans=ans.AfterFirst('\n');
		if (line.StartsWith("~") || line.StartsWith("&")) {
			wxString ue=mxUT::UnEscapeString(line.Mid(1));
			if (ue.Len() && ue.Last()=='\n') ue.RemoveLast();
			msg<<"   "<<line[0]<<" "<<ue<<"\n";
		} else if (line.StartsWith("^")) {
			if (line.StartsWith("^error")) msg<<"^error: "<<debug->GetValueFromAns(line,"msg",true,true)<<"\n";
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


