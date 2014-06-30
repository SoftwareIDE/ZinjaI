#include <wx/font.h>
#include <wx/dc.h>
#include "mxOSD.h"
#include "mxMainWindow.h"
#include "ids.h"
#include "mxUtils.h"

BEGIN_EVENT_TABLE(mxOSD, wxDialog)
	EVT_PAINT (mxOSD::OnPaint)
	EVT_TIMER(mxID_TIMER_OSD, mxOSD::OnTimer)
END_EVENT_TABLE()
	
wxFont *mxOSD::font = NULL;
wxColour *mxOSD::cb = NULL;
wxColour *mxOSD::cf = NULL;
wxBrush *mxOSD::br = NULL;

mxOSD::mxOSD(wxWindow *aparent, wxString str, int time, bool corner) : wxDialog(aparent?aparent:main_window,wxID_ANY,"",wxPoint(200,200),wxSize(400,100),(aparent?wxFRAME_FLOAT_ON_PARENT:wxSTAY_ON_TOP)|wxNO_3D|wxNO_BORDER) {
	if (!font) {
		font = new wxFont(12,wxFONTFAMILY_TELETYPE,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false);
		cb = new wxColour("Z LIGHT YELLOW");
		cf = new wxColour("RED");
		br = new wxBrush(*mxOSD::cb);
	}
	
	parent = aparent;
	timer = NULL;
	if (str.Len()) ShowText(str,time,corner);
}

mxOSD::~mxOSD() {
	if (timer) delete timer;
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
