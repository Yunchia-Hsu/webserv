#include <arpa/inet.h>

#include "utils.hpp"
#include "ServerConf.hpp"

class SocketWrapper {
	int sockfd;
	std::vector<std::shared_ptr<ServerConf>> servers;
	std::string host;
	int port;

public:
	SocketWrapper() : sockfd(-1) {}

	void addServer(const ServerConf &conf) {
		servers.push_back(std::make_shared<ServerConf>(conf));
		host = conf.host;
		port = conf.port;
	}

	void initSocket() {
		sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		// bind(), listen(), etc...
		// use this->host and this->port
	}

	int getFd() const { return sockfd; }
	int getPort() const { return port; }
	std::shared_ptr<ServerConf> getPrimaryServer() const {
		if (!servers.empty())
			return servers[0];
		return nullptr;
	}

	const std::vector<std::shared_ptr<ServerConf>>& getServers() const {
		return servers;
	}
};
