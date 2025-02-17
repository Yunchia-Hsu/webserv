
#include <iostream>
#include <string>
#include "confiParser.hpp"


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
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }

    return (0);
}