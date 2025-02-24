
#include <arpa/inet.h> //socket(), bind(), accept(), inetntop(), listen()
#include <iostream>
#include <unistd.h> // cloase(), write(), read()
#include <semaphore.h>//sem_t()
#include <pthread.h> //pthread
#include <vector>
#include "server.hpp" 
#define PORT 8080
#define cilent_message_SIZE 1024
sem_t mutex;
int thread_count = 0;

std::vector<std::string > serverdata;

std::string getStr(std::string sql, char end)
{
    int counter = 0;
    std::string retStr = "";
    while(sql[counter]) != '\0'
    {
        if(sql[counter] == end)
        {
            break;
        }   
        retStr += sql[counter];
        counter++;
    }
    return (retStr);
}



void send_message(int fd, std::string filePath, std:;string headerFile)
{
    
}



void getData(std::string requestType, std::string client_message)
{
    std::string extract;
    std::string data = client_message;

    if(requestType == "GET")
    {
        data.erase(0, getstring(data,' ').length()  + 1);
        data = getStr(data, ' ');//extract file path
        data.erase(0, getStr(data, '?').length() + 1); //index.html/?a=c  so we get a=c
    }
    else if (requestType == "POST")
    {
       int counter = data.length*( ) - 1;
       while(counter > 0)
       {
            if (data[counter] == ' ' || data[counter] == '\n')
            {
                break;
            }
            counter--;
       }
       data.erase(0,counter + 1);
       int found = data.find("=");
       if(found == string :: npos)
       {
            data = ""; // 如果沒有= 就不是post直接設成null
       }
    }
    //check cookies
    int found = client_message.find("Cookie:");
    if (found != std::string::npos)//found cookie
    {
        client_message.erase(0, found + 8); //delete everything before cookie and cookie this word
        client_message = getStr(client_message, ' ');
        data = data+"&" + getStr(client_message, '\n');
    }

    while(data.length() > 0)
    {
        extract = getStr(data, '&');
        serverData.push_back(extract);
        data.erase(0,getStr(data,'&').length() + 1);
    }
}


std::string findFileExt(std::string fileEx)
{
    for (int i = 0; i < sizeof(fileExtension); i++)
    {
        if (fileEx == fileExtension[i])
        {
            return ContentType[i];
        }
       
    }
     std::cout << "serving as text/html" << fileEx.c_str()<<std::endl;
     return("Content-Type:text text/html\r\n\r\n");
}



void *connection_handler(void *socket_desc)
{
    int newSock = *((int *)socket_desc);
    char client_message[client_message_SIZE];
    int request = read(newSocke, client_message, client_message_SIZE);//讀取client HTTP 請求
    std::string message = client_message;
    sem_wait(&mutex);
    thread_count++
    std::cout << thread_count << " threads are running" << std::endl;
    if (thread_count > 20)
    {
        write(newSock, Message[BAD_REQUEST].c_str(), Message[BAD_REQUEST].length());
        thread_count--;
        close(newSock);
        sem_post(&mutex);
        pthread_exit(NULL);
    }
    sem_post(&mutex);
    if (request < 0)
    {
        puts("recv failed");
    }
    else if (request == 0)
    {
        puts("client disconnected unexpectedly");
    }
    else
    {
        //std::cout << "client message:" << message << std::endl;
         // 3) 解析 HTTP 請求行
        std::string requestType = getStr(message, ' ');// 例如取 "GET" 或 "POST"
        message.erase(0, requestType.length() + 1);  // 移除已取走的部分
        std::string requestFile = getStr(message, ' '); // 例如取 "/index.html"
        std::string requestF = requestFile;
        std::string requestExt = requestF.erase(0,getStr(requestF, '.').length() + 1);
        std::string fileExt = getStr(getStr(fileExt,'/'), '?');
        requestFile = getStr(requestFile,'.')+"."+fileEx;//取得副檔名 fileExt (如 html / php / png

        if(requestType == "GET" || requestType == "POST")
        {
            if(requestType == "GET" || requestType == "POST")
            {
                reqestFile = "/index.html";
            }
            if(fileEx == "php")
            {
                //php-cgi
                getData(requestType, client_message);
            }
            sem_wait(&mutex);
            send_message(newSock, requestFile, findFileExt(fileEx));//傳送對應的檔案內容與 MIME Type。
            sem_post(&mutex);
        }
        std::cout<< "\n------exiting server------\n";
        close(newSock);
        sem_wait(&mutex);//進入臨界區，取得互斥鎖，確保同一時間只有一個執行緒能修改全域變數
        thread_count--;
        sem_post(&mutex);//離開臨界區，釋放互斥鎖
        pthread_exit(NULL);

       
    }
    return 0;
}

int main(int argc , char ** argv)
{
    
    sem_init(&mutex, 0, 1);
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

    //伺服器為每個客戶端連線建立一個執行緒 thread
	if(pthread_create(&multi_thread, NULL, connection_handler, (void*)thread_sock_) > 0)
	{
		perror("could not create thread");
		return 0;
	}


    }
}