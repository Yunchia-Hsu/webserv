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
        size_t start = line.find_first_not_of("\t");
        if (start != std::string::npos)
            line = line.substr(start);
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;


		if  (line.find("server") == 0) //		if  (line.find("server {") == 0)
		{
			ServerConf server;
			parseServerStuff(file, server);
			this->servers.push_back(server);
		}
    }
    file.close();

    testPrinter();	/// DEBUGPRINT
}

void ConfiParser::parseRouteStuff(std::ifstream& file, RouteConf& route)
{
	std::string line;
	while (std::getline(file, line))
	{
		/*
			EXEPTION CHECKS:
				check if line is empty, handle brackets, trim whitespace, remove  semicolons
		*/
		size_t start = line.find_first_not_of("\t");
		if (start != std::string::npos)
			line = line.substr(start);
		if (line.empty() || line == "{")
			continue ;
		if (line.find("}") == 0)
			return ;
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

		if (keyWord == "root")
		{
			route.root = value;
		}
		else if (keyWord == "methods")
		{
			route.methods.insert(value);
		}
		else if (keyWord == "cgi_path")
		{
			route.CGIPath = value;
		}
	}
	route.printConfig(); // DEBUGPRINT
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
		size_t start = line.find_first_not_of("\t");
		if (start != std::string::npos)
			line = line.substr(start);
		if (line.empty() || line == "{")
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

		if (keyWord  == "listen")
		{
			server.port = std::stoi(value);
		}
		else if (keyWord == "server_name")
		{
			server.serverName  = value;
		}
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
			route.location = value;;
			parseRouteStuff(file, route);
			server.routes.push_back(route.location);
		}
		else
		{
			server.extraConfi[keyWord] = value;
		}
	}
	server.printConfig(); // DEBUGPRINT
}

void ConfiParser::testPrinter() const
{
	std::cout << "File is saved and dandy!" << std::endl;

}