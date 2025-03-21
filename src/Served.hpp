
#ifndef SERVED_HPP
#define SERVED_HPP

#include "ClientConnection.hpp"
#include "confiParser.hpp"
#include <iostream>
#include <vector>
#include <map>

// POSIX 網路相關標頭 in order
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>//定義 sockaddr_in、in_addr_t
#include <unistd.h>// close(), read(), write()

// C++ 封裝標頭
#include <cstring>   // for memset
#include <cerrno>    // for errno


class ClientConnection;

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