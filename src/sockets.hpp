#ifndef SOCKETS_HPP
# define SOCKETS_HPP
#include <iostream>

struct sockaddr_in
{
    __uint128_t sin_addr;
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];


};




#endif