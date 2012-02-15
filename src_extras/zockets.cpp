#include "zockets.h"
#include <iostream>
#include <cstring>
using namespace std;

#if defined(_WIN32) || defined(__WIN32__)
#else
void Sleep(int delay) {
	clock_t t1 = clock();
	while (float(clock()-t1)/CLOCKS_PER_SEC<float(delay)/1000);
}
#endif


ZOCKET zocket_llamar(short int port, const char *host) {
#if defined(_WIN32) || defined(__WIN32__)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1,0),&wsaData))
		return ZOCKET_ERROR;
#endif
	
	struct hostent *hp;
	hp=(struct hostent *)gethostbyname(host?host:"localhost");
	
	if(!hp)
		return ZOCKET_ERROR;
	
	ZOCKET conn_socket=socket(AF_INET,SOCK_STREAM, 0);
	if(conn_socket==ZOCKET_ERROR)
		return ZOCKET_ERROR;
	
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server)) ;
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_family = hp->h_addrtype;
	server.sin_port = htons(port);
	
	if(connect(conn_socket,(struct sockaddr *)&server,sizeof(server))==ZOCKET_ERROR){
#if defined(_WIN32) || defined(__WIN32__)
		closesocket(conn_socket);
		WSACleanup();
#else
		close(conn_socket);
#endif
		return ZOCKET_ERROR;
	}
#if defined(_WIN32) || defined(__WIN32__)
	WSAAsyncSelect (conn_socket, NULL, 0 ,0);
	unsigned long ln = 1;
	ioctlsocket(conn_socket,FIONBIO,&ln);
#else
	fcntl(conn_socket,F_SETFL,O_NONBLOCK);
#endif
	return conn_socket;
	
}

ZOCKET zocket_esperar_llamada(short int port) {
#if defined(_WIN32) || defined(__WIN32__)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1,0),&wsaData))
		return ZOCKET_ERROR;
#endif
#if defined(_WIN32) || defined(__WIN32__)
	int desc_srv = socket (AF_INET, SOCK_STREAM, 0);
#else
	int desc_srv = socket (AF_UNIX, SOCK_STREAM, 0);
#endif
	if (desc_srv == -1)
		return ZOCKET_ERROR;
	struct hostent *hp=(struct hostent *)gethostbyname("localhost");
	if (!hp)
		return ZOCKET_ERROR;
	struct sockaddr_in dir;
	memset(&dir, 0, sizeof(dir));
	memcpy(&dir.sin_addr, hp->h_addr, hp->h_length);
	dir.sin_family = AF_INET;
	dir.sin_port = htons(port);
	//	dir.sin_addr.s_addr = INADDR_ANY;
	if (bind (desc_srv, (struct sockaddr *)&dir, sizeof (dir)) == -1)
		return ZOCKET_ERROR;
	if (listen(desc_srv,1)==-1)
		return ZOCKET_ERROR;
	struct sockaddr cli;
	memset(&cli, 0, sizeof(cli));
	socklen_t long_cli=sizeof(struct sockaddr);
	cerr<<"Presione aceptar en la ventana de PSeInt para comenzar la ejecucion paso a paso"<<endl;
	int desc_cli = accept (desc_srv, &cli, &long_cli);
	if (desc_cli == -1)
		return ZOCKET_ERROR;
	return desc_cli;
}


bool zocket_leer(ZOCKET z, char *data, int &cant) {
	int n = recv (z, data, cant, 0);
	if (n==ZOCKET_ERROR) {
		cant=0;
		return false;
	}
	if (n>0) {
		cant=n;
		return true;
	} else {
		cant=0;
		return false;
	}
} 

bool zocket_escribir(ZOCKET z, const char *data, int cant) {
	int es=0,n;
	do {
		n = send (z, data+es, cant, 0);
		if (n>0) 
			es+=n;
		else
			return false;
	} while (es<cant);
	return true;
} 

void zocket_cerrar(ZOCKET z) {
#if defined(_WIN32) || defined(__WIN32__)
	closesocket(z);
	WSACleanup();
#else
	close(z);
#endif
}

