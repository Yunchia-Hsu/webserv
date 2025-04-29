
#include "Served.hpp"
#include <fcntl.h>    // For fcntl(), F_GETFL, F_SETFL, O_NONBLOCK  
Served::Served(const std::vector<ServerConf>& parsedServers, const std::map<std::string, std::shared_ptr<SocketWrapper>> portToSockets) : servers(parsedServers) {
	_portsToSockets = portToSockets;
	std::cout << "server lengthhhhhhhhhhhhhhh: " << servers.size() << std::endl;
	// _locations = locations;
	// std::cout << "Here is the locations lallalaalalalalala:" << _locations[0];
}



void	*ft_memcpy(void *dst, const void *src, size_t n)
{
	size_t			i;
	unsigned char	*p;
	unsigned char	*q;

	i = 0;
	p = (unsigned char *)dst;
	q = (unsigned char *)src;
	while (i < n)
	{
		p[i] = q[i];
		i++;
	}
	return (dst);
}


//建立 Socket 與 Bind + Listen
void Served::start()//將不同port存入不同的vector
{
	// std::vector<int> serverSockets;


	for (size_t i = 0; i < servers.size(); i++)
	{
		
		int serverPort = servers[i].port;

		// for linux
		int serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

		//for mac  no error handling
		// int serverFd = socket(AF_INET, SOCK_STREAM, 0);
		// int flags = fcntl(serverFd, F_GETFL, 0);
		// flags |= O_NONBLOCK;
		// fcntl(serverFd, F_SETFL, flags);
		//for mac


		if (serverFd == -1)
		{
			std::cerr << "Error: failed to create socks!!" << std::endl;
			continue ;
		}

		int opt = 1;
		 if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "❌ Error: setsockopt(SO_REUSEADDR) failed" << std::endl;
            close(serverFd);
            continue;
        }

		

		struct sockaddr_in server_addr;
		std::memset(&server_addr, 0, sizeof(server_addr));
		
		server_addr.sin_family = AF_INET;//ipv4
		server_addr.sin_addr.s_addr = INADDR_ANY;//all web address
		server_addr.sin_port = htons(serverPort);

		if (bind(serverFd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		{
			std::cerr << "fd:" << serverFd <<  "❌ Error: failed to bind port " << serverPort << std::endl;
			close(serverFd);
			continue ;
		}


		if (listen(serverFd, 1000) < 0)
		{
			std::cerr << "Error at the port" << std::endl;
			close(serverFd);
			continue ;
		}
		std::cout << "✅ Server " << i + 1 << " is listening on port " << serverPort << std::endl;
		serverSockets.push_back(serverFd);
		_socketFdToServerConf[serverFd] = std::make_shared<ServerConf>(servers[i]);
		_socketToPort[serverFd] = serverPort;

		if (serverSockets.empty()) 
		{
			std::cerr << "❌ Error: no server sockets created" << std::endl;
			return ;
		}
		
		
	}

}

std::vector<std::shared_ptr<ServerConf>> Served::matching_configs(int port){
	std::vector<std::shared_ptr<ServerConf>> configs;

	for (auto const &x : _portsToSockets){
		std::cout << "xxxxxxxxxxxxxxxx:" << x.first << "PORT: " << port << std::endl;
		std::regex ptrn("(.*):(.*)");
		std::smatch match_res;
		std::regex_match(x.first, match_res, ptrn);
	
		if (_portsToSockets.count(x.first) && port == std::stoi(match_res[2]))
		{
			for (auto con : _portsToSockets[x.first]->getServers())
				configs.push_back(con);
		}
	}
	return configs;
}

void Served::set_config(std::shared_ptr<ClientConnection>  client){
	// std::cout<<"ip:port:" <<client.get_server()->servers[0].host<<":"<<client.get_server()->servers[0].port<<std::endl;
	// std::cout<<"ip:port:" <<client.get_server()->servers[1].host<<":"<<client.get_server()->servers[1].port<<std::endl;
	// std::cout<<"ip:port:" <<client.get_server()->servers[2].host<<":"<< client.get_server()->servers[2].port<<std::endl;
	if (client->host_matched || client->conn_type != CONN_REGULAR)
		return ;
	// client.conf
	if  (client->_headers.count("host") == 0)
		return ;
	const std::string host = client->_headers["host"];
	
	int port = client->getServerPort();
	std::cout << "port: " << port << std::endl;
	std::vector <std::shared_ptr<ServerConf>> configs = matching_configs(port);
	client->conf = configs.front();
	std::cout << "hhhhhhhhhhhhhhhhhhhost name: " << host << std::endl;
	std::cout << "--------------------------------- SIZE: " << configs.size() << std::endl;
	std::vector<std::string> servernames;
	for (const auto &c : configs)
	{
		// std::cout << "hereeeeeeeeeeeeeeeeeeeee" << std::endl;
		// c->printConfig();
		for(std::string c : c->serverNames)
		{	std::cout << "HostName: " << c << std::endl;
			servernames.push_back(c);
		}
			// std::cout << "Thereeeeeeeeeeeeeeeeeeeee" << std::endl;
		// client.conf = c;
		// std::cout << "ccccccccccccccc is: " << c->printConfig() << std::endl;
		// for (const auto &name : c->getNames())
		// {
		// 	std::cout << "name " << name << std::endl;
		// 	if (host == name)
		// 	{
		// 		std::cerr << "matched host header: " << name << std::endl;
		// 		client.host_matched = true;
		// 		client.conf = c;
		// 		return;
		// 	}
		// }
	}
	for (const auto &c : configs)
	{
		for (const auto &name : servernames)
		{
			std::cout << "AAAAAname " << name << std::endl;
			if (host == name)
			{
				std::cerr << "matched host header: " << name << std::endl;
				client->host_matched = true;
				client->conf = c;
				return;
			}
		}
	}
	std::cout<<"------------------------config:"<<client->conf->index<<std::endl;
}

const char* sstate_to_string(State s) {
    switch (s) {
        case State::OK: return "OK";
        case State::ERROR: return "ERROR";
        case State::STATUSLINE: return "STATUSLINE";
        case State::HEADER: return "HEADER";
        case State::BODY: return "BODY";
        case State::CHUNKED: return "CHUNKED";
        case State::MULTIPART: return "MULTIPART";
        case State::CGIHEADER: return "CGIHEADER";
        case State::CGIBODY: return "CGIBODY";
        case State::PARTIALSTATUS: return "PARTIALSTATUS";
        case State::PARTIALHEADER: return "PARTIALHEADER";
        case State::PARTIALCHUNKED: return "PARTIALCHUNKED";
        case State::PARTIALCGI: return "PARTIALCGI";
        case State::PARTIALBODY: return "PARTIALBODY";
        default: return "UNKNOWN";
    }
}

// void WebServed::runEventloop(std::vector<int> &serverSockets)
void Served::runEventloop()
{
	// save client info in a map
	//std::map<int, ClientConnection> clients;
	std::cout << "Eventloop 🚀" << std::endl;
	while(true)
	{
		//1.建立並清空 readSet, writeSet。
		fd_set readSet, writeSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		int maxfd = 0;

		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;


		//2.將「所有 server socket」加入 readSet，以便檢查是否有新連線要 accept()。
		for (size_t i = 0; i < serverSockets.size(); i++)
		{
			
			int serverfd = serverSockets[i];
			//std::cout << "serverfd: " << serverfd << std::endl;
			FD_SET(serverfd, &readSet);
			if (serverfd > maxfd)
				maxfd = serverfd;
		}
		
		
		//3.將「所有已連線的 client socket」根據需要讀/寫的狀況加入 readSet / writeSet。
		
		for (std::map<int, std::shared_ptr<ClientConnection>>::iterator it= clients.begin(); it!= clients.end(); ++it)
		{
			
			int clientfd = it->first;
			std::shared_ptr<ClientConnection> &conn = it->second;
			if (conn->needRead()== true)
				FD_SET(clientfd, &readSet);
				
			if (conn->needWrite() == true)
				FD_SET(clientfd, &writeSet);
				
			if (clientfd > maxfd)
					maxfd = clientfd;
		}
		
		///test///
		std::cout << "&& connected clients: &&\n";

	
		for (auto it = clients.begin(); it != clients.end(); ++it) {
			std::cout << it->first << " ";
			
			
		}
		//std::cout << std::endl;
		// if (clients.empty())
		// {
		// 	std::cerr << "❌ Error: clients empty" << std::endl;
		// }




		//4.呼叫 select()，等待有任何 fd 就緒。
		// for (size_t i = 0; i < serverSockets.size(); ++i) {
		// 		std::cout << "🔍 server socket fd: " << serverSockets[i] << std::endl;
		// 	}
		

		int readycount = select(maxfd + 1, &readSet, &writeSet, NULL, &timeout);
		// std::cout << "readycount: " << readycount << std::endl;
		if (readycount < 0)
		{
			std::cerr<< "Error: select()" << std::endl;
			//close fd
			
			for (std::map<int, std::shared_ptr<ClientConnection>>::iterator it = clients.begin(); it != clients.end(); it++)
			{
				close(it->first);
			}
			clients.clear();
			break;//break the while loop and cleanup();
		}
		
		//timeout control
		auto now = std::chrono::steady_clock::now();
		// const int TIMEOUT_SECONDS = 60;
		for (auto it = clients.begin(); it != clients.end();) 
		{
			int cfd = it->first;
			std::shared_ptr<ClientConnection> conn = it->second;
		
			auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - conn->getLastActivity()).count();
			std::cout << "[timeout check] client " << cfd << " inactive for " << duration << "s" << std::endl;
			if ( duration > 10)
			{
				std::cout<< "Client: " << cfd << " timeout." << std::endl;



				// // 1. 組裝 403 回應字串
				// std::string resp = 
				// "HTTP/1.1 403 Forbidden\r\n"
				// "Content-Type: text/html\r\n"
				// "Content-Length: 23\r\n"
				// "\r\n"
				// "<h1>403 Forbidden</h1>";
	
				// // 2. 直接透過 socket 發送回應
				// ssize_t sent = send(cfd, resp.c_str(), resp.size(), 0);
				// if (sent < 0) 
				// {
				// 	std::cerr << "❌ Error sending 403 to client " << cfd 
				// 			<< ": " << strerror(errno) << std::endl;
						
				// }


			

				close (cfd);
				it = clients.erase(it);
				continue;
			}
			else
			{
				++it;
			}
		}
		
		if (readycount == 0)
		{
			continue;
		}
		//5.handle new clients (accept)、
		for (unsigned int i = 0; i < serverSockets.size(); i++)
		{
			
			int sfd = serverSockets[i];
			if (FD_ISSET(sfd, &readSet))
			{
				while(true)//there might be multiple new connections in non-blocking mode at a port
				{
					
					std::cout << " in while true loop"<< std::endl;
					struct sockaddr_in clientAddr;
					socklen_t addrLen = sizeof(clientAddr);
					int clientFd = accept(sfd, (sockaddr*)&clientAddr, &addrLen);
					std::cout << "Clients count after accept(): " << clients.size() << std::endl;

					if (clientFd < 0)
					{
						std::cerr << "❌ Error: failed to accept new connection" << std::endl;
						break;
					}
					else
					{
						std::cout << "📡 New connection accepted on port: " << clientFd << std::endl;

						std::shared_ptr<ClientConnection> conn = std::make_shared<ClientConnection>(clientFd, servers[i].port, this);// init client
						//ClientConnection conn (clientFd);
						// conn.appendToWriteBuffer("Hello from server!  here there (Test Message)\n");
						//把這個新clientFd 以及對應的 ClientConnection 物件，放進 clients 這個container
						clients.insert(std::make_pair(clientFd, conn));
						///test///
						std::cout << "Current connected clients: ";
						for (auto it = clients.begin(); it != clients.end(); ++it) {
							std::cout << it->first << " ";
						}
						std::cout << std::endl;
					}
				}	
			}
		}

		//檢查所有現有 client FD，是否可讀
		// auto it = clients.begin();
		// while (it != clients.end())
		for (std::map<int, std::shared_ptr<ClientConnection>>::iterator it = clients.begin(); it != clients.end(); it++)
		{
			
			
			int cfd = it->first;
			std::shared_ptr<ClientConnection> conn = it->second;//?
			bool closed = false;

			//if can read
			if (FD_ISSET(cfd, &readSet))
			{
				int n = conn->readData();
			
				if (n < 0)
				{
					std::cout << "Client: " << cfd << " disconnected. hahaha\n";
					close(cfd);
					it = clients.erase(it);
					
					FD_CLR(cfd, &readSet);
					it = clients.begin();
					closed = true;
					break;
					
				}
				else if (n == 0) //接收完畢
				{
					std::cout << "Client " << cfd << " connected on server port: " << conn->getServerPort() << std::endl;

					
					State state = conn->parse(State::STATUSLINE,conn->getwritebubffer(),n);
					std::cout << sstate_to_string(state) << std::endl;
					//routeconfig
					set_config(conn);
					// if (state == State::OK || state == State::ERROR)
					// {
					// 	std::cout<< static_cast<int>(state)<< std::endl;
					// 	std::cout<< "B______________________________-:"<< conn._headers["host"]<<std::endl;
					// 	}
					// std::cout<<"------------------------config:"<<conn->conf->index<<std::endl;
					FD_CLR(cfd, &readSet);
					FD_SET(cfd, &writeSet);
				}
				if (closed) continue;
			}
			
				
			//6.若有 client 可寫，就 send()。
			//if (!closed && FD_ISSET(cfd, &writeSet) )
			if (FD_ISSET(cfd, &writeSet) && conn->needWrite())
			{
				// int sent = conn.writeData();
				int sent = 0;
				if (!conn->resp) {
					conn->resp = std::make_shared<Response>(conn);
					conn->response = conn->resp->buffer.str();
					std::cout << "response result: " << conn->response << std::endl;
					conn->writeOffset = 0;
				}

				size_t	remaining = conn->response.size() - conn->writeOffset;
				ssize_t	totalSent = 0;

				while (remaining > 0) {
					std::cout << "remain: " << remaining << "totalSent: " << totalSent << std::endl;
					ssize_t sent = send(conn->fd, conn->response.data() + conn->writeOffset, remaining, 0);
					if (sent > 0) {
						conn->writeOffset += sent;
						totalSent += sent;
						remaining -= sent;
						conn->lastActivity = std::chrono::steady_clock::now();
					} else if (sent < 0) {
						std::perror("send failed");
						sent = -1;
					} else {
						sent = 0;    // Or -1. Will check it later
					}
				}
				if (conn->writeOffset == conn->response.size()) {
					conn->response.clear();
					conn->writeOffset = 0;
					sent = totalSent;
					std::cout << "ssssssssssssssent: " << sent << std::endl;
				}

				if (sent < 0 || sent == 0)
				{
					if (sent < 0)
						std::cerr << "❌ Error: failed to send data to client " << cfd << std::endl;
					else
						std::cout << "❌ Client " << cfd << " disconnected.hehehe" << std::endl;
					
					
					conn->resp.reset();
					close(cfd);
					it = clients.erase(it);
					
					closed = true;
					break;
				}
				if (closed) continue;
			}
			// ++it;
		}

	}
	std::cout << "Eventloop end" << std::endl;
	
}



void Served::cleanup(void)
{
	std::cout << "call cleanup " << std::endl;
	//close server sockets
	for (size_t i = 1; i < serverSockets.size() ; ++i)
	{
		::close(serverSockets[i]);

	}
	//close client sockets
	for (auto it = clients.begin(); it != clients.end(); it ++)
	{
		::close(it->first);
	}
	//clean container
	clients.clear();
	serverSockets.clear();
}
	