#include <arpa/inet.h> //socket(), bind(), accept(), inetntop(), listen()
#include <unistd.h>    //close(), write(), read()
#include <semaphore.h> //sem_t()
#include <pthread.h>   //pthread()
#include <sys/sendfile.h>// sendfile()
#include <fcntl.h>      //O_CREAT, O_WRONLY, O_RDONLY
#include <vector>
#include <sys/stat.h>
#include "server.h"

#define client_message_SIZE 1024
#define PORT 8080
sem_t mutex;
int thread_count = 0;
std::vector<std::string> serverData;

string getStr(string sql, char end)
{
    int counter = 0;
    string retStr = "";
    while(sql[counter] != '\0')
    {
        if(sql[counter] == end)
        {    break;}
        retStr += sql[counter];
        counter++;
    }return(retStr);
}

void send_message(int fd, string filePath, string headerFile)
{
    string header = Messages[HTTP_HEADER] + headerFile;
    filePath = "./public"+ filePath;
    struct stat stat_buf; // hold information about our file to send

    write(fd, header.c_str(), header.length());
    int fdimg = open(filePath.c_str(), O_RDONLY);

    if(fdimg < 0){
        printf("cannot open file path: %s\n", filePath.c_str());
        return;
    }

    fstat(fdimg, &stat_buf);
    int img_total_size = stat_buf.st_size;
    int block_size = stat_buf.st_blksize;

    if(fdimg >= 0){
        size_t sent_size;
        while(img_total_size > 0){
            int send_bytes = ((img_total_size < block_size)?img_total_size:block_size);
            int done_bytes = sendfile(fd, fdimg, NULL, send_bytes);
            img_total_size = img_total_size - done_bytes;
        }
        if(sent_size >= 0){
            printf("sent file: %s\n", filePath.c_str());
        }close(fdimg);
    }

}

string findFileExt(string fileEx)
{
    for(int i = 0; i <= sizeof(fileExtension); i++){
        if(fileExtension[i] == fileEx){
            return ContentType[i];
        }
    }
    printf("serving .%s as html\n", fileEx.c_str());
    return("Content-Type: text/html\r\n\r\n");
}

void getData(string requestType, string client_message){
    string extract;
    string data = client_message;

    if(requestType == "GET"){
        data.erase(0, getStr(data, ' ').length()+1);
        data = getStr(data, ' ');
        data.erase(0, getStr(data, '?').length()+1);
    }else
    if(requestType == "POST"){
        int counter = data.length()-1;
        while(counter > 0){
            if(data[counter] == ' ' || data[counter] == '\n'){break;}
            counter--;
        }
        data.erase(0, counter+1);
        int found = data.find("=");
        if(found == string::npos){data = "";}
    }

    int found = client_message.find("cookie");
    if(found != string::npos){
        client_message.erase(0, found+8);
        client_message = getStr(client_message, ' ');
        data = data+"&"+getStr(client_message, '\n');
    }
    while(data.length() > 0){
        extract = getStr(data, '&');
        serverData.push_back(extract);
        data.erase(0, getStr(data, '&').length()+1);
    }
}

void *connection_handler(void *socket_desc)
{
    int newSock = *((int *)socket_desc);
    char client_message[client_message_SIZE];
    int request = read(newSock, client_message, client_message_SIZE);
    string message = client_message;
    sem_wait(&mutex);
    thread_count++;
    printf("thread counter %d\n", thread_count);
    if(thread_count > 20){
        write(newSock, Messages[BAD_REQUEST].c_str(), Messages[BAD_REQUEST].length());
        thread_count--;
        close(newSock);
        sem_post(&mutex);
        pthread_exit(NULL);
    }
    sem_post(&mutex);

    if(request < 0)
    {
        puts("Receive failed");
    }else 
    if(request == 0)
    {
        puts("Client disconnected unexpectedly");
    }else
    {
        string mess      = client_message;
        int found        = mess.find("multipart/form-data");
        if(found != string::npos){
            found        = mess.find("Content-length:");
            mess.erase(0, found+16);
            int length= stoi(getStr(mess, ' '));
            found        = mess.find("filename=");
            mess.erase(0, found+10);
            string newf  = getStr(mess, '"');
            newf         = "./public/downloads/"+newf;
            found        = mess.find("Content-Type:");
            mess.erase(0, found + 15);
            mess.erase(0, getStr(mess, '\n').length()+3);

            char client_mess[client_message_SIZE];
            int fd, req, rcc, counter;
            if((fd = open(newf.c_str(), O_CREAT | O_WRONLY, S_IRWXU)) < 0)
            {
                perror("cannot open filepath");
            }
            write(fd, mess.c_str(), client_message_SIZE);
            printf("filesize: %d\n", length);

            while(length > 0){
                req = read(newSock, client_mess, client_message_SIZE);
                if((rcc = write(fd, client_mess, req)) < 0){
                    perror("write failed:");
                    return(0);
                }
                length  -= req;
                counter += req;
                printf("remains: %d. received size: %d. total size received: %d. \n", length, req, counter);
                if(req < 1000){
                    break;
                }
            }
            if((rcc = close(fd)) < 0){
                perror("close failed");
                return(0);
            }
        }
        
        //printf("client message: %s\n", client_message);
        string requestType = getStr(message, ' ');
        message.erase(0, requestType.length()+1);
        string requestFile = getStr(message, ' ');

        string requestF = requestFile;
        string fileExt  = requestF.erase(0, getStr(requestF, '.').length()+1);
        string fileEx   = getStr(getStr(fileExt, '/'), '?');
        requestFile     = getStr(requestFile, '.')+"."+fileEx;

        if(requestType == "GET" || requestType == "POST")
        {
            if(requestFile.length() <= 1)
            {
                requestFile = "/index.html";
            }
            if(fileEx == "php")
            {
                //do nothing
                getData(requestType, client_message);
            } 
            sem_wait(&mutex);
            send_message(newSock, requestFile, findFileExt(fileEx));
            sem_post(&mutex);           
        }
    }printf("\n -----exiting server--------");
    close(newSock);
    sem_wait(&mutex);
    thread_count--;
    sem_post(&mutex);
    pthread_exit(NULL);

}

int main(int argc, char const *argv[])
{
    sem_init(&mutex, 0, 1);
    int server_socket, client_socket, *thread_sock;
    int randomPORT = PORT;
    struct sockaddr_in server_address, client_address;
    char ip4[INET_ADDRSTRLEN];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket <= 0)
    {
        perror("error in socket:");
        exit(EXIT_FAILURE);
    }

    randomPORT = 8080 + (rand() % 10);
	memset(&server_address, 0, sizeof server_address); 
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);
	server_address.sin_port = htons(randomPORT);

    while(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) <0)
    {
        randomPORT = 8080 + (rand() % 10);
        server_address.sin_port = htons(randomPORT);
    }
    if(listen(server_socket, 10) < 0)
    {
        perror("error in listen:");
        exit(EXIT_FAILURE);
    }

    while(1){
        socklen_t len = sizeof(client_address);
        printf("listening port: %d \n", randomPORT);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &len);
        if(client_socket < 0)
        {
            perror("unable to accept connection:");
            return 0;
        }
        else
        {
            inet_ntop(AF_INET, &(client_address.sin_addr), ip4, INET_ADDRSTRLEN);
            printf("connected to %s \n", ip4);
        }
        pthread_t multi_thread;
        thread_sock = new int();
        *thread_sock = client_socket;

        if(pthread_create(&multi_thread, NULL, connection_handler, (void*)thread_sock)  > 0)
        {
            perror("could not create thread");
            return 0;
        }
    }
}