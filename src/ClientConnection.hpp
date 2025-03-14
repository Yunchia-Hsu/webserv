#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include <iostream>
#include <map>
#include <vector>
#include <cstring> //strerror
#include <sys/select.h> // select()
#include <unistd.h> //close()

class ClientConnection
{
private:
    int fd;
    std::string writeBffer; // 儲存待傳送給客戶端的資料
    size_t writeOffset = 0;// 已經送了多少

public:
  
    int getFd() const {return fd;}
    bool needWrite ()
    {
        return (writeOffset < writeBuffer.size());//?
    }
    bool needRead (){return true;}
    ClientConnection(int cfd): fd(cfd), writeOffset(0)
    {
        //for test
        writeBuffer = "Hello from server!\n";
    }

    int readData()
    {
        char buffer[1024];
        //memset(buffer, 0, sizeof(buffer));
        //int bytesRead = read(fd, buffer, sizeof(buffer));
        int n =  recv(fd, buffer, sizeof(buffer), 0);
        if (n > 0)
        {
            // parse http request
            writeBuffer.append(buffer, n);
            std::cout << "client: "<< fd << " recieved "<< n <<bytes << std::endl;
        }
        return n;
    }

    void appendToWriteBuffer(const std::string &data)
    {
        writeBuffer = += data;
    }

    int writeData()
    {
        if (!needWrite())
            return 0;
        // count data length
        size_t remaining = writeBuffer.size() - writeOfsset;

        //call send
        ssize_t sent = send (fd, writeBuffer.data() + writeOffset, remaining, 0 );
        if (sent > 0)
        {
            writeOffset += sent;
            std::cout << "client: " << fd<< " wrote " << n << " bytes." <<std::endl; 
        }
        if (sent < 0)
        {
            std::error << "send failed" << std::endl;
            
        }
    }
};


#endif