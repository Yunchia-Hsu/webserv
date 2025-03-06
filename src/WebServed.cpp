/*
	This is Inka's test file!

*/

#include "WebServed.hpp"

WebServed::WebServed(const std::vector<ServerConf>& parsedServers) : servers(parsedServers) {}

void WebServed::start()
{
	/* 
		SOCKETS
	*/
	std::vector<int> serverSockets;

	for (size_t i = 0; i < servers.size(); i++)
	{
		int serverPort = servers[i].port;

		int serverFd = socket(AF_INET, SOCK_STREAM, 0);
		if (serverFd == -1)
		{
			std::cerr << "Error in your socks!!" << std::endl;
			continue ;
		}

		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		//OR//inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);
		address.sin_port = htons(serverPort);

		if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0)
		{
			std::cerr << "Failed to create the port" << std::endl;
			close(serverFd);
			continue ;
		}

		if (listen(serverFd, 10) < 0)
		{
			std::cerr << "Error at the port" << std::endl;
			close(serverFd);
			continue ;
		}
		std::cout << "✅ Server " << i + 1 << " is listening on port " << serverPort << std::endl;
		serverSockets.push_back(serverFd);
	}

	/*
		CLIENT
	*/
	int clientFd = -1;
	while (true)
	{
		for (size_t i = 0; i < serverSockets.size(); i++)
		{
			struct sockaddr_in clientAddr;
			socklen_t clientLen = sizeof(clientAddr);
			int clientFd = accept(serverSockets[i], (struct sockaddr*)&clientAddr, &clientLen);
		}
		if (clientFd >= 0)
		{
			std::cout << "📡 Connection received " << std::endl; //on port " << servers[i].port << std::endl;

			/*
				Simple response
			*/ 
			std::string response =
				"HTTP/1.1 200 OK\r\n"
				"Content-Length: 13\r\n"
				"Content-Type: text/plain\r\n"
				"\r\n"
				"Hello!";
			send(clientFd, response.c_str(), response.size(), 0);
			close(clientFd);
		}
	}

}