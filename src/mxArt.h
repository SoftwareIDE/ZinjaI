#ifndef MXART_H
#define MXART_H
#include <wx/string.h>
#include <map>
#include <wx/filename.h>
using namespace std;

class wxBitmap;

#define ICON_SIZE 20
#define SKIN_FILE(fname) bitmaps->GetSkinImagePath(fname)
//#define SKIN_FILE_OPT(fname) bitmaps->GetSkinImagePath(fname,false)

#define PROPX (bitmaps->text_ref_x)
#define PROPY (bitmaps->text_ref_y)

//! Estructura para almacenar recuros gráficos comunes
class mxArt {
public:
	int text_ref_x,text_ref_y;
	wxString GetSkinImagePath(const wxString &fname, bool replace_if_missing=true);
	struct {
		const wxBitmap *blank;
		const wxBitmap *source;
		const wxBitmap *header;
		const wxBitmap *other;
	} files;
	struct {
		const wxBitmap *icon02_define;
		const wxBitmap *icon03_func;
		const wxBitmap *icon04_class;
		const wxBitmap *icon05_att_unk;
		const wxBitmap *icon06_att_pri;
		const wxBitmap *icon07_att_pro;
		const wxBitmap *icon08_att_pub;
		const wxBitmap *icon09_mem_unk;
		const wxBitmap *icon10_mem_pri;
		const wxBitmap *icon11_mem_pro;
		const wxBitmap *icon12_mem_pub;
		const wxBitmap *icon13_none;
		const wxBitmap *icon14_global_var;
		const wxBitmap *icon15_res_word;
		const wxBitmap *icon16_preproc;
		const wxBitmap *icon17_doxygen;
		const wxBitmap *icon18_typedef;
		const wxBitmap *icon19_enum_const;
		const wxBitmap *icon20_argument;
		const wxBitmap *icon21_local;
	} parser;
	struct {
		const wxBitmap *ok;
		const wxBitmap *cancel;
		const wxBitmap *stop;
		const wxBitmap *help;
		const wxBitmap *next;
		const wxBitmap *prev;
		const wxBitmap *find;
		const wxBitmap *replace;
	} buttons;
	struct {
		const wxBitmap *question;
		const wxBitmap *warning;
		const wxBitmap *info;
		const wxBitmap *error;
	} icons;
private:
	mxArt(wxString img_dir);
public:
	static void Initialize();
	~mxArt();
	
	
	
	
	
	
	
	
private:
	struct BitmapPack {
		bool pack_exists;
		map<wxString,wxBitmap*> bitmaps;
		BitmapPack() : pack_exists(false) {}
	};
	map<wxString,BitmapPack*> packs; ///< says if a path has the pack file (use BoolFlag to ensure its false initialization)
	const wxBitmap *last_bmp; ///< after calling HasBitmap the result is cached here to avoid searching again if then we call GetBitmap
	BitmapPack *LoadPack(const wxString &path);
	const wxBitmap *auxHasBitmap(const wxFileName &fullpath);
public:
	bool HasBitmap(const wxString &fname, bool is_optional=false);
	const wxBitmap &GetBitmap(const wxString &fname, bool is_optional=false);
};

extern mxArt *bitmaps;

#endif

