#include <wx/bitmap.h>
#include <wx/textfile.h>
#include "mxUpdatesChecker.h"
#include "ids.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include "ConfigManager.h"
#include "mxBitmapButton.h"
#include "mxHelpWindow.h"
#include "mxSizers.h"
#include "Language.h"
#include "version.h"
#include "mxArt.h"
#include "mxTextDialog.h"

BEGIN_EVENT_TABLE(mxUpdatesChecker, wxDialog)
	EVT_BUTTON(wxID_CANCEL,mxUpdatesChecker::OnCloseButton)
	EVT_BUTTON(wxID_OK,mxUpdatesChecker::OnChangesButton)
	EVT_BUTTON(wxID_FIND,mxUpdatesChecker::OnProxyButton)
	EVT_CLOSE(mxUpdatesChecker::OnClose)
	EVT_END_PROCESS(wxID_ANY,mxUpdatesChecker::OnProcessEnds)
END_EVENT_TABLE()

wxProcess *mxUpdatesChecker::process=NULL;
	
mxUpdatesChecker::mxUpdatesChecker(bool show) : wxDialog(main_window, wxID_ANY, LANG(UPDATE_CAPTION,"Buscar Actualizaciones"), wxDefaultPosition, wxSize(450,200) ,wxALWAYS_SHOW_SB | wxALWAYS_SHOW_SB | wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER) {

	wxBoxSizer *iSizer = new wxBoxSizer(wxHORIZONTAL);
	
	done=false; shown=show;
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	check = new wxCheckBox(this,wxID_ANY,LANG(UPDATE_CHECK_AT_STARTUP,"Verificar siempre al iniciar"));
	check->SetValue(config->Init.check_for_updates);
	
//	wxBoxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
//	pSizer->Add(new wxStaticText(this,wxID_ANY,LANG(UPDATE_PROXY_DIR,"Direccion Proxy: "),wxDefaultPosition,wxDefaultSize),sizers->BA5_Exp0);
//	pSizer->Add(proxy_dir = new wxTextCtrl(this,wxID_ANY,""),sizers->BA5_Exp1);
//	pSizer->Add(new wxStaticText(this,wxID_ANY,LANG(UPDATE_PROXY_PORT,"   Puerto: "),wxDefaultPosition,wxDefaultSize),sizers->BA5_Exp0);
//	pSizer->Add(proxy_port = new wxTextCtrl(this,wxID_ANY,""),sizers->BA5_Exp0);
	
	text = new wxStaticText(this,wxID_ANY,LANG(UPDATE_CONSULTING_WEB,"Consultando web..."),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE|wxST_NO_AUTORESIZE);
	mySizer->AddStretchSpacer();
	mySizer->Add(text,sizers->BA5_Exp0);
	mySizer->AddStretchSpacer();
	mySizer->Add(check,sizers->BA5);
//	mySizer->Add(pSizer,sizers->BA5);
	
	close_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CLOSE_BUTTON,"&Cerrar"));
	changes_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(UPDATE_GOTO_SITE,"Ir al &sitio..."));
	proxy_button = new mxBitmapButton (this,wxID_FIND,bitmaps->buttons.ok,LANG(UPDATE_CONF_PROXY,"Conf. Proxy..."));
	buttonSizer->Add(changes_button,sizers->BA5_Right);
	buttonSizer->Add(proxy_button,sizers->BA5_Right);
	buttonSizer->Add(close_button,sizers->BA5_Right);
	mySizer->Add(buttonSizer,sizers->Right);
	
	iSizer->Add(new wxStaticBitmap(this,wxID_ANY, wxBitmap(SKIN_FILE(_T("upgrade.png")), wxBITMAP_TYPE_PNG)),sizers->BA10);
	iSizer->Add(mySizer,sizers->Exp1);
	SetSizer(iSizer);
	changes_button->Hide();
	if (show) { 
		Show();
		close_button->SetFocus();
		wxYield();
	}
	CheckNow();
}

