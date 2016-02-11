#ifndef MXSINGLETON_H
#define MXSINGLETON_H
#include <wx/event.h>
#include <wx/socket.h>

struct singleton_data {
	wxSocketBase *socket;
	wxString *data;
	int id;
};


class mxSingleton: public wxEvtHandler {
private:
	wxArrayString m_to_open; ///< para encolar pedidos que llegan antes de inicializar main_window
	bool m_ready; ///< para saber cuando dejar de encolar en to_open y empezar a abrir directamente
	wxSocketServer *m_server;
	singleton_data *m_data;
	int m_calls_count, m_max_count, m_gid;
public:
	mxSingleton();
	bool Start();
	void Stop();
	void OnSocket(wxSocketEvent &evt);
	void LocalOpen(wxString fname);
	void ProcessToOpenQueue();
	bool RemoteOpen(wxString fname);
	bool IsRunning();
	~mxSingleton();
	DECLARE_EVENT_TABLE();
};

extern mxSingleton *g_singleton;

#endif

