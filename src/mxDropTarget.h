#ifndef MXDROPTARGET_H
#define MXDROPTARGET_H

#include <wx/dnd.h>

class mxSource;

class mxDropTarget:public wxDropTarget {
private:
	wxDataObjectComposite *data;
	wxFileDataObject *file_data;
	wxTextDataObject *text_data;
	mxSource *src;
	static bool last_drag_cancel;
	static mxSource *current_drag_source;
	friend class mxSource;
public:
	bool OnDrop(wxCoord x, wxCoord y);
	wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
	wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
	void OnLeave();
	wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def);
	mxDropTarget(mxSource *s);
};

#endif
