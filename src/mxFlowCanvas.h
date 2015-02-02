#ifndef MXFLOWCANVAS_H
#define MXFLOWCANVAS_H

#include <wx/scrolwin.h>
#include <wx/dcclient.h>
#include <list>
#include "Cpp11.h"
using namespace std;


#define FC_FRONT(a) while(p<=pe && (a)) p++;
#define FC_BACK(l,a) while(p>=l && (a)) p--;
#define FC_IS_EMPTY ((c=source->GetCharAt(p))==' ' || c=='\t' || c=='\n' || c=='\r' || ((s=source->GetStyleAt(p))==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_PREPROCESSOR || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR))
#define FC_IS(c) ((source->GetCharAt(p))==c)
#define FC_IS_2(c1,c2) ((c=source->GetCharAt(p))==c1 || c==c2)
#define FC_IS_3(c1,c2,c3) ((c=source->GetCharAt(p))==c1 || c==c2 || c==c3)
#define FC_IS_4(c1,c2,c3,c4) ((c=source->GetCharAt(p))==c1 || c==c2 || c==c3 || c==c4)
#define FC_IS_COMMENT ((s=source->GetStyleAt(p))==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_COMMENTLINEDOC || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_PREPROCESSOR || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR)
#define FC_SHOULD_IGNORE ((s=source->GetStyleAt(p))==wxSTC_C_COMMENT || s==wxSTC_C_COMMENTLINE || s==wxSTC_C_CHARACTER || s==wxSTC_C_STRING || s==wxSTC_C_COMMENTLINEDOC || s==wxSTC_C_COMMENTDOC || s==wxSTC_C_PREPROCESSOR || s==wxSTC_C_COMMENTDOCKEYWORD || s==wxSTC_C_COMMENTDOCKEYWORDERROR)

class mxSource;
class wxDC;

class mxFlowCanvas : public wxScrolledWindow {
private:
	struct fc_line {
		int x1,x2,y1,y2;
		wxPen *pen;
		fc_line(int ax1, int ay1, int ax2, int ay2, wxPen *apen) {
			x1=ax1; x2=ax2; y1=ay1; y2=ay2; pen=apen;
		}
	};
	struct fc_arc {
		int x,y,w,h;
		wxPen *pen;
		fc_arc(int ax, int ay, int aw, int ah, wxPen *apen) {
			x=ax; y=ay;
			w=aw; h=ah;
			pen=apen;
		}
	};
	struct fc_text {
		int x,y;
		wxString text;
		wxColour *colour;
		fc_text(int ax=0, int ay=0, wxString atext="", wxColour *acolour=nullptr) {
			x=ax; y=ay;
			text = atext;
			colour = acolour;
		}
	};
	struct draw_data {
		int x,y, izquierda, derecha, alto;
		list <fc_line> lines;
		list <fc_text> texts;
		list <fc_arc> arcs;
		list <draw_data> draws;
		draw_data(int ax=0, int ay=0) {
			x=ax; y=ay;
			alto=0; 
			derecha=0; izquierda=0;
		}
	} draw;
	struct fc_object {
		int x1,y1,x2,y2;
		int p1,p2;
	};
	list<fc_object> objects;
	mxSource *source;
	wxColour *colour_black;
	wxColour *colour_blue;
	wxColour *colour_red;
	wxColour *colour_green;
	wxPen *pen_black;
	wxPen *pen_blue;
	wxPen *pen_red;
	wxPen *pen_green;
	wxFont *font;
	double scale;
	int m_x,m_y;
	int m_sx,m_sy;
	int reference_index;
	bool m_motion;
public:
	void ChangeScale(double rel_size, int mx=-1, int my=-1);
	mxFlowCanvas(wxWindow *parent, mxSource *src);
	~mxFlowCanvas();
	virtual void OnPaint (wxPaintEvent &event);
	bool Analize(int ps, int pe, draw_data &draw, wxDC *dc, int &break_index, int &continue_index);
	void GetTextSize(wxString text, wxDC *dc, int &m_ancho, int &alto);
	void Draw(wxPaintDC &dc, draw_data &draw, int x, int y);
	wxString GetText(int p1, int p2);
	bool IsWord(wxString &text, wxString word);
	int FindBlockEnd(int p, int pe);
	void OnMouseWheel(wxMouseEvent &evt);
	void OnMouseUp(wxMouseEvent &evt);
	void OnMouseDown(wxMouseEvent &evt);
	void OnMouseMotion(wxMouseEvent &evt);
	DECLARE_EVENT_TABLE();
};

#endif
