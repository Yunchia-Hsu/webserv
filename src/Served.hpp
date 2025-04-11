
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

#include "request.hpp"
#include "socketWrapper.hpp"

class ClientConnection;

class Served
{
	private:
		std::vector<ServerConf> servers;
		std::vector<int> serverSockets;
		std::map<int, ClientConnection> clients;//_sockets

		std::map<int, std::shared_ptr<ServerConf>> _socketFdToServerConf;
		std::map<std::string, std::shared_ptr<SocketWrapper>> _portsToSockets;
		std::map<int, int> _socketToPort;

	public:
		Served(const std::vector<ServerConf>& parsedServers);
		void start();
		
		int getport(int i)
		{
			return servers[i].port;
		}
		void set_config(ClientConnection  client);
		//void runEventloop(std::vector<int> &serverSockets);
		void runEventloop();//?

		void cleanup(void);

		std::vector<std::shared_ptr<ServerConf>> matching_configs(int port);
};

void	*ft_memcpy(void *dst, const void *src, size_t n);


#endif