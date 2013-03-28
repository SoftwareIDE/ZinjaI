#ifndef MXGCOVSIDEBAR_H
#define MXGCOVSIDEBAR_H
#include <wx/panel.h>

class mxSource;

class mxGCovSideBar : public wxWindow {
	int line_count, *hits, hits_max;
	mxSource *the_source;
	wxString last_path;
	bool LoadData(mxSource *src);
	mxSource *should_refresh;
	void MyRefresh(mxSource *src);
public:
	mxGCovSideBar(wxWindow *parent);
	void OnPaint(wxPaintEvent &event);
	void ShouldRefresh(mxSource *src);
	DECLARE_EVENT_TABLE();
};

#endif

