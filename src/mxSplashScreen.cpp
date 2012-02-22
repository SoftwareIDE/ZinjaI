#include <wx/dcclient.h>
#include <wx/clrpicker.h>
#include <wx/timer.h>
#include <wx/dcmemory.h>

#include "mxSplashScreen.h"

#include "mxApplication.h"
#include "mxUtils.h"

#include<iostream>
using namespace std;

mxSplashScreen *splash;

BEGIN_EVENT_TABLE(mxSplashScreen, wxFrame)
	EVT_PAINT(mxSplashScreen::OnPaint)
	EVT_TIMER(wxID_ANY,mxSplashScreen::OnTimer)
	EVT_LEFT_DOWN(mxSplashScreen::OnMouse)
	EVT_CLOSE(mxSplashScreen::OnClose)
#ifdef __WXGTK__
	EVT_WINDOW_CREATE(mxSplashScreen::OnWindowCreate)
#endif
END_EVENT_TABLE()

mxSplashScreen::mxSplashScreen(wxString image_path):wxFrame(NULL,wxID_ANY,_T("Cargando ZinjaI..."),wxDefaultPosition,wxDefaultSize, wxNO_BORDER | wxSTAY_ON_TOP ) {
	SHOW_MILLIS("Loading splash...");
	splash=this;
	m_bmp = wxBitmap(image_path,wxBITMAP_TYPE_PNG);
	w = m_bmp.GetWidth(); h = m_bmp.GetHeight();
//	m_cpu = wxBitmap(DIR_PLUS_FILE("imgs","splash_cpu.png"),wxBITMAP_TYPE_PNG);
//	cpu_w=m_cpu.GetWidth(); cpu_h=m_cpu.GetHeight();
//	cpu_count=wxThread::GetCPUCount(); if (cpu_count<0) cpu_count=1;
	SetSize(w,h);
	CentreOnScreen();
	Show(true);
	Refresh();
	wxYield();
	timer = new wxTimer(GetEventHandler(),wxID_ANY);
	timer->Start(2500,true);
	timer_done=should_close=false;
}

mxSplashScreen::mxSplashScreen(const char *xpm[], int x, int y):wxFrame(NULL,wxID_ANY,_T(""),wxPoint(x,y),wxDefaultSize, wxFRAME_SHAPED | wxNO_BORDER | wxSTAY_ON_TOP | wxFRAME_NO_TASKBAR ) {
	m_bmp = wxBitmap(xpm);
	SetSize(wxSize(m_bmp.GetWidth(), m_bmp.GetHeight()));
#ifndef __WXGTK__
	wxRegion region(m_bmp, wxColour(0,0,0,0));
	SetShape(region);
#endif
	should_close=true;
	timer=NULL;
	Show(true);
	Refresh();
	wxYield();
}

mxSplashScreen::~mxSplashScreen() {
	
}

void mxSplashScreen::OnPaint(wxPaintEvent& WXUNUSED(evt)) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.DrawBitmap(m_bmp, 0, 0, true);
//	if (splash==this) {
//		for (int i=0;i<cpu_count;i++)
//			dc.DrawBitmap(m_cpu,w-cpu_w*(i+1),h-cpu_h,true);
//	}
}

void mxSplashScreen::OnWindowCreate(wxWindowCreateEvent& WXUNUSED(evt)) {
	wxRegion region(m_bmp, wxColour(0,0,0,0));
	SetShape(region);
}

void mxSplashScreen::OnTimer(wxTimerEvent &evt) {
	if (should_close)
		Close();
	else timer_done=true;
}

void mxSplashScreen::ShouldClose() {
	if (timer_done)
		Close();
	else
		should_close=true;
}

void mxSplashScreen::OnMouse(wxMouseEvent &evt) {
	Hide();
}

void mxSplashScreen::OnClose(wxCloseEvent &evt) {
	if (timer) timer->Stop();
	if (this==splash) splash=NULL;
	Destroy();
}
