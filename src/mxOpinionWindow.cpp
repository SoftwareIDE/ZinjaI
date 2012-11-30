#include "mxOpinionWindow.h"
#include "mxMainWindow.h"
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/textdlg.h>
#include "mxSizers.h"
#include "mxBitmapButton.h"
#include "mxMessageDialog.h"
#include "mxOSD.h"
#include "version.h"
#include "mxUtils.h"
#include "Language.h"
#include "ConfigManager.h"
#include "mxTextDialog.h"
#include <wx/statbmp.h>

BEGIN_EVENT_TABLE(mxOpinionWindow, wxDialog)
	EVT_SOCKET(wxID_ANY,mxOpinionWindow::OnSocketEvent)
	EVT_BUTTON(wxID_CANCEL,mxOpinionWindow::OnCloseButton)
	EVT_BUTTON(wxID_OK,mxOpinionWindow::OnOkButton)
	EVT_BUTTON(wxID_FIND,mxOpinionWindow::OnProxyButton)
	EVT_CLOSE(mxOpinionWindow::OnClose)
END_EVENT_TABLE()
	
mxOpinionWindow::mxOpinionWindow():wxDialog(main_window,wxID_ANY,LANG(OPINION_CAPTION,"Envia tu mensaje"),wxDefaultPosition,wxDefaultSize) {
	
	osd=NULL;
	
	the_socket=NULL;
	
	wxBoxSizer *sizerU = new wxBoxSizer(wxHORIZONTAL);
	
	sizerU->Add(new wxStaticBitmap(this,wxID_ANY, wxBitmap(SKIN_FILE(_T("feedback.png")), wxBITMAP_TYPE_PNG)), sizers->BA10);
	
	wxBoxSizer *mySizer = new wxBoxSizer(wxVERTICAL);
	
	mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(OPINION_TEXT,""
		"Desde esta ventana puedes enviar un mensaje directamente al desarrollador de ZinjaI.\n"
		"Puedes utilizar este espacio para sugerir mejoras, comentar que te gusta y que no del\n"
		"entorno, reportar errores, o cualquier otro comentario. Los mensajes de los usuarios\n"
		"sirven para saber que aspectos de ZinjaI son utiles y funcionan bien, y que cosas se\n"
		"deben mejorar, agregar o quitar, y son tenidos en cuenta para el desarrollo de nuevas\n"
		"versiones."
		),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE|wxST_NO_AUTORESIZE),sizers->BA5_Exp0);
	text = new wxTextCtrl(this,wxID_ANY,_T(""),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_TAB|wxTE_MULTILINE|wxTE_PROCESS_ENTER);
	mySizer->Add(text,sizers->BA5_Exp1);
	mySizer->Add(new wxStaticText(this,wxID_ANY,LANG(OPINION_EMAIL,""
		"Si esperas recibir alguna respuesta directa no olvides incluir tu email en el mensaje."
		),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE|wxST_NO_AUTORESIZE),sizers->BA5_Exp0);
	

	wxBoxSizer *sizerB = new wxBoxSizer(wxHORIZONTAL);
	wxBitmapButton *cancel_button = new mxBitmapButton (this,wxID_CANCEL,bitmaps->buttons.cancel,LANG(GENERAL_CLOSE_BUTTON,"Cerrar"));
	send_button = new mxBitmapButton (this,wxID_OK,bitmaps->buttons.ok,LANG(OPINION_SEND_BUTTON,"Enviar"));
	wxBitmapButton *proxy_button = new mxBitmapButton (this,wxID_FIND,bitmaps->buttons.ok,LANG(UPDATE_CONF_PROXY,"Conf. Proxy..."));
	sizerB->Add(proxy_button,sizers->BA10);
	sizerB->AddStretchSpacer(1);
	sizerB->Add(cancel_button,sizers->BA10);
	sizerB->Add(send_button,sizers->BA10);
	mySizer->Add(sizerB,sizers->Right);
	
	sizerU->Add(mySizer, sizers->BA10_Exp1);
	SetEscapeId(wxID_CANCEL);
	SetSizerAndFit(sizerU);
	Show();
	text->SetFocus();
}

