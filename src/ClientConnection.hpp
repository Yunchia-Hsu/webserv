#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP


#include "confiParser.hpp"
#include <iostream>
#include <vector>
#include <map>

// POSIX 網路相關標頭 in order
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>//定義 sockaddr_in、in_addr_t
#include <unistd.h>// close(), read(), write()

// C++ 封裝標頭
#include <cstring>   // for memset
#include <cerrno>    // for errno

#include <chrono>
#include "Served.hpp"

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
    std::string sendBuffer;
    size_t writeOffset = 0;// 已經送了多少
    std::chrono::time_point<std::chrono::steady_clock> lastActivity;
	

    ConnectionState state;

public:
  
    int getFd() const {return fd;}
    bool needWrite ();
    bool needRead ();
    ClientConnection(int cfd);
    int readData(); //要進一步確保HTTP request 是完整的
   int checkend(std::string str, std::string end); //return 0 done, 1 not done
    void appendToWriteBuffer(const std::string &data);
    int writeData();

    //lastactivity getter
	std::chrono::steady_clock::time_point getLastActivity() const;
   
	void clean();

};
#endif