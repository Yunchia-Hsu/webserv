
#include <iostream>
#include <string>
#include "confiParser.hpp"
#include "WebServed.hpp"


int main(int arc, char** arv)
{
    if (arc != 2)
    {
        std::cout << "Input is not inputting correctly!" << std::endl;
        std::cout << "Please, give me a file to work with!" <<std::endl;
        return (1);
    }
    try
    {
        ConfiParser parser;
        parser.parseFile(arv[1]);

        /*
            Try WebServed
        */
        WebServed server(parser.getServers());
        server.start();
        server.cleanup();


    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }

    return (0);
}