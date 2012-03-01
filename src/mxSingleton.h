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
	wxArrayString to_open; ///< para encolar pedidos que llegan antes de inicializar main_window
	bool ready; ///< para saber cuando dejar de encolar en to_open y empezar a abrir directamente
	wxSocketServer *server;
	singleton_data *data;
	int calls_count, max_count, gid;
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

extern mxSingleton *singleton;

#endif

