#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#define PORT "80" 
#define BACKLOG 10 
#define BUFSIZE 8096

struct
{
    	char *ext;
    	char *filetype;
} extensions [] = {
    	{"gif", "image/gif" },
    	{"jpg", "image/jpeg"},
    	{"jpeg","image/jpeg"},
    	{"png", "image/png" },
    	{"zip", "image/zip" },
    	{"gz",  "image/gz"  },
    	{"tar", "image/tar" },
    	{"htm", "text/html" },
    	{"html","text/html" },
    	{"exe","text/plain" },
    	{0,0}
	};


void handle_socket(int fd)
{
    	int j, file_fd, buflen, len;
    	long i, ret;
    	char * fstr;
    	static char buffer[BUFSIZE+1];
    	ret = read(fd,buffer,BUFSIZE);
    	if(ret==0||ret==-1)
        	exit(3);
    	if (ret>0&&ret<BUFSIZE)
        	buffer[ret] = 0;
    	else
        	buffer[0] = 0;
    	for(i=0;i<ret;i++) 
        	if(buffer[i]=='\r'||buffer[i]=='\n')
            		buffer[i] = 0;
    	if(strncmp(buffer,"GET ",4)&&strncmp(buffer,"get ",4))
        	exit(3);
    	for(i=4;i<BUFSIZE;i++)
	{
        	if(buffer[i] == ' ')
		{
        		buffer[i] = 0;
            		break;
        	}
    	}
    	if(!strncmp(&buffer[0],"GET /\0",6)||!strncmp(&buffer[0],"get /\0",6) )
        	strcpy(buffer,"GET /index.html\0");
    	buflen = strlen(buffer);
    	fstr = (char *)0;
    	for(i=0;extensions[i].ext!=0;i++)
	{
        	len = strlen(extensions[i].ext);
        	if(!strncmp(&buffer[buflen-len], extensions[i].ext, len))
		{
            		fstr = extensions[i].filetype;
            		break;
        	}
    	}
    	if(fstr == 0)
 	      	fstr = extensions[i-1].filetype;
    	if((file_fd=open(&buffer[5],O_RDONLY))==-1)
	 	write(fd, "Failed to open file", 19);
    	sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
    	write(fd,buffer,strlen(buffer));
    	while((ret=read(file_fd, buffer, BUFSIZE))>0)
       		write(fd,buffer,ret);
	exit(1);
}

void sigchld_handler(int s)
{
    while(waitpid(-1,NULL,WNOHANG)>0);
}

int main(void)
{
	 int sockfd, new_fd;
	 struct addrinfo hints, *servinfo, *p;
	 struct sockaddr_storage their_addr;
	 socklen_t sin_size;
	 struct sigaction sa;
	 int yes=1;
 	 char s[INET6_ADDRSTRLEN];
 	 int rv;
 	 memset(&hints, 0, sizeof hints);
 	 hints.ai_family = AF_UNSPEC;
 	 hints.ai_socktype = SOCK_STREAM;
 	 hints.ai_flags = AI_PASSIVE; 
	 hints.ai_protocol=0 ;
	 getaddrinfo(NULL, PORT, &hints, &servinfo);
 	 sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol);
 	 setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int));
 	 if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)==-1)
 	 {
 	 	perror("bind");
 	  	exit(1);
 	 }	  
  	 freeaddrinfo(servinfo);
	 listen(sockfd, BACKLOG);
  	 sa.sa_handler = sigchld_handler;
 	 sigemptyset(&sa.sa_mask);
 	 sa.sa_flags = SA_RESTART;
 	 sigaction(SIGCHLD, &sa, NULL);
  	 printf("running...\n");
  	 while(1)
  	 {	  
    		sin_size = sizeof their_addr;
    		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    		if(new_fd == -1) 
    		{
      			perror("accept");
      			continue;
    		}
    		if(!fork())
    		{
      			close(sockfd);
      			handle_socket(new_fd);
      			close(new_fd);
      			exit(0);
    		}
    		close(new_fd);
  	 }
  	return 0;
}

