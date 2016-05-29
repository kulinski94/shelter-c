#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void readResponse(const int sockfd){
    char buffer[256];
    int n;
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
}

void validateArgumentsCount(const int argc,char *argv[]){
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
}

int openSocket(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    return sockfd;
}

struct hostent* getServer(char* host){
    struct hostent *server;
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    return server;
}

void connectToServer(struct hostent *server, int portno, int sockfd){
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
}

void writeToServer(int sockfd){
    int n;
    char buffer[256];
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
}
int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct hostent *server;
    char buffer[256];
    
    validateArgumentsCount(argc,argv);
    
    portno = atoi(argv[2]);
    
    sockfd = openSocket();
    
    server = getServer(argv[1]);
    
    connectToServer(server, portno, sockfd);
    
    printf("Please enter your name: ");
    
    writeToServer(sockfd);
    
    readResponse(sockfd);
    
    close(sockfd);
    return 0;
}