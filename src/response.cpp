#include "response.hpp"

Response::~Response() {}

Response::Response(std::shared_ptr<ClientConnection> client, ServerConf* server)
	: _request(client),
	_server(server),
	_status_code(STATUS_NOT_FOUND) {
	int error_code = this->has_errors();

	if (error_code) {
		create_response(error_code);
		return;
	}
	if (!_location->_redirectPath.empty()) {
		create_response(_location->_redirectCode);
		return;
	}

	fix_uri();

	auto req = _request.lock();
	if (!req) return ;
	if (Cgi::is_cgi(_location, req->_uri)) {
		if (!init_cgi(client)) {
			_status_code = STATUS_INTERNAL_ERROR;
			finish_response();
			return;
		}
		_status_code = STATUS_OK;
		return;
	}

	if (req->_method == METHOD_GET)
		_status_code = handle_get();
	if (req->_method == METHOD_POST)
		_status_code = handle_post();
	if (req->_method == METHOD_DELETE)
		_status_code = handle_delete();
	std::cout << "status-----------------------"<<_status_code <<std::endl;
	finish_response();
}

void Response::fix_uri(void) {
	auto req = _request.lock();
	if (!req) return;
	if (req->_uri.rfind(_location->_path, 0) == 0) {
		req->_uri.erase(0, _location->_path.size());
		if (req->_uri.front() != '/')
			req->_uri = "/" + req->_uri;
	}
	if (req->_uri.back() != '/')
		return;
	std::string target = _location->_rootPath + req->_uri;
	int fs = Io::file_stat(target);

	if (!(fs & FS_ISDIR))
		return;

	if (!_location->_index.empty()) {
		target = _location->_rootPath + req->_uri + _location->_index;
		int flags = Io::file_stat(target);

		if (flags & FS_ISFILE)
			req->_uri += _location->_index;
	}
}

void Response::finish_response(void) {
	create_response(_status_code);
}

int Response::has_errors(void) {
	auto req = _request.lock();
	if (!req) 
		return STATUS_INTERNAL_ERROR;
	if (req->parse_error)
		return req->parse_error;
	_location = find_location();
	if (_location == nullptr)
		return STATUS_NOT_FOUND;

	if (!ClientConnection::is_method_allowed(_location->_methods, req->_method_str))
		return STATUS_METHOD_NOT_ALLOWED;

	if (!_location->_redirectPath.empty())
		return 0;
	try {
		std::string filename = _location->_rootPath + req->_uri;
		std::filesystem::path resolved = std::filesystem::canonical(filename);

		std::string uri_normalized = resolved.generic_string();
		if (uri_normalized.rfind(_location->_rootPath, 0) != 0)
			return STATUS_NOT_FOUND;
	} catch (const std::exception &e) {
		return 0;
	}
	return 0;
}

void Response::create_response(int status) {
	auto req = _request.lock();
	if (!req) return;
	set_error_page(status);

	const std::string &bs = _body.str();

	buffer << "HTTP/1.1 " << status << " " << code_map[status] << CRLF;
	buffer << "Content-Length: " << bs.size() << CRLF;
	for (auto const &hdr : _additional_headers)
	{
		buffer << hdr.first << ": " << hdr.second << CRLF;
	}
	buffer << "Connection: close" << CRLF;
	buffer << "Date: " << Utils::date_str_now() << CRLF;
	buffer << "Server: " << SERVER_NAME << CRLF;
	if (_location && !_location->_redirectPath.empty())
	{
		buffer << "Location: " << _location->_redirectPath << CRLF;
	}

	if (!_additional_headers.count("Content-Type") && bs.size() > 0)
		buffer << "Content-Type: " << get_content_type(req->_uri) << CRLF;
	if (!_setCookie.empty())
		buffer << "Set-Cookie: " << _setCookie << CRLF;
	buffer << CRLF;
	buffer << bs;
}

