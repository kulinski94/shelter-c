/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/shm.h> 
#include <sys/ipc.h> 
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct shelter
{
    char* name;
    int free_places;
}shelter;

void handleClient(int);
int reservePlace(shelter*);
shelter* getShelterData(int);
void initializeShelterData(shelter*);
void readClientMessage(int);
void writeResponse(int, int);

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     printf("Listening on port: %d\n",portno);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     while (1) {
         newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             error("ERROR on fork");
         if (pid == 0)  {
             close(sockfd);
             handleClient(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     } /* end of while */
     close(sockfd);
     return 0; /* we never get here */
}

int reservePlace(shelter* shm)
{
    //should use semaphore
    if(shm->free_places <= 0)
    {
        return 0;
    }
    shm->free_places--;
    return 1;
}

shelter* getShelterData(int id){
   int shmid;
   key_t key;
   key = id;
   shmid = shmget(key,sizeof(shelter),IPC_CREAT | 0666);
   if(shmid < 0)
   {
       error("shmid");
   }
   return (shelter*)shmat(shmid,NULL,0);
}

void initializeShelterData(shelter* shm){
    if(shm->name == NULL)
    {
        shm->name = "Noah's Ark\0";
        shm->free_places = 5;
        printf("Shelter initialized\n");
    }
}
void readClientMessage(int sock){
   int n;
   char buffer[256];      
   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) error("ERROR reading from socket");   
   printf("Here is the message: %s\n",buffer);
}
void writeResponse(int sock, int success){
   int n;
   if(success == 1)
   {
        n = write(sock,"You have reserved",17);
   }else
   {
       n = write(sock,"Sorry we dont have free places",29);
   }
   if (n < 0) error("ERROR writing to socket");
}
void handleClient(int sock)
{
   shelter *shm;
   char *s;
   
   shm = getShelterData(9574);
   
   initializeShelterData(shm);

   printf("Shelter Name: %s\n",shm->name);
   printf("Free Places: %d\n",shm->free_places);
   
   readClientMessage(sock);

   int success = reservePlace(shm);
   
   writeResponse(sock,success);
}
