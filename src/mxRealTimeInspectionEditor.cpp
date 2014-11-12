#include "mxRealTimeInspectionEditor.h"
#include "mxMessageDialog.h"
#include "mxMainWindow.h"
#include <wx/textctrl.h>
#include "mxSizers.h"
#include <wx/sizer.h>
#include "mxArt.h"

BEGIN_EVENT_TABLE(mxRealTimeInspectionEditor,wxFrame)
	EVT_CLOSE(mxRealTimeInspectionEditor::OnClose)
	EVT_BUTTON(wxID_ANY,mxRealTimeInspectionEditor::OnButton)
	EVT_TEXT_ENTER(wxID_ANY,mxRealTimeInspectionEditor::OnText)
END_EVENT_TABLE()

mxRealTimeInspectionEditor::mxRealTimeInspectionEditor(const wxString &expression)
	: wxFrame(main_window,wxID_ANY,expression,wxGetMousePosition()-wxPoint(25,10),wxDefaultSize,wxDEFAULT_FRAME_STYLE|wxSTAY_ON_TOP) 
{
	SetBackgroundColour(wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ));
	DebuggerInspection *di; mask_events=false;
	(di = DebuggerInspection::Create(expression,DIF_DONT_USE_HELPER,this,false))->Init();
	if (!di->GetDbiType()==DIT_VARIABLE_OBJECT) {
		mxMessageDialog(main_window,LANG(REALTIMEINSP_ERROR_CREATING_INSPECTION,"Ha ocurrido un error al registrar la inspección"),LANG(GENERAL_ERROR,"Error"),mxMD_ERROR|mxMD_OK).ShowModal();
		Destroy(); return;
	}
	
	sizer = new wxFlexGridSizer(2); 
	sizer->AddGrowableCol(1);
	sizer->SetFlexibleDirection(wxBOTH);
	sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);
	
	Add(0,0,di); if (inspections[0].button) Break(0);
	SetSizer(sizer); sizer->Layout(); Resize(false);
	Show();
}

mxRealTimeInspectionEditor::~mxRealTimeInspectionEditor() {
	for(int i=0;i<inspections.GetSize();i++)
		inspections[i].di->Destroy();
	debug->InvalidatePauseEvent(this);
}

void mxRealTimeInspectionEditor::Break (int num) {
	if (!debug->IsPaused()) {
		class OnPauseBreak : public DebugManager::OnPauseAction {
			mxRealTimeInspectionEditor *win;
			int num;
		public:
			OnPauseBreak(mxRealTimeInspectionEditor *w, int n):win(w),num(n) {}
			void Do() /*override*/ { win->Break(num); }
			bool Invalidate(void *ptr) /*override*/ { return ptr==win; }
		};
		debug->PauseFor(new OnPauseBreak(this,num));
		return;
	}
	DebuggerInspection *di = inspections[num].di;
	SingleList<DebuggerInspection*> children;
	if (!di->Break(children,true,true) || children.GetSize()==0) return;
	if (inspections[num].button) inspections[num].button->Enable(false);
	for(int i=0;i<children.GetSize();i++) { 
		Add(num+i+1,inspections[num].level+1,children[i]);
	}
	Layout();
}

void mxRealTimeInspectionEditor::OnClose (wxCloseEvent & evt) {
	Destroy();
}

