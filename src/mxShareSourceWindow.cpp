#include "mxShareSourceWindow.h"

#include <wx/wx.h>
#include <wx/utils.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/socket.h>
#include "mxSource.h"
#include "ids.h"
#include "mxUtils.h"
#include "ShareManager.h"
#include "mxBitmapButton.h"
#include "mxMessageDialog.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include "Language.h"


BEGIN_EVENT_TABLE(mxShareSourceWindow, wxDialog)

	EVT_BUTTON(wxID_OK,mxShareSourceWindow::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxShareSourceWindow::OnCancelButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxShareSourceWindow::OnHelpButton)
	EVT_CLOSE(mxShareSourceWindow::OnClose)

END_EVENT_TABLE()

mxShareSourceWindow::mxShareSourceWindow(mxSource *source, wxString name, wxWindow* parent, wxWindowID id, const wxPoint& pos , const wxSize& size , long style) : wxDialog(parent, id, LANG(SHARESOURCE_CAPTION,"Compartir Archivo"), pos, size, style) {
	
	if (!share) share = new ShareManager();
	
	source_ctrl=source;
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer= new wxBoxSizer(wxHORIZONTAL);

	source_name = new wxComboBox(this, wxID_ANY);
	share->GetMyList((wxControlWithItems*)source_name);
	
	freeze_check = new wxCheckBox(this, wxID_ANY, LANG(SHARESOURCE_SHARE_WITHOUT_MODIFICATIONS,"Compartir sin modificaciones"));
	freeze_check->SetValue(false);
	
	mySizer->Add(new wxStaticText(this, wxID_ANY, LANG(SHARESOURCE_ENTER_NAME,"Ingrese el nombre con el que desea compartir el archivo,\no seleccione uno de la lista para reemplazarlo: "), wxDefaultPosition, wxDefaultSize, 0), sizers->BLRT5);
	mySizer->Add(source_name,sizers->BLRB5_Exp0);
	mySizer->Add(freeze_check,sizers->BA5_Exp0);

	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancel"));
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(SHARESOURCE_SHARE_BUTTON,"Compartir"));
	ok_button->SetDefault();
	
	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	wxString whoami=LANG(SHARESOURCE_YOUR_HOSTNAME,"Para que otros puedan descargar este codigo nesecitaran identificar tu PC.\n   Tu nombre de host es: ");
	mySizer->Add(new wxStaticText(this, wxID_ANY, whoami<<wxGetHostName(), wxDefaultPosition, wxDefaultSize, 0), sizers->BA5);
	
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	
	SetSizerAndFit(mySizer);

	source_name->SetFocus();
	source_name->SetValue(name);
	source_name->SetSelection(-1,-1);
	
	Show();

}


void mxShareSourceWindow::OnOkButton(wxCommandEvent &event) {
	wxString name=source_name->GetValue();
	if (!name.Len()) {
		mxMessageDialog(this,LANG(SHARESOURCE_EMPTY_SHARING_NAME,"Debe ingresar un nombre para compartir el archivo."), LANG(GENERAL_ERROR,"Error"), mxMD_OK|mxMD_ERROR).ShowModal();
		return;
	}
	if (name.Len()>255) {
		mxMessageDialog(this,LANG(SHARESOURCE_NAME_TOO_LONG,"El nombre no puede tener mas de 255 caracteres."), LANG(GENERAL_ERROR,"Error"), mxMD_OK|mxMD_ERROR).ShowModal();
		return;
	}
	
	if (!share->CheckServer()) {
		mxMessageDialog(this,LANG(SHARESOURCE_COULDNT_START_SERVER,"No se pudo inciar el servidor. Compruebe que el puerto este correctamente configuardo y libre."), LANG(GENERAL_ERROR,"Error"), mxMD_OK|mxMD_ERROR).ShowModal();
		return;
	}
	
	if (share->Exists(name))
		if (mxMD_YES==mxMessageDialog(this,LANG(SHARESOURCE_SOURCE_ALREADY_EXISTS_REPLACE,"Ya existe un archivo compartido con ese nombre. Desea reemplazarlo?"),LANG(GENERAL_WARNING,"Aviso"),mxMD_YES_NO|mxMD_WARNING).ShowModal() )
			share->Replace(name,source_ctrl,freeze_check->GetValue());
		else
			return;
	else
		share->Insert(name,source_ctrl,freeze_check->GetValue());
	Close();
}

void mxShareSourceWindow::OnCancelButton(wxCommandEvent &event) {
	Close();
}

void mxShareSourceWindow::OnClose(wxCloseEvent &event) {
	Destroy();
}

void mxShareSourceWindow::OnHelpButton(wxCommandEvent &event){
	SHOW_HELP("share_source.html");
}

