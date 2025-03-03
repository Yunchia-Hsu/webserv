#include "RouteConf.hpp"

RouteConf() : directoryListings(false), enableCGI(false) {}



/*
    printer for debug
*/

void RouteConf::printConfig() const
{
    std::cout << "Route Config:" << std::endl;
    std::cout << "Location: " << location << std::endl;
    std::cout << "Root Directory: " << root << std::endl;
    
    std::cout << "Allowed Methods: ";
    for (const auto& method : methods)
        std::cout << method << " ";
    std::cout << std::endl;
    
    std::cout << "Directory Listing: " << (directoryListing ? "Enabled" : "Disabled") << std::endl;
    std::cout << "Default File: " << defaultFile << std::endl;
    std::cout << "CGI Enabled: " << (enableCGI ? "Yes" : "No") << std::endl;
    std::cout << "CGI Path: " << cgiPath << std::endl;
    std::cout << "Upload Directory: " << uploadDir << std::endl;
}