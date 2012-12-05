#include <wx/dcclient.h>
#include "mxDiffSideBar.h"
#include "mxMainWindow.h"
#include "mxSource.h"

BEGIN_EVENT_TABLE(mxDiffSideBar, wxWindow)
	EVT_MOUSEWHEEL(mxDiffSideBar::OnMouseWheel)
	EVT_PAINT  (mxDiffSideBar::OnPaint)
	EVT_LEFT_DOWN(mxDiffSideBar::OnMouseDown)
END_EVENT_TABLE()

mxDiffSideBar::mxDiffSideBar():wxWindow(main_window,wxID_ANY) {
	SetBackgroundColour(*wxWHITE);
	pen_del = wxPen(wxColour(255,0,0));
	pen_add = wxPen(wxColour(0,200,0));
	pen_mod = wxPen(wxColour(200,200,0));
	last_h=0;
}


void mxDiffSideBar::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	dc.Clear();
	mxSource* src=main_window->GetCurrentSource();
	if (!src) return;
	DiffInfo *d1 = src->first_diff_info;
	int l=src->GetLineCount(), h=last_h=GetRect().GetHeight()-10;
	dc.Clear();
	while (d1) {
		switch (d1->marker) {
		case mxSTC_MARK_DIFF_CHANGE: dc.SetPen(pen_mod); break;
		case mxSTC_MARK_DIFF_DEL: dc.SetPen(pen_del); break;
		case mxSTC_MARK_DIFF_ADD: dc.SetPen(pen_add); break;
		}
		for (int j=0;j<d1->len;j++) {
			int i=src->MarkerLineFromHandle(d1->handles[j])*h/l+10;
			dc.DrawLine(0,i+2,20,i+2);
			dc.DrawLine(0,i+1,20,i+1);
			dc.DrawLine(0,i,20,i);
			dc.DrawLine(0,i-1,20,i-1);
			dc.DrawLine(0,i-2,20,i-2);
		}
		d1=d1->next;
	}
}

void mxDiffSideBar::OnMouseWheel(wxMouseEvent &evt) {
	mxSource* src=main_window->GetCurrentSource();
	if (src) src->GotoDiffChange(evt.m_wheelRotation<0);
}

void mxDiffSideBar::OnMouseDown(wxMouseEvent &evt) {
	mxSource* src=main_window->GetCurrentSource();
	if (!src) return;
	int line=-1, dist=0, bhand=-1;
	DiffInfo *d1 = src->first_diff_info;
	int l=src->GetLineCount(),mi=evt.GetPosition().y;
	int h=last_h,ll,d,i,j;
	while (d1) {
		for (j=0;j<d1->len;j++) {
			ll=src->MarkerLineFromHandle(d1->handles[j]);
			i=ll*h/l+10;
			d=mi>i?mi-i:i-mi;
			if (line<0||d<dist) {
				dist=d; line=ll;
				if (d1->bhandles) bhand=d1->bhandles[j];
			}
		}
		d1=d1->next;
	}
	if (bhand>=0 && src->diff_brother) {
		src->diff_brother->MarkError(src->diff_brother->MarkerLineFromHandle(bhand),false);
	}
	if (line>=0) {
		src->MarkError(line);
	}
}
