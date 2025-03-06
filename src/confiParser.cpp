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
			servers.push_back(server);
			//this->servers.push_back(server);
		}
	}
	file.close();
	testPrinter();
}

		/*
		else if (line.find("http") == 0 || line.find("events") == 0)
		{
			std::getline(file, line);
			continue ;
		}
		else
		{
			if (line == "}")
				continue ;
			// store global  settinng outside of the {}
			size_t emptyPos = line.find(" ");
			if (emptyPos == std::string::npos)
			{
				std::cerr << "WARNING: Uknown (gotta catch them all!) global directive -> '" << line << "'" << std::endl;
				continue ;
			}
			std::string key =line.substr(0, emptyPos);
			std::string value = line.substr(emptyPos + 1);
			if (!value.empty() && value.back() == ';')
				value.pop_back();
			globalConfi[key]  = value;

		}
    }
    file.close();

    testPrinter();	/// DEBUGPRINT
}
	*/

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

		
		/*
			"SIMPLE" KEY WORDS
		*/

		if (keyWord  == "listen")
		{
			server.port = std::stoi(value);
		}
		else if (keyWord == "server_name")
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
			server.clientMaxBodySize = value;
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

		else
		{
			std::cerr << "HOIKS! Unrecognized server key -> '" << keyWord << "'" << std::endl;
			server.extraConfi[keyWord] = value;
		}
	}
	server.printConfig(); // DEBUGPRINT
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


		/*
			Simple  Key Words
		*/

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

		else if (keyWord == "alias")
		{
			if (value.empty())
			{
				continue ;
			}
			route.alias = value;
		}

		else if (keyWord == "methods" || keyWord == "allow_methods")
		{
			std::istringstream methodStream(value);
			std::string method;
			while (methodStream >> method)
				route.methods.insert(method);
		}

		/*
			Nested locations! --> It will create loop untill all is set!
		*/
		else if  (keyWord == "location")
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
		
		else
		{
			std::cerr << "HOIKS! Unrecognized route key-> '" << keyWord << "'" << std::endl;
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