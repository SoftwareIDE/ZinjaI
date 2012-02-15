#ifndef MXDIFFSIDEBAR_H
#define MXDIFFSIDEBAR_H
#include <wx/window.h>
#include <wx/pen.h>

class mxSource;

class mxDiffSideBar : public wxWindow {
	wxPen pen_del,pen_add,pen_mod;
	int last_h;
public:
	mxDiffSideBar();
	void OnPaint(wxPaintEvent &event);
	void OnMouseDown(wxMouseEvent &evt);
	void OnMouseWheel(wxMouseEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif

