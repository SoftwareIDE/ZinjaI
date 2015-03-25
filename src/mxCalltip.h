#ifndef MXCALLTIP_H
#define MXCALLTIP_H
#include <wx/popupwin.h>
#include "SingleList.h"

class mxSource;

class mxCalltip : public wxPopupWindow {
private:
	struct wrap_info {
		int from, to; // output
		int indent; // output
		int max_len; // input
		int p0,p1; // internal
		int Len() { return to-from+indent; }
		void Reset(int ml) {
			p0=0; p1=ml; max_len=ml;
		}
	};
	struct entry { 
		wxString line;
		wxString bold_line;
		int argp[50];
		int argc, len;
		entry() {};
		entry(const wxString &s);
		bool ShouldDraw(int cur_arg) { return cur_arg==-1 || argc==-1 || argc>cur_arg; }
		bool GetWrapPoint(wrap_info &w) {
			if (w.p0>=len) return false;
			w.indent=w.p0==0?0:4;
			if (w.p1>=len) {
				w.from=w.p0; w.to=len; w.p0=len;
			} else {
				int j = w.p1;
				while ( j>w.p0 && line[j]!=' ' && line[j]!='\t' && line[j]!='<' && line[j]!='>' && 
						( j==0 || (line[j-1]!=','&&(line[j-1]!='('||line[j]==')')&&line[j-1]!=')') )
					  ) j--;
				if (j==w.p0) { j=w.p1; while (j<len && (line[j]==' '||line[j]=='\t')) j++; }
				w.from = w.p0; w.to = j; 
				while (j<len && (line[j]==' '||line[j]=='\t')) j++;
				w.p0 = j; w.p1 = w.p0+w.max_len-w.indent;
			}
			return true;
		}
	};
	SingleList<entry> entries;
	mxSource *parent;
	int current_arg, max_len, char_h, char_w, pos_x, pos_y, delta_chars;
	wxFont my_font;
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
	bool AcceptsFocus() const { return false; } // parece no tener efecto
	void OnActivate(wxActivateEvent & event);
	DECLARE_EVENT_TABLE();
};

#endif

