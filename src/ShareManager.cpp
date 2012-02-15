#include "ids.h"
#include "ShareManager.h"
#include "mxSource.h"
#include "mxMainWindow.h"
#include "mxUtils.h"
#include <wx/socket.h>
#include <wx/ctrlsub.h>

#define SHARE_SEND_BS 1024
#include "Language.h"
#include "mxMessageDialog.h"

ShareManager *share=NULL;

/*
	Breve explicacion del protocolo:
	hay dos sockets especiales:
	index_server: a el se le hacen todos los pedidos
	index_client: mediante el se recupera la lista de compartidos
	
	
	Para pasar la lista de compartidos:
		El cliente se conecta mediante su index_client al index_server del servidor.
		El servidor crea un nuevo socket para atender la conexion.
		El cliente envia un l para indicar que quiere la lista.
		El servidor en un nuevo socket atiende al cliente, y al recibir la l envia la cnatidad de archivos que comparte y sus nombres separados por \n.
		El cliente espera a que llegue la cantidad de nombres indicados y luego se encarga de cerrar la conexion.
	
	Para pedir un fuente
		El cliente se conecta mediante un nuevo socket creado para tal fin con el index_server del servidor.
		El servidor crea un nuevo socket para atender la conexion.
		El cliente envia un * y el nombre del fuente que desea recibir
		El cliente, envia la longitud del fuente, y el contenido.
		El cliente recibe el fuente, y se encarga de cerrar la conexion.
	
	Para distinguir si una entrada por un socket es un pedido o una fuente que se pidio, los sockets que corresponden a fuentes que se espera recibir se encuentran en el hash map clients, por lo tanto al llegar la entrada, si no esta en clients, seguramente es porque fue creado por el index_server para atender una conexion.
*/


ShareManager::ShareManager () {
	index_is_waiting=false;
	index_server=NULL;
//	if (config->Init.load_sharing_server)
//		CheckServer();
}

bool ShareManager::CheckServer() {
	if (index_server!=NULL) {
		if(index_server->IsOk())
			return true;
		else {
			index_server->Destroy();
			index_server=NULL;
		}
	}
	wxIPV4address adrs;
//	adrs.Hostname(_T("127.0.0.1")); // esta linea era el problema por el cual no se aceptaban conexiones externas
	adrs.Service(config->Init.zinjai_server_port);
	index_server = new wxSocketServer(adrs);
	index_server->SetEventHandler(*(main_window->GetEventHandler()), mxID_SOCKET_SERVER);
	index_server->SetNotify(wxSOCKET_CONNECTION_FLAG);
	index_server->Notify(true);
	return index_server->IsOk();
}

ShareManager::~ShareManager () {
}

bool ShareManager::AskFor(wxString name) {
	wxIPV4address adrs;
	if (!adrs.Hostname(list_hostname)) {
		return false;
	}
	adrs.Service(config->Init.zinjai_server_port);
	wxSocketClient *sock = new wxSocketClient(wxSOCKET_NOWAIT);
	sock->Connect(adrs,false);
	sock->SetEventHandler(*(main_window->GetEventHandler()), wxID_ANY);
	sock->SetNotify(wxSOCKET_CONNECTION_FLAG|wxSOCKET_LOST_FLAG|wxSOCKET_INPUT_FLAG);
	sock->Notify(true);
	share_conection_c scc;
	scc.len=-1;
	scc.count=0;
	scc.name=name;
	clients[sock]=scc;
	return true;
}

bool ShareManager::Exists(wxString name) {
	return (data.count(name)!=0);
}

bool ShareManager::Exists(mxSource *source) {
	HashStringSource::iterator it;
	for( it = data.begin(); it != data.end(); ++it )
		if ((void*)source==it->second.source)
			return true;
	return false;
}


void ShareManager::Delete(wxString name) {
	data.erase(name);
}

void ShareManager::Delete(mxSource *source) {
	HashStringSource::iterator it;
	it = data.begin();
	while (it!=data.end()) {
		if ((void*)source==it->second.source) {
			data.erase(it);
			it=data.begin();
		} else
			it++;
	}
}

int ShareManager::GetMyList(wxControlWithItems *ctrl) {
	HashStringSource::iterator it;
	ctrl->Clear();
	for( it = data.begin(); it != data.end(); ++it )
		ctrl->Append(it->first);
	return data.size();
}

void ShareManager::Insert(wxString name, mxSource *source, bool freezed) {
	source_share ss;
	if (freezed) {
		ss.source=(void*) new wxChar[source->GetLength()+1];
		ss.len=0;
		for (int l,i=0;i<source->GetLineCount();i++) {
			if ((l=source->GetLine(i).Len())!=0){
				utils->strcpy(((wxChar*)ss.source)+ss.len,source->GetLine(i));
				ss.len+=l;
			}
		}
	} else {
		ss.source=source;
		ss.len=-1;
	}
	data[name]=ss;	
}

