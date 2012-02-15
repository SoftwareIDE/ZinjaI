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
	
mxArt *bitmaps;

mxArt::mxArt(wxString img_dir) {
	
//	// Esto se hace para tener una referencia de cuanto miden las cosas (tiene que haber algo mejor)
//	wxDialog *dlg = new wxDialog(NULL,wxID_ANY,_T("Ventana de Prueba"),wxDefaultPosition,wxDefaultSize);
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
	
	parser.icon2 =new wxBitmap(SKIN_FILE(_T("as_define.png")),wxBITMAP_TYPE_PNG);
	parser.icon3 = new wxBitmap(SKIN_FILE(_T("as_func.png")),wxBITMAP_TYPE_PNG);
	parser.icon4 = new wxBitmap(SKIN_FILE(_T("as_class.png")),wxBITMAP_TYPE_PNG);
	parser.icon5 = new wxBitmap(SKIN_FILE(_T("as_att_unk.png")),wxBITMAP_TYPE_PNG);
	parser.icon6 = new wxBitmap(SKIN_FILE(_T("as_att_pri.png")),wxBITMAP_TYPE_PNG);
	parser.icon7 = new wxBitmap(SKIN_FILE(_T("as_att_pro.png")),wxBITMAP_TYPE_PNG);
	parser.icon8 = new wxBitmap(SKIN_FILE(_T("as_att_pub.png")),wxBITMAP_TYPE_PNG);
	parser.icon9 = new wxBitmap(SKIN_FILE(_T("as_mem_unk.png")),wxBITMAP_TYPE_PNG);
	parser.icon10 = new wxBitmap(SKIN_FILE(_T("as_mem_pri.png")),wxBITMAP_TYPE_PNG);
	parser.icon11 = new wxBitmap(SKIN_FILE(_T("as_mem_pro.png")),wxBITMAP_TYPE_PNG);
	parser.icon12 = new wxBitmap(SKIN_FILE(_T("as_mem_pub.png")),wxBITMAP_TYPE_PNG);
	parser.icon13 = new wxBitmap(SKIN_FILE(_T("as_none.png")),wxBITMAP_TYPE_PNG);
	parser.icon14 = new wxBitmap(SKIN_FILE(_T("as_global.png")),wxBITMAP_TYPE_PNG);
	parser.icon15 = new wxBitmap(SKIN_FILE(_T("as_res_word.png")),wxBITMAP_TYPE_PNG);
	parser.icon16 = new wxBitmap(SKIN_FILE(_T("as_preproc.png")),wxBITMAP_TYPE_PNG);
	parser.icon17 = new wxBitmap(SKIN_FILE(_T("as_doxygen.png")),wxBITMAP_TYPE_PNG);
	parser.icon18 = new wxBitmap(SKIN_FILE(_T("as_typedef.png")),wxBITMAP_TYPE_PNG);

	icons.error = new wxBitmap(SKIN_FILE(_T("icono_error.png")),wxBITMAP_TYPE_PNG);
	icons.info = new wxBitmap(SKIN_FILE(_T("icono_info.png")),wxBITMAP_TYPE_PNG);
	icons.warning = new wxBitmap(SKIN_FILE(_T("icono_warning.png")),wxBITMAP_TYPE_PNG);
	icons.question = new wxBitmap(SKIN_FILE(_T("icono_question.png")),wxBITMAP_TYPE_PNG);
//#if defined(_WIN32) || defined(__WIN32__)
//	wxColour background_colour=wxButton(NULL,wxID_ANY,_T("lala")).GetBackgroundColour();
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
	delete parser.icon2;
	delete parser.icon3;
	delete parser.icon4;
	delete parser.icon5;
	delete parser.icon6;
	delete parser.icon7;
	delete parser.icon8;
	delete parser.icon9;
	delete parser.icon10;
	delete parser.icon11;
	delete parser.icon12;
	delete parser.icon13;
	delete parser.icon14;
	delete parser.icon15;
	delete parser.icon16;
	
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
#ifdef DEBUG
		if (!wxFileName::FileExists(DIR_PLUS_FILE(default_path,fname)))
			cerr<<"MISSING IMAGE: "<<DIR_PLUS_FILE(default_path,fname)<<endl;
#endif
		return DIR_PLUS_FILE(default_path,fname);
	} else {
		wxString fskin = DIR_PLUS_FILE(skin_path,fname);
		if (!replace_if_missing || wxFileName::FileExists(fskin)) {
#ifdef DEBUG
			if (!wxFileName::FileExists(fskin))
				cerr<<"MISSING IMAGE: "<<fskin<<endl;
#endif
			return fskin;
		} else {
#ifdef DEBUG
			if (!wxFileName::FileExists(DIR_PLUS_FILE(default_path,fname)))
				cerr<<"MISSING IMAGE: "<<DIR_PLUS_FILE(default_path,fname)<<endl;
#endif
			return DIR_PLUS_FILE(default_path,fname);
		}
	}
}
