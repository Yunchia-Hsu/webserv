#include "confiParser.hpp"

ConfiParser::ConfiParser() {}

ConfiParser::~ConfiParser()  {}

void ConfiParser::parseFile(const std::string& filename)
{
    std::cout << "HERE I AM" << std::endl;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open the file: " + filename);
    }
    std::string line;
    while (std::getline(file, line))
    {
        //push_back every line to the vector
        confiLines.push_back(line);
    }
    file.close();
    std::cout << "File is saved and dandy!" << std::endl;
    testPrinter();

}

void ConfiParser::testPrinter() const
{
    for (size_t i = 0; i < confiLines.size(); i++)
    {
        std::cout << i + 1 << ": " << confiLines[i] << std::endl;
    }
}