#include "confiParser.hpp"
#include "RouteConf.hpp"
#include "ServerConf.hpp"

ConfiParser::ConfiParser() {}

ConfiParser::~ConfiParser()  {}

void ConfiParser::parseFile(const std::string& filename)
{
    std::cout << "Starting to PARSE the file" << std::endl;  /// DEBUGPRINT
    
	std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file: " + filename);
    }

	std::set<std::pair<std::string, int> >  usedPorts;
    std::string line;

    while (std::getline(file, line))
    {
        /*
			Ignore whitespace
			Ignore notes etc.
		*/
        line = Utils::trimLine(line);
		if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;


		if  (line == "server")
		{
			if (!std::getline(file, line))
				break;
			line = Utils::trimLine(line);
			
			
			if (line != "{")
			{
				std::cerr << "❌ Expected '{' after 'server'" << std::endl;
				continue ;
			}
		}
		else if (line != "server {")
			continue;

		ServerConf server;
		try {
			parseServerStuff(file, server);
		}
		catch (const std::exception& e) {
			std::cerr << "❌ Error in the parsing block: " << e.what() << std::endl;
			continue ; // skips the whole server if it's faulty
		}
			
		if (server.port == 0 || server.root.empty() || server.index.empty())
		{
			std::cerr << "❌ Skipping incomplete server config (missing port, root, or index)\n";
			continue;
		}

		std::pair<std::string, int> hostPort = std::make_pair(server.host, server.port);
		std::string ip_and_port = hostPort.first + ":" + std::to_string(hostPort.second);

		std::shared_ptr<SocketWrapper> socket(new SocketWrapper(server));
		portsToSockets.insert(std::make_pair(ip_and_port, socket));

		if (usedPorts.find(hostPort) != usedPorts.end()) 
		{
   			std::cerr << "⚠️ WARNING: You're trying to use the same port multiple times! " << std::endl;
		}
		usedPorts.insert(hostPort);

		servers.push_back(server);
		
	}
	file.close();
	testPrinter(); // FOR DEBUG
}

void ConfiParser::serverKeys(const std::string& keyWord, const std::string& value, ServerConf& server)
{
	if (keyWord == "server"  || keyWord == "location" || keyWord == "{")
	{
		std::cerr << "⚠️ Unexpected block directive inside serve. Will ignore it" << std::endl;
		return ;
	}
	if (keyWord  == "listen")
	{
		std::string cleanedValue = value;
		cleanedValue.erase(std::remove_if(cleanedValue.begin(), cleanedValue.end(), ::isspace), cleanedValue.end());

		size_t doubleDot = value.find(":");
		if (doubleDot != std::string::npos)
		{
			server.host = cleanedValue.substr(0, doubleDot);

			std::string portStr = cleanedValue.substr(doubleDot + 1);
			int port = std::stoi(portStr);
			if (port < 1 || port > 65535)
				throw std::runtime_error("Invalid port is not a good port!");

			server.port = port;
		}
		else
		{
			server.host = "0.0.0.0"; // set to default
			int port = std::stoi(cleanedValue);
			if (port < 1 || port > 65535)
				throw std::runtime_error("Invalid port is not a good port!");
			server.port = port;
		}
		
		std::cout << "✅ Parsed port: " << server.port << std::endl; // DEBUG

	}
	else if (keyWord == "server_name")
	{
		std::istringstream iss(value);
		std::string name;
		while (iss >> name)
		{
			server.serverNames.push_back(name);
			// std::cout << "nnnnnnnnn-------------------------------------------------nnnnname: " << name << std::endl;
		}
	}
	else if (keyWord == "root")
	{
		server.root = value;
	}
	else if (keyWord == "index")
	{
		server.index = value;
	}
	else if  (keyWord == "ssl_certificate")
	{
		server.sslCertificate =  value;
	}
	else if (keyWord == "ssl_certificate_key")
	{
		server.sslCertificateKey = value;
	}
	else if (keyWord == "gzip")
	{
		server.gzip  = value;
	}
	else if (keyWord == "client_max_body_size")
	{
		server.clientMaxBodySize = value; // this not right!
		server.clientMaxBodySize = Utils::parseBody(value);
	}
	else if (keyWord == "error_page" || keyWord == "location" || keyWord == "methods" || keyWord == "allow_methods")
	{
		return ;
	}
	else
	{
		std::cerr << "HOIKS! Unrecognized server key -> '" << keyWord << "'" << std::endl;
		server.extraConfi[keyWord] = value;
	}
}

