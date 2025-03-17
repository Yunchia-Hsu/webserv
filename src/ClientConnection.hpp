#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include <iostream>
#include <map>
#include <vector>
#include <cstring> //strerror
#include <sys/select.h> // select()
#include <unistd.h> //close()
#include <chrono>
#include <sys/socket.h>

enum ConnectionState
{
    READ_HEADER,
    READ_BODY,
    PROCESSING,//如果執行 CGI → fork()、execve()；若要回傳靜態檔案 → 讀檔案內容。
    WRITE_RESPONSE,
    DONE
};


class ClientConnection
{
private:
    int fd;
    std::string writeBuffer; // 儲存待傳送給客戶端的資料
    size_t writeOffset = 0;// 已經送了多少
    std::chrono::time_point<std::chrono::steady_clock> lastActive;


    ConnectionState state;

public:
  
    int getFd() const {return fd;}
    bool needWrite ()
    {
        if (writeOffset < writeBuffer.size())
            return true;
        else
            return false;
    }
    bool needRead ()
    {
        //depend on ConnectionState

        //connectionstate are PROCESSING / WRITE_RESPONSE return false 
        return true;
        
    }

    ClientConnection(int cfd): fd(cfd), writeOffset(0)
    {
        //for test
        writeBuffer = "Hello from server!   for test\n";
    }

    int readData() //要進一步確保HTTP request 是完整的
    //檢查是否出現「\r\n\r\n」（標示 HTTP header 結束），以及根據 Content-Length 或 Transfer-Encoding 的資訊來判斷請求體是否接收完整。
    {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int n =  recv(fd, buffer, sizeof(buffer), 0);
        if (n > 0)
        {
           
            writeBuffer.append(buffer, n);
            std::cout << "client: "<< fd << " recieved "<< n <<" bytes" << std::endl;
            // parse http request
           // lastActive = std::chrono::steady_clock::now();
        }
        return n;
    }

    void appendToWriteBuffer(const std::string &data)
    {
        writeBuffer  += data;
    }

    int writeData()
    {
        if (!needWrite())
            return 0;
        // count data length
        size_t remaining = writeBuffer.size() - writeOffset;
        ssize_t totalSent = 0;

        while(remaining >0)
        {
             //call send     writeBuffer.data() 是response
            ssize_t sent = send(fd, writeBuffer.data() + writeOffset, remaining, 0 );
            if (sent > 0)
            {
                writeOffset += sent;
                totalSent += sent;
                remaining -= sent;
                std::cout << "client: " << fd<< " sent " << sent << " bytes." <<std::endl; 
                //lastActive = std::chrono::steady_clock::now();
            }
            else if (sent < 0)
            {
                std::perror("send() failed");
                return -1;
            }
            else//fd may close
                return 0; 

        }
  
        return static_cast<int> (totalSent);
    }


    //lastactivity getter
    std::chrono::steady_clock::time_point getLastActivity() const
    {
        return lastActive;
    }

	void clean()
	{
		if (fd > 0)
			::close(fd);
		
		fd = -1;  //in case reclose the fd
	}
};


#endif