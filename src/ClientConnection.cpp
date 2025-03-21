
#include "ClientConnection.hpp"


ClientConnection::ClientConnection(int cfd): fd(cfd), writeOffset(0),lastActivity(std::chrono::steady_clock::now())  // ✅ 初始化
{
    //for test
    // writeBuffer = "Hello from server!   for test\n";
    writeBuffer = "init wirte buffer";
    sendBuffer = "";
    std::string response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 8\r\n"
    "Content-Type: text/plain\r\n"
    "Connection: close\r\n";
    // "\r\n"
    // "fifahey!";
    sendBuffer = response;
    
                
}

bool ClientConnection::needWrite ()
{
    if (writeOffset < writeBuffer.size())
        return true;
    else
        return false;
}

bool ClientConnection::needRead ()
{
    //depend on ConnectionState
    //connectionstate are PROCESSING / WRITE_RESPONSE return false 
    return true;
}

int ClientConnection::readData() //要進一步確保HTTP request 是完整的
//檢查是否出現「\r\n\r\n」（標示 HTTP header 結束），以及根據 Content-Length 或 Transfer-Encoding 的資訊來判斷請求體是否接收完整。
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    
    int n =  recv(fd, buffer, sizeof(buffer), 0);
    //lastActive = std::chrono::high_resolution_clock::now();
	
    // 先使用 recv() 讀取資料，將讀取到的資料追加到一個暫存的字串（例如 writeBuffer 或 _requests[cfd]）。
    //接著檢查這個字串是否包含 "\r\n\r\n"，確定 Header 是否接收完整。
    if (n <= 0)
        return (-1); // recv error

    if (n > 0)
    {
        writeBuffer.append(buffer, n);
        std::cout << "client: "<< fd << " recieved "<< n <<" bytessss" << std::endl;
		lastActivity = std::chrono::steady_clock::now();
        //writeBuffer =
        //check content length
        /*
        "POST / HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, world!";
        */
        //check chunked
        /*
        "POST / HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "7\r\n"
        "Mozilla\r\n"
        "9\r\n"
        "Developer\r\n"
        "7\r\n"
        "Network\r\n"
        "0\r\n"
        "\r\n";
        */

   // std::cout << "sent content: \n" << writeBuffer <<std::endl;

		if(writeBuffer.find("\r\n\r\n") != std::string::npos) //如果 Header 完整
		{
			
			std::cout << "header checkend done" << std::endl;
			//如果有 "Content-Length:" 標頭，則計算剩餘的資料是否達到或超過 Content-Length 指定的值。
			if (writeBuffer.find("Content-Length:") != std::string::npos)//有content lenth
			{
				//content length 
				int contentlenpos = writeBuffer.find("Content-Length:");
				int contentlenend = writeBuffer.find("\r\n", contentlenpos); 
				int headerend = writeBuffer.find("\r\n\r\n");
				std::string str = writeBuffer.substr(writeBuffer.find(contentlenpos+15, contentlenend- (contentlenpos+15)));
				size_t contentlen = atoi(str.c_str());
				if (writeBuffer.size() >= headerend + 4 +  contentlen)
				{
					std::cout << "Content-Length checkend done" << std::endl;
					return 0;
				}
			}
				//如果是 chunked 編碼，則解析每個分塊，直到讀到分塊大小為 0。
			else if (writeBuffer.find("Transfer-Encoding: chunked") != std::string::npos)//chunked
			{ 
				if (checkend(writeBuffer, "0\r\n\r\n") == 0)
				{
					std::cout << "Transfer-Encoding: chunked checkend done" << std::endl;
					return 0;
				}
			}
			else if((writeBuffer.find("Content-Length:") == std::string::npos)  && (writeBuffer.find("chunked transfer encoding") == std::string::npos))
			{
				std::cout <<"no length or chuncked"    << std::endl;
				return 0;
			}
			
		}
        std::cout << "header not completed" << std::endl;
        return 1;
    } 
    
    
    return 1;
}

int ClientConnection::checkend(std::string str, std::string end) //return 0 done, 1 not done
{
    int strlen = str.size() - 1;
    int endlen = end.size() - 1;

    while(endlen >= 0)
    {
        std::cout << "✅str[strlen]: " << str[strlen] << " strlen: " << strlen << " end[end]: "<< end[endlen]  << " endlen: " << endlen << std::endl;
        if (str[strlen] != end[endlen])
            return 1;
        strlen--;
        endlen--;
    }
    return 0; 
}

void ClientConnection::appendToWriteBuffer(const std::string &data)
{
    writeBuffer  += data;
}

int ClientConnection::writeData()
{
    if (!needWrite())
        return 0;
    // count data length
    size_t remaining = sendBuffer.size() - writeOffset;
    ssize_t totalSent = 0;

    while(remaining >0)
    {
    
        ssize_t sent = send(fd, sendBuffer.data() + writeOffset, remaining, 0 );
        if (sent > 0)
        {
            writeOffset += sent;
            totalSent += sent;
            remaining -= sent;
            std::cout << "client: " << fd<< " sent " << sent << " bytes." <<std::endl; 
            
            //lastActive = std::chrono::high_resolution_clock::now();
			lastActivity = std::chrono::steady_clock::now();
        }
        else if (sent < 0)
        {
            std::perror("send() failed");
            return -1;
        }
        else//fd may close
        {
            if (writeOffset == sendBuffer.size()) 
            {
                sendBuffer.clear();
                writeOffset = 0;
            }
            return 0;
        }
    }

    return static_cast<int> (totalSent);
}

//lastactivity getter
std::chrono::steady_clock::time_point ClientConnection::getLastActivity() const
{
    return lastActivity;
}

void  ClientConnection::clean()
{
    if (fd > 0)
        ::close(fd);
    
    fd = -1;  //in case reclose the fd
}