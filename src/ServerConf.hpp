/*

    - PORT and HOST
    -  Server name (optional)
    - ERROR list for HTTP
    - max body size
    - A LIST of route settings
*/

#ifndef SERVERCONF_HPP
#define SERVERCONF_HPP

class ServerConf 
{
    private:

    public:

        int port;
        size_t maxBody;
        std::string host;
        std::String serverName;

        std::map<int, std::string> errorError; // 404 -> "404 .html not connecting"

};

#endif