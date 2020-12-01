#include "udp.h"


socket_s* newSocket(int port)
{
	socket_s* ret = (socket_s*)malloc(sizeof(socket_s));
	ret->s = -1;
	ret->port = port;
	ret->sock_len = sizeof(ret->sockaddr);

	memset((char *) &(ret->sockaddr), 0, ret->sock_len);
	ret->sockaddr.sin_family = AF_INET;
	ret->sockaddr.sin_port = htons(port);
	ret->sockaddr.sin_addr.s_addr = INADDR_ANY;
	return ret;
}




void startSocket(socket_s* sk)
{
	int opt;
	if ((sk->s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		fatalErr("Could not generate socket errno: %d\n", errno);
	}
	/*
	if (inet_aton("127.0.0.1" , &(sk->sockaddr.sin_addr)) == 0)
	{
		fatalErr("Could not connect to WF on port %d",sk->port);
	}
	*/
	/*
    opt = fcntl(sk->s, F_GETFL);
    if (opt < 0) 
	{
        printf("fcntl(F_SETFL) fail.");
		exit(-1);
    }
    opt |= O_NONBLOCK;
    if (fcntl(sk->s, F_SETFL, opt) < 0) 
	{
        printf("fcntl(F_SETFL) fail.");
		exit(-1);
	}
	*/
}

void startSocket_ws(socket_s* sk)
{
	int opt;
	if ((sk->s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		fatalErr("Could not generate socket errno: %d\n", errno);
	}
	
}


int sendToSocket(socket_s* sk, void* buff, int size)
{
	return sendto(sk->s, buff, size, 0, (struct sockaddr*) &(sk->sockaddr), sk->sock_len);
}

int getFromSocket(socket_s* sk, void* buff)
{
	return recvfrom(sk->s, buff, MAX_TRANS_SIZE, 0, (struct sockaddr*) &(sk->sockaddr), &(sk->sock_len));
}

void closeSocket(socket_s* sk){
	close(sk->s); //no need for shutdows because it's not TCP
}