int Response::handle_get(void) {
	auto req = _request.lock();
	if (!req)
		return STATUS_INTERNAL_ERROR;
	if (_server) {
		Location* matchedLoca = findBestLocation(_server->locations, req->_uri);
		for (const std::shared_ptr<Location>& loc : _server->locations) {
			if (loc.get() == matchedLoca) {
				_location = loc;
				break;
			}
		}
	}

	std::string path_inside_location = req->_uri;
	if (path_inside_location.find(_location->_path) == 0)
		path_inside_location = path_inside_location.substr(_location->_path.size());
	if (path_inside_location.empty() || path_inside_location[0] != '/')
		path_inside_location = "/" + path_inside_location;

	std::string filename = _location->_rootPath + path_inside_location;
	
	int flags = Io::file_stat(filename);

	if (!flags)
		return STATUS_NOT_FOUND;
	if (!(flags & FS_READ))
		return STATUS_FORBIDDEN;

	if (flags & FS_ISFILE) {
		if (!Io::read_file(filename, _body))
			return STATUS_INTERNAL_ERROR;
		return STATUS_OK;
	}
	if (flags & FS_ISDIR) {
		if (req->_uri.back() != '/')
			return STATUS_NOT_FOUND;

		std::string index = _location->_index.empty() ? "index.html" : _location->_index;
		std::string pathToIndex = filename;
		if (pathToIndex.back() != '/')
			pathToIndex += '/';
		pathToIndex += index;

		int indexFlags = Io::file_stat(pathToIndex);
		if (indexFlags & FS_ISFILE) {
				if (!Io::read_file(pathToIndex, _body))
					return STATUS_INTERNAL_ERROR;
				return STATUS_OK;
			}
		}
		
	// fallback autoinex 
	if (!_location->_autoIndex)
		return STATUS_FORBIDDEN;
	if (!directory_index(filename))
		return STATUS_INTERNAL_ERROR;
	
	return STATUS_OK;
}

int Response::handle_post(void) {
	auto req = _request.lock();
	if (!req) return STATUS_INTERNAL_ERROR;
	std::string filename = _location->_rootPath + req->_uri;
	bool wrote = false;

	if (req->_body_type == BODY_TYPE_CHUNKED &&
			!_location->_uploadPath.empty()
			&& filename.rfind(_location->_uploadPath, 0) == 0) {
		if (!Io::write_file(filename, req->_body))
			return STATUS_INTERNAL_ERROR;
		return STATUS_CREATED;
	}
	int flags = Io::file_stat(filename);

	if (req->parts.empty()) {
		std::cerr << "❌ ERROR: No parts found in multipart POST!\n";
		return STATUS_BAD_REQUEST; // Or STATUS_INTERNAL_ERROR
	}
	for (auto &part : req->parts) {
		if (_location->_uploadPath.empty())
			return STATUS_FORBIDDEN;
		std::string path = _location->_uploadPath + "/" + part.filename;

		if (!Io::write_file(path, part.data))
			return STATUS_INTERNAL_ERROR;
		wrote = true;
	}
	if (wrote)
		return STATUS_CREATED;
	if (!flags)
		return STATUS_NOT_FOUND;
	if (!(flags & FS_READ))
		return STATUS_FORBIDDEN;
	return STATUS_OK;
}

int Response::handle_delete(void) {
	auto req = _request.lock();
	if (!req) return STATUS_INTERNAL_ERROR;
	std::string filename = _location->_rootPath + req->_uri;
	int flags = Io::file_stat(filename);
	if (!flags)
		return STATUS_NOT_FOUND;
	if (!(flags & FS_WRITE))
		return STATUS_FORBIDDEN;
	if (!std::filesystem::remove(filename.c_str()))
		return STATUS_INTERNAL_ERROR;
	return STATUS_OK;
}

void Response::finish_cgi(std::shared_ptr<ClientConnection> req) {
	if (req->parse_error) {
		_status_code = req->parse_error;
		create_response(_status_code);
		return;
	}
	if (req->_headers.count("status"))
		_status_code = std::atoi(req->_headers["status"].c_str());
	if (req->_headers.count("content-type"))
		_additional_headers["Content-Type"] = req->_headers["content-type"];
	if (req->_headers.count("set-cookie"))
		_setCookie = req->_headers["set-cookie"];

	_body << req->_body;
	create_response(_status_code);
}

bool Response::init_cgi(std::shared_ptr<ClientConnection> client) {
	auto req = _request.lock();
	if (!req) return false;
	if (req->_method == METHOD_DELETE) {
		_status_code = STATUS_METHOD_NOT_ALLOWED;
		return false;
	}
	Cgi cgi(_location, req);

	if (!cgi.start(client)) {
		_status_code = STATUS_INTERNAL_ERROR;
		return false;
	}
	client->conn_type = CONN_WAIT_CGI;
	std::cout << "client pid: " << client->pid
          << ", client->cgi_fd_read: " << client->cgi_fd_read << " (read from CGI)"
          << ", client->cgi_fd_write: " << client->cgi_fd_write << " (write to CGI)"
          << std::endl;
	return true;
}

void Response::set_error(int code) {
	this->_status_code = code;
}

