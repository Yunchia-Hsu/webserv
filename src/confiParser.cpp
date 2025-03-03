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
        // whitespace off
        size_t start = line.find_first_not_of("\t");
        if (start != std::string::npos)
            line = line.substr(start);

        //ignore # and empty lines

        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;


		if  (line.find("server { ") == 0)
		{
			ServerConf server;
			parseServerStuff(file, server);
			this->servers.push_back(server);
		}
        //push_back every line to the vector
        //confiLines.push_back(line);
    }
    file.close();

    std::cout << "File is saved and dandy!" << std::endl;
    testPrinter();
}

void ConfiParser::parseRouteStuff(std::ifstream& file, RouteConf& route)
{
	std::string line;
	while (std::getline(file, line))
	{
		line = line.substr(line.find_first_not_of("\n"));
		if (line.find("}") == 0)
			return ;
		if (line.find("root") == 0)
		{
			route.root = line.substr(5);
		}
		else if (line.find("methods") == 0)
		{
			route.methods.insert("GET");
		}
		else if (line.find("cgi_path") == 0)
		{
			route.CGIPath = line.substr(9);
		}
	}
}

void ConfiParser::parseServerStuff(std::ifstream& file, ServerConf& server)
{
	std::string line;
	while (std::getline(file, line))
	{
		line = line.substr(line.find_first_not_of("\t"));
		if (line.find("location") == 0)
		{
			RouteConf route;
			parseRouteStuff(file, route);
			server.routes.push_back(route.location);
		}
		else if (line.find("listen") == 0)
		{
			server.port = std::stoi(line.substr(7));
		}
		else if (line.find("server_name") == 0)
		{
			server.serverName  = line.substr(12);
		}
		else if (line.find("error_page") == 0)
		{
			size_t spPos = line.find(" ");
			int errorCode = std::stoi(line.substr(11, spPos - 11));
			std::string errorFile = line.substr(spPos + 1);
			server.errorPages[errorCode] = errorFile;

		}

	}
}

void ConfiParser::testPrinter() const
{
    for (size_t i = 0; i < confiLines.size(); i++)
    {
        std::cout << i + 1 << ": " << confiLines[i] << std::endl;
    }
}