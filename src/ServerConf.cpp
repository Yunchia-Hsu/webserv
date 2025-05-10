#include "ServerConf.hpp"
#include <iostream>

ServerConf::ServerConf() : port(80), maxBody(1048576), host("localhost") 
{
    this->serverNames.push_back("default");
}
ServerConf:: ServerConf(std::string& name)
{
    if (name.length() > 13)
        this->serverNames.push_back(name.substr(13));
}
/*
    This is here for debugging
*/

void ServerConf::printConfig() const
{
//    std::cout << "Name: " << serverNames << std::endl;
    std::cout << "Port: " << port << std::endl;
  
    if (!root.empty())
        std::cout << "Root Directory: " << root << std::endl;

    if (!index.empty())
        std::cout << "Index Files: " << index << std::endl;

    std::cout << "Error Pages:" << std::endl;
    for (const auto& entry : errorPages)
        std::cout << "  " << entry.first << " -> " << entry.second << std::endl;
}

std::vector<std::shared_ptr<Location> > &ServerConf::getLocations()
{
	return locations;
}
