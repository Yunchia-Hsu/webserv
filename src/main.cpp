
#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>
#include "confiParser.hpp"
#include "Served.hpp"

volatile sig_atomic_t g_running = 1;

void handle_sigint(int signal) 
{
    std::cout << "\n🔧 Caught SIGINT (signal " << signal << "). Cleaning up before exit..." << std::endl;
    g_running = 0; 
}



int main(int arc, char** arv)
{
    // Register SIGINT handler
    std::signal(SIGINT, handle_sigint);

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
        Served server(parser.getServers(), parser.portsToSockets);
		std::vector<int> serverSockets;
        server.start();
		//server.runEventloop();
		while(g_running)
		{
			server.runEventloop();
		}
		std::cout << "🧹 Cleaning resources..." << std::endl;
        server.cleanup();


    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }

    return (0);
}