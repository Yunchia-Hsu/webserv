
#ifndef ConfiParser_HPP
#define ConfiParser_HPP

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>
#include <memory>

#include "ServerConf.hpp"
#include "RouteConf.hpp"
#include "location.hpp"
#include "socketWrapper.hpp"
#include "utils.hpp"

class Location;

class ConfiParser
{
    private:
        //store every line of the confifile
        std::vector<std::string> confiLines;
        std::vector<ServerConf> servers;
        std::map<std::string, std::string> globalConfi;

        //NO THIS, BUT save in ServerConf with vector
        std::vector<std::shared_ptr<Location> > _locations;

    public:

        std::map<std::string, std::shared_ptr<SocketWrapper>> portsToSockets;

        ConfiParser();
        ~ConfiParser();

        void parseFile(const std::string& filename);
        void parseServerStuff(std::ifstream& file, ServerConf& server);
        void parseRouteStuff(std::ifstream& file, RouteConf& route);
        void routeKeys(const std::string& keyWord, const std::string& value, RouteConf& route);
        void serverKeys(const std::string& keyWord, const std::string& value, ServerConf& route);
        void parseListener(const std::string& rawValue, std::string& hostOut, int& portOut);
        
        //getter for servers!
        std::vector<ServerConf>& getServers();// { return servers; }


        void testPrinter() const;

        std::vector<std::shared_ptr<Location>> &getLocations();
};

#endif