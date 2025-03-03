
#ifndef ConfiParser_HPP
#define ConfiParser_HPP


#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>

#include "ServerConf.hpp"
#include "RouteConf.hpp"


class ConfiParser
{
    private:
        //store every line of the confifile
        std::vector<std::string> confiLines;
        std::vector<ServerConf> servers;

    public:
        ConfiParser();
        ~ConfiParser();

        void parseFile(const std::string& filename);
        void parseServerStuff(std::ifstream& file, ServerConf& server);
        void parseRouteStuff(std::ifstream& file, RouteConf& route);


        void testPrinter() const;

};

#endif