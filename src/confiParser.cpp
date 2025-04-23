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

	std::set<std::pair<std::string, int> >  usedPorts;
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
			/*
			// OLD  VERSION
			std::getline(file, line);
			// std::cout<<"---------------------line:" << line <<std::endl;
			ServerConf server(line);                                  // Maybe need to change it to shared point. Will check it later
			parseServerStuff(file, server);
			
			//std::pair<std::string, int> hostPort = {server.host, server.port};
			std::pair<std::string, int> hostPort = std::make_pair(server.host, server.port);
			// std::cout << "hhhhhhhhhhhhost port: " << hostPort.first << " and " << hostPort.second << std::endl;
			std::string ip_and_port = hostPort.first + ":" + std::to_string(hostPort.second);
			// std::cout << "nnnnnnnew ip_and_port: " << ip_and_port << std::endl;
			std::shared_ptr<SocketWrapper> socket(new SocketWrapper(server));
			portsToSockets.insert(std::make_pair(ip_and_port, socket));
			if (usedPorts.find(hostPort) != usedPorts.end()) 
			{
    			std::cerr << "⚠️ WARNING: You're trying to use the same port multiple times! " << std::endl;
			}
			usedPorts.insert(hostPort);

			servers.push_back(server);
			*/
			//NEW VERSION
			if (line.find("{") == std::string::npos)
			{
				std::getline(file, line);
				start = line.find_first_not_of("\t");
				if (start != std::string::npos)
					line = line.substr(start);
				if (line != "{")
					throw std::runtime_error("Expected '{' after server");
			}
			//ServerConf server(line);                                  // Maybe need to change it to shared point. Will check it later
			ServerConf server;
			parseServerStuff(file, server);

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
	}
	file.close();
	testPrinter(); // FOR DEBUG
}

//This function will parser the max-client_body_size from str to size_t
size_t parseBody(const std::string& value)
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
		if (size > 1048576 || size < 0)
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

void ConfiParser::serverKeys(const std::string& keyWord, const std::string& value, ServerConf& server)
{
	// std::cout << "nnnnnnnnn-------------------------------------------------nnnnname: " << std::endl;
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
		server.clientMaxBodySize = parseBody(value);
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
		// std::cout<<"i----------------:"<<i<<":----------"<<line<<std::endl;
		i++;
		size_t start = line.find_first_not_of(" \t");
		if (start != std::string::npos)
			line = line.substr(start);

		/*
		//old version
		if (line.empty() || line == "{" || line[0] == '#')
			continue ;

		if (line.find("}") == 0)
			return ;
		*/
		//new version
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
			value.pop_back(); //Removes the last element in the vector, effectively reducing the container size by one. This destroys the removed elemen


		/*
			21.4
			ADDED HERE the location handler to save all the locations!
		*/
	
		if (keyWord == "location")
		{
			//THIS IS PROBABLY NOT NEEDED!!!!
			// std::string path = value;

			// if (path.find("{") != std::string::npos)
			// {
			// 	path = path.substr(0, path.find("{"));
			// 	size_t end = path.find_last_not_of("\t");
			// 	if (end != std::string::npos)
			// 		path = path.substr(0, end + 1);
			// }
			// else
			// {
			// 	std::getline(file, line);
			// 	start = line.find_first_not_of("\t");
			// 	if (start != std::string::npos)
			// 		line = line.substr(start);
			// 	if (line != "{")
			// 		throw std::runtime_error("Expected '{' after location path");
			// }

			//test lines:
		
			std::shared_ptr<Location> location(new Location(&server));
//			std::cout << "pppppppppppppppppppath: " << path <<std::endl;
			location->parseLocation(file, line);
			
			if (!location->_methods.empty())
				std::cout << "original location: " << location << " method: " << location->_methods.front() << std::endl;
			else
				std::cout << "EMPTY EMPYT EMPTY" << std::endl;
			server.locations.push_back(location);
		}


//		serverKeys(keyWord, value, server);

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

std::vector<std::shared_ptr<Location>>	&ConfiParser::getLocations() {
	return _locations;
}