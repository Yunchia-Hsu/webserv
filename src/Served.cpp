
#include "Served.hpp"
#include <fcntl.h>    // For fcntl(), F_GETFL, F_SETFL, O_NONBLOCK  

Served::Served(const std::vector<ServerConf>& parsedServers, const std::map<std::string, std::shared_ptr<SocketWrapper>> portToSockets) : servers(parsedServers) {
	_portsToSockets = portToSockets;
	//std::cout << "server lengthhhhhhhhhhhhhhh: " << servers.size() << std::endl;
}

Served::~Served(){}

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


//build, bind Sockets and Listen
void Served::start()//將不同port存入不同的vector
{
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

		// if (serverSockets.empty()) 
		// {
		// 	std::cerr << "❌ Error: no server sockets created" << std::endl;
		// 	return ;
		// }
	}
	if (serverSockets.empty()) 
	{
		std::cerr << "❌ Error: no server sockets created" << std::endl;
		return std::exit(EXIT_FAILURE);
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
	// std::cout << "hhhhhhhhhhhhhhhhhhhost name: " << host << std::endl;
	// std::cout << "--------------------------------- SIZE: " << configs.size() << std::endl;
	std::vector<std::string> servernames;
	for (const auto &c : configs)
	{
		// std::cout << "hereeeeeeeeeeeeeeeeeeeee" << std::endl;
		// c->printConfig();
		for(std::string c : c->serverNames)
		{	
			// std::cout << "HostName: " << c << std::endl;
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

void Served::runEventloop()
{
	// save client info in a map
	//std::cout << "Eventloop 🚀" << std::endl;
	while(true)
	{
		//1.build and cleanup readSet, writeSet。
		fd_set readSet, writeSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);
		int maxfd = 0;

		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		//2.add all serverSockets in readSet，so we can check new connection by accept()。
		for (size_t i = 0; i < serverSockets.size(); i++)
		{
			int serverfd = serverSockets[i];
			//std::cout << "serverfd: " << serverfd << std::endl;
			FD_SET(serverfd, &readSet);
			if (serverfd > maxfd)
				maxfd = serverfd;
		}
		
		//3.add all client socket」 into readSet or writeSet。
		for (std::map<int, std::shared_ptr<ClientConnection>>::iterator it= clients.begin(); it!= clients.end(); ++it)
		{
			int clientfd = it->first;
			std::shared_ptr<ClientConnection> &conn = it->second;
			if (conn->needRead()== true && clientfd >= 0)
				FD_SET(clientfd, &readSet);
				
			if (conn->needWrite() == true)
				FD_SET(clientfd, &writeSet);
			if (clientfd > maxfd)
					maxfd = clientfd;
		}
		
		//4.呼叫 select()，等待有任何 fd 就緒。
		// for (size_t i = 0; i < serverSockets.size(); ++i) {
		// 		std::cout << "🔍 server socket fd: " << serverSockets[i] << std::endl;
		// 	}
		
		int readycount = select(maxfd + 1, &readSet, &writeSet, NULL, &timeout);
		// std::cout << "readycount: " << readycount << std::endl;
		if (readycount < 0)
		{
			// std::cerr<< "Error: select()" << std::endl;
			std::perror("select");
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
			// std::shared_ptr<ClientConnection>& conn = it->second;
			if ( duration > 10)
			{
				std::cout<< "Client: " << cfd << " timeout." << std::endl;
				close (cfd);
				FD_CLR(cfd, &readSet);
				it = clients.erase(it);
				// continue;
			}
			else
				++it;
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
						//std::cout << "📡 New connection accepted on port: " << clientFd << std::endl;
						std::shared_ptr<ClientConnection> conn = std::make_shared<ClientConnection>(clientFd, servers[i].port, this);// init client
						//insert new clientFd and its ClientConnection in the container
						clients.insert(std::make_pair(clientFd, conn));
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
			std::shared_ptr<ClientConnection> conn = it->second;
			bool closed = false;

			std::cout << "LLLLLet's start new round, the current client fd is: " << conn->fd << std::endl;

			// bool cgi_finish = false;
			// while (conn->conn_type == CONN_CGI && cgi_finish == false) 
			// 	cgi_finish = handle_cgi_read(conn, readSet, writeSet, maxfd);
			// cgi_finish = true;
			
			if (conn->conn_type == CONN_CGI) {
				handle_cgi_read(conn, readSet, writeSet, maxfd);
				// continue;
				// break;
			}

			//if can read
			if (FD_ISSET(cfd, &readSet))
			{
				int n = conn->readData();
			
				if (n < 0)
				{
					//std::cout << "Client: " << cfd << " disconnected. hahaha\n";
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
			
				
			//6. client sent request -> send()
			//if (!closed && FD_ISSET(cfd, &writeSet) )
			if (FD_ISSET(cfd, &writeSet) && conn->needWrite())
			{
				int sent = 0;
				if (!conn->resp) {
					conn->resp = std::make_shared<Response>(conn);
					if (conn->conn_type == CONN_WAIT_CGI) {
						if (init_cgi_fds(conn, readSet, writeSet, maxfd)) {
							// std::cout << "fffffinally, come here\n";
							return ;
						}
					}
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
					//std::cout << "sent: " << sent << std::endl;
				}

				if (sent < 0 || sent == 0)
				{
					if (sent < 0)
						std::cerr << "❌ Error: failed to send data to client " << cfd << std::endl;
					//else
						//std::cout << "❌ Client " << cfd << " disconnected" << std::endl;
					
					
					conn->resp.reset();
					close(cfd);
					it = clients.erase(it);
					
					closed = true;
					break;
				}
				if (closed) continue;
			}
			// if (conn->conn_type == CONN_CGI)
			// 	handle_cgi_read(conn, readSet, writeSet, maxfd);
		}

	}
	std::cout << "Eventloop end" << std::endl;
	
}

bool Served::add_fd(int fd, bool want_read, bool want_write, std::shared_ptr<ClientConnection> cl, fd_set& readSet, fd_set& writeSet, int& maxfd)
{
	std::cout << "[DEBUG] add_fd: adding fd " << fd << " for " 
          << (want_read ? "read " : "") << (want_write ? "write " : "") 
          << (cl ? "(has ClientConnection)" : "(nullptr)") << std::endl;

	if (clients.count(fd))
		std::cout << "[WARNING] Overwriting existing client with fd " << fd << std::endl;

	if (fd < 0) return false;

	if (want_read)
		FD_SET(fd, &readSet);
	if (want_write)
		FD_SET(fd, &writeSet);

	if (cl && clients.count(fd) == 0)
		clients[fd] = cl;
	else if (cl && clients[fd] != cl) 
		std::cerr << "[ERROR] Attempt to overwrite fd " << fd << " in clients map" << std::endl;

	if (fd > maxfd)
		maxfd = fd;

	return true;
}

void Served::remove_fd(int fd, fd_set& readSet, fd_set& writeSet, int& maxfd)
{
	if (fd < 0 || clients.count(fd) == 0)
		return;

	FD_CLR(fd, &readSet);
	FD_CLR(fd, &writeSet);
	clients.erase(fd);
	_cgi_to_client.erase(fd);

	if (fd == maxfd)
	{
		// Recompute max_fd
		maxfd = 0;
		for (const auto& entry : clients)
		{
			if (entry.first > maxfd)
				maxfd = entry.first;
		}
	}
}

bool Served::init_cgi_fds(std::shared_ptr<ClientConnection> conn, fd_set& readSet, fd_set& writeSet, int& maxfd)
{
	std::cout << "First, what is the client fd: " << conn->fd << std::endl;
	_cgi_to_client[conn->cgi_fd_write] = conn->fd;
	_cgi_to_client[conn->cgi_fd_read] = conn->fd;

	std::shared_ptr<ClientConnection> read_cgi = std::make_shared<ClientConnection>(conn->cgi_fd_read, this);
	std::shared_ptr<ClientConnection> write_cgi = std::make_shared<ClientConnection>(conn->cgi_fd_write, this);
	write_cgi->response = conn->_body;

	int count = 0;
	if (add_fd(conn->cgi_fd_write, false, true, write_cgi, readSet, writeSet, maxfd))
		count++;
	if (add_fd(conn->cgi_fd_read, true, false, read_cgi, readSet, writeSet, maxfd))
		count++;

	if (count != 2)
	{
		// std::cout << "ai, so it still comes here: " << count << std::endl;
		remove_fd(conn->cgi_fd_write, readSet, writeSet, maxfd);
		remove_fd(conn->cgi_fd_read, readSet, writeSet, maxfd);
		conn->resp->set_error(500);
		return false;
	}

	std::cerr << "[webserv] CGI initialized: " << conn->cgi_fd_read << "/" << conn->cgi_fd_write << " and the client fd: " << conn->fd << std::endl;
	return true;
}

void Served::finish_cgi_client(std::shared_ptr<ClientConnection> cgi_client,
	fd_set& readSet, fd_set& writeSet, int& maxfd)
{
	if (_cgi_to_client.count(cgi_client->fd) == 0)
		return;
	int conn_fd = _cgi_to_client[cgi_client->fd];
	if (clients.count(conn_fd) == 0)
		return;
	std::shared_ptr<ClientConnection> conn = clients[conn_fd];

	if (!conn || conn->fd == cgi_client->fd)
		return;

	remove_fd(conn->cgi_fd_write, readSet, writeSet, maxfd);
	remove_fd(conn->cgi_fd_read, readSet, writeSet, maxfd);

	if (!Cgi::finish(conn->pid))
	{
		std::cerr << "[webserv] CGI error\n";
		conn->resp->set_error(500);
	}
	conn->pid = -1;
	if (conn->resp)
		conn->resp->finish_cgi(cgi_client);
	conn->response = conn->resp->buffer.str();
	std::cout << "in here the response result is: " << conn->response << std::endl;

	// Now we want to write back to client socket
	FD_SET(conn->fd, &writeSet);
	if (conn->fd > maxfd)
		maxfd = conn->fd;
}

bool Served::handle_cgi_read(std::shared_ptr<ClientConnection> client,
	fd_set& readSet, fd_set& writeSet, int& maxfd)
{
	// char buffer[READ_BUFFER_SIZE];
	std::string buffer(READ_BUFFER_SIZE, 0);
	std::cout << "so where died, client fd is here: " << client->fd << std::endl;
	ssize_t bytes_read = read(client->fd, &buffer[0], READ_BUFFER_SIZE);

	std::cout << "so where died, bytes read is there: " << bytes_read << std::endl;
	if (bytes_read == -1)
	{
		std::cerr << "[ERROR] read() failed on fd " << client->fd
	          << " with errno " << errno << ": " << strerror(errno) << std::endl;
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			FD_SET(client->fd, &readSet);
			return false;
		}
		// remove_fd(client->fd, readSet, writeSet, maxfd);
		return true;
	}
	client->cgi_buffer.append(buffer, bytes_read);
	std::cout << "so what is the cgi buffer: " << &buffer[0] << "and the byte: " << bytes_read << " as you know" << std::endl;
	State s = client->parse(State::CGIHEADER, &buffer[0], bytes_read);
	// State s = client->parse(State::CGIHEADER, client->cgi_buffer, client->cgi_buffer.length());
	// client->cgi_buffer.clear();
	std::cout << "Okkkkk it comes here (no Content-Length, still receiving): " << client->fd << "and the byte: " << bytes_read << std::endl;

	if (s == State::OK || s == State::ERROR || bytes_read == 0)
	{
		finish_cgi_client(client, readSet, writeSet, maxfd);
		return true;
	}
	// Still expecting more data from CGI — ensure fd is in readSet
	FD_SET(client->fd, &readSet);
	// add_fd(client->fd, false, true, client, readSet, writeSet, maxfd);
	if (client->fd > maxfd)
		maxfd = client->fd;
	return false;
}



void Served::cleanup(void)
{
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
	_socketFdToServerConf.clear();
	_portsToSockets.clear();

	_locations.clear();

	serverSockets.clear();

	 _socketToPort.clear();
}
	