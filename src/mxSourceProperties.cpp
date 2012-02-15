#include "mxMainWindow.h"
#include "mxSourceProperties.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "mxUtils.h"
#include "mxSource.h"
#include "Parser.h"

BEGIN_EVENT_TABLE(mxSourceProperties, wxDialog)
	EVT_TIMER(wxID_ANY,mxSourceProperties::OnTimer)
	EVT_BUTTON(wxID_OK,mxSourceProperties::OnOkButton)
	EVT_CLOSE(mxSourceProperties::OnClose)
END_EVENT_TABLE()

mxSourceProperties::mxSourceProperties(wxString afname, mxSource *src) : wxDialog(main_window, wxID_ANY, _T("Propiedades del Archivo"), wxDefaultPosition, wxSize(450,400),wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	fname=afname;
	
	wait_for_parser = NULL;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);

	wxFileName filename(fname);
	filename.Normalize();
	
	wxTextCtrl *name_ctrl=utils->AddTextCtrl(mySizer,this,_T("Ubicacion"),filename.GetFullPath());
	name_ctrl->SetEditable(false);
	
	double fsize = filename.GetSize().ToDouble();
	wxString tsize;
	if (fsize>1024) {
		if (fsize>1024*1024) {
			tsize<<int((fsize/1024/1024))<<_T(".")<<int(fsize/1024/1.024)%1000<<_T(" MB = ");
		} else
			tsize<<int(fsize/1024)<<_T(".")<<int(fsize/1.024)%1000<<_T(" KB = ");
	}
	tsize<<filename.GetSize()<<_T(" B");
	utils->AddTextCtrl(mySizer,this,_T("Tamaño"),tsize)->SetEditable(false);
	utils->AddTextCtrl(mySizer,this,_T("Fecha ultima modifiacion"),filename.GetModificationTime().Format(_T("%H:%M:%S - %d/%B/%Y")))->SetEditable(false);
	
	if (!src) src=main_window->FindSource(fname);
	utils->AddShortTextCtrl(mySizer,this,_T("Abierto"),(src?_T("Si"):_T("No")))->SetEditable(false);
	if (src) utils->AddShortTextCtrl(mySizer,this,_T("Modificado"),(src->GetModify()?_T("Si"):_T("No")))->SetEditable(false);
	
	wxString file_type = _T("No se puede determinar el tipo mientras el parser esta analizando fuentes");
	if (!parser->working) 
		file_type = utils->GetOutput(wxString(_T("file -b \""))<<fname<<_T("\""));
	else if (!wait_for_parser) {
		wait_for_parser = new wxTimer(GetEventHandler(),wxID_ANY); 
		wait_for_parser->Start(1000,true);
	}
	
	utils->AddLongTextCtrl(mySizer,this,_T("Tipo de Archivo"),file_type)->SetEditable(false);
	
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, _T("Cerrar"));

	mySizer->Add(ok_button,sizers->BA5_Right);
	
	SetSizer(mySizer);
	
	SetEscapeId(wxID_OK);
	ok_button->SetDefault();
	
	name_ctrl->SetFocus();
		
	Show();

}

void mxSourceProperties::OnOkButton(wxCommandEvent &event) {
	Close();
}

void mxSourceProperties::OnClose(wxCloseEvent &event) {
	Destroy();
}
void mxSourceProperties::OnTimer(wxTimerEvent &evt) {
	if (parser->working) { wait_for_parser->Start(1000,true); return; }
	text_type->SetValue( utils->GetOutput(wxString(_T("file -b \""))<<fname<<_T("\"")) );
}
