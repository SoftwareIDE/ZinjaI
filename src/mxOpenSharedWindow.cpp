#include <wx/wx.h>
#include "mxOpenSharedWindow.h"
#include "mxSource.h"
#include "ids.h"
#include "mxUtils.h"
#include "ShareManager.h"
#include "mxBitmapButton.h"
#include "mxMessageDialog.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include "Language.h"

BEGIN_EVENT_TABLE(mxOpenSharedWindow, wxDialog)

	EVT_BUTTON(mxID_SHARE_GET_LIST,mxOpenSharedWindow::OnGetListButton)
	EVT_BUTTON(wxID_OK,mxOpenSharedWindow::OnGetSourceButton)
	EVT_BUTTON(wxID_CANCEL,mxOpenSharedWindow::OnCloseButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxOpenSharedWindow::OnHelpButton)
	EVT_CLOSE(mxOpenSharedWindow::OnClose)
	EVT_CHAR_HOOK(mxOpenSharedWindow::OnCharHook)
	EVT_LISTBOX_DCLICK(wxID_ANY,mxOpenSharedWindow::OnGetSourceButton)

END_EVENT_TABLE()

mxOpenSharedWindow::mxOpenSharedWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos , const wxSize& size , long style) : wxDialog(parent, id, LANG(OPENSHARED_CAPTION,"Abrir Compartido"), pos, size, style) {

	if (!share) share = new ShareManager();
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer= new wxBoxSizer(wxHORIZONTAL);

	hostname = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	
	mySizer->Add(new wxStaticText(this, wxID_ANY, LANG(OPENSHARED_HOSTNAME_OR_ADDRESS,"Nombre/Direccion del host: "), wxDefaultPosition, wxDefaultSize, 0), sizers->BLRT5);
	mySizer->Add(hostname,sizers->BLRB5_Exp0);

	list = new wxListBox(this,wxID_ANY,wxDefaultPosition, wxSize(150,150));

	mySizer->Add(list,sizers->BA5_Exp1);

	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*bitmaps->buttons.help);
	wxButton *close_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CLOSE_BUTTON," &Cerrar ")); 
	wxButton *list_button = new mxBitmapButton (this, mxID_SHARE_GET_LIST, bitmaps->buttons.next,LANG(OPENSHARED_UPDATE_LIST_BUTTON,"&Actualizar lista"));
	wxButton *source_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(OPENSHARED_GET_SOURCE_BUTTON,"&Obtener fuente"));

	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(close_button,sizers->BA5);
	bottomSizer->Add(list_button,sizers->BA5);
	bottomSizer->Add(source_button,sizers->BA5);

	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	SetSizerAndFit(mySizer);

	hostname->SetFocus();

}

void mxOpenSharedWindow::OnGetListButton(wxCommandEvent &event){
	if (!(hostname->GetValue().Len())) {
		mxMessageDialog(this,LANG(OPENSHARED_ENTER_HOSTNAME_OR_IP,"Debe introducir primero el nombre de host o el IP de la pc que esta compartiendo el archivo."), LANG(GENERAL_ERROR,"Error"), mxMD_OK|mxMD_ERROR).ShowModal();
	} else {
		if (share->GetList(hostname->GetValue(),list))
			list->SetFocus();
		else 
			mxMessageDialog(this,LANG(OPENSHARED_HOST_NOT_FOUND,"No se encontro el host especificado."), LANG(GENERAL_ERROR,"Error"), mxMD_OK|mxMD_ERROR).ShowModal();
	}
}

void mxOpenSharedWindow::OnHelpButton(wxCommandEvent &event){
	mxHelpWindow::ShowHelp("open_shared.html");
}

void mxOpenSharedWindow::OnCloseButton(wxCommandEvent &event){
	Close();
}

void mxOpenSharedWindow::OnGetSourceButton(wxCommandEvent &event){
	if (!list->GetStringSelection().Len() ) {
		if (list->GetCount()==0) {
			mxMessageDialog(this,LANG(OPENSHARED_GET_SHARED_LIST_FIRST,"Debe obtener la lista de fuentes compartidas primero. Para ello ingrese el nombre de host\no el IP de la pc que esta compartiendo el archivo y seleccione \"Actualizar lista\"."), LANG(GENERAL_ERROR,"Error"), mxMD_OK|mxMD_ERROR).ShowModal();
			hostname->SetFocus();
		} else {
			mxMessageDialog(this,LANG(OPENSHARED_SELECT_ONE_FIRST,"Debe seleccionar un fuente de la lista primero."), LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			list->SetFocus();
		}
	} else {
		if (share->AskFor(list->GetStringSelection()))
			Close();
		else
			mxMessageDialog(this,LANG(OPENSHARED_HOST_NOT_FOUND,"No se encontro el host especificado."), LANG(GENERAL_ERROR,"Error"), mxMD_OK|mxMD_ERROR).ShowModal();
	}
}

void mxOpenSharedWindow::OnClose(wxCloseEvent &event){
	Hide();
}

void mxOpenSharedWindow::OnCharHook (wxKeyEvent &event) {
	if ( event.GetKeyCode() == WXK_ESCAPE ) {
		Close();
	} else if (event.GetKeyCode()==WXK_RETURN || event.GetKeyCode()==WXK_NUMPAD_ENTER) {
		if (FindFocus()==hostname) {
			wxCommandEvent evt;
			OnGetListButton(evt);
		} else if (FindFocus()==list) {
			wxCommandEvent evt;
			OnGetSourceButton(evt);
		}
	} else event.Skip();
}

