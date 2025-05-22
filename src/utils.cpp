#include "utils.hpp"

std::unordered_map<int, std::string> code_map = {
	{ 100, "Continue" },
	{ 101, "Switching Protocols" },
	{ 102, "Processing" },

	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 203, "Non-authoritative Information" },
	{ 204, "No Content" },
	{ 205, "Reset Content" },
	{ 206, "Partial Content" },
	{ 207, "Multi-Status" },
	{ 208, "Already Reported" },
	{ 226, "IM Used" },

	{ 300, "Multiple Choices" },
	{ 301, "Moved Permanently" },
	{ 302, "Found" },
	{ 303, "See Other" },
	{ 304, "Not Modified" },
	{ 305, "Use Proxy" },
	{ 307, "Temporary Redirect" },
	{ 308, "Permanent Redirect" },

	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 402, "Payment Required" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 405, "Method Not Allowed" },
	{ 406, "Not Acceptable" },
	{ 407, "Proxy Authentication Required" },
	{ 408, "Request Timeout" },
	{ 409, "Conflict" },
	{ 410, "Gone" },
	{ 411, "Length Required" },
	{ 412, "Precondition Failed" },
	{ 413, "Payload Too Large" },
	{ 414, "Request-URI Too Long" },
	{ 415, "Unsupported Media Type" },
	{ 416, "Requested Range Not Satisfiable" },
	{ 417, "Expectation Failed" },
	{ 418, "I’m a teapot" },
	{ 421, "Misdirected Request" },
	{ 422, "Unprocessable Entity" },
	{ 423, "Locked" },
	{ 424, "Failed Dependency" },
	{ 426, "Upgrade Required" },
	{ 428, "Precondition Required" },
	{ 429, "Too Many Requests" },
	{ 431, "Request Header Fields Too Large" },
	{ 444, "Connection Closed Without Response" },
	{ 451, "Unavailable For Legal Reasons" },
	{ 499, "Client Closed Request" },

	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 502, "Bad Gateway" },
	{ 503, "Service Unavailable" },
	{ 504, "Gateway Timeout" },
	{ 505, "HTTP Version Not Supported" },
	{ 506, "Variant Also Negotiates" },
	{ 507, "Insufficient Storage" },
	{ 508, "Loop Detected" },
	{ 510, "Not Extended" },
	{ 511, "Network Authentication Required" },
	{ 599, "Network Connect Timeout Error" }
};

std::unordered_map<std::string, std::string> mime_map = {
	{ "", "text/plain" },
	{ ".au", "audio/basic" },
	{ ".avi", "video/x-msvideo" },
	{ ".css", "text/css" },
	{ ".csv", "text/csv" },
	{ ".gif", "image/gif" },
	{ ".gz", "application/x-gzip" },
	{ ".htm", "text/html" },
	{ ".html", "text/html" },
	{ ".ico", "image/x-icon" },
	{ ".jpeg", "image/jpeg" },
	{ ".jpg", "image/jpeg" },
	{ ".json", "application/json" },
	{ ".mp3", "audio/mpeg" },
	{ ".mp4", "video/mp4" },
	{ ".mpeg", "video/mpeg" },
	{ ".mpg", "video/mpeg" },
	{ ".pdf", "application/pdf" },
	{ ".png", "image/png" },
	{ ".rtf", "application/rtf" },
	{ ".svg", "image/svg+xml" },
	{ ".tar", "application/x-tar" },
	{ ".txt", "text/plain" },
	{ ".word", "application/msword" },
	{ ".xhtml", "application/xhtml+xml" },
	{ ".xml", "text/xml" },
	{ "default", "text/plain" }
};

int Utils::decode_hex(const char *s, int *out_len) {
	int ret = 0;
	int val = 0;
	char c;
	const char *charset = "0123456789abcdefABCDEF";

	if (std::strchr(charset, *s) == NULL)
		return -1;
	while (std::strchr(charset, *s)) {
		c = *s;
		if (c >= '0' && c <= '9')
			val = c - '0';
		else if (c >= 'a' && c <= 'f')
			val = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			val = c - 'A' + 10;
		ret = (ret * 16) + val;
		*out_len = *out_len + 1;
		s++;
	}
	return ret;
}

