#ifndef MXGCOVSIDEBAR_H
#define MXGCOVSIDEBAR_H
#include <wx/panel.h>
#include <wx/timer.h>

class mxSource;

class mxGCovSideBar : public wxWindow {
	int line_count, *hits, hits_max;
	mxSource *the_source;
	wxString last_path;
	bool ShouldLoadData(mxSource *src);
	mxSource *should_refresh;
	mxSource *src_load;
public:
	void LoadData();
	mxGCovSideBar(wxWindow *parent);
	void OnPaint(wxPaintEvent &event);
	void Refresh(mxSource *src);
	DECLARE_EVENT_TABLE();
};

#endif

