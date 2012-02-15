#ifndef MXOSD_H
#define MXOSD_H
#include <wx/dialog.h>
#include <wx/timer.h>
#include "mxMainWindow.h"
	
class wxFont;
class wxColour;
class wxBrush;

/**
* @brief Clase para mostrar mensajes en pantalla tipo OSD
* 
* Esta clase se desarrolló inicialmente para mostrar mensajes de estado durante la
* depuración, pero no se utiliza.
**/
class mxOSD : public wxDialog {
private:
	wxTimer *timer;
	wxString text;
	wxWindow *parent;
	static wxFont *font;
	static wxColour *cb,*cf;
	static wxBrush *br;
	bool corner;
public:
	mxOSD(wxWindow *aparent = main_window, wxString str=_T(""), int time=0, bool coner=false);
	~mxOSD();
	void ShowText(wxString str, int time=0, bool corner=false);
	void OnTimer(wxTimerEvent &evt);
	void OnPaint (wxPaintEvent &event);
	DECLARE_EVENT_TABLE();
};

#endif

