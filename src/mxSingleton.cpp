#include <iostream>
using namespace std;
#include "mxSingleton.h"
#include "ConfigManager.h"
#include "mxMainWindow.h"

BEGIN_EVENT_TABLE(mxSingleton,wxEvtHandler)
	EVT_SOCKET(wxID_ANY,mxSingleton::OnSocket)
END_EVENT_TABLE()

mxSingleton *g_singleton=nullptr;

mxSingleton::mxSingleton():m_server(nullptr) {
	m_calls_count=0;
	m_data=nullptr;
	m_gid=0;
	m_ready=false;
}

mxSingleton::~mxSingleton() {
	if (m_server) m_server->Destroy();
	if (m_data) free(m_data);
}

bool mxSingleton::Start() {
	wxIPV4address adrs;
	adrs.Hostname(_T("127.0.0.1")); // esta linea era el problema por el cual no se aceptaban conexiones externas
	adrs.Service(config->Init.zinjai_server_port+1);
	m_server = new wxSocketServer(adrs);
	m_server->SetEventHandler(*this, wxID_ANY);
	m_server->SetNotify(wxSOCKET_CONNECTION_FLAG);
	m_server->Notify(true);
	if (!m_server->IsOk()) {
		delete m_server;
		m_server=nullptr;
	}
	return m_server!=nullptr;
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
	if (!m_ready) { 
		m_to_open.Add(fname);
	} else {
		main_window->OpenFileFromGui(fname);
		main_window->Raise();
	}
}

void mxSingleton::ProcessToOpenQueue() {
	m_ready = true;
	int n = m_to_open.GetCount();
	if (!n) return;
	for(int i=0;i<n;i++) { 
		main_window->OpenFileFromGui(m_to_open[i]);
	}
	m_to_open.Clear();
	main_window->Raise();
}

void mxSingleton::Stop() {
	if (m_server) delete m_server;
	m_server=nullptr;
}

void mxSingleton::OnSocket(wxSocketEvent &evt) {
	wxSocketBase *who = evt.GetSocket();
	wxSocketNotify what = evt.GetSocketEvent();
	if (who==m_server) {
		wxSocketBase *sock = m_server->Accept(false);
		if (m_data) {
			if (m_calls_count+1==m_max_count) {
				m_max_count*=2;
				// cppcheck-suppress memleakOnRealloc
				m_data = (singleton_data*)realloc(m_data,sizeof(singleton_data)*m_max_count);
			}	
		} else {
			m_max_count=10;
			m_data = (singleton_data*)malloc(sizeof(singleton_data)*m_max_count);
		}
		m_data[m_calls_count].socket=sock;
		m_data[m_calls_count].data=new wxString;
		m_data[m_calls_count].id=++m_gid;
		m_calls_count++;
		sock->SetEventHandler(*this, wxID_ANY);
		sock->SetNotify(wxSOCKET_LOST_FLAG|wxSOCKET_INPUT_FLAG);
		sock->Notify(true);
	} else {
		int cnum=0; // buscar cual llamada es
		while (cnum<m_calls_count && m_data[cnum].socket!=who) cnum++;
		if (cnum==m_calls_count) return;
		if (what==wxSOCKET_INPUT) {
			wxString mdat=(*m_data[cnum].data);
			int mid = m_data[cnum].id;
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
			if (mid==m_data[cnum].id) (*m_data[cnum].data)=mdat;
		} else if (what==wxSOCKET_LOST) {
			delete m_data[cnum].data;
			m_data[cnum].id=0;
			m_calls_count--;
			if (cnum!=m_calls_count && m_calls_count) {
				m_data[cnum]=m_data[m_calls_count];
			}
			delete who;
		}
	}
}

bool mxSingleton::IsRunning() {
	return m_server!=nullptr;
}
