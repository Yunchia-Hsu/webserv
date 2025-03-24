#include "request.hpp"

std::unordered_map<std::string, int> method_map = { { "GET", METHOD_GET },
						    { "POST", METHOD_POST },
						    { "DELETE", METHOD_DELETE } };

Request::Request()
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

Request::Request(bool cgi)
	: Request()
{
	this->_cgi = cgi;
}

Request::~Request()
{
}

State Request::parse(State s_start, char *data, size_t size)
{
	_buffer.append(data, size);
	_bytes_read = size;
	_total_read += size;

	if (_state < s_start)
		_state = s_start;
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

State Request::parse_status_line(void)
{
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
	if (!std::regex_match(str, m, r))
		return State::ERROR;
	_method_str = m[1];
	_uri = Utils::url_decode(m[2]);
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

State Request::parse_header(void)
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

State Request::parse_header_cgi(void)
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

bool Request::parse_header_field(size_t pos)
{
	std::string line = _buffer.substr(0, pos);
	_buffer.erase(0, pos + CRLF_LEN);

	std::regex regex(R"((\S+): (.+))");
	std::smatch m;

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

State Request::parse_body(void)
{
	if (_headers.count("host") == 0)
		return State::ERROR;
	// if (conf && _buffer.size() > conf->getMaxSize())
	// {
	// 	this->parse_error = STATUS_TOO_LARGE;
	// 	return State::ERROR;
	// }
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

State Request::parse_body_cgi(void)
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

State Request::parse_chunked(void)
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

void Request::parse_multipart(void)
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

void Request::check_body_limit(void)
{
	if (!this->conf || _headers.count("host") == 0)
		return;
	// if (_body.size() > this->conf->getMaxSize())
	// {
	// 	this->parse_error = STATUS_TOO_LARGE;
	// 	this->_state = State::ERROR;
	// }
}

bool Request::is_method_allowed(std::vector<std::string> allowed, std::string method)
{
	if (std::find(allowed.begin(), allowed.end(), method) == allowed.end())
	{
		return false;
	}
	return true;
}

