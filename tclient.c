#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

static const int MAX_NAME = 16;
static const int DATA_SIZE = 512;
static const int BUF_SIZE;

// typedef for packets
typedef struct __attribute__((__packed__)) packet {
    unsigned short type;
    char user[MAX_NAME];
    unsigned int length;
    char data[DATA_SIZE];
} packet;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, w;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUF_SIZE];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    // after connection
    printf("Please enter your username: ");
    bzero(buffer, BUF_SIZE);
    fgets(buffer, BUF_SIZE, stdin);
    
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
      
    bzero(buffer,400);
    n = read(sockfd,buffer,400);        
    printf("%s\n",buffer);
    
    while (n < 0) {
        
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        write(sockfd,buffer,strlen(buffer));
        
        bzero(buffer,400);
        n = read(sockfd,buffer,400);        
        printf("%s\n",buffer);
        
    }
    
    
            
    close(sockfd);
    return 0;
}