void mxOpinionWindow::OnSocketEvent(wxSocketEvent &event) {
	if (event.GetSocket()!=the_socket) return;
	if (event.GetSocketEvent()==wxSOCKET_LOST) {
		if (osd) {
			osd->Hide();
			osd->Destroy();
			osd=NULL;
		}
		the_socket->Notify(false);
		the_socket->Destroy();
		the_socket=NULL;
		send_button->Enable();
	} else if (event.GetSocketEvent()==wxSOCKET_CONNECTION) {
		wxString content("sng=Zinjai-");
		wxString message=text->GetValue();
		message.Replace("\n","<BR>");
		message.Replace("\t","&nbsp;&nbsp;&nbsp;&nbsp;");
		content<<utils->UrlEncode(ARCHITECTURE)<<"-"<<VERSION<<"&msg="<<utils->UrlEncode(message);
		wxString request("POST ");
		if (proxy.Len()) request<<"http://zinjai.sourceforge.net";
		request
			<<"/mensajes.php"
			<<" HTTP/1.1\n"
				"host:zinjai.sourceforge.net\n"
				"From: user@zinjai\n"
				"User-Agent: zinjai\n"
				"Content-Type: application/x-www-form-urlencoded\n"
				"Content-Length: ";
		request<<content.Len()<<"\n\n"<<content<<"\n\n";
		the_socket->Write(request.c_str(),request.Len());
	} else if (event.GetSocketEvent()==wxSOCKET_INPUT) {
		wxChar read_chars[1024];
		the_socket->Read(read_chars,1024);
		read_chars[the_socket->LastCount()]='\0';
		wxString str(read_chars);
		if (str.Contains("ha sido enviado")) {
			if (osd) {
				osd->Hide();
				osd->Destroy();
				osd=NULL;
			}
			mxMessageDialog(this,LANG(OPINION_MESSAGE_SENT,"El mensaje ha sido enviado correctamente.\nGracias por aportar su opinion."),
				LANG(OPINION_SENT_CAPTION,"Mensaje enviado"),mxMD_OK|mxMD_INFO).ShowModal();;
			if (the_socket) {
				the_socket->Notify(false);
				the_socket->Destroy();
				the_socket=NULL;
			}
			send_button->Enable();
		}
	}
}
	
void mxOpinionWindow::OnClose(wxCloseEvent &evt) {
	if (osd) {
		osd->Hide();
		osd->Destroy();
		osd=NULL;
	}
	if (the_socket) {
		the_socket->Notify(false);
		the_socket->Destroy();
		the_socket=NULL;
	}
	Destroy();
}

void mxOpinionWindow::OnOkButton(wxCommandEvent &evt) {
	
	if (text->GetValue().Len()==0) {
		mxMessageDialog(this,LANG(OPINION_MESSAGE_EMPTY,"No se puede enviar un mensaje en blanco."),LANG(GENERAL_WARNING,"Advertencia"),mxMD_OK|mxMD_WARNING).ShowModal();;
		return;
	}
	
	proxy=config->Init.proxy;
	port=80;
	if (proxy.Contains(":")) { 
		proxy.AfterLast(':').ToLong(&port);
		proxy=proxy.BeforeLast(':');
	}
	
	osd = new mxOSD(this,LANG(OPINION_SENDING_OSD,"Enviando mensaje..."));
	send_button->Disable();
	wxIPV4address adrs;
	if (!adrs.Hostname(proxy.Len()?proxy:"zinjai.sourceforge.net")) {
		mxMessageDialog(this,LANG(OPINION_ADDRESS_ERROR,"No se pudo contactar al servidor. Puede deberse a problemas\n"
							"de conexion o la presencia de un proxie. Si el problema persiste\n"
							"puede enviar el mensaje desde su navegador accediendo a la\n"
							"seccion Contactos del sitio web de ZinjaI.\n"
							"http://zinjai.sourceforge.net"),
							LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_WARNING).ShowModal();;
	} else {
		adrs.Service(proxy.Len()?port:80);
		the_socket = new wxSocketClient(wxSOCKET_NOWAIT);
		the_socket->SetEventHandler(*(GetEventHandler()), wxID_ANY);
		the_socket->SetNotify(wxSOCKET_CONNECTION_FLAG|wxSOCKET_LOST_FLAG|wxSOCKET_INPUT_FLAG);
		the_socket->Notify(true);
		the_socket->Connect(adrs,false);
	}
}

void mxOpinionWindow::OnCloseButton(wxCommandEvent &evt) {
	Close();	
}

void mxOpinionWindow::OnProxyButton(wxCommandEvent &evt) {
	wxString res;
	if (mxGetTextFromUser(res,LANG(UPDATE_PROXY_ASK,"Ingrese la direccion del proxy ( ej: 192.168.0.120:3128 ):"), LANG(UPDATE_CAPTION,"Buscar Actualizaciones") , config->Init.proxy, this))
		config->Init.proxy=res;
}
