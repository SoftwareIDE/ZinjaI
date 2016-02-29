#include <wx/file.h>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <iostream>
#include "mxCommonConfigControls.h"
using namespace std;
#include "mxComplementInstallerWindow.h"
#include "mxBitmapButton.h"
#include "Language.h"
#include "mxUtils.h"
#include "ids.h"
#include "mxSizers.h"
#include "mxHelpWindow.h"
#include "ConfigManager.h"
#include "mxMessageDialog.h"

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
	
	mxDialog::AddStaticText(tSizer,this,LANG(COMPLEMENTS_WHATIS,""
		"Los complementos consisten en packs de archivos adicionales\n"
		"para ZinjaI, que pueden incluir bibliotecas, documentación,\n"
		"temas de íconos, plantillas de proyectos, indices para el \n"
		"autocompletado, etc."
		));
	mxDialog::AddStaticText(tSizer,this,LANG(COMPLEMENTS_INSTRUCCIONS,""
		"Para instalar un complemento debe: 1) descargarlo desde la\n"
		"sección de descargas del sitio de ZinjaI (haga click\n"
		"en el botón \"Descargar...\" para ir al sitio); 2) presionar\n"
		"\"Instalar...\"; 3) seleccionar el archivo descargado.\n"
		));
	
	iSizer->Add(new wxStaticBitmap(this,wxID_ANY, bitmaps->GetBitmap("upgrade.png")),sizers->BA10);
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
	
	wxFileDialog dlg (this, LANG(COMPLEMENTS_CAPTION,"Instalación de Complementos"), config->Files.last_dir, " ", _T("Complement files (zcp)|*.zcp;*.ZCP|Any file (*)|*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_OK) {
		wxFileName fname=dlg.GetPath();
		config->Files.last_dir=fname.GetPath();
		Install(dlg.GetPath());
	}
	Hide();
	Destroy();
}

void mxComplementInstallerWindow::OnHelpButton (wxCommandEvent & evt) {
	mxHelpWindow::ShowHelp("complements.html");
}

void mxComplementInstallerWindow::OnDownloadButton (wxCommandEvent & evt) {
	mxUT::OpenZinjaiSite("downextras.php");
}

void mxComplementInstallerWindow::OnCancelButton (wxCommandEvent & evt) {
	Close();
}

void mxComplementInstallerWindow::OnClose (wxCloseEvent & evt) {
	Destroy();
}

void mxComplementInstallerWindow::Install(wxString fname) {
	bool writable=true;
	wxString wtestf=DIR_PLUS_FILE(config->zinjai_dir,"complement.tmp");
	if (wxFileName::FileExists(wtestf)) {
		remove(wtestf.c_str());
		if (wxFileName::FileExists(wtestf)) writable=false;
	}
	if (writable) {
		ofstream of(wtestf.c_str(),ios::trunc);
		of<<"temporary file for testing writability"<<endl;
		of.close();
		if (!wxFileName::FileExists(wtestf)) writable=false;
	}
	wxRemoveFile(wtestf);
	
	wxString caller,installer=
#ifdef __WIN32__
			"complement.exe"
#else
			"complement.bin"
#endif
		;
	
	if (!writable) {
		if (mxMessageDialog(this,LANG(COMPLEMENTS_SUDO_WARNING,"ZinjaI está instalado en un directorio para el cual su usuario no\n"
							 "tiene permisos de escritura. Se le solicitará confirmación y/o contraseña\n"
							 "de root/administrador para continuar con la instalación. ¿Continuar?"),
							 LANG(GENERAL_WARNING,"Advertencia"),mxMD_YES_NO|mxMD_INFO).ShowModal()==mxMD_NO)
			return;
#ifdef __WIN32__
		caller = DIR_PLUS_FILE(config->zinjai_bin_dir,"complement_wrap.exe");
#else
		wxString gksu = mxUT::GetOutput("gksu --version",true);
		if (gksu.Contains("--message"))
			caller = "gksu";
		else {
			caller<<config->Files.terminal_command;
			caller.Replace("${TITLE}",LANG(COMPLEMENTS_CAPTION,"Instalacion de Complementos"));
			caller<<" "<<mxUT::Quotize(DIR_PLUS_FILE(config->zinjai_bin_dir,"complement_wrap.bin"));
		}
#endif
	}
	
	wxCopyFile(DIR_PLUS_FILE(config->zinjai_bin_dir,installer),DIR_PLUS_FILE(config->temp_dir,installer),true);
	wxString zdir=config->zinjai_dir;
#ifdef __WIN32__
	if (zdir.Last()=='\\') zdir.RemoveLast(); // wx parsea mal los argumentos, si uno termina en \ lo pega con el que sigue
#endif
	wxString command = mxUT::Quotize(DIR_PLUS_FILE(config->temp_dir,installer))+" --lang="+config->Init.language_file+" "+mxUT::Quotize(zdir)+" "+mxUT::Quotize(fname);
#ifdef __WIN32__
	if (!writable) {
		char *cmd=new char[command.Len()+1];
		char *ccaller=new char[caller.Len()+1];
		strcpy(cmd,command.c_str());
		strcpy(ccaller,caller.c_str());
			
		SHELLEXECUTEINFO sinfo;
		memset(&sinfo, 0, sizeof(SHELLEXECUTEINFO));
		sinfo.cbSize       = sizeof(SHELLEXECUTEINFO);
		sinfo.fMask        = 0;
		sinfo.hwnd         = nullptr;
		sinfo.lpFile       = ccaller;
		sinfo.lpParameters = cmd;
		sinfo.lpVerb       = "runas"; // <<-- this is what makes a UAC prompt show up
		sinfo.nShow        = SW_NORMAL;
		// The only way to get a UAC prompt to show up
		// is by calling ShellExecuteEx() with the correct
		// SHELLEXECUTEINFO struct.  Non privlidged applications
		// cannot open/start a UAC prompt by simply spawning
		// a process that has the correct XML manifest.
		/*BOOL result =*/ ShellExecuteEx(&sinfo);
		
	} else
#endif
	wxExecute(caller.Len()?caller+" "+mxUT::EscapeString(command,true):command);
}

