#pragma once

#include <unordered_map>
#include <string>
#include <sstream>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <string>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <sys/wait.h>
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <csignal>

enum {
	STATUS_OK = 200,
	STATUS_CREATED = 201,
	STATUS_BAD_REQUEST = 400,
	STATUS_FORBIDDEN = 403,
	STATUS_NOT_FOUND = 404,
	STATUS_METHOD_NOT_ALLOWED = 405,
	STATUS_TOO_LARGE = 413,
	STATUS_INTERNAL_ERROR= 500,
};

enum class State {
    OK,
	ERROR,
	STATUSLINE,
	HEADER,
    BODY,
	CHUNKED,
	MULTIPART,
    CGIHEADER,
	CGIBODY,
    PARTIALSTATUS,
	PARTIALHEADER,
	PARTIALCHUNKED,
	PARTIALCGI,
	PARTIALBODY,
};

enum { METHOD_GET, METHOD_POST, METHOD_DELETE };

#define TIMEOUT_SECONDS (60)
#define MAX_HEADER_BYTES 8 * 1024

#define READ 0
#define WRITE 1

#define READ_BUFFER_SIZE 2048

#define SERVER_NAME "webserv"
#define REQUEST_BODY_LIMIT 10 * 1024 * 1024 //10MB
#define CRLF "\r\n"
#define CRLF_LEN 2

extern std::unordered_map<int, std::string> code_map;
extern std::unordered_map<std::string, std::string> mime_map;

template <class T> T stringToType(std::string str)
{
	std::istringstream iss(str);
	T result;
	char remain;
	if (!(iss >> result) || iss >> remain)
	{
	    throw std::runtime_error("stringToType: Failed to cast string to given type T!");
	}
	return result;

}

class Utils {
    public:
	static int decode_hex(const char *s, int *out_len);
	static std::string url_decode(const std::string &s);
	static std::string date_str_now(void);
	static std::string date_str_hour_from_now(void);
	static std::string time_to_str(time_t t);
	static std::string get_key_data(std::string &buf, std::string key);
	static std::string safe_substr(std::string &buf, std::string before,
				       std::string after);
	static std::string trim_start(std::string &str, const std::string &needle);
	static int content_len_int(const std::string& input);

	static void removeComments(std::string &line);
	static std::string WspcTrim(std::string string);
	static void skipEmptyLines(std::ifstream &configFile, std::string &line);
	static std::string leftWspcTrim(std::string string);
	static std::string rightWspcTrim(std::string string);
};