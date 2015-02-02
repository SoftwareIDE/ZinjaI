#include <iostream>
using namespace std;
#include "mxSingleton.h"
#include "ConfigManager.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxSingleton,wxEvtHandler)
	EVT_SOCKET(wxID_ANY,mxSingleton::OnSocket)
END_EVENT_TABLE()

mxSingleton *singleton=nullptr;

mxSingleton::mxSingleton():server(nullptr) {
	calls_count=0;
	data=nullptr;
	gid=0;
	ready=false;
}

mxSingleton::~mxSingleton() {
	if (server) server->Destroy();
	if (data) free(data);
}

bool mxSingleton::Start() {
	wxIPV4address adrs;
	adrs.Hostname(_T("127.0.0.1")); // esta linea era el problema por el cual no se aceptaban conexiones externas
	adrs.Service(config->Init.zinjai_server_port+1);
	server = new wxSocketServer(adrs);
	server->SetEventHandler(*this, wxID_ANY);
	server->SetNotify(wxSOCKET_CONNECTION_FLAG);
	server->Notify(true);
	if (!server->IsOk()) {
		delete server;
		server=nullptr;
	}
	return server!=nullptr;
}

bool mxSingleton::RemoteOpen(wxString fname) {
	wxIPV4address adrs;
	adrs.Hostname(_T("127.0.0.1"));
	adrs.Service(config->Init.zinjai_server_port+1);
	wxSocketClient *client = new wxSocketClient(wxSOCKET_WAITALL);
	client->SetEventHandler(*this, wxID_ANY);
	if (client->Connect(adrs,true)) {
		fname<<"\n";
		client->Write(fname.c_str(),fname.Len());
		client->Close();
		delete client;
		return true;
	} else {
		delete client;
		return false;
	}
}

void mxSingleton::LocalOpen(wxString fname) {
	if (!ready) { 
		to_open.Add(fname);
	} else {
		main_window->OpenFileFromGui(fname);
		main_window->Raise();
	}
}

void mxSingleton::ProcessToOpenQueue() {
	ready=true;
	int n=to_open.GetCount();
	if (!n) return;
	for(int i=0;i<n;i++) { 
		main_window->OpenFileFromGui(to_open[i]);
	}
	to_open.Clear();
	main_window->Raise();
}

void mxSingleton::Stop() {
	if (server) delete server;
	server=nullptr;
}

void mxSingleton::OnSocket(wxSocketEvent &evt) {
	wxSocketBase *who = evt.GetSocket();
	wxSocketNotify what = evt.GetSocketEvent();
	if (who==server) {
		wxSocketBase *sock = server->Accept(false);
		if (data) {
			if (calls_count+1==max_count) {
				max_count*=2;
				// cppcheck-suppress memleakOnRealloc
				data = (singleton_data*)realloc(data,sizeof(singleton_data)*max_count);
			}	
		} else {
			max_count=10;
			data = (singleton_data*)malloc(sizeof(singleton_data)*max_count);
		}
		data[calls_count].socket=sock;
		data[calls_count].data=new wxString;
		data[calls_count].id=++gid;
		calls_count++;
		sock->SetEventHandler(*this, wxID_ANY);
		sock->SetNotify(wxSOCKET_LOST_FLAG|wxSOCKET_INPUT_FLAG);
		sock->Notify(true);
	} else {
		int cnum=0; // buscar cual llamada es
		while (cnum<calls_count && data[cnum].socket!=who) cnum++;
		if (cnum==calls_count) return;
		if (what==wxSOCKET_INPUT) {
			wxString mdat=(*data[cnum].data);
			int mid=data[cnum].id;
			static char buf[256];
			who->Read(buf,255);
			int lc = who->LastCount();
			if (lc) {
				buf[lc]='\0';
				mdat<<buf;
			}
			while (mdat.Contains("\n")) {
				wxString fname = mdat.BeforeFirst('\n');
				mdat = mdat.AfterFirst('\n');
				if (fname.Last()=='\r') fname.RemoveLast();
				LocalOpen(fname);
			}
			// en el idle del openfromgui puede haber procesado el lost y borrado el data
			if (mid==data[cnum].id) (*data[cnum].data)=mdat;
		} else if (what==wxSOCKET_LOST) {
			delete data[cnum].data;
			data[cnum].id=0;
			calls_count--;
			if (cnum!=calls_count && calls_count) {
				data[cnum]=data[calls_count];
			}
			delete who;
		}
	}
}

bool mxSingleton::IsRunning() {
	return server!=nullptr;
}
