/*
	This is Inka's test file!

*/

#include "WebServed.hpp"

WebServed::WebServed(const std::vector<ServerConf>& parsedServers) : servers(parsedServers) {}



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
void WebServed::start()//將不同port存入不同的vector
{
	std::vector<int> serverSockets;


	for (size_t i = 0; i < servers.size(); i++)
	{
		
		int serverPort = servers[i].port;

		//int serverFd = socket(AF_INET, SOCK_STREAM, 0);
		int serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

		if (serverFd == -1)
		{
			std::cerr << "Error: failed to create socks!!" << std::endl;
			continue ;
		}

		int opt = 1;//常見為 SO_REUSEADDR，讓程式在重新啟動時，不會因為 TIME_WAIT 而無法綁定同個 port：
		setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		

		// struct sockaddr_in address;
		// address.sin_family = AF_INET;
		// address.sin_addr.s_addr = INADDR_ANY;
		// //OR//inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);
		// address.sin_port = htons(serverPort);

		struct sockaddr_in server_addr;
		std::memset(&server_addr, 0, sizeof(server_addr));
		server
		server_addr.sin_family = AF_INET;//ipv4
		server_add.in_addr.s_addr = INADDR_ANY;//all web address
		server_addr.sin_port = htons(serverPort);

		if (bind(serverFd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
		{
			std::cerr << "❌ Error: failed to bind port " << serverPort << std::endl;
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
		else
		{
			runEventloop(serverSockets);
		}
		
	}


	void WebServed::runEventloop(std::vector<int> &serverSockets)
	{
		// save client info in a map
		//std::map<int, ClientConnection> clients;
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
				FD_SET(serverfd, &readSet);
				if (serverfd > maxfd)
					maxfd = serverfd;
			}
			
			//3.將「所有已連線的 client socket」根據需要讀/寫的狀況加入 readSet / writeSet。
			
			
			for (std::map<int, ClientConnection>::iterator it= clients.begin(); it!= clients.end(); it++)
			{
				int clientfd = it->first;
				ClientConnection &conn = it->second;
				if (conn.needRead()== true)
				{
					FD_SET(clientfd, &readSet);
					if (clientfd > maxfd)
						maxfd = clientfd;
				}
				if (conn.neeWrite() == true)
				{
					FD_SET(clientfd, &writeSet);
					if (clientfd > maxfd)
						maxfd = clientfd;
				}
			}
			

			//4.呼叫 select()，等待有任何 fd 就緒。
			int readycount = select(maxfd + 1, &readSet, &wrteSet, NULL, &timeout);
			
			if (readycount < 0)
			{
				std::"Error: select()" << std::endl;
				//close fd
				for (std::map<int, ClientConnection>::iterator it = clients.begin(); it != clients.end(); it++)
				{
					close(it->first);
				}
				clients.clear();
				break;//?
			}
			else if (readycount == 0)
			{
				//timeout
				continue;
			}
			//5.處理新連線 (accept)、
			for (int i = 0; i < serverSockets.size(); i++)
			{
				int sfd = serverSockets[i];
				if (FD_ISSET(sfd, &readSet))
				{
					while(true)//there might be multiple new connections in non-blocking mode at a port
					{
						struct sockaddr_in clientAddr;
						socklen_t addrLen = sizeof(clientAddr);
						int clientFd = accept(sfd, (sockaddr*)&clientAddr, &addrLen);
						if (clientFd < 0)
						{
							std::cerr << "❌ Error: failed to accept new connection" << std::endl;
							break;
						}
						else
						{
							std::cout << "📡 New connection accepted on port: " << clientFd << std::endl;
							//把這個新clientFd 以及對應的 ClientConnection 物件，放進 clients 這個container
							clients.insert(std::makepair(clientFd, ClientConnection(clientFd)));
						}
					}	
				}
			}

			//檢查所有現有 client FD，是否可讀
			for (std::map<int, ClientConnection>::iterator it = clients->begin; it != clients.end(); it++)
			{
				int cfd = it->first;
				ClinetConnection &conn = it->second;//?
				bool cosed = false;

				//if can read
				if (FD_ISSET(cfd, &readSet))
				{
					int n = conn.readData();
					if (n <= 0)
					{
						std::cout << "Client: " << cfd << " disconnected.\n";
						close(cfd);
						std::map<int, ClientConnection>::iterator tmp = it;
						++it;
						clients.erase(tmp);
						//may need to FD_CLR(cfd, &readSet);
						closed = true;
					}
				}
				

				//6.若有 client 可寫，就 send()。
				if (!closed && FD_ISSET(cfd, &writeSet) )
				{
					int sent = conn.writeData();
					if (sent < 0 || sent == 0)
					{
						if (sent < 0)
							std::cerr << "❌ Error: failed to send data to client " << cfd << std::endl;
						else
							std::cout << "❌ Client " << cfd << " disconnected." << std::endl;
						close(cfd);
						std::map<int, ClientConnection>::iterator tmp = it;
						clients.erase(tmp);
						closed = true;
					}
				}

				if (closed == false)
				{
					++it;
				}
			}
		
			//timeout control
			auto now = std::chrono::steady_clock::now();//?
			const int TIMEOUT_SECONDS = 60;
			for (auto it = client.begin(); it != clients.end();) 
			{
				int cfd = it->first;
				ClientConnection &conn = it->second;

				auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - conn.getLastActive()).count();
				if (if elapsed > TIMEOUT_SECONDS)
				{
					std::cout<< "Client: " << cfd << " timeout." << std::endl;
					close (cfd);
					std::map<int, ClientConnection>::iterator tmp = it;
					it++;
					client.erase(tmp);

				}
				else
				{
					++it;
				}
			}
			


		
		
		}
		
		
	}


	WebServed::cleanup()
	{
		for (std::)
	}
	/*
	client
	*/
	// int clientFd = -1;
	// while (true)
	// {
	// 	for (size_t i = 0; i < serverSockets.size(); i++)
	// 	{
	// 		struct sockaddr_in clientAddr;
	// 		socklen_t clientLen = sizeof(clientAddr);
	// 		int clientFd = accept(serverSockets[i], (struct sockaddr*)&clientAddr, &clientLen);

	// 		if (clientFd >= 0)
	// 		{
	// 			std::cout << "📡 Connection received on port " << servers[i].port << std::endl;

	// 			/*
	// 				Simple response
	// 			*/ 
	// 			std::string response =
	// 				"HTTP/1.1 200 OK\r\n"
	// 				"Content-Length: 6\r\n"
	// 				"Content-Type: text/plain\r\n"
	// 				"Connection: close\r\n"
	// 				"\r\n"
	// 				"Hello!";

	// 			std::cout << "📡 Sending Response:\n" << response << std::endl;


	// 			//send(clientFd, response.c_str(), response.size(), 0);

	// 			ssize_t bytesSent = send(clientFd, response.c_str(), response.size(), 0);
    //         	if (bytesSent == -1)
    //             	std::cerr << "❌ Error sending response: " << std::endl;
    //         	else
	//                 std::cout << "✅ Successfully sent " << bytesSent << " bytes." << std::endl;

	// 			close(clientFd);
	// 		}
	// 	}
	// }

}

//2.進入事件迴圈
//select() → 檢查哪些 fd 可讀/可寫。
//可讀的 server_fd → accept() 新連線 → 加到 clientConnections。
//可讀/可寫的 client_fd → 依狀態做 recv() 或 send() → 更新狀態。

//3.狀態管理
//STATE_READ_HEADER → STATE_READ_BODY → STATE_PROCESSING → STATE_WRITE_RESPONSE → STATE_DONE。
//適時關閉連線（或繼續保持連線，若要支援 keep-alive）。

//4.錯誤/Timeout 處理
//不正常就斷線，並清理資源。