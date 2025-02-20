
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

	//identify sockets
	struct socketaddr_in;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = INADDR_ANY;// allow any IP 192.168...8080  or 127.0.0.1.8080

	bind(socket_fd, (struct sockaddr*) & serverAddr, sizeof (serverAddr));



	return 0; 
}