#pragma once

#include "io.hpp"
#include "utils.hpp"
#include "location.hpp"
#include "ClientConnection.hpp"

extern std::unordered_map<std::string, std::string> cgi_map;

class Location;
class ClientConnection;

static_assert(__cplusplus >= 201703L, "C++17 is not enabled!");

class Cgi {
    private:
	std::string _document_root;
	std::string _interpreter;
	std::string _script_abs;
	std::string _script_path;
	std::string _script_dir;
	std::vector<std::string> _env;

	void env_set(const std::string &key, const std::string &value);
	void env_set_vars(std::shared_ptr<ClientConnection> request);

	bool parent_init(int pid, int *fd_from, int *fd_to);
	void child_process(std::shared_ptr<ClientConnection> client,
			   int *fd_from, int *fd_to);

    public:
	Cgi();
	Cgi(std::shared_ptr<Location> location, std::shared_ptr<ClientConnection> request);
	~Cgi();

	bool start(std::shared_ptr<ClientConnection> client);
	static bool finish(int pid);
	static void close_pipes(int *fd);
	static bool is_cgi(std::shared_ptr<Location> location, std::string uri);
};