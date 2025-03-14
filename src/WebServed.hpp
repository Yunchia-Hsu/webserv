/*
	This is Inka's test file!

*/

#ifndef WEBSERVED_HPP
#define WEBSERVED_HPP

#include "clientConnection.hpp"
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
#include <fnctl.h>

class WebServed
{
	private:
		std::vector<ServerConf> servers;
		 
		std::map<int, ClientConnection> clients;//_sockets

	public:
		WebServed(const std::vector<ServerConf>& parsedServers);
		void start();
		void runEventloop(int serverSockets);

};

void	*ft_memcpy(void *dst, const void *src, size_t n);
#endif