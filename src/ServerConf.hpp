/*

    - PORT and HOST
    -  Server name (optional)
    - ERROR list for HTTP
    - max body size
    - A LIST of route settings
*/

#ifndef SERVERCONF_HPP
#define SERVERCONF_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>


class ServerConf 
{
    private:

    public:

        int port;
        size_t maxBody;
        std::string host;
        std::string serverName;
		std::map<int, std::string> errorPages; // for exmpl 404 -> "404 .html not connecting"
		std::vector<std::string> routes;

		ServerConf();
		void printConfig() const;

};

#endif