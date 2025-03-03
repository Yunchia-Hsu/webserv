#include "ServerConf.hpp"
#include <iostream>

ServerConf::ServerConf() : port(80), maxBody(1048576), host("localhost"), serverName("default") {}



/*
    This is here for debugging
*/
void serverName::printConfig()const
{
    std::cout <<  "Server Config: " << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Max Body Size: " << maxBody << " bytes" << std::endl;
    std::cout << "Host: " << host << std::endl;
    std::cout << "Server Name: " << serverName << std::endl;

    std::cout << "Error Pages: " << std::endl;
    for (const auto& entry : errorPages)
        std::cout << "  " << entry.first << " -> " << entry.second << std::endl;

    std::cout << "Routes:" << std::endl;
    for (const auto& route : routes)
        std::cout << "  " << route << std::endl;
}
