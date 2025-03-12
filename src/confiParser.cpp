#include "confiParser.hpp"
#include "RouteConf.hpp"
#include "ServerConf.hpp"

ConfiParser::ConfiParser() {}

ConfiParser::~ConfiParser()  {}

void ConfiParser::parseFile(const std::string& filename)
{
    std::cout << "Starting to PARSE the file" << std::endl;  /// DEBUGPRINT
    
	std::ifstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open the file: " + filename);
    }

	std::set<std::pair<std::string, int>> usedPorts;

    std::string line;
    while (std::getline(file, line))
    {
        /*
			Ignore whitespace
			Ignore notes etc.
		*/
        size_t start = line.find_first_not_of(" \t");
        if (start != std::string::npos)
            line = line.substr(start);
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;


		if  (line.find("server") == 0) //if  (line.find("server {") == 0)
		{
			std::getline(file, line);
			ServerConf server;
			parseServerStuff(file, server);
		
			std::pair<std::string, int> hostPort = {server.host, server.port};
			if (usedPorts.find(hostPort) != usedPorts.end()) 
			{
    			std::cerr << "⚠️ WARNING: You're trying to use the same port multiple times! " << std::endl;
			}
			usedPorts.insert(hostPort);

			servers.push_back(server);
		}
	}
	file.close();
	testPrinter();
}

void ConfiParser::serverKeys(const std::string& keyWord, const std::string& value, ServerConf& server)
{
	if (keyWord  == "listen")
	{
		std::cout  << "Raw listen value: " << value << std::endl; //testestestest
		server.port = std::stoi(value.substr(value.find(":") + 1));
		std::cout << "✅ Parsed port: " << server.port << std::endl;
	}
	else if (keyWord == "server_name") // we should handle multiple
	{
		server.serverName  = value;
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
	while (std::getline(file, line))
	{
		/*
			EXEPTION CHECKS:
				check if line is empty, handle brackets, trim whitespace, remove  semicolons
		*/
		size_t start = line.find_first_not_of(" \t");
		if (start != std::string::npos)
			line = line.substr(start);

		if (line.empty() || line == "{" || line[0] == '#')
			continue ;

		if (line.find("}") == 0)
			return ;
		size_t spaces = line.find(" ");
		if (spaces == std::string::npos)
		{
			std::cerr<< "ERROR: Invalid route configurations! '" << line << "'" << std::endl;
			continue ;
		}

		std::string keyWord = line.substr(0, spaces);
		std::string value = line.substr(spaces + 1);

		if (!value.empty() && value.back() == ';')
			value.pop_back(); //Removes the last element in the vector, effectively reducing the container size by one. This destroys the removed elemen

		serverKeys(keyWord, value, server);

		/* 
			Complex Key Words
		*/

		if (keyWord == "error_page")
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
		else if (keyWord == "location")
		{
			RouteConf route;
			route.location = value;
			/*
				CHANGED!
			*/
			std::getline(file, line);
			start = line.find_first_not_of("\t ");
			if (start != std::string::npos)
				line = line.substr(start);
			if (line!= "{")
			{
				std::cerr << "ERROR: Missing '{' after location" << std::endl;
				continue ;
			}
			/*
				END OF CHANGE
			*/
			parseRouteStuff(file, route);
			server.routes.push_back(route.location);
		}

		else if (keyWord == "methods" || keyWord == "allow_methods")
		{
			std::istringstream methodStream(value);
			std::string method;
			while (methodStream >> method)
				server.methods.insert(method);
		}
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

	server.printConfig(); // DEBUGPRINT
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
		size_t start = line.find_first_not_of(" \t");
		if (start != std::string::npos)
			line = line.substr(start);
	
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
			value.pop_back(); //Removes the last element in the vector, effectively reducing the container size by one. This destroys the removed elemen		

		routeKeys(keyWord, value, route);

		/*
			Nested locations! --> It will create loop untill all is set!
		*/
		if  (keyWord == "location")
		{
			RouteConf  nestedRoute;
			nestedRoute.location = value;

			std::getline(file, line);
			start  = line.find_first_not_of("\t ");
			if (start != std::string::npos)
				line  = line.substr(start);
			if (line != "{")
			{
				std::cerr << "You need '{' after location" << std::endl;
				continue ;
			}
			parseRouteStuff(file, nestedRoute);
			route.nestedRoutes.push_back(nestedRoute);
		}
	}
	route.printConfig(); // DEBUGPRINT
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