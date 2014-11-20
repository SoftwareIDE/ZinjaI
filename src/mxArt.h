#ifndef MXART_H
#define MXART_H
#include <wx/string.h>

class wxBitmap;

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
		wxBitmap *icon02_define;
		wxBitmap *icon03_func;
		wxBitmap *icon04_class;
		wxBitmap *icon05_att_unk;
		wxBitmap *icon06_att_pri;
		wxBitmap *icon07_att_pro;
		wxBitmap *icon08_att_pub;
		wxBitmap *icon09_mem_unk;
		wxBitmap *icon10_mem_pri;
		wxBitmap *icon11_mem_pro;
		wxBitmap *icon12_mem_pub;
		wxBitmap *icon13_none;
		wxBitmap *icon14_global_var;
		wxBitmap *icon15_res_word;
		wxBitmap *icon16_preproc;
		wxBitmap *icon17_doxygen;
		wxBitmap *icon18_typedef;
		wxBitmap *icon19_enum_const;
		wxBitmap *icon20_argument;
		wxBitmap *icon21_local;
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
private:
	mxArt(wxString img_dir);
public:
	static void Initialize();
	~mxArt();
};

extern mxArt *bitmaps;

#endif

