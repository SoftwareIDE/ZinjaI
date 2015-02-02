#include "mxArt.h"
#include "mxUtils.h"

#include <wx/dcmemory.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include "ConfigManager.h"
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <iostream>
using namespace std;
	
mxArt *bitmaps = nullptr;

mxArt::mxArt(wxString img_dir) {
	
//	// Esto se hace para tener una referencia de cuanto miden las cosas (tiene que haber algo mejor)
//	wxDialog *dlg = new wxDialog(nullptr,wxID_ANY,_T("Ventana de Prueba"),wxDefaultPosition,wxDefaultSize);
//	wxSizer *sz = new wxBoxSizer(wxHORIZONTAL);
//	wxTextCtrl *tc = new wxTextCtrl(dlg,wxID_ANY,_T("Texto de Prueba"));
//	sz->Add(tc);
//	dlg->SetSizerAndFit(sz);
//	text_ref_x = tc->GetSize().GetWidth();
//	text_ref_y = tc->GetSize().GetHeight();
////	wxMessageBox(wxString()<<text_ref_x<<" "<<text_ref_y);
//	dlg->Destroy();
	
	files.source = new wxBitmap(SKIN_FILE(_T("ap_source.png")),wxBITMAP_TYPE_PNG);
	files.header = new wxBitmap(SKIN_FILE(_T("ap_header.png")),wxBITMAP_TYPE_PNG);
	files.other = new wxBitmap(SKIN_FILE(_T("ap_other.png")),wxBITMAP_TYPE_PNG);
	files.blank = new wxBitmap(SKIN_FILE(_T("ap_blank.png")),wxBITMAP_TYPE_PNG);
	
	parser.icon02_define =new wxBitmap(SKIN_FILE(_T("as_define.png")),wxBITMAP_TYPE_PNG);
	parser.icon03_func = new wxBitmap(SKIN_FILE(_T("as_func.png")),wxBITMAP_TYPE_PNG);
	parser.icon04_class = new wxBitmap(SKIN_FILE(_T("as_class.png")),wxBITMAP_TYPE_PNG);
	parser.icon05_att_unk = new wxBitmap(SKIN_FILE(_T("as_att_unk.png")),wxBITMAP_TYPE_PNG);
	parser.icon06_att_pri = new wxBitmap(SKIN_FILE(_T("as_att_pri.png")),wxBITMAP_TYPE_PNG);
	parser.icon07_att_pro = new wxBitmap(SKIN_FILE(_T("as_att_pro.png")),wxBITMAP_TYPE_PNG);
	parser.icon08_att_pub = new wxBitmap(SKIN_FILE(_T("as_att_pub.png")),wxBITMAP_TYPE_PNG);
	parser.icon09_mem_unk = new wxBitmap(SKIN_FILE(_T("as_mem_unk.png")),wxBITMAP_TYPE_PNG);
	parser.icon10_mem_pri = new wxBitmap(SKIN_FILE(_T("as_mem_pri.png")),wxBITMAP_TYPE_PNG);
	parser.icon11_mem_pro = new wxBitmap(SKIN_FILE(_T("as_mem_pro.png")),wxBITMAP_TYPE_PNG);
	parser.icon12_mem_pub = new wxBitmap(SKIN_FILE(_T("as_mem_pub.png")),wxBITMAP_TYPE_PNG);
	parser.icon13_none = new wxBitmap(SKIN_FILE(_T("as_none.png")),wxBITMAP_TYPE_PNG);
	parser.icon14_global_var = new wxBitmap(SKIN_FILE(_T("as_global.png")),wxBITMAP_TYPE_PNG);
	parser.icon15_res_word = new wxBitmap(SKIN_FILE(_T("as_res_word.png")),wxBITMAP_TYPE_PNG);
	parser.icon16_preproc = new wxBitmap(SKIN_FILE(_T("as_preproc.png")),wxBITMAP_TYPE_PNG);
	parser.icon17_doxygen = new wxBitmap(SKIN_FILE(_T("as_doxygen.png")),wxBITMAP_TYPE_PNG);
	parser.icon18_typedef = new wxBitmap(SKIN_FILE(_T("as_typedef.png")),wxBITMAP_TYPE_PNG);
	parser.icon19_enum_const = new wxBitmap(SKIN_FILE(_T("as_enum_const.png")),wxBITMAP_TYPE_PNG);
	parser.icon20_argument = new wxBitmap(SKIN_FILE(_T("as_arg.png")),wxBITMAP_TYPE_PNG);
	parser.icon21_local = new wxBitmap(SKIN_FILE(_T("as_local.png")),wxBITMAP_TYPE_PNG);

	icons.error = new wxBitmap(SKIN_FILE(_T("icono_error.png")),wxBITMAP_TYPE_PNG);
	icons.info = new wxBitmap(SKIN_FILE(_T("icono_info.png")),wxBITMAP_TYPE_PNG);
	icons.warning = new wxBitmap(SKIN_FILE(_T("icono_warning.png")),wxBITMAP_TYPE_PNG);
	icons.question = new wxBitmap(SKIN_FILE(_T("icono_question.png")),wxBITMAP_TYPE_PNG);
//#ifdef __WIN32__
//	wxColour background_colour=wxButton(nullptr,wxID_ANY,_T("lala")).GetBackgroundColour();
//	wxMemoryDC dc;
//	dc.SelectObject(*icons.question);
//	if (dc.IsOk()) {
//		dc.SetBackground(wxBrush(background_colour));
//		dc.Clear();
//		dc.DrawBitmap(wxBitmap(SKIN_FILE(_T("icono_question.png")),wxBITMAP_TYPE_PNG),0,0);
//	}
//	dc.SelectObject(*icons.error);
//	if (dc.IsOk()) {
//		dc.SetBackground(wxBrush(background_colour));
//		dc.Clear();
//		dc.DrawBitmap(wxBitmap(SKIN_FILE(_T("icono_error.png")),wxBITMAP_TYPE_PNG),0,0);
//	}
//	dc.SelectObject(*icons.info);
//	if (dc.IsOk()) {
//		dc.SetBackground(wxBrush(background_colour));
//		dc.Clear();
//		dc.DrawBitmap(wxBitmap(SKIN_FILE(_T("icono_info.png")),wxBITMAP_TYPE_PNG),0,0);
//	}
//	dc.SelectObject(*icons.warning);
//	if (dc.IsOk()) {
//		dc.SetBackground(wxBrush(background_colour));
//		dc.Clear();
//		dc.DrawBitmap(wxBitmap(SKIN_FILE(_T("icono_warning.png")),wxBITMAP_TYPE_PNG),0,0);
//	}
//#endif
	
	buttons.ok = new wxBitmap(SKIN_FILE(_T("boton_ok.png")),wxBITMAP_TYPE_PNG);
	buttons.cancel = new wxBitmap(SKIN_FILE(_T("boton_cancel.png")),wxBITMAP_TYPE_PNG);
	buttons.help = new wxBitmap(SKIN_FILE(_T("boton_ayuda.png")),wxBITMAP_TYPE_PNG);
	buttons.find = new wxBitmap(SKIN_FILE(_T("boton_buscar.png")),wxBITMAP_TYPE_PNG);
	buttons.replace = new wxBitmap(SKIN_FILE(_T("boton_reemplazar.png")),wxBITMAP_TYPE_PNG);
	buttons.next = new wxBitmap(SKIN_FILE(_T("boton_next.png")),wxBITMAP_TYPE_PNG);
	buttons.prev = new wxBitmap(SKIN_FILE(_T("boton_prev.png")),wxBITMAP_TYPE_PNG);
	buttons.stop = new wxBitmap(SKIN_FILE(_T("boton_stop.png")),wxBITMAP_TYPE_PNG);
	
}


