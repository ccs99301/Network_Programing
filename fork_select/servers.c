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
    	ret=read(fd,buffer,BUFSIZE);
    	if(ret==0||ret==-1)
        	exit(3);
    	if(ret>0&&ret<BUFSIZE)
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
    	return;
}

void *get_in_addr(struct sockaddr *sa)
{
  	if(sa->sa_family == AF_INET)
    		return &(((struct sockaddr_in*)sa)->sin_addr);
  	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
  	fd_set master;
  	fd_set read_fds;
  	int fdmax;
  	int listener;
  	int newfd;
  	struct sockaddr_storage remoteaddr;
  	socklen_t addrlen;
  	char buf[256];
  	int nbytes;
  	char remoteIP[INET6_ADDRSTRLEN];
  	int yes=1;
  	int i, j, rv;
 	struct addrinfo hints, *ai, *p;
  	FD_ZERO(&master);
  	FD_ZERO(&read_fds);
  	memset(&hints, 0, sizeof hints);
  	hints.ai_family = AF_UNSPEC;
  	hints.ai_socktype = SOCK_STREAM;
  	hints.ai_flags = AI_PASSIVE;
  	if((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
	{
    		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
  	  	exit(1);
  	}
  	for(p = ai; p != NULL; p = p->ai_next)
	{
    		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    		if (listener < 0)
      			continue;
    		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
      			close(listener);
      			continue;
    		}
    		break;
  	}
  	if(p == NULL)
	{
    		fprintf(stderr, "selectserver: failed to bind\n");
    		exit(2);
  	}
  	freeaddrinfo(ai);
  	listen(listener,BACKLOG);
  	FD_SET(listener, &master);
    	fdmax=listener;
  	for( ; ; )
	{
    		read_fds = master;
    		if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
      			perror("select");
      			exit(4);
    		}
    		for(i = 0; i <= fdmax; i++)
		{
      			if(FD_ISSET(i, &read_fds))
			{
        			if(i == listener)
				{
          				addrlen = sizeof remoteaddr;
          				newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
          				if (newfd == -1)
            					perror("accept");
          				else
					{
            					FD_SET(newfd, &master);
            					if (newfd > fdmax)
              						fdmax = newfd;
            					printf("selectserver: new connection from %s on socket %d\n",inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP,INET6_ADDRSTRLEN),newfd);
          				}
        			} 
				else 
				{
	  				handle_socket(i);
  	  				close(i);
	  				FD_CLR(i,&master);
        			}
      			}
    		}
  	}
  	return 0;
}

