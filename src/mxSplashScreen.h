#ifndef MXSPLASHSCREEN_H
#define MXSPLASHSCREEN_H
#include <wx/frame.h>
#include <wx/bitmap.h>
#include <wx/timer.h>

#if defined(DEBUG) || defined(__APPLE__)
#define SPLASH_FILE "splashvb.png"
#else
//#warning SHOULD DEFINE CORRECT SPLASH BEFORE RELEASE
#define SPLASH_FILE "splashv2g.png"
#endif

/**
* @brief Ventana sin marco y siempre visible para mostrar un png como splash
**/
class mxSplashScreen : public wxFrame {
private:
	wxBitmap m_bmp;
//	wxBitmap m_cpu;
	bool should_close,timer_done;
	wxTimer *timer;
	int cpu_count, cpu_w, cpu_h, w, h;
public:
	mxSplashScreen(wxString image_pah);
	mxSplashScreen(const char *xpm[], int x, int y);
	void OnPaint(wxPaintEvent& WXUNUSED(evt));
	void OnWindowCreate(wxWindowCreateEvent& WXUNUSED(evt));
	void ShouldClose();
	void OnMouse(wxMouseEvent &evt);
	void OnTimer(wxTimerEvent &evt); 
	void OnClose(wxCloseEvent &evt); 
	~mxSplashScreen();
	DECLARE_EVENT_TABLE();
};

extern mxSplashScreen *splash;

#endif

