#ifndef MXCALLTIP_H
#define MXCALLTIP_H
#include <wx/frame.h>
#include "SingleList.h"

class mxSource;

class mxCalltip : public wxFrame {
private:
	struct entry { 
		wxString line;
		wxString bold_line;
		int argp[50];
		int argc, len;
		entry() {};
		entry(const wxString &s);
		bool ShouldDraw(int cur_arg) { return cur_arg==-1 || argc==-1 || argc>cur_arg; }
	};
	SingleList<entry> entries;
	mxSource *parent;
	int current_arg, max_line, char_h, pos_x, pos_y, delta_chars;
	friend class mxSource;
public:
	mxCalltip(mxSource *src);
	void OnPaint(wxPaintEvent &event);
	void OnFocus(wxFocusEvent &event);
	void ShowCommon(wxString text);
	void Show(int source_pos, const wxString &text);
	void Show(int x, int y, int delta_chars, const wxString &text);
	void Hide();
	void SetArg(int cur_arg);
	bool AcceptsFocus() const { return false; }
	DECLARE_EVENT_TABLE();
};

#endif

