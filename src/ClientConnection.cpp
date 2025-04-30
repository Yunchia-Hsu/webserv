#include <iostream>
#include "ClientConnection.hpp"


ClientConnection::ClientConnection(int cfd, int port, Served *serve): fd(cfd), writeOffset(0),lastActivity(std::chrono::steady_clock::now()), serverPort(port) // ✅ 初始化
{
    //for test
    // writeBuffer = "Hello from server!   for test\n";
    writeBuffer = "";
    sendBuffer = "";
    std::string response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 8\r\n"
    "Content-Type: text/plain\r\n"
    "Connection: close\r\n"
    "\r\n"
    "fifahey!";
    sendBuffer = response;
    
    this->serve = serve;
	this->_bytes_read = 0;
	this->parse_error = 0;
	this->_state = State::STATUSLINE;
	this->_content_len = 0;
	this->_body_type = BODY_TYPE_NORMAL;
	this->_cgi = false;
	this->conf = nullptr;
	this->_total_read = 0;
	this->host_matched = false;
	this->conn_type = CONN_REGULAR;
}

ClientConnection::ClientConnection(int cgi_fd, Served *serve): fd(cgi_fd), writeOffset(0),lastActivity(std::chrono::steady_clock::now()){
	writeBuffer = "";
    sendBuffer = "";
    std::string response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 8\r\n"
    "Content-Type: text/plain\r\n"
    "Connection: close\r\n"
    "\r\n"
    "fifahey!";
    sendBuffer = response;
    
    this->serve = serve;
	this->_bytes_read = 0;
	this->parse_error = 0;
	this->_state = State::STATUSLINE;
	this->_content_len = 0;
	this->_body_type = BODY_TYPE_NORMAL;
	this->_cgi = false;
	this->conf = nullptr;
	this->_total_read = 0;
	this->host_matched = false;
	this->conn_type = CONN_CGI;
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
				std::cout << "subaaaaaaaaa" << std::endl;
				
				std::string str = writeBuffer.substr(contentlenpos + 15, contentlenend - (contentlenpos + 15));

				std::cout << "subbbbbbbbbbbbbbb: " << str << std::endl;
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

// int ClientConnection::writeData(std::shared_ptr<ClientConnection> client) {
// 	if (!needWrite()) return 0;

// 	// 🆕 Step: Generate response if not already generated
// 	if (!resp) {
// 		resp = std::make_shared<Response>(client);
// 		// Fill response buffer
// 		response = resp->buffer.str();  // or whatever your Response uses
// 		writeOffset = 0; // reset
// 	}

// 	size_t remaining = response.size() - writeOffset;
// 	ssize_t totalSent = 0;

// 	while (remaining > 0) {
// 		ssize_t sent = send(fd, response.data() + writeOffset, remaining, 0);
// 		if (sent > 0) {
// 			writeOffset += sent;
// 			totalSent += sent;
// 			remaining -= sent;
// 			lastActivity = std::chrono::steady_clock::now();
// 		} else if (sent < 0) {
// 			std::perror("send() failed");
// 			return -1;
// 		} else {
// 			return -1; // send returned 0 — assume closed
// 		}
// 	}

// 	if (writeOffset == response.size()) {
// 		response.clear();
// 		writeOffset = 0;
// 	}

// 	return static_cast<int>(totalSent);
// }

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

void    ClientConnection::cleanup_child(void){
    this->serve->cleanup();
    this->clean();
}

std::unordered_map<std::string, int> method_map = { { "GET", METHOD_GET },
						    { "POST", METHOD_POST },
						    { "DELETE", METHOD_DELETE } };

ClientConnection::ClientConnection()
{
	this->_bytes_read = 0;
	this->parse_error = 0;
	this->_state = State::STATUSLINE;
	this->_content_len = 0;
	this->_body_type = BODY_TYPE_NORMAL;
	this->_cgi = false;
	this->conf = nullptr;
	this->_total_read = 0;
	this->host_matched = false;
}

ClientConnection::ClientConnection(bool cgi)
	: ClientConnection()
{
	this->_cgi = cgi;
}

const char* state_to_string(State s) {
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


State ClientConnection::parse(State s_start, std::string data, size_t size)
{
	_buffer.append(data, size);
	_bytes_read = size;
	_total_read += size;

	if (_state < s_start)
		_state = s_start;
	// std::cout << "string: " << data << "Current state: " << state_to_string(_state) << "s_state: " << state_to_string(s_start) << std::endl;
	while (_state != State::OK && _state != State::ERROR)
	{
		switch (_state)
		{
		case State::PARTIALSTATUS:
		case State::STATUSLINE:
			_state = parse_status_line();
			break;
		case State::PARTIALHEADER:
		case State::HEADER:
			_state = parse_header();
			break;
		case State::PARTIALBODY:
		case State::BODY:
			_state = parse_body();
			break;
		case State::CGIHEADER:
			_state = parse_header_cgi();
			break;
		case State::PARTIALCGI:
		case State::CGIBODY:
			_state = parse_body_cgi();
			break;
		case State::PARTIALCHUNKED:
		case State::CHUNKED:
			_state = parse_chunked();
			break;
		case State::MULTIPART:
			parse_multipart();
			break;
		default:
			_state = State::ERROR;
			break;
		}
		if (_state >= State::PARTIALSTATUS && _state <= State::PARTIALBODY)
			break;
		// std::cout << "Current eee: " << this->parse_error << std::endl;
	}
	if (_state == State::ERROR && !this->parse_error)
	{
		if (_cgi)
			this->parse_error = 502;
		else
			this->parse_error = STATUS_BAD_REQUEST;
	}
	return _state;
}

static std::regex r(R"((\S+) (\S+) (\S+))");

State ClientConnection::parse_status_line(void)
{
	
	// std::cout << "ppac state: "  << std::endl;
	if (_total_read >= MAX_HEADER_BYTES)
	{
		this->parse_error = STATUS_BAD_REQUEST;
		return State::ERROR;
	}
	size_t pos = _buffer.find(CRLF);

	if (pos == std::string::npos)
	{
		return State::PARTIALSTATUS;
	}
	std::string str = _buffer.substr(0, pos);
	std::smatch m;
	std::cout << "ppa state: " << str << std::endl;
	if (!std::regex_match(str, m, r))
		return State::ERROR;
	
	_method_str = m[1];
	_uri = Utils::url_decode(m[2]);
//	std::cout << "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuurl: " << _uri << std::endl;
	_version = m[3];

	if (method_map.count(_method_str) == 0)
	{
		this->parse_error = 501;
		return State::ERROR;
	}
	if (_uri.at(0) != '/' || _version.rfind("HTTP/", 0) != 0)
		return State::ERROR;
	if (_version != "HTTP/1.1") {
		this->parse_error = 505;
		return State::ERROR;
	}
	size_t pos_q = _uri.find("?");
	if (pos_q != std::string::npos)
	{
		_query_string = _uri.substr(pos_q + 1, _uri.size());
		_uri.erase(pos_q, _uri.size());
	}
	_method = method_map[_method_str];

	_buffer.erase(0, pos + 2);
	
	return State::HEADER;
}

State ClientConnection::parse_header(void)
{
	if (_total_read >= MAX_HEADER_BYTES)
		return State::ERROR;
	size_t pos = _buffer.find(CRLF);

	if (pos == std::string::npos)
		return State::PARTIALHEADER;
	if (pos == 0)
	{
		_buffer.erase(0, CRLF_LEN);
		return State::BODY;
	}
	return parse_header_field(pos) ? State::HEADER : State::ERROR;
}

State ClientConnection::parse_header_cgi(void)
{
	if (_total_read >= MAX_HEADER_BYTES)
		return State::ERROR;
	size_t pos = _buffer.find(CRLF);

	if (pos == std::string::npos)
		return State::CGIBODY;
	if (pos == 0)
	{
		_buffer.erase(0, CRLF_LEN);
		return State::CGIBODY;
	}
	_state = parse_header_field(pos) ? State::CGIHEADER : State::ERROR;
	return _state;
}

bool ClientConnection::parse_header_field(size_t pos)
{
	std::string line = _buffer.substr(0, pos);
	_buffer.erase(0, pos + CRLF_LEN);

	std::regex regex(R"((\S+): (.+))");
	std::smatch m;
	std::cout<<"Header:"<<line<<std::endl;
	if (!std::regex_match(line, m, regex))
		return false;
	std::string key = m[1];
	std::string value = m[2];
	std::transform(key.begin(), key.end(), key.begin(),
		       [](unsigned char c) { return std::tolower(c); });

	_headers[key] = value;
	if (key == "content-length")
	{
		int tmp = Utils::content_len_int(value);
		if (tmp < 0)
			return false;
		_content_len = tmp;
	}
	if (key == "transfer-encoding" && value == "chunked")
		_body_type = BODY_TYPE_CHUNKED;
	if (key == "content-type" && value.find("multipart/form-data; boundary=") == 0)
		_body_type = BODY_TYPE_MULTIPART;
	return true;
}

State ClientConnection::parse_body(void)
{
	if (_headers.count("host") == 0)
		return State::ERROR;
	if (_body_type == BODY_TYPE_CHUNKED)
		return State::CHUNKED;
	bool has_length = _headers.count("content-length");
	if (!has_length && _buffer.size() > 0) {
		return State::ERROR;
	}

	if (has_length && _buffer.size() < _content_len)
		return State::PARTIALBODY;
	if (_body_type == BODY_TYPE_MULTIPART)
		return State::MULTIPART;
	_body = _buffer.substr(0, _content_len);
	_buffer.clear();
	return State::OK;
}

State ClientConnection::parse_body_cgi(void)
{
	if (_bytes_read == 0) //eof
		return State::OK;
	if (_body_type == BODY_TYPE_CHUNKED)
		return State::CHUNKED;
	if (!_headers.count("content-length"))
	{
		_body += _buffer;
		_buffer.clear();
		return State::PARTIALBODY;
	}
	_body += _buffer;
	_buffer.clear();
	if (_body.size() < _content_len)
		return State::PARTIALCGI;
	_buffer.clear();
	return State::OK;
}

State ClientConnection::parse_chunked(void)
{
	// if (conf && _buffer.size() > conf->getMaxSize())
	// {
	// 	this->parse_error = STATUS_TOO_LARGE;
	// 	return State::ERROR;
	// }
	if (_bytes_read == 0)
		return State::OK;
	size_t pos = _buffer.find(CRLF);
	if (pos == std::string::npos)
		return State::PARTIALCHUNKED;

	int num_len = 0;
	int chunk_len = Utils::decode_hex(_buffer.c_str(), &num_len);

	if (chunk_len == -1 || num_len == 0)
		return State::ERROR;
	if (chunk_len == 0)
		return State::OK;
	_buffer.erase(0, num_len + CRLF_LEN);
	std::string chunk = _buffer.substr(0, chunk_len);
	_body += chunk;
	_buffer.erase(0, chunk.size());
	_buffer.erase(0, 2);

	return State::CHUNKED;
}

void ClientConnection::parse_multipart(void)
{
	std::regex ptrn(".*boundary=(.*)");
	std::smatch match_res;

	if (!std::regex_match(_headers["content-type"], match_res, ptrn))
	{
		this->_state = State::ERROR;
		return;
	}
	std::string boundary = "--";
	boundary += match_res[1];
	boundary += "\r\n";

	size_t pos = 0, end = 0, header_end = 0;
	while ((pos = _buffer.find(boundary, pos)) != std::string::npos)
	{
		if ((end = _buffer.find(boundary, pos += boundary.size())) ==
		    std::string::npos)
			break;
		size_t buf_size = end - pos;
		if (end - pos >= 2)
			buf_size -= 2;
		std::string part_buf = _buffer.substr(pos, buf_size);
		pos = end;
		if ((header_end = part_buf.find("\r\n\r\n")) == std::string::npos)
			continue;

		struct Part part;
		part.data = part_buf.substr(header_end + 4);
		part_buf.erase(header_end);
		part.name = Utils::get_key_data(part_buf, "name");
		part.filename = Utils::get_key_data(part_buf, "filename");
		part.content_type = Utils::safe_substr(part_buf, "Content-Type: ", CRLF);
		if (part.filename.empty() || part.data.empty())
			continue;

		/*
		std::cout << "part.name: " << part.name << std::endl;
		std::cout << "part.filename: " << part.filename << std::endl;
		std::cout << "part.content_type: " << part.content_type << std::endl;
		std::cout << "part.data.size: " << part.data.size() << std::endl;
		*/
		this->parts.push_back(part);
	}
	_state = State::OK;
	_buffer.clear();
}

void ClientConnection::check_body_limit(void)
{
	if (!this->conf || _headers.count("host") == 0)
		return;
	// if (_body.size() > this->conf->getMaxSize())
	// {
	// 	this->parse_error = STATUS_TOO_LARGE;
	// 	this->_state = State::ERROR;
	// }
}

bool ClientConnection::is_method_allowed(std::vector<std::string> allowed, std::string method)
{
	// std::cout << "allow begin: " << *allowed.begin() << " allow end: " << *allowed.end() << " method: " << method << std::endl;
	// for (const auto &var : allowed){
	// 	std::cout << "allowwwwwwwwwww: " << var << std::endl;
	// }

	if (std::find(allowed.begin(), allowed.end(), method) == allowed.end())
	{
		return false;
	}
	return true;
}