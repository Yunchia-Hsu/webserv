#include "ServerConf.hpp"
#include <iostream>

ServerConf::ServerConf() : port(80), maxBody(1048576), host("localhost") 
{
    this->serverNames.push_back("default");
}

/*
    This is here for debugging
*/

void ServerConf::printConfig() const
{
    std::cout << "Server Configuration:" << std::endl;
    std::cout << "Port: " << port << std::endl;
    //std::cout << "Server Name: " << serverNames << std::endl;

    if (!root.empty())
        std::cout << "Root Directory: " << root << std::endl;

    if (!index.empty())
        std::cout << "Index Files: " << index << std::endl;

    std::cout << "Error Pages:" << std::endl;
    for (const auto& entry : errorPages)
        std::cout << "  " << entry.first << " -> " << entry.second << std::endl;

    std::cout << "Routes:" << std::endl;
    for (const auto& route : routes)
        std::cout << "  " << route << std::endl;

    std::cout << "Extra Configurations:" << std::endl;
    for (const auto& entry : extraConfi)
        std::cout << "  " << entry.first << " = " << entry.second << std::endl;
}

