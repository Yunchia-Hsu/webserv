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
#include <set>
#include <memory>

class Location;

class ServerConf 
{
    private:

    public:

        int port;
        size_t maxBody;
        std::string host;
        std::vector<std::string> serverNames;
		std::map<int, std::string> errorPages; // for exmpl 404 -> "404 .html not connecting"
		std::vector<std::string> routes;
        std::string root;
        std::string index;
        std::string sslCertificate;
        std::string sslCertificateKey;

        std::string gzip;
        std::string clientMaxBodySize;

        // additional  settings
        std::map<std::string, std::string> extraConfi;
        std::set<std::string> methods;

        //ADD Server location vector 21.4
        std::vector<std::shared_ptr<Location> > locations;

		ServerConf();
        ServerConf(std::string& name);
		void printConfig() const;
        std::vector<std::shared_ptr<Location> > &getLocations();

};

#endif