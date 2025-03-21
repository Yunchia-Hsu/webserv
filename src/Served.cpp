

#include "Served.hpp"
#include <fcntl.h>    // For fcntl(), F_GETFL, F_SETFL, O_NONBLOCK  
Served::Served(const std::vector<ServerConf>& parsedServers) : servers(parsedServers) {}



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

		if (serverSockets.empty())
		{
			std::cerr << "❌ Error: no server sockets created" << std::endl;
			return ;
		}
		
		
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
		int ret = 0;

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
		
		for (std::map<int, ClientConnection>::iterator it= clients.begin(); it!= clients.end(); ++it)
		{
			
			int clientfd = it->first;
			ClientConnection &conn = it->second;
			if (conn.needRead()== true)
				FD_SET(clientfd, &readSet);
				
			if (conn.needWrite() == true)
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
		//std::cout << "readycount: " << readycount << std::endl;
		if (readycount < 0)
		{
			std::cerr<< "Error: select()" << std::endl;
			//close fd
			
			for (std::map<int, ClientConnection>::iterator it = clients.begin(); it != clients.end(); it++)
			{
				close(it->first);
			}
			clients.clear();
			break;//break the while loop and cleanup();
		}
		
		//timeout control
		auto now = std::chrono::steady_clock::now();
		const int TIMEOUT_SECONDS = 60;
		for (auto it = clients.begin(); it != clients.end();) 
		{
			int cfd = it->first;
			ClientConnection &conn = it->second;
		
			auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - conn.getLastActivity()).count();
			std::cout << "[timeout check] client " << cfd << " inactive for " << duration << "s" << std::endl;
			if ( duration > TIMEOUT_SECONDS)
			{
				std::cout<< "Client: " << cfd << " timeout." << std::endl;
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
		for (int i = 0; i < serverSockets.size(); i++)
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

						ClientConnection conn (clientFd);
						conn.appendToWriteBuffer("Hello from server!  here there (Test Message)\n");
						//把這個新clientFd 以及對應的 ClientConnection 物件，放進 clients 這個container
						clients.insert(std::make_pair(clientFd, ClientConnection(clientFd)));
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
		for (std::map<int, ClientConnection>::iterator it = clients.begin(); it != clients.end(); it++)
		{
			
			std::cout << "-1" << std::endl;
			int cfd = it->first;
			ClientConnection &conn = it->second;//?
			bool closed = false;

			//if can read
			if (FD_ISSET(cfd, &readSet))
			{
				int n = conn.readData();
			
				if (n < 0)
				{
					std::cout << "Client: " << cfd << " disconnected. hahaha\n";
					close(cfd);
					clients.erase(it);
					
					FD_CLR(cfd, &readSet);
					it = clients.begin();
					closed = true;
					break;
					
				}
				else if (n == 0) //接收完畢
				{
					FD_CLR(cfd, &readSet);
					FD_SET(cfd, &writeSet);
				}
			}
			
				
			//6.若有 client 可寫，就 send()。
			//if (!closed && FD_ISSET(cfd, &writeSet) )
			if (!closed && FD_ISSET(cfd, &writeSet) && conn.needWrite())
			{
				int sent = conn.writeData();
				if (sent < 0 || sent == 0)
				{
					if (sent < 0)
						std::cerr << "❌ Error: failed to send data to client " << cfd << std::endl;
					else
						std::cout << "❌ Client " << cfd << " disconnected.hehehe" << std::endl;
					close(cfd);
					
					it = clients.erase(it);
					
					closed = true;
					break;
				}
			}
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
	