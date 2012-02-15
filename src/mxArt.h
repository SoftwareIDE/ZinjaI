#ifndef MXART_H
#define MXART_H

#include <wx/bitmap.h>
#include <wx/clrpicker.h>

#define ICON_SIZE 20
#define SKIN_FILE(fname) bitmaps->GetSkinImagePath(fname)
#define SKIN_FILE_OPT(fname) bitmaps->GetSkinImagePath(fname,false)

#define PROPX (bitmaps->text_ref_x)
#define PROPY (bitmaps->text_ref_y)

//! Estructura para almacenar recuros gráficos comunes
class mxArt {
public:
	int text_ref_x,text_ref_y;
	wxString GetSkinImagePath(wxString fname, bool replace_if_missing=true);
	struct {
		wxBitmap *blank;
		wxBitmap *source;
		wxBitmap *header;
		wxBitmap *other;
	} files;
	struct {
		wxBitmap *icon2;
		wxBitmap *icon3;
		wxBitmap *icon4;
		wxBitmap *icon5;
		wxBitmap *icon6;
		wxBitmap *icon7;
		wxBitmap *icon8;
		wxBitmap *icon9;
		wxBitmap *icon10;
		wxBitmap *icon11;
		wxBitmap *icon12;
		wxBitmap *icon13;
		wxBitmap *icon14;
		wxBitmap *icon15;
		wxBitmap *icon16;
		wxBitmap *icon17;
		wxBitmap *icon18;
	} parser;
	struct {
		wxBitmap *ok;
		wxBitmap *cancel;
		wxBitmap *stop;
		wxBitmap *help;
		wxBitmap *next;
		wxBitmap *prev;
		wxBitmap *find;
		wxBitmap *replace;
	} buttons;
	struct {
		wxBitmap *question;
		wxBitmap *warning;
		wxBitmap *info;
		wxBitmap *error;
	} icons;
	mxArt(wxString img_dir);
	~mxArt();
};

extern mxArt *bitmaps;

#endif

