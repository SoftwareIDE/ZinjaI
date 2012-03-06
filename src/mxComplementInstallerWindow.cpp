#include "mxComplementInstallerWindow.h"
#include "mxMainWindow.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "mxUtils.h"
#include "ids.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include "ConfigManager.h"

BEGIN_EVENT_TABLE(mxComplementInstallerWindow, wxDialog)
	EVT_BUTTON(wxID_OK,mxComplementInstallerWindow::OnOkButton)
	EVT_BUTTON(wxID_CANCEL,mxComplementInstallerWindow::OnCancelButton)
	EVT_BUTTON(wxID_FIND,mxComplementInstallerWindow::OnDownloadButton)
	EVT_BUTTON(mxID_HELP_BUTTON,mxComplementInstallerWindow::OnHelpButton)
	EVT_CLOSE(mxComplementInstallerWindow::OnClose)
END_EVENT_TABLE()

mxComplementInstallerWindow::mxComplementInstallerWindow(wxWindow *parent):wxDialog(parent,wxID_ANY,LANG(COMPLEMENTS_CAPTION,"Instalación de Complementos"),wxDefaultPosition,wxDefaultSize) {
	
	wxBoxSizer *iSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *tSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bottomSizer= new wxBoxSizer(wxHORIZONTAL);
	
	
	wxButton *download_button = new mxBitmapButton (this, wxID_FIND, bitmaps->buttons.next, LANG(COMPLEMENTS_DOWNLOAD,"&Descargar..."));
	wxButton *cancel_button = new mxBitmapButton (this, wxID_CANCEL, bitmaps->buttons.cancel, LANG(GENERAL_CANCEL_BUTTON,"&Cancelar"));
	wxButton *ok_button = new mxBitmapButton (this, wxID_OK, bitmaps->buttons.ok, LANG(COMPLEMENTS_INSTALL,"&Instalar..."));
	wxBitmapButton *help_button = new wxBitmapButton (this,mxID_HELP_BUTTON,*(bitmaps->buttons.help));
	
	bottomSizer->Add(help_button,sizers->BA5_Exp0);
	bottomSizer->Add(download_button,sizers->BA5_Exp0);
	bottomSizer->AddStretchSpacer();
	bottomSizer->Add(cancel_button,sizers->BA5);
	bottomSizer->Add(ok_button,sizers->BA5);
	
	utils->AddStaticText(tSizer,this,LANG(COMPLEMENTS_WHATIS,""
		"Los complementos consisten en packs de archivos adicionales\n"
		"para ZinjaI, que pueden incluir bibliotecas, documentación,\n"
		"temas de íconos, plantillas de proyectos, indices para el \n"
		"autocompletado, etc."
		));
	utils->AddStaticText(tSizer,this,LANG(COMPLEMENTS_INSTRUCCIONS,""
		"Para instalar un complemento debe: 1) descargarlo desde la\n"
		"sección de descargas del sitio de ZinjaI (haga click\n"
		"en el botón \"Descargar...\" para ir al sitio); 2) presionar\n"
		"\"Instalar...\"; 3) seleccionar el archivo descargado.\n"
		));
	
	
	iSizer->Add(new wxStaticBitmap(this,wxID_ANY, wxBitmap(SKIN_FILE(_T("upgrade.png")), wxBITMAP_TYPE_PNG)),sizers->BA10);
	iSizer->Add(tSizer,sizers->Exp1);
	mySizer->Add(iSizer,sizers->Exp1);
	mySizer->Add(bottomSizer,sizers->BA5_Exp0);
	SetSizerAndFit(mySizer);
	
	ok_button->SetFocus();
	
	SetEscapeId(wxID_CANCEL);
	ok_button->SetDefault();
	
	Show();
}

void mxComplementInstallerWindow::OnOkButton (wxCommandEvent & evt) {
	Hide();
	Destroy();
}

void mxComplementInstallerWindow::OnHelpButton (wxCommandEvent & evt) {
	SHOW_HELP("complements.html");
}

void mxComplementInstallerWindow::OnDownloadButton (wxCommandEvent & evt) {
	if (config->Init.language_file=="spanish")
		utils->OpenInBrowser(_T("http://zinjai.sourceforge.net?page=downextras.html"));
	else
		utils->OpenInBrowser(_T("http://zinjai.sourceforge.net?page=downextras_en.html"));
}

void mxComplementInstallerWindow::OnCancelButton (wxCommandEvent & evt) {
	Close();
}

void mxComplementInstallerWindow::OnClose (wxCloseEvent & evt) {
	Destroy();
}

