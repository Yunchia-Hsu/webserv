#pragma once

#include "cgi.hpp"
#include "location.hpp"
#include "ClientConnection.hpp"

class Location;
class ClientConnection;

class Response {
    private:
	std::weak_ptr<ClientConnection> _client;
	std::weak_ptr<ClientConnection> _request;
	std::shared_ptr<Location> _location;
	std::vector<std::shared_ptr<Location>> _locations;

	std::map<std::string, std::string> _additional_headers;

	ServerConf* _server = nullptr;

	int _status_code;
	std::string _setCookie;

	int has_errors(void);
	void create_response(int status);

	void fix_uri(void);

	int handle_post(void);
	int handle_delete(void);
	int handle_get(void);

	std::shared_ptr<Location> find_location(void);
	void set_error_page(int code);
	void generate_error_page(int code);

	bool directory_index(std::string path);
	std::string get_content_type(std::string uri);

	bool init_cgi(std::shared_ptr<ClientConnection> client);

    public:
	std::ostringstream _body;
	std::ostringstream buffer;

	Response(std::shared_ptr<ClientConnection> client, ServerConf* server);
	void finish_response(void);
	void finish_cgi(std::shared_ptr<ClientConnection> req_cgi);
	void set_error(int code);
	~Response();

	void setServer(ServerConf* server);

	Location* findBestLocation(const std::vector<std::shared_ptr<Location>>& locations, const std::string& uri);
};
