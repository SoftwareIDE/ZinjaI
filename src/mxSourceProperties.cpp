#include "mxMainWindow.h"
#include "mxSourceProperties.h"
#include "mxBitmapButton.h"
#include "mxSizers.h"
#include "mxUtils.h"
#include "mxSource.h"
#include "Parser.h"
#include "mxCommonConfigControls.h"

BEGIN_EVENT_TABLE(mxSourceProperties, wxDialog)
	EVT_BUTTON(wxID_OK,mxSourceProperties::OnOkButton)
	EVT_CLOSE(mxSourceProperties::OnClose)
END_EVENT_TABLE()

mxSourceProperties::mxSourceProperties(wxString afname, mxSource *src) : wxDialog(main_window, wxID_ANY, "Propiedades del Archivo", wxDefaultPosition, wxSize(450,400),wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	fname=afname;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);

	wxFileName filename(fname);
	filename.Normalize();
	
	wxTextCtrl *name_ctrl=mxCCC::AddTextCtrl(mySizer,this,"Ubicacion",filename.GetFullPath());
	name_ctrl->SetEditable(false);
	
	double fsize = filename.GetSize().ToDouble();
	wxString tsize;
	if (fsize>1024) {
		if (fsize>1024*1024) {
			tsize<<int((fsize/1024/1024))<<"."<<int(fsize/1024/1.024)%1000<<" MB = ";
		} else
			tsize<<int(fsize/1024)<<"."<<int(fsize/1.024)%1000<<" KB = ";
	}
	tsize<<filename.GetSize()<<" B";
	mxCCC::AddTextCtrl(mySizer,this,"Tamaño",tsize)->SetEditable(false);
	mxCCC::AddTextCtrl(mySizer,this,"Fecha ultima modifiacion",filename.GetModificationTime().Format("%H:%M:%S - %d/%B/%Y"))->SetEditable(false);
	
	if (!src) src=main_window->FindSource(fname);
	mxCCC::AddShortTextCtrl(mySizer,this,"Abierto",(src?"Si":"No"))->SetEditable(false);
	if (src) mxCCC::AddShortTextCtrl(mySizer,this,"Modificado",(src->GetModify()?"Si":"No"))->SetEditable(false);
	
	wxString file_type = mxUT::GetFileTypeDescription(fname);
	
	mxCCC::AddLongTextCtrl(mySizer,this,"Tipo de Archivo",file_type)->SetEditable(false);
	
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, "Cerrar");

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
