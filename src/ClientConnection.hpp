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

class Served;
#include "Served.hpp"
#include "utils.hpp"

enum ConnectionState
{
    READ_HEADER,
    READ_BODY,
    PROCESSING,//如果執行 CGI → fork()、execve()；若要回傳靜態檔案 → 讀檔案內容。
    WRITE_RESPONSE,
    DONE
};

enum {
	CONN_REGULAR,
	CONN_WAIT_CGI,
	CONN_CGI,
};

enum {
	BODY_TYPE_NORMAL,
	BODY_TYPE_CHUNKED,
	BODY_TYPE_MULTIPART,
};

struct Part
{
    std::string data;
	std::string name;
	std::string filename;
	std::string content_type;
};

#include <regex>
#include <string>
#include <unordered_map>

#include "utils.hpp"
#include "confiParser.hpp"
#include "response.hpp"

// #define URI_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;="
// #define FIELD_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

// enum {
// 	BODY_TYPE_NORMAL,
// 	BODY_TYPE_CHUNKED,
// 	BODY_TYPE_MULTIPART,
// };

// enum class State {
//     OK,
// 	ERROR,
// 	STATUSLINE,
// 	HEADER,
//     BODY,
// 	CHUNKED,
// 	MULTIPART,
//     CGIHEADER,
// 	CGIBODY,
//     PARTIALSTATUS,
// 	PARTIALHEADER,
// 	PARTIALCHUNKED,
// 	PARTIALCGI,
// 	PARTIALBODY,
// };

// struct Part
// {
//     std::string data;
// 	std::string name;
// 	std::string filename;
// 	std::string content_type;
// };

class Served;

class ClientConnection
{
private:
    

    State parse_status_line(void);
    State parse_header(void);
    bool parse_header_field(size_t pos);
    State parse_body(void);
    State parse_body_cgi(void);
    State parse_chunked(void);
    void parse_multipart(void);

    State parse_header_cgi(void);

public:

    int fd;
    std::string writeBuffer; // 儲存待傳送給客戶端的資料
    std::string sendBuffer; //
    size_t writeOffset = 0;// 已經送了多少
    std::chrono::time_point<std::chrono::steady_clock> lastActivity;
	

    // ConnectionState state;
	int serverPort;  // 新增：用於存儲服務器端口
    // ConnectionState state;

    Served *serve;

    size_t _bytes_read;
    size_t _total_read;
    std::string _buffer;

    bool _cgi;
    
    std::string getwritebubffer()
    {
        return writeBuffer;
    }
    int getFd() const {return fd;}
    bool needWrite ();
    bool needRead ();
    ClientConnection(int cfd, int port, Served *serve);
    int readData(); //要進一步確保HTTP request 是完整的
   int checkend(std::string str, std::string end); //return 0 done, 1 not done
    void appendToWriteBuffer(const std::string &data);
    int writeData();
    // int writeData(std::shared_ptr<ClientConnection> client);
	int getServerPort() const { return serverPort; }
    std::string get_buffer()
    {
        return _buffer;
    }

    Served *get_server()
    {
        return serve;
    }
    //lastactivity getter
	std::chrono::steady_clock::time_point getLastActivity() const;
	
	void clean();

    void cleanup_child();

    State _state;
    int _method;
    int conn_type;
    int parse_error;
    std::string _uri;
    std::string _version;
    std::string _method_str;
    std::string _query_string;
    std::map<std::string, std::string> params;
    std::map<std::string, std::string> _headers;

    size_t _content_len;
    std::string _body;
    int _body_type;

    std::shared_ptr<Response> resp;
    std::string response;

    bool host_matched;
    std::vector<Part> parts;
    std::shared_ptr<ServerConf> conf;

    ClientConnection();
    ClientConnection(bool cgi);
    ClientConnection(int cgi_fd, Served *serve);

    State parse(State s_state,  std::string data, size_t size);
    void check_body_limit(void);
    static bool is_method_allowed(std::vector<std::string> allowed, std::string method);
};
#endif