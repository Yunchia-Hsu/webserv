/*
	This is Inka's test file!

*/

#ifndef WEBSERVED_HPP
#define WEBSERVED_HPP

#include "confiParser.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class WebServed
{
	private:
		std::vector<ServerConf> servers;


	public:
		WebServed(const std::vector<ServerConf>& parsedServers);
		void start();

};


#endif