void ShareManager::Replace(wxString name, mxSource *source, bool freezed) {
	source_share ss=data[name];
	if (ss.len!=-1)
		delete [] (wxChar*)(ss.source);
	if (freezed) {
		ss.len=source->GetLength();
		ss.source=(void*) new wxChar[ss.len];
		int c=0;
		for (int i=0;i<source->GetLineCount();i++) {
			utils->strcpy(((wxChar*)ss.source)+c,source->GetLine(i));
			c+=source->GetLine(i).Len();
		}
	} else {
		ss.len=-1;
		ss.source=source;
	}
	data[name]=ss;
}

bool ShareManager::GetList(wxString hostname, wxListBox *lst) {
	if (index_is_waiting) {
		index_client->Notify(false);
		index_client->Destroy();
		index_client=NULL;
	} else
		index_is_waiting=true;
	list_hostname=hostname;
	my_list=lst;
	my_list->Clear();
	wxIPV4address adrs;
	if (!adrs.Hostname(hostname)) {
		index_is_waiting=false;
		return false;
	}
	adrs.Service(config->Init.zinjai_server_port);
	index_client = new wxSocketClient(wxSOCKET_NOWAIT);
	index_client->Connect(adrs,false);
	index_client->SetEventHandler(*(main_window->GetEventHandler()), wxID_ANY);
	index_client->SetNotify(wxSOCKET_CONNECTION_FLAG|wxSOCKET_LOST_FLAG|wxSOCKET_INPUT_FLAG);
	index_client->Notify(true);
	list_tot=-1;
	return true;
}

