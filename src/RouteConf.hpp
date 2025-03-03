
/*
- Which URLs to apply.
- The root directory where files are stored.
- The allowed HTTP methods (GET, POST, DELETE).
- Whether directory listing is enabled.
- The default file for a directory.
- Whether CGI scripts are executed.
- The CGI interpreter path (e.g., /usr/bin/python3).
- The upload directory for POST file uploads.
*/

#ifndef ROUTECONF_HPP
#define ROUTECONF_HPP

class RouteConf 
{
    private:

    public:
        std::string location; //for URL
        std::string root;
        std::set<std::string> methods // allowed methods:GET, POST, DELETE
        std::string defaultFile;
        std::string uploadDir;

        bool directoryListings;
        bool enableCGI;

        std::strinf CGIPath;

        RouteConf();
//		void printConfig() const;

};

#endif