std::string Utils::url_decode(const std::string &s) {
	std::string res = "";

	for (size_t i = 0; i < s.length(); i++) {
		if (s[i] == '%' && (s.length() - i) >= 2) {
			int out;
			sscanf(s.substr(i + 1, 2).c_str(), "%x", &out);
			char ch = static_cast<char>(out);
			res += ch;
			i += 2;
			continue;
		}
		res += s[i];
	}
	return res;
}

std::string Utils::date_str_now(void) {
	time_t t;
	struct tm *time_struct;
	char buf[128];

	std::time(&t);
	time_struct = std::gmtime(&t);
	std::strftime(buf, sizeof(buf) - 1, "%a, %d %b %Y %H:%M:%S GMT", time_struct);
	return std::string(buf);
}

std::string Utils::time_to_str(time_t t) {
	std::string res = "";
	char time_buf[64];
	struct tm* ti = localtime(&t);

	if (!ti)
		return res;
	std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", ti);
	return std::string(time_buf);
}

std::string Utils::get_key_data(std::string &buf, std::string key) {
	size_t pos = buf.find(key + "=\"");
	if (pos == std::string::npos)
		return "";
	pos += key.size() + 2;
	size_t end = buf.find("\"", pos);
	if (end == std::string::npos)
		return "";
	return buf.substr(pos, end - pos);
}

std::string Utils::safe_substr(std::string &buf, std::string before, std::string after) {
	size_t pos = buf.find(before);
	if (pos == std::string::npos)
		return "";
	size_t end = buf.find(after, pos);
	if (end == std::string::npos)
		return buf.substr(pos + before.size());
	return buf.substr(pos, end - pos);
}

int Utils::content_len_int(const std::string& input) {
	int result = -1;
	try {
		result = std::stoul(input);
	} catch (const std::invalid_argument& e) {
		return -1;
	} catch (const std::out_of_range& e) {
		return -1;
	}
	return result;
}

void Utils::removeComments(std::string &line) {
	line = std::regex_replace(line, std::regex("#.*$"), "");
}

std::string Utils::leftWspcTrim(std::string string) {
	const char *ws = " \t\n\r\f\v";
	string.erase(0, string.find_first_not_of(ws));
	return string;
}

std::string Utils::rightWspcTrim(std::string string) {
	const char *ws = " \t\n\r\f\v";
	string.erase(string.find_last_not_of(ws) + 1);
	return string;
}

std::string Utils::WspcTrim(std::string string) {
	return leftWspcTrim(rightWspcTrim(string));
}

void Utils::skipEmptyLines(std::ifstream &configFile, std::string &line) {
	while ((std::getline(configFile, line)) &&
	       (removeComments(line), WspcTrim(line).empty()))
		;
}

std::string Utils::trimLine(const std::string& str) {
	size_t first = str.find_first_not_of(" \t");
	if (first == std::string::npos)
		return "";
	size_t last = str.find_last_not_of(" \t");
	return str.substr(first, (last - first + 1));
}

//This function will parser the max-client_body_size from str to size_t
size_t Utils::parseBody(const std::string& value)
{
	size_t multip = 1;
	std::string n = value;

	if (value.empty())
	{
		std::cerr << "⚠️ Warning! Wrong max_body set, default(1M) used instead" << std::endl;
		return 1048576; // 1M as bytes
	}

	//converter
	char sizeC = value.back();

	if (sizeC == 'K' || sizeC == 'k')
	{
        multip = 1024; // Convert KB to bytes
        n.pop_back();		
	}
	else if (sizeC == 'M' || sizeC == 'M')
	{
        multip = 1024 * 1024; // Convert MB to bytes
        n.pop_back();
	}
	else if (sizeC == 'G' || sizeC == 'g')
	{
        multip = 1024 * 1024 * 1024; // Convert GB to bytes
        n.pop_back();
	}
	else
	{
		std::cerr << "⚠️ Warning! I need a unit for my  Body, assuming (M)" << std::endl;
		multip = 1024 * 1024; // Convert MB to bytes
	}
	try
	{
		size_t size = std::stoul(n) * multip;
		if (size > 1048576 )
		{
			std::cerr << "⚠️ Warning! Max body is 1M, so it was set as 1M" << std::endl;
			return 1048576;
		}
		return size;
	}
	catch (...)
	{
		std::cerr << "Error error, inalid BodySize, setting the size as 1M" << std::endl;
		return 1048576;
	}
}