void ConfiParser::parseServerStuff(std::ifstream& file, ServerConf& server)
{
	std::string line;
	int depth = 1; // to track the {}

	int i = 0;
	while (std::getline(file, line))
	{
		/*
			EXEPTION CHECKS:
				check if line is empty, handle brackets, trim whitespace, remove  semicolons
		*/
		i++;
		size_t start = line.find_first_not_of(" \t");
		if (start != std::string::npos)
			line = line.substr(start);
		if (line.empty() || line[0] == '#')
			continue ;
		if (line.find("{") != std::string::npos)
			depth++;
		if (line.find("}") != std::string::npos)
		{
			depth--;
			if (depth == 0)
				break;
			continue ;
		}

		size_t spaces = line.find(" ");
		if (spaces == std::string::npos)
		{
			std::cerr<< "ERROR: Invalid route configurations! '" << line << "'" << std::endl;
			continue ;
		}

		std::string keyWord = line.substr(0, spaces);
		std::string value = line.substr(spaces + 1);

		if (!value.empty() && value.back() == ';')
			value.pop_back();
		if (keyWord == "location")
		{
			std::shared_ptr<Location> location(new Location(&server));
			location->parseLocation(file, line);
		
			server.locations.push_back(location);
		}

		/* 
			Complex Key Words
		*/

		else if (keyWord == "error_page")
		{
			size_t spPos = line.find(" ");
			if  (spPos == std::string::npos)
			{
				std::cerr << "ERROR: Invalid error_page format '" << value << "'" << std::endl;
				continue ;
			}
			int errorCode = std::stoi(value.substr(0, spPos));
			std::string errorFile = value.substr(spPos + 1);
			server.errorPages[errorCode] = errorFile;

		}
		/*
		else if (keyWord == "location")
		{
			RouteConf route;
			route.location = value;
			
		
			std::shared_ptr<Location> location(new Location(&server));
			location->parseLocation(file, line);
			// std::getline(file, line);
			// start = line.find_first_not_of("\t ");
			// if (start != std::string::npos)
			// 	line = line.substr(start);
			// if (line!= "{")
			// {
			// 	std::cerr << "ERROR: Missing '{' after location" << std::endl;
			// 	continue ;
			// }
			// parseRouteStuff(file, route);
			// server.routes.push_back(route.location);
			// std::shared_ptr<Location> location(new Location(&server));
			// location->parseLocation(file, line);
			// location->_path = route.location;
			// loc->_serverRootPath = server.root;
			
			//NO THIS
			//_locations.push_back(location);
			server.locations.push_back(location);
		}
		*/

		else if (keyWord == "methods" || keyWord == "allow_methods")
		{
			std::istringstream methodStream(value);
			std::string method;
			while (methodStream >> method)
				server.methods.insert(method);
		}
		else
			serverKeys(keyWord, value, server);
	}
	
	/*	
		Add default values if needed	
	*/
	if (server.errorPages.empty())
	{
		server.errorPages[404] = "/default_404.html";
		server.errorPages[500] = "/default_500.html";
	}
	if (server.clientMaxBodySize.empty())
	{
		server.clientMaxBodySize = "1M";
	}

	/*
		 Validate before saving
	*/

	if (server.port == 0)
		throw std::runtime_error("Invalid or missing port in server block");

	if (server.root.empty())	
		throw std::runtime_error("Missing root in server block");

	if (server.locations.empty())
		std::cerr << "⚠️ Warning: Server block has no locations defined\n";

//	server.printConfig(); // DEBUGPRINT
}

void ConfiParser::routeKeys(const std::string& keyWord, const std::string& value, RouteConf& route)
{
	if (keyWord == "root")
	{
		route.root = value;
	}
	else if (keyWord == "cgi_path")
	{
		route.CGIPath = value;
	}
	else if (keyWord == "index")
	{
		route.index = value;
	}
	else if (keyWord == "try_files")
	{
		route.tryFiles = value;
	}
	else if (keyWord == "include")
	{
		route.includeFiles.push_back(value);
	}
	else if (keyWord == "return" || keyWord == "redirect")
	{
		route.redirect = value;
	}
	else if (keyWord == "autoindex")
	{
		route.autoindex = (value == "on");
	}

	else if (keyWord == "alias")
	{
		if (!value.empty())
		{
			route.alias = value;
		}
	}

	else if (keyWord == "methods" || keyWord == "allow_methods")
	{
		std::istringstream methodStream(value);
		std::string method;
		while (methodStream >> method)
			route.methods.insert(method);
	}
	else if (keyWord == "location")
	{
		return ;
	}
	else
	{
		std::cerr << "HOIKS! Unrecognized route key-> '" << keyWord << "'" << std::endl;
	}
}

void ConfiParser::parseRouteStuff(std::ifstream& file, RouteConf& route)
{
	std::string line;
	int depth = 1;

	while (std::getline(file, line))
	{
		/*
			EXEPTION CHECKS:
				check if line is empty, handle brackets, trim whitespace, remove  semicolons
		*/
		line = Utils::trimLine(line);
			if (line.empty() || line[0] == '#')
			continue ;

		if (line == "{")
		{
			depth++;
			continue ;
		}

		if  (line.find("}") == 0)
		{
			depth--;
			if(depth == 0)
				return ;
			continue ;
		}
	
		size_t spaces = line.find(" ");
		if (spaces == std::string::npos)
		{
			std::cerr<< "ERROR:: Invalid route configurations! '" << line << "'" << std::endl;
			continue ;
		}

		std::string keyWord = line.substr(0, spaces);
		std::string value = line.substr(spaces + 1);

		if (!value.empty() && value.back() == ';')
			value.pop_back();

		routeKeys(keyWord, value, route);

		/*
			Nested locations! --> It will create loop untill all is set!
		*/

		if  (keyWord == "location")
		{
			RouteConf  nestedRoute;
			nestedRoute.location = value;

			std::getline(file, line);

			line = Utils::trimLine(line);
			if (line != "{")
			{
				std::cerr << "You need '{' after location" << std::endl;
				continue ;
			}
			parseRouteStuff(file, nestedRoute);
			route.nestedRoutes.push_back(nestedRoute);
		}
	}
//	route.printConfig(); // DEBUGPRINT
}

//DEBUG FUNCTION
void ConfiParser::testPrinter() const 
{
	std::cout << "=== Parsed Global Config ===" << std::endl;
    for (const auto& entry : globalConfi)
        std::cout << "  " << entry.first << " = " << entry.second << std::endl;

    std::cout << "\n=== Parsed Servers ===" << std::endl;
    for (size_t i = 0; i < servers.size(); i++)
    {
        std::cout << "Server " << i + 1 << ":" << std::endl;
        servers[i].printConfig();
        std::cout << std::endl;
    }
	std::cout << "File is saved and dandy!" << std::endl;
}

std::vector<std::shared_ptr<Location>>	&ConfiParser::getLocations() {
	return _locations;
}