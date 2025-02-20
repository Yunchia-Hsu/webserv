
#include <arpa/inet.h> //socket(), bind(), accept(), inetntop(), listen()
#include <iostream>
#define PORT 8080

int main(int argc , char ** argv)
{
    int server_socker, client_socket;
    int randonPORT = PORT;
    struct sockaddr_in server_address;
    char ip4[INET_ADDRSTRLEN];

    server_socker = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket <= 0)
    {
        perror("erro in socket:");
        exit 1;
    }
    randomPORT = 8080 + (rand() % 10);
    memset(&server_address, 0, sizeof server_address); 
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = hton1(INADDR_ANY);
    server_address.sein_port = htons(radomPort);    
    
    while(bind(server_socket, (struct sockaddr *) &server_address, sizeof(srver_address)) < 0)
    {
        randomPORT = 8080 + (rand() % 10);
        server_address.sin_port = htons(randomPORT);
    }
    
    //listen for incoming connections
    //int listen (int socketfd, int backlog); //backlog is the max number of connections that can be queued at the same time usually 5-10                                                                                                                                                                                                                                                                                                                                                                                                                                                                          );

    if (listen(server_socket, 10) < 0)
    {
        perror("error in listen:");
        exit(1);
    }

    //int client_socket = accept (server_socket, (struct sockaddr *)& client_address, &len);
    
    while(1)
    {
        socklen_t len = sizeof(client_address);
        std::cout << "server listening port:" << randonPORT << std::endl;
        client_socket = accept (soclet_fd, (struct sockaddr *)&client_address, &len);
        if (client_socket < 0)
        {
            perror("error in accept:");
            exit(1);
        }
        else
        {
            int_ntop(AF_INET, &(client_address.sin_addr), ip4, INET_ADDRSTRLEN); 
            std::cout << "client connected from ip:" << ip4 << " port:" << ntohs(client_address.sin_port) << std::endl;
        }
    }
}