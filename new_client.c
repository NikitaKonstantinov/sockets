#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define DEF_TIMEOUT 1*60*1000

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char* argv[])
{
	int sockfd, portno, n;
	int rc;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	char close_connect[4]="exit";
	int timeout;
	
	struct pollfd fds[1];
	int nfds = 1;
	
	if (argc<3) {
		fprintf(stderr,"Usage %s hostname port\n",argv[0]);
		exit(0);
	}
	
	//getting port number
	portno = atoi(argv[2]);
	
	//Opening a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
		
	}
	
	//setting server
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
        exit(0);
	}
	
	//initialisation of structure
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
	
	//establishing connection to the server
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		
        error("ERROR connecting");
        
	}
    //Set up initial socket to write
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    
    //setting maximum waiting time    
    timeout = (DEF_TIMEOUT*0.1);
    
    // 
    do {
		printf("Please enter the message: ");
        bzero(buffer,256);
        //reading data from STDIN
        fgets(buffer,255,stdin);
        //writing data to server
        n = write(sockfd,buffer,strlen(buffer));
		if (n < 0) {
			
			error("ERROR writing to socket");
			
		}
		
		//Calling poll()
		printf("Waiting on poll()...\n");
		rc = poll(fds,nfds,timeout);
		if (rc < 0)
		{
			perror("  poll() failed");
			break;
		}
		//Checking whether WAITING_TIME has expired
		if (rc == 0)
		{
			printf("  poll() timed out. No reply from server. Ending program.\n");
			break;
		}
		
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) {
			
         error("ERROR reading from socket");
         
		}
		printf("Reply from server recieved: %s\n",buffer);
		
	} while (strncmp(buffer,close_connect,4) != 0);
	
	close(sockfd);
	printf("Closing socket\n"); 
	
	return 0;	
}
