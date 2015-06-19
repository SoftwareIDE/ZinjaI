#ifndef MXFLOWWINDOW_H
#define MXFLOWWINDOW_H

#include <wx/frame.h>
class mxSource;
class mxFlowCanvas;

class mxFlowWindow : public wxFrame {
private:
	mxSource *source;
	mxFlowCanvas *canvas;
public:
	mxFlowWindow(mxSource *s, wxString title);
	void OnCharHook(wxKeyEvent &evt);
	void OnClose(wxCloseEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

