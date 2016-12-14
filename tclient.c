#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <termios.h>
#include <time.h>



static const int BUF_SIZE = 2048;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
int max(int num1, int num2) {

   int result;
 
   if (num1 > num2)
      result = num1;
   else
      result = num2;
 
   return result; 
}
int main(int argc, char *argv[])
{
    int sockfd, portno, n, w, maxfdp1, numset, fd;
    fd = fileno(stdin);
    struct termios custom;
    struct timespec tm;
    tm.tv_sec = 0;
    tm.tv_nsec = 1000000;
    struct termios save;
    tcgetattr(fd, &save);
    custom = save;
    custom.c_lflag &= ~(ICANON|ECHO); // Turn off canonical mode and echo immmediately
    tcsetattr(fd,TCSANOW, &custom);
    struct sockaddr_in serv_addr;
    struct hostent *server;
    fd_set readfds, tempfds;
    char cmd;
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
    bzero(buffer, BUF_SIZE);
    maxfdp1 = max(sockfd, fd) + 1;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    FD_SET(sockfd, &readfds);

    while (1) {
        nanosleep(&tm, NULL);
        bzero(buffer, BUF_SIZE);
        tempfds = readfds;
        numset = select(maxfdp1, &tempfds, NULL, NULL, NULL);

        if (numset == -1) {
            error("select");
        }
        
        if (FD_ISSET(fd, &tempfds)) {
            cmd = getchar();
            if (cmd == 'a' || cmd == 'w' || cmd  == 's' || cmd == 'd') {
                buffer[0] = cmd;
            }
            n = write(sockfd, buffer, 1);
            if (n != 1) {
                fprintf(stderr, "Client write error\n");
            }
        }
        if (FD_ISSET(sockfd, &tempfds)) {
            n = read(sockfd, buffer, BUF_SIZE);
            if (n == 0) {
                printf("%s", buffer);
                printf("*** GAME OVER ***\n");
                printf("Thanks for playing. Please reconnect to your host to play again.\n");
                break;
            }
            printf("%s", buffer);

        }
    }
    



    // n = write(sockfd, buffer, strlen(buffer));
    // if (n < 0) 
    //      error("ERROR writing to socket");
      
    // bzero(buffer,400);
    // n = read(sockfd,buffer,400);        
    // printf("%s\n",buffer);
    
    // while (n < 0) {
        
    //     bzero(buffer,256);
    //     fgets(buffer,255,stdin);
    //     write(sockfd,buffer,strlen(buffer));
        
    //     bzero(buffer,400);
    //     n = read(sockfd,buffer,400);        
    //     printf("%s\n",buffer);
        
    // }
    
    
            
    close(sockfd);
    return 0;
}
