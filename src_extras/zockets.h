#ifndef ZOCKETS_H
#define ZOCKETS_H

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#include <winsock.h>
#define socklen_t int
#define ZOCKET_ERROR SOCKET_ERROR
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
void Sleep(int delay);
#define ZOCKET_ERROR -1
#endif
#define ZOCKET int
#include <cstddef>

ZOCKET zocket_llamar(short int port, const char *host=NULL);

ZOCKET zocket_esperar_llamada(short int port);


bool zocket_leer(ZOCKET z, char *data, int &cant); 

bool zocket_escribir(ZOCKET z, const char *data, int cant); 

void zocket_cerrar(ZOCKET z);

#endif
