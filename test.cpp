
#include <iostream>

int checkend(std::string str, std::string end) //return 0 done, 1 not done
{
    int strlen = str.size()-1;
    int endlen = end.size()-1;

    while(endlen >= 0)
    {
        std::cout << "✅str[strlen]: " << str[strlen] << " strlen: " << strlen << " end[end]: "<< end[endlen]  << " endlen: " << endlen << std::endl;
        if (str[strlen] != end[endlen])
            return 1;
        strlen--;
        endlen--;
    }
    return 0; 
}



int main ()
{
    std::string str = "7\r\nMozilla\r\n9\r\nDeveloper\r\n7\r\nNetwork\r\n0\r\n\r\n";
    std::string end = "0\r\n\r\n";
    std::cout << checkend(str, end) << std::endl;
    return 0;
}
