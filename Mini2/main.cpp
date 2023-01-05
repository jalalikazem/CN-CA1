#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <errno.h>

using namespace std;


char* parse(char line[], const char symbol[])
{
    char *copy = (char *)malloc(strlen(line) + 1);
    strcpy(copy, line);
    
    char *message;
    char *token = strtok(copy, symbol);
    printf("buffer2 : %s\n", line);

    while(token != NULL) {
        token = strtok(NULL, " ");
        message = token;
        if(message == NULL){
            message = "";
        }
        printf("here2 : %s\n", message);
        return message;
    }
   free(token);
   free(copy);
   return message;
}

char* parse_method(char line[], const char symbol[])
{
    printf("buffer1 : %s\n", line);

    char *copy = (char *)malloc(strlen(line) + 1);
    strcpy(copy, line);
        
    char *message;
    char * token = strtok(copy, symbol);
    int current = 0;

    while(token != NULL) {
        message = token;
        if(message == NULL){
            message = "";
        }
        printf("here1 : %s\n", message);
        return message;
        current = current + 1;
    }
    free(copy);
    free(token);
    return message;
}

int send_message(int fd, char file_path[], char head[]){

    struct stat stat_buf; 

    write(fd, head, strlen(head));

    int fd_file = open(file_path, O_RDONLY);
    
    if(fd_file < 0){
        printf("Cannot Open file path : %s with error %d\n", file_path, fd_file); 
    }
     
    fstat(fd_file, &stat_buf);
    int file_total_size = stat_buf.st_size;
    int block_size = stat_buf.st_blksize;
    
    if(fd_file >= 0){
        ssize_t sent_size;
        while(file_total_size > 0){
            
            int send_bytes = ((file_total_size < block_size) ? file_total_size : block_size);
            int done_bytes = sendfile(fd, fd_file, NULL, block_size);
            file_total_size = file_total_size - done_bytes;
        }
        if(sent_size >= 0){
            printf("send file: %s \n" , file_path);
        }
        close(fd_file);
    }
}

int getPort() 
{
    int port = 0;
    scanf("localhost:%d", &port);
    return port;
}

void handleHomeIndex(int new_socket, char* head) 
{
    char path_head[500] = ".";
    strcat(path_head, "/index.html");
    strcat(head, "Content-Type: text/html\r\n\r\n");
    send_message(new_socket, path_head, head);
}


void parse_request(char *buffer, char **requestMethod, char **parse_path, char **parse_ext)
{
    *requestMethod = parse_method(buffer, " "); 
     
    printf("request method: %s\n", *requestMethod);

    *parse_path = parse(buffer, " ");  
    printf("browser request path: %s\n", *parse_path);

    char *copy = (char *)malloc(strlen(*parse_path) + 1);
    strcpy(copy, *parse_path);
    *parse_ext = parse(copy, "."); 
    printf("parse ext: %s\n", *parse_ext);
    free(copy);
}

void handleRequest(int new_socket) 
{
    char buffer[30000] = {0};
    long requestData;
    requestData = read( new_socket , buffer, 30000);

    char *requestMethod; 
    char *parse_path;
    char *parse_ext; 
    parse_request(buffer, &requestMethod, &parse_path, &parse_ext);
    char http_header[25] = "HTTP/1.1 200 Ok\r\n";
    char *copy_head = (char *)malloc(strlen(http_header) +200);
    strcpy(copy_head, http_header);


    if(strcmp(requestMethod, "GET") == 0){
        if(strlen(parse_path) <= 1 || strcmp(parse_path, "/index.html") == 0)
        {
            handleHomeIndex(new_socket, copy_head);
        }
        printf("\n--------Server sent------\n");
    }

    close(new_socket);
    free(copy_head);  
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, pid; 
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In sockets");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    
    int port = getPort();
    while(!port){
        port = getPort();
    }
    address.sin_port = htons(port);
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        printf("\n waiting for connection on port %d ...\n\n", port);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        handleRequest(new_socket);
    }
    close(server_fd);
    return 0;
}









