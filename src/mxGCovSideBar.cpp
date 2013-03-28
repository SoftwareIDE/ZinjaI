#include "mxGCovSideBar.h"
#include <wx/dcclient.h>
#include "mxSource.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include "mxColoursEditor.h"
#include "mxOSD.h"

BEGIN_EVENT_TABLE(mxGCovSideBar, wxWindow)
	EVT_PAINT  (mxGCovSideBar::OnPaint)
END_EVENT_TABLE()

mxGCovSideBar::mxGCovSideBar(wxWindow *parent):wxWindow(parent,wxID_ANY) {
	SetBackgroundColour(*wxWHITE); hits=NULL; SetSize(80,60); should_refresh=NULL;
}

void mxGCovSideBar::OnPaint(wxPaintEvent &event) {
	if (should_refresh) MyRefresh(should_refresh);
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.Clear();
	dc.SetPen(wxPen(wxColour(255,0,0)));
	if (!main_window) return;
	mxSource* src=main_window->GetCurrentSource();
	if (!hits || !src) return;
	int cl=src->GetCurrentLine();
	int l0=src->GetFirstVisibleLine();
	int lh=src->TextHeight(l0);
	int sx,sy; src->GetScreenPosition(&sx,&sy);
	int sw,sh; src->GetSize(&sw,&sh);
	int mx,my; GetScreenPosition(&mx,&my);
	int mw,mh; GetSize(&mw,&mh);
	int y=sy-my, l=l0, m2=hits_max/2+1; if (!m2) m2++;
	while (y<sy+sh && l<line_count) {
		if (l==cl) {
			dc.SetBrush(wxBrush(ctheme->CURRENT_LINE));
			dc.SetPen(wxPen(ctheme->CURRENT_LINE));
			dc.DrawRectangle(0,y,mw,lh);
		}
		int h=hits[l++];
		double hp=h+1;
		int r=hp>=m2?255:(hp)*255/m2;
		int g=hp>=m2?(128-(hp-m2)*128/m2):128;
		dc.SetTextForeground(wxColour(r,g,0));
		wxString text;
		if (h>=0) {
			if (h==0) text="#####";
			else if (h>=0) text<<h;
			int tw,th;
			GetTextExtent(text,&tw,&th);
			dc.DrawText(text,mw-5-tw,y);
		}
		y+=lh;
	}
}

/// @return false si no pudo ejecutar gcov porque hay otra cosa ejecutandose
bool mxGCovSideBar::LoadData (mxSource * src) {
#warning VER QUE NO REVIENTE CUANDO HAY OTRO wxExcute PENDIENTES
	if (!src->sin_titulo && src->source_filename.GetFullPath()==last_path) return true;
	last_path=src->source_filename.GetFullPath();
		
	mxOSD osd(main_window,"Generando y leyendo información de cobertura (gcov)");
	if (hits) delete hits; hits=NULL;
	
	wxFileName binary= src->GetBinaryFileName();
	wxFileName fname= binary.GetFullPath().BeforeLast('.')+"."+src->source_filename.GetExt()+".gcov";
	if (binary.FileExists() && (!fname.FileExists() || fname.GetModificationTime()<binary.GetModificationTime())) { 
		wxString command="gcov "; command<<utils->Quotize(binary.GetName());
		utils->Execute(binary.GetPath(),command,wxEXEC_SYNC);
	}
	
	wxTextFile fil(fname.GetFullPath());
	if (!fil.Exists()) return true; else fil.Open();
	if (!fil.GetLineCount()) return true;
		
	hits=new int[line_count=fil.GetLineCount()+1];
	memset(hits,0,sizeof(int)*line_count);
	hits_max=0;
	for ( wxString line = fil.GetFirstLine(); !fil.Eof(); line = fil.GetNextLine() ) {
		long l=0,h;
		if (!line.AfterFirst(':').BeforeFirst(':').ToLong(&l) || l==0 ||l>=line_count) continue;
		line=line.BeforeFirst(':');
		if (line.Last()=='#') hits[l-1]=0;
		else if (line.Last()=='-') hits[l-1]=-1;
		else if (line.ToLong(&h)) {
			hits[l-1]=h;
			if (h>hits_max) hits_max=h;
		}
	}
	return true;
}

void mxGCovSideBar::MyRefresh (mxSource *src) {
	should_refresh=NULL;
	static int fvl=-1, cl=-1; static mxSource *lsrc=NULL;	
	if (lsrc!=src || src->GetCurrentLine()!=cl || src->GetFirstVisibleLine()!=fvl) {
		if (LoadData(src)) {
			lsrc=src;
			cl=src->GetCurrentLine();
			fvl=src->GetFirstVisibleLine();
		}
	}
}

void mxGCovSideBar::ShouldRefresh (mxSource *src) {
	should_refresh=src; Refresh();
}
