#include <wx/font.h>
#include <wx/dc.h>
#include "mxOSD.h"
#include "mxMainWindow.h"
#include "ids.h"
#include "mxUtils.h"
#include "execution_workaround.h"

BEGIN_EVENT_TABLE(mxOSD, wxDialog)
	EVT_PAINT (mxOSD::OnPaint)
	EVT_BUTTON(wxID_CANCEL,mxOSD::OnCancel)
	EVT_TIMER(mxID_TIMER_OSD, mxOSD::OnTimer)
	EVT_SIZE(mxOSD::OnResize) // el resize por default me cambia el tamaño del cancel button
END_EVENT_TABLE()
	
wxFont *mxOSD::font = nullptr;
wxColour *mxOSD::cb = nullptr;
wxColour *mxOSD::cf = nullptr;
wxBrush *mxOSD::br = nullptr;
mxOSD *mxOSD::current_osd = nullptr;

mxOSD::mxOSD(wxWindow *aparent, wxString str, int time, bool corner, GenericAction *aon_cancel) 
	: wxDialog( aparent?aparent:main_window,wxID_ANY,"",wxPoint(200,200),wxSize(400,100),
	            (aparent?wxFRAME_FLOAT_ON_PARENT:wxSTAY_ON_TOP)|wxNO_3D|wxNO_BORDER ),
	  on_cancel(aon_cancel), timer(nullptr), parent(aparent)
{
	cancel_button = on_cancel ? new wxButton(this,wxID_CANCEL,LANG(GENERAL_CANCEL_BUTTON,"Cancelar")) : nullptr;
	if (!font) {
		font = new wxFont(12,wxFONTFAMILY_TELETYPE,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false);
		cb = new wxColour("Z LIGHT YELLOW");
		cf = new wxColour("RED");
		br = new wxBrush(*mxOSD::cb);
	}
	if (str.Len()) ShowText(str,time,corner);
	current_osd = this;
}

mxOSD::~mxOSD() {
	if (timer) delete timer;
	if (current_osd==this) current_osd = nullptr;
}

void mxOSD::ShowText(wxString str, int time, bool corner) {
	this->corner=corner;
	
	if (timer && timer->IsRunning()) {
		timer->Stop();
	}
	text=str;
	
	wxClientDC dc(this);
	PrepareDC(dc);
	
	dc.SetFont(*font);
	
	int margin=corner?8:15;
	wxSize s = dc.GetTextExtent(text);
	int tx = s.GetWidth() + margin+margin;
	int ty = s.GetHeight() + margin+margin;
	if (cancel_button) {
		wxSize cbsz = cancel_button->GetSize();
		cancel_button->SetSize(cbsz);
		int bw = cbsz.GetWidth()+margin*2;
		if (bw>tx) tx=bw;
		ty += cbsz.GetHeight()+margin;
	}
	int cx,cy;
	if (!corner) {
		cx = parent?(parent->GetSize().GetWidth()/2+parent->GetPosition().x):100;
		cy = parent?(parent->GetSize().GetHeight()/2+parent->GetPosition().y):100;
		SetSize(cx-tx/2,cy-ty/2,tx,ty);
	} else {
		cx = parent?(parent->GetSize().GetWidth()-tx-margin*2+parent->GetPosition().x):100;
		cy = parent?(parent->GetSize().GetHeight()-ty-margin*2+parent->GetPosition().y):100;
		SetSize(cx,cy,tx,ty);
	}
	
	if (cancel_button) {
		wxSize cbsz = cancel_button->GetSize();
		cancel_button->Move((tx-cbsz.GetWidth())/2,ty-cbsz.GetHeight()-margin);
	}
	
	if (parent) parent->Raise();
	if (time){
		if (!timer)
			timer = new wxTimer(GetEventHandler(),mxID_TIMER_OSD);
		timer->Start(time,true);
	}
	Show(); Refresh(); 
DEBUG_INFO("wxYield:in  mxOSD::ShowText");
	wxYield();
DEBUG_INFO("wxYield:out mxOSD::ShowText");
}

void mxOSD::OnPaint (wxPaintEvent &event) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.SetFont(*font);
	dc.SetBackground(*br);
	dc.SetTextBackground(*cb);
	dc.Clear();
	dc.SetTextForeground(*cf);
	int margin=corner?8:15;
	dc.DrawText(text,margin,margin);
}

void mxOSD::OnTimer(wxTimerEvent &evt) {
	Hide();
}

void mxOSD::OnCancel(wxCommandEvent & evt) {
	on_cancel->Run();
}

void mxOSD::OnResize (wxSizeEvent & evt) {
	
}


void mxOSD::Execute(wxString command, wxString message, GenericActionEx<int> *aon_end) {
	
	class mxOSDProcess : public wxProcess {
	public:
		bool ended;
		bool cancelled; ///< when trying to cancel, first click uses sigint, thenext one sigkill
		int pid;
		mxOSD *osd_win;
		GenericActionEx<int> *on_end;
		mxOSDProcess(GenericActionEx<int> *aon_end) 
			: ended(false), cancelled(false), osd_win(nullptr), on_end(aon_end) {}
		void OnTerminate(int pid, int status) { 
			ended=true;
			if (osd_win) { osd_win->Destroy(); osd_win=nullptr; }
			if (on_end && !cancelled) on_end->Do(status);
			delete on_end; 
		}
	};
	
	mxOSDProcess *osd_proc = new mxOSDProcess(aon_end);
	osd_proc->Redirect();
	osd_proc->pid = mxExecute(command, /*wxEXEC_NODISABLE|*/wxEXEC_ASYNC|wxEXEC_MAKE_GROUP_LEADER,osd_proc); // el wxEXEC_MAKE_GROUP_LEADER es para que funcione el wxKILL_CHILDREN en linux
	if (!osd_proc->pid) { delete osd_proc; aon_end->Do(-1); delete aon_end; return; } // si no se lanzó
	
	osd_proc->Detach();
	_LAMBDA_1( lmbKillProcess, mxOSDProcess*,proc, {
		wxProcess::Kill(proc->pid,wxSIGKILL,wxKILL_CHILDREN); proc->cancelled=true; 
	} );
	osd_proc->osd_win = new mxOSD(main_window,message,0,false,new lmbKillProcess(osd_proc));
	if (osd_proc->ended && osd_proc->osd_win) // can happen, some yield hidden over there called from mxOSD constructor
		osd_proc->osd_win->Destroy();
	
}