void Response::set_error_page(int code) {
	auto req = _request.lock();
	if (!req) return ;
	if (_body.str().size() > 0)
		return;
	if (!(code >= 400 && code <= 599))
		return;
	_additional_headers["Content-Type"] = "text/html";

	if (!req->conf || req->conf->errorPages.count(code) == 0) {
		generate_error_page(code);
		return;
	}

	std::string page_path = req->conf->errorPages[code];
	if (!Io::read_file(page_path, _body))
		generate_error_page(code);
}

void Response::generate_error_page(int code) {
	std::string msg = std::to_string(code) + " " + code_map[code];

	_body << "<!DOCTYPE html><html><head><title>";
	_body << msg;
	_body << "</title>";
	_body << "<style>"
	      << "body { display: flex; justify-content: center; align-items: center; "
	      << "height: 100vh; margin: 0; font-family: Arial, sans-serif; background-color: #f0f0f0; }"
	      << "div { text-align: center; }"
	      << "h1 { font-size: 2em; color: #cc0000; }"
	      << "p { font-size: 1.2em; }"
	      << "</style></head><body>";
	_body << "<div>";
	_body << "<p>This is the error!!!</p>";
	_body << "<h1>" << msg << "</h1>";
	_body << "</div>";
	_body << "</body></html>";
}



std::shared_ptr<Location> Response::find_location(void) {
	auto req = _request.lock();

	if (!req || req->conf == nullptr) 
		return nullptr;
	
	if (req->conf->getLocations().empty()) {
		std::cerr << "❌ ERROR: No parts found in multipart POST!\n";
		return nullptr; // Or STATUS_INTERNAL_ERROR
	}

	std::shared_ptr<Location> ret = req->conf->getLocations().front();
	
	Location defaultpath;
	
	for (const auto &loc : req->conf->getLocations()) {
		if (req->_uri == loc->_path) {
			ret = loc;
			break;
		}
		if (req->_uri.compare(0, loc->_path.length(), loc->_path) == 0 &&
		    (req->_uri.length() == loc->_path.length() || req->_uri[loc->_path.length()] == '/')) {
			if (!ret || loc->_path.size() > ret->_path.size())
				ret = loc;
		}
	
	}
	if (!ret) {
		std::cout << "🌐 No matching location block — falling back to server config\n";
		std::shared_ptr<Location> fallback = std::make_shared<Location>();
		fallback->_rootPath = req->conf->root;
		fallback->_index = req->conf->index;
		fallback->_methods = std::vector<std::string>(req->conf->methods.begin(), req->conf->methods.end());
		return fallback;
	}

	return ret;
}

bool Response::directory_index(std::string path) {
	auto req = _request.lock();
	if (!req) return false;
	DIR *dir;
	struct dirent *entry;

	dir = opendir(path.c_str());
	if (!dir)
		return false;
	_body << "<html><head><title>Index</title></head><body><h2>Index of "
	      << req->_uri << "</h2>";

	std::string href;
	href.reserve(256);

	_body << "<table><thead><tr>";
	_body << "<th>Name</th>";
	_body << "<th>Last Modified</th>";
	_body << "<th>Size</th>";
	_body << "</thead></tr>";
	_body << "<tr><th colspan=\"3\"><hr/></th></tr>";
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		href = entry->d_name;
		std::string e = path + href;

		struct stat sb;
		if (stat(e.c_str(), &sb) != 0)
			continue;
		if (S_ISDIR(sb.st_mode))
			href += "/";
		_body << "<tbody>";
		_body << "<tr><td><a href=\"" << href << "\">" << href << "</td>";
		_body << "<td align=\"right\">" << Utils::time_to_str(sb.st_mtime) << "</td>";
		_body << "<td align=\"right\">" << sb.st_size << "</td></tr>";
		_body << "</tbody>";
	}
	closedir(dir);
	_body << "<tr><th colspan=\"3\"><hr/></th></tr>";
	_body << "</table>";
	_body << "</body></html>";

	return true;
}

std::string Response::get_content_type(std::string uri) {
	size_t pos = uri.find_last_of(".");
	if (pos != std::string::npos) {
		std::string extension = uri.substr(pos);
		if (mime_map.count(extension) > 0)
			return mime_map[extension];
	}
	return mime_map[".html"];
}

void Response::setServer(ServerConf* server) {
	_server = server;
}

Location* Response::findBestLocation(const std::vector<std::shared_ptr<Location>>& locations, const std::string& uri) {
	Location* best = nullptr;
	size_t lenOfBest = 0;

	for (std::vector<std::shared_ptr<Location>>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		const std::string& path = (*it)->_path;

		if (uri.compare(0, path.length(), path) == 0 && 
			(uri.length() == path.length() || uri[path.length()] == '/')) {
			if (uri.length() > lenOfBest) {
				best = it->get();
				lenOfBest = path.length();
			}
		}
	}
	return best;
}