void ShareManager::OnSocketEvent(wxSocketEvent *event) {
	HashPtrClient::iterator it;
	
//wxString se;
//se<<"GetSocket:"<<(int)event->GetSocket()<<"   is:"<<(int)index_server<<" ic:"<<(int)index_client;
//wxString set;
//if (event->GetSocketEvent()==wxSOCKET_LOST) set+="LOST ";
//if (event->GetSocketEvent()==wxSOCKET_INPUT) set+="INPUT ";
//if (event->GetSocketEvent()==wxSOCKET_CONNECTION) set+="CONNECTION ";
//DEBUG_INFO(set);
//DEBUG_INFO(se);
	
	// someone wants to start a conversation
	if (event->GetSocket()==index_server) { // someone ask for the shared sources list
		wxSocketBase *sock = index_server->Accept(false);
		sock->SetEventHandler(*(main_window->GetEventHandler()), wxID_ANY);
		sock->SetNotify(wxSOCKET_LOST_FLAG|wxSOCKET_INPUT_FLAG);
		sock->Notify(true);
	} else 
	
	// if we have to ask for the list
	if (event->GetSocket()==index_client) { // someone ask for the shared sources list
		if (event->GetSocketEvent()==wxSOCKET_CONNECTION) {
			index_client->Write(_T("l"),1);
		} else if (event->GetSocketEvent()==wxSOCKET_INPUT) {
			int n=0;
			wxChar buf[26];
			if (list_tot==-1) {
				while (n<25) {
					index_client->Read(buf+n,1);
					if (buf[n++]=='\n') break;
				}
				if (n<25) {
					buf[n]='\0';
					wxString(buf).ToLong(&(list_tot));
					if (list_tot>0) {
						list_buf=new wxChar[256*list_tot];
					}
					list_rec=list_count=list_last=0;
				}
			}
			if (list_tot>=0) {
				do { // read while we can
					index_client->Read(list_buf+list_count,SHARE_SEND_BS);
					if (!index_client->Error()) {
						list_count+=n=index_client->LastCount();
						for (int j=list_last;j<list_count;j++) 
							if (list_buf[j]=='\n') {
								list_buf[j]='\0';
								my_list->Append(wxString(list_buf+list_last));
								list_last=j+1;
								list_rec++;
							}
					} else 
						n=0;
				} while (list_rec<list_tot && n!=0);
				if (list_rec>=list_tot) { 
					index_is_waiting=false;
					index_client->Notify(false);
					index_client->Destroy();
					index_client=NULL;
				}
			} 
		} else if (event->GetSocketEvent()==wxSOCKET_LOST) {
			if (list_tot>0) delete [] list_buf;
			index_client->Destroy();
			index_client=NULL;
			index_is_waiting=false;
		}
	} else
	
	// someone sends for a piece of source or we must say witch source
	if ((it=clients.find(event->GetSocket()))!=clients.end()) { 
		wxSocketBase *sock = (wxSocketBase*)it->first;
		if (event->GetSocketEvent()==wxSOCKET_LOST) {
			if (it->second.len==-1||it->second.count<it->second.len) {
				mxMessageDialog(main_window,LANG(OPENSHARED_ERROR_GETTING_SOURCE,"Ha ocurrido un error al intentar obtener el fuente. Pruebe actualizar la lista de fuentes compartidos."), LANG(GENERAL_ERROR,"Error"),mxMD_OK|mxMD_ERROR).ShowModal();
			}
		} else if (event->GetSocketEvent()==wxSOCKET_CONNECTION) {
			event->GetSocket()->Write((wxString(_T("*"))+it->second.name).c_str(),it->second.name.Len()+1);
		} else if (event->GetSocketEvent()==wxSOCKET_INPUT) { // if comes a piece of source
			int n=0;
			if (it->second.len==-1) { // if that's the first message, get the source len
				wxChar buf[25]=_T("a");
				while (n<25) {
					sock->Read(buf+n,1);
					if (buf[n++]==' ') break;
				}
				if (n<25) {
					buf[n]='\0';
					it->second.count=0;
					wxString(buf).ToLong(&(it->second.len));
				} 
				if (it->second.len<0) { // if there was an error, delete that socket
					clients.erase(it);
					sock->Destroy();
					return;
				} else {
					it->second.source = new wxChar[it->second.len+255];
				}
			}
			// now get source text
			if (it->second.len>=0) {
				do { // read while we can
					sock->Read(it->second.source+it->second.count,SHARE_SEND_BS);
					if (!sock->Error()) {
						it->second.count+=n=sock->LastCount();
					} else
						n=0;
				} while (n!=0 && it->second.count>=it->second.len);
				// see if the transfer is complete
				if (it->second.count>=it->second.len) {
					// destroy the socket
					it->second.source[it->second.len]='\0';
					main_window->NewFileFromText(it->second.source,it->second.name,0);
					delete [] it->second.source; 
					clients.erase(it);
					sock->Destroy();
					return;
				};
			}
		} else if (event->GetSocketEvent()==wxSOCKET_LOST) {
			if (it->second.len!=-1)
				delete [] it->second.source; 
			clients.erase(it);
			event->GetSocket()->Destroy();
			return;
		}	
	} else {
		
		
	// someone asks for a piece of source or asks for shared sources list
	  
		if (event->GetSocketEvent()==wxSOCKET_INPUT) { // if ask for something
			
			// read the request
			wxChar buf[256]=_T("");
			event->GetSocket()->Read(buf,255);
			HashStringSource it;
			
			if (buf[0]=='l') { // if he wants the list
				HashStringSource::iterator it;
				wxString str=wxString()<<data.size()<<_T("\n");
				event->GetSocket()->Write(str.c_str(),str.Len());
				for (it = data.begin(); it != data.end(); ++it ) {
					event->GetSocket()->Write((it->first+_T("\n")).c_str(),it->first.Len()+1);
				}
			} else if (buf[0]=='*') { // if he wants a source
				buf[event->GetSocket()->LastCount()]='\0';
				HashStringSource::iterator it=data.find(wxString(buf+1));
				wxString str;
				if (it!=data.end()) { // if that source exists
					if (it->second.len==-1) { // if its not a freezed one
						mxSource *source = (mxSource*)it->second.source;
						str<<source->GetLength()<<_T(" ");
						event->GetSocket()->Write(str.c_str(),str.Len());
						for (int i=0;i<source->GetLineCount();i++) {
							event->GetSocket()->Write(source->GetLine(i).c_str(),source->GetLine(i).Len());
						}
					} else { // if its a freezed one
						str<<it->second.len<<_T(" ");
						event->GetSocket()->Write(str.c_str(),str.Len());
						event->GetSocket()->Write(it->second.source,it->second.len);
					}
				} else {
					event->GetSocket()->Close();
				}
			}
		
		} if (event->GetSocketEvent()==wxSOCKET_LOST) {
			event->GetSocket()->Destroy();
		}
	}
}

bool ShareManager::Freeze(mxSource *source) {
	HashStringSource::iterator it;
	for( it = data.begin(); it != data.end(); ++it )
		if ((void*)source==it->second.source) {
			Replace(it->first,source,true);
			return true;
		}
	return false;
}

int ShareManager::GetSharedList(wxArrayString &a) {
	int count=0;
	HashStringSource::iterator it;
	for( it = share->data.begin(); it != share->data.end(); ++it ) {
		a.Add(it->first); count++;
	} 
	return count;
}
