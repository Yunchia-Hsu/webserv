
#include <arpa/inet.h> //socket(), bind(), accept(), inetntop(), listen() 
#include <iostream>
#include <stdio.h>

// UDP Socket 的流程，主要就 socket() -> 填位址 -> sendto()/recvfrom()
//TCP Socker connect() / listen() / accept()。







int main (int argc, char ** argv)
{

	//create sockets
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);// (ipv4  domain, type of the service, protocol)
	if (socket_fd <= 0)
		perror("socket created error");





	return 0; 
}