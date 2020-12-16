#ifndef UTILS_UDP
#define UTILS_UDP

#include<arpa/inet.h>
#include<sys/socket.h>
#include<fcntl.h>

#include "../utils/debug.h"

typedef struct{
	int s;
	struct sockaddr_in sockaddr;
	socklen_t sock_len;
	int port;
}socket_s;

socket_s* newSocket(int port);
void startSocket(socket_s* sk);
void startSocket_ws(socket_s* sk);
int sendToSocket(socket_s* sk, void* buff, int size);
int getFromSocket(socket_s* sk, void* buff);
void closeSocket(socket_s* sk);


#endif
