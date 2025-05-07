#pragma once

#include "utils.hpp"
#include "ServerConf.hpp"

class Location {
    private:
    public:
	ServerConf *_serverConfig;       //check it later
	std::string _path;
	std::string _rootPath;
	std::string _serverRootPath;
	std::string _index;
	bool _autoIndex;
	bool _autoIndexSet;
	std::vector<std::string> _methods;
	int _redirectCode;
	std::string _redirectPath;
	std::string _uploadPath;
	std::map<std::string, std::string> _cgi;
	bool _session;
	bool _sessionSet;
	size_t clientMaxBodySize;

	Location(ServerConf *serverConfig);
	Location(const Location &original);
	Location();
	~Location();
	Location &operator=(const Location &original);

	//void parseLocation(std::ifstream &configFile, std::string &location_line);
	void  parseLocation(std::ifstream &configFile, const std::string& path);
	void _addPath(std::string &line);
	void _addAutoIndex(std::string &line);
	void _addRoot(std::string &line);
	void _addIndex(std::string &line);
	void _addMethods(std::string &line);
	void _addRedirect(std::string &line);
	void _addUpload(std::string &line);
	void _addCgi(std::string &line);
	void _addSession(std::string &line);
	void _addClientBodySize(const std::string& line);

	bool getAutoIndex();
	void dump(void);
};