mxArt::~mxArt() {
	
	delete parser.icon02_define;
	delete parser.icon03_func;
	delete parser.icon04_class;
	delete parser.icon05_att_unk;
	delete parser.icon06_att_pri;
	delete parser.icon07_att_pro;
	delete parser.icon08_att_pub;
	delete parser.icon09_mem_unk;
	delete parser.icon10_mem_pri;
	delete parser.icon11_mem_pro;
	delete parser.icon12_mem_pub;
	delete parser.icon13_none;
	delete parser.icon14_global_var;
	delete parser.icon15_res_word;
	delete parser.icon16_preproc;
	delete parser.icon17_doxygen;
	delete parser.icon18_typedef;
	delete parser.icon19_enum_const;
	delete parser.icon20_argument;
	
	delete files.blank;
	delete files.source;
	delete files.header;
	delete files.other;

	delete buttons.ok;
	delete buttons.cancel;
	delete buttons.help;
	delete buttons.replace;
	delete buttons.find;
	delete buttons.next;
	delete buttons.prev;
	delete buttons.stop;
	
	delete icons.info;
	delete icons.error;
	delete icons.warning;
	delete icons.question;
}

wxString mxArt::GetSkinImagePath(wxString fname, bool replace_if_missing) {
	static wxString default_path=DIR_PLUS_FILE(config->zinjai_dir,_T("imgs"));
	static wxString skin_path=DIR_PLUS_FILE(config->zinjai_dir,config->Files.skin_dir);
	static bool is_default = default_path==skin_path;
	if (is_default) {
#ifdef _ZINJAI_DEBUG
		if (!wxFileName::FileExists(DIR_PLUS_FILE(default_path,fname)))
			cerr<<"MISSING IMAGE: "<<DIR_PLUS_FILE(default_path,fname)<<endl;
#endif
		return DIR_PLUS_FILE(default_path,fname);
	} else {
		wxString fskin = DIR_PLUS_FILE(skin_path,fname);
		if (!replace_if_missing || wxFileName::FileExists(fskin)) {
#ifdef _ZINJAI_DEBUG
			if (!wxFileName::FileExists(fskin))
				cerr<<"MISSING IMAGE: "<<fskin<<endl;
#endif
			return fskin;
		} else {
#ifdef _ZINJAI_DEBUG
			if (!wxFileName::FileExists(DIR_PLUS_FILE(default_path,fname)))
				cerr<<"MISSING IMAGE: "<<DIR_PLUS_FILE(default_path,fname)<<endl;
#endif
			return DIR_PLUS_FILE(default_path,fname);
		}
	}
}

void mxArt::Initialize ( ) {
	bitmaps = new mxArt(config->Files.skin_dir);
}

