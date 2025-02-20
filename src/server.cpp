
#include <arpa/inet.h> //socket(), bind(), accept(), inetntop(), listen()
#include <iostream>
#include <unistd.h> // cloase(), write(), read()
#include <semaphore.h>//sem_t()
#include <pthread.h> //pthread

#define PORT 8080

void *connection_handler(void *socket_desc)
{
// recv to get client's http request

//parse HTTP header 

//if parse fail return error 400


//try to open requested file if failed return 404

//after found the file according to eh file extension, send back the file to the clien by using send()

//close clinet_fd/ client_socket



}

int main(int argc , char ** argv)
{
    int server_socket, client_socket, *thread_sock;
    int randomPORT = PORT;
    struct sockaddr_in server_address, client_address;
    char ip4[INET_ADDRSTRLEN];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);// server_socket fd
    if (server_socket <= 0)
    {
        perror("erro in socket:");
        exit 1;
    }
    randomPORT = 8080 + (rand() % 10);

	//1. set up sockaddr_in
    memset(&server_address, 0, sizeof server_address); 
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(randomPORT);    
    //2. bind socket to secific IP and port
    while(bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        randomPORT = 8080 + (rand() % 10);
        server_address.sin_port = htons(randomPORT);
    }
    
    //3. listen for incoming connections
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
        std::cout << "server listening port:" << randomPORT << std::endl;
        client_socket = accept (server_socket, (struct sockaddr *)&client_address, &len);
        if (client_socket < 0)//accept is wrong
        {
            perror("error in accept:");
            return 0;
        }
        else
        {
            inet_ntop(AF_INET, &(client_address.sin_addr), ip4, INET_ADDRSTRLEN); 
            std::cout << "client connected from ip:" << ip4 << " port:" << ntohs(client_address.sin_port) << std::endl;
        }
		
//4. execute multi threads 		
		pthread_t multi_thread;
		thread_sock = new_int(); //a pointer to the client socket
		*thread_sock = client_socket;

	if(pthread_create(&multi_thread, NULL, connection_handler, (void*)thread_sock_) > 0)
	{
		perror("could not create thread");
		return 0;
	}


    }
}