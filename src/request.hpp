#pragma once

#include <map>
#include <regex>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "utils.hpp"
#include "confiParser.hpp"

#define URI_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;="
#define FIELD_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

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

// class ClientConnection;

class ConfiParser;

class Request {
    public:
        State _state;
        int _method;
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

        bool host_matched;
        std::vector<Part> parts;
        std::shared_ptr<ConfiParser> conf;
        std::shared_ptr<ServerConf> _server;

        Request();
        Request(bool cgi);
        ~Request();

        State parse(State s_state, char *data, size_t size);
        void check_body_limit(void);
	    static bool is_method_allowed(std::vector<std::string> allowed, std::string method);

        
    
    private:
        size_t _bytes_read;
        size_t _total_read;
        std::string _buffer;

        bool _cgi;

        State parse_status_line(void);
        State parse_header(void);
        bool parse_header_field(size_t pos);
        State parse_body(void);
        State parse_body_cgi(void);
        State parse_chunked(void);
        void parse_multipart(void);

        State parse_header_cgi(void);

};