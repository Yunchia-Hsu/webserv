/*
	This is Inka's test file!

*/

#ifndef SERVED_HPP
#define SERVED_HPP


#include "ClientConnection.hpp"
#include "confiParser.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>//memset
#include <cerrno>
#include <iostream>
#include <arpa/inet.h> // sockadd_in

class Served
{
	private:
		std::vector<ServerConf> servers;
		std::vector<int> serverSockets;
		std::map<int, ClientConnection> clients;//_sockets

	public:
		Served(const std::vector<ServerConf>& parsedServers);
		void start();
		
		//void runEventloop(std::vector<int> &serverSockets);
		void runEventloop();//?

		void cleanup(void);
};

void	*ft_memcpy(void *dst, const void *src, size_t n);



#endif