void mxUpdatesChecker::CheckNow() {
	
	text->SetLabel(LANG(UPDATE_CONSULTING_WEB,"Consultando web..."));

#ifdef __WIN32__
	wxString command("updatem.exe ");
#else
	wxString command("./updatem.bin ");
#endif
	
	command<<"--version "<<VERSION<<" ";
#if defined(__WIN32__) || defined(__WIN64__)
	command<<"--postfix windows ";
#elif defined(__APPLE__)
	command<<"--postfix mac ";
#elif defined(__linux__)
	command<<"--postfix linux ";
#endif
	
	if (config->Init.proxy.Len())
		command<<"--proxy "<<config->Init.proxy<<" ";
	
	wxString temp_file(DIR_PLUS_FILE(config->temp_dir,"updatem.res"));
	command<<"--child \""<<temp_file<<"\"";
	command<<" zinjai ";
	
	if (process) process->Detach();
	process = new wxProcess(this->GetEventHandler(),wxID_ANY);
	if (wxExecute(command,wxEXEC_ASYNC,process)<=0) {
		if (shown) {
			text->SetLabel(LANG(UPDATE_ERROR_CONNECTING,"Error al conectarse al servidor."));
		} else 
			Destroy();
	}
}

void mxUpdatesChecker::OnClose(wxCloseEvent &evt) {
	config->Init.check_for_updates = check->GetValue();
	if (process) process->Detach();
	process=NULL;
	Destroy();
}

void mxUpdatesChecker::OnProxyButton(wxCommandEvent &evt) {
	wxString res;
	if (mxGetTextFromUser(res,LANG(UPDATE_PROXY_ASK,"Ingrese la direccion del proxy ( ej: 192.168.0.120:3128 ):"), LANG(UPDATE_CAPTION,"Buscar Actualizaciones") , config->Init.proxy, this)) {
		config->Init.proxy=res;
		CheckNow();
	}
}

void mxUpdatesChecker::OnCloseButton(wxCommandEvent &evt) {
	Close();
}

void mxUpdatesChecker::OnChangesButton(wxCommandEvent &evt) {
//#ifdef DEBUG
//	wxExecute(config->Files.browser_command+_T(" http://127.0.0.1/zinjai"));
//#else
	if (config->Init.language_file=="spanish")
		utils->OpenInBrowser(_T("http://zinjai.sourceforge.net?page=descargas.php"));
	else
		utils->OpenInBrowser(_T("http://zinjai.sourceforge.net?page=descargas_en.php"));
//#endif
	Close();
}

void mxUpdatesChecker::BackgroundCheck() {
	
	wxString cur_date(wxDateTime::Now().Format("%Y%m%d"));
	wxString temp_file(DIR_PLUS_FILE(config->temp_dir,"updatem.ts"));
	wxTextFile fil(temp_file);
	if (fil.Exists() && fil.Open() && fil.GetLineCount() && fil.GetFirstLine()==cur_date) {
		fil.Close();
		return;
	}
	fil.Close(); fil.Create(); fil.Clear();
	fil.AddLine(cur_date);
	fil.Write();
	fil.Close();
	
	main_window->SetStatusText(LANG(UPDATE_CONSULTING_STATUS,"Buscar actualizaciones: Consultando..."));
	
	new mxUpdatesChecker(false);
}

void mxUpdatesChecker::OnProcessEnds(wxProcessEvent &evt) {
	delete process;
	wxString temp_file(DIR_PLUS_FILE(config->temp_dir,"updatem.res"));
	wxTextFile fil(temp_file);
	if (!fil.Exists() || !fil.Open() || !fil.GetLineCount()) {
		main_window->SetStatusText(LANG(UPDATE_ERROR_CONNECTING_STATUS,"Buscar actualizaciones: Error al conectarse al servidor."));
		text->SetLabel(LANG(UPDATE_ERROR_CONNECTING,"Error al conectarse al servidor."));
		GetSizer()->Layout();
		if (!shown) Destroy();
		fil.Close();
		return;
	}
	wxString res=fil.GetFirstLine();
	if (res=="nonews") {
		main_window->SetStatusText(LANG(UPDATE_NO_NEW_VERSION_STATUS,"Buscar actualizaciones: No hay versiones nuevas disponibles."));
		text->SetLabel(LANG(UPDATE_NO_NEW_VERSION,"No hay nuevas versiones disponibles."));
		GetSizer()->Layout();
		if (!shown) Destroy();
	} else if (res=="update") {
		wxString str;
		str<<LANG(UPDATE_NEW_VERSION_IN_SITE_PRE,"Hay una nueva version disponible en\nhttp://zinjai.sourceforge.net (")<<fil.GetNextLine()<<LANG(UPDATE_NEW_VERSION_IN_SITE_POST,")");
		text->SetLabel(str);
		proxy_button->Hide();
		changes_button->Show();
		GetSizer()->Layout();
		if (!shown) Show();
		changes_button->SetFocus();
	} else {
		text->SetLabel(LANG(UPDATE_ERROR_CONNECTING,"Error al conectarse al servidor."));
	}
	fil.Close();
}
