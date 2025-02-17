
#ifndef ConfiParser_HPP
#define ConfiParser_HPP


#include <iostream>
#include <string>
#include <vector>

class ConfiParser
{
    private:
        //store every line of the confifile
        std::vector<std::string> confiLines;

    public:
        ConfiParser();
        ~ConfiParser();

        void parseFile(const std::string& filename);
//        void testPrinter() const;

};

#endif