void mxRealTimeInspectionEditor::Add (int pos, int lev, DebuggerInspection * di) {
	mask_events = true;
	AuxRTIE aux; aux.di=di; wxString tabs(' ',2*lev); aux.level=lev;
	aux.label = new wxStaticText(this,wxID_ANY,tabs+di->GetShortExpression()+": ");
	sizer->Insert(2*pos,aux.label,0,wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	wxControl *aux_control;
	if (di->AskGDBIfIsEditable()) {
		aux.button = NULL; 
		aux_control = aux.text = new wxTextCtrl(this,wxID_ANY,di->GetValue(),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	} else {
		aux.text= NULL; 
		aux_control = aux.button = new wxButton(this,wxID_ANY,di->GetValue(),wxDefaultPosition,wxDefaultSize,wxNO_BORDER|wxBU_EXACTFIT);
	}
	
	if (pos==0) {
		wxBoxSizer *aux_sizer = new wxBoxSizer(wxHORIZONTAL);
		aux_sizer->Add(aux_control,sizers->Exp1);
		aux_sizer->Add(new wxBitmapButton(this,wxID_REDO,wxBitmap(SKIN_FILE("boton_recargar.png"),wxBITMAP_TYPE_PNG)));
		sizer->Insert(2*pos+1,aux_sizer,sizers->Exp1);
	} else 
		sizer->Insert(2*pos+1,aux_control,sizers->Exp1);
	
	inspections.Insert(pos,aux);
	mask_events = false;
}

void mxRealTimeInspectionEditor::OnButton (wxCommandEvent & evt) {
	if (evt.GetId()==wxID_REDO) { OnUpdateValues(evt); return; }
	for(int i=0;i<inspections.GetSize();i++) { 
		if (evt.GetEventObject()==inspections[i].button) { Break(i); Resize(true); return; }
	}
}

void mxRealTimeInspectionEditor::OnText (wxCommandEvent & evt) {
	if (mask_events) return;
	if (!debug->IsDebugging()) return;
	for(int i=0;i<inspections.GetSize();i++) {
		if (evt.GetEventObject()==inspections[i].text) {
			mask_events = true;
			inspections[i].di->ModifyValue(inspections[i].text->GetValue());
//			if (debug->IsPaused()) {
//				if (!
//					inspections[i].text->SetValue(inspections[i].di->GetValue());
//			} else {
//#warning TODO
//			}
			mask_events = false;
			return;
		}
	}
}

void mxRealTimeInspectionEditor::Resize(bool only_grow_h) {
	wxSize old_size = GetSize();
	Fit(); int w=0,h=GetSize().GetHeight(); 
	
	if  (only_grow_h) {
		if (h>old_size.GetHeight()) {
			SetSize(ClientToWindowSize(wxSize(old_size.GetWidth(),h)));
		}
	} else {
		
		for(int i=0;i<inspections.GetSize();i++) { 
			wxSize sz = inspections[i].label->GetSize();
			if (sz.GetWidth()>w) w=sz.GetWidth();
		}
		SetSize(ClientToWindowSize(wxSize(w+100,h)));
	}
}

void mxRealTimeInspectionEditor::OnDIError (DebuggerInspection * di) {
	for(int i=0;i<inspections.GetSize();i++) { 
		if (inspections[i].di==di) {
			if (inspections[i].button) inspections[i].button->Enable(false);
			if (inspections[i].text) inspections[i].text->Enable(false);
			return;
		}
	}
}

void mxRealTimeInspectionEditor::OnDIValueChanged (DebuggerInspection * di) {
	for(int i=0;i<inspections.GetSize();i++) { 
		if (inspections[i].di==di) {
			if (inspections[i].button) inspections[i].button->SetLabel(di->GetValue());
			if (inspections[i].text) inspections[i].text->SetValue(di->GetValue());
			return;
		}
	}
}

void mxRealTimeInspectionEditor::OnDIOutOfScope (DebuggerInspection * di) {
	OnDIError(di);
}


// none of this seems to work if gdb is stopped without a breakpoint (so it stops 
// but do not now where, evaluations gives old value or nothing)
void mxRealTimeInspectionEditor::OnUpdateValues (wxCommandEvent & evt) {
	if (!debug->IsPaused()) {
		class OnPauseUpdateRTIEditor : public DebugManager::OnPauseAction {
			mxRealTimeInspectionEditor *win;
		public:
			OnPauseUpdateRTIEditor(mxRealTimeInspectionEditor *w) : win(w) {}
			void Do() /*override*/ { wxCommandEvent evt; win->OnUpdateValues(evt); }
			bool Invalidate(void *p) /*override*/ { return win==p; }
		};
		debug->PauseFor(new OnPauseUpdateRTIEditor(this));
	} else {
		inspections[0].di->ForceVOUpdate();
	}
}

