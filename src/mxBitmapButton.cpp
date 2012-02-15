#include "mxBitmapButton.h"
#include "mxUtils.h"
#include <wx/dcmemory.h>
#include <wx/settings.h>

#if defined(__APPLE__) || defined(__WIN32__)
wxColour *mxBitmapButton::background_colour = NULL;
#endif

mxBitmapButton::mxBitmapButton(wxWindow *parent, wxWindowID id, wxBitmap *abmp, wxString atext, wxSize size) : wxBitmapButton(parent,id,mxBitmapButton::GenerateButtonImage(atext,abmp),wxDefaultPosition,size) {
	bmp = abmp;
	text = atext;
//	SetLabel(text);
}

mxBitmapButton::~mxBitmapButton() {
}

bool mxBitmapButton::SetThings(wxBitmap *abmp,wxString atext) {
	if (bmp != abmp || text != atext) {
		bmp = abmp;
		text = atext;
		SetBitmapLabel(GenerateButtonImage(atext,abmp));
//		SetLabel(text);
		return true;
	} else
		return false;
}


wxBitmap mxBitmapButton::GenerateButtonImage(wxString text, wxBitmap *bmp) {
	
#if defined(__APPLE__) || defined(__WIN32__)
	if (!background_colour)
		background_colour = new wxColour(wxButton(NULL,wxID_ANY,_T("lala")).GetBackgroundColour());
#endif

	wxColour c(253,253,253);
	wxBitmap full(200,100,32);
	wxMemoryDC *dc=new wxMemoryDC(full);
	wxRect r;
	dc->SetBackground(wxBrush(c));
	dc->Clear();
	dc->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	int p = text.Find('&');
	if (p==wxNOT_FOUND) {
		p=-1;
	} else {
		text.Remove(p++,1);
	}
	dc->DrawLabel(wxString(_T(" "))<<text,*bmp,wxRect(wxPoint(0,0),wxPoint(200,100)), wxALIGN_LEFT | wxALIGN_TOP,p,&r);
	r.height = r.height>bmp->GetHeight()?r.height:bmp->GetHeight()+1;
	r.height += r.y+1;
	r.y = 0;
	r.width += /*bmp->GetWidth()+*/r.x+1;	
	r.x = 0;
	delete dc;
	wxMask *m=new wxMask();
	m->Create(full,c);
	full.SetMask(m);
#if defined(_WIN32) || defined(__WIN32__)
	full.SetWidth(r.width);
	full.SetHeight(r.height<bmp->GetHeight()?bmp->GetHeight():r.height);
	return full;
#else
	return wxBitmap(full.GetSubBitmap(r));
#endif
}
