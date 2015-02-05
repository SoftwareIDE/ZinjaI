#ifndef MXBITMAPBUTTON_H
#define MXBITMAPBUTTON_H

#include <wx/bmpbuttn.h>
#include <wx/colour.h>
#include "mxArt.h" // todo el que use esta clase le va a poner algun icono que tiene mxArt

class mxBitmapButton:public wxBitmapButton {
private:
	static wxColour *background_colour;
	const wxBitmap *bmp;
	wxString text;
public:
	mxBitmapButton(wxWindow *parent, wxWindowID id, const wxBitmap *abmp, wxString atext, wxSize size = wxDefaultSize);
	bool SetThings(const wxBitmap *abmp, wxString atext);
	~mxBitmapButton();
	static wxBitmap GenerateButtonImage(wxString text, const wxBitmap *img);

};

#endif

