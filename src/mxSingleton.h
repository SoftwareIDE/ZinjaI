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
	wxSocketServer *server;
	singleton_data *data;
	int calls_count, max_count, gid;
public:
	mxSingleton();
	bool Start();
	void Stop();
	void OnSocket(wxSocketEvent &evt);
	void LocalOpen(wxString fname);
	bool RemoteOpen(wxString fname);
	bool IsRunning();
	~mxSingleton();
	DECLARE_EVENT_TABLE();
};

extern mxSingleton *singleton;

#endif

