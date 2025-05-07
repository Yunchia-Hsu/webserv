
#ifndef SERVED_HPP
#define SERVED_HPP

#include "ClientConnection.hpp"
#include "confiParser.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>//sockaddr_in、in_addr_t
#include <unistd.h>// close(), read(), write()

#include <cstring>   // for memset
#include <cerrno>    // for errno

#include "response.hpp"
#include "socketWrapper.hpp"

class ClientConnection;
class Response;

class Served
{
	private:
		std::vector<ServerConf> servers;

		std::vector<int> serverSockets;
		std::map<int, std::shared_ptr<ClientConnection>> clients;//_sockets
		std::map<int, std::shared_ptr<ServerConf>> _socketFdToServerConf;
		std::map<std::string, std::shared_ptr<SocketWrapper>> _portsToSockets;
		std::vector<std::shared_ptr<Location>> _locations;
		std::map<int, int> _socketToPort;
		std::shared_ptr<Response> resp;
		std::string response;

		std::unordered_map<int, int> _cgi_to_client;

		bool init_cgi_fds(std::shared_ptr<ClientConnection> conn);
		bool mod_fd(int fd, int ctl, int mask, std::shared_ptr<ClientConnection> cl);

	public:
		Served(const std::vector<ServerConf>& parsedServers, const std::map<std::string, std::shared_ptr<SocketWrapper>> portToSockets);
		~Served();
		void start();
		int getport(int i)
		{
			return servers[i].port;
		}
		void set_config(std::shared_ptr<ClientConnection>  client);
		void runEventloop();
		void cleanup(void);
		std::vector<std::shared_ptr<ServerConf>> matching_configs(int port);
};

void	*ft_memcpy(void *dst, const void *src, size_t n);


#endif