#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>

int sendfile_y=0;
char sendbuf[2048]={0},recvbuf[2048]={0},name[2048]={0};

void* recv_thread(void *fd)
{
	int cli_fd=*(int*)fd;
	while(1)
    	{
		if((recv(cli_fd,recvbuf,2048,0))<=0)
		{
			printf("recv error!\n");
			exit(1);
		}
		printf("%s",recvbuf);
		memset(recvbuf,0,sizeof(recvbuf));
	}
}

int main(int argc,char *argv[])
{
	int num_bytes,sockfd;
	struct sockaddr_in cli_addr;
	struct hostent *he;
	if(argc!=2)
		printf("usage:<ipaddress>!\n");
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
		printf("socket error!\n");
	bzero(&cli_addr,sizeof(cli_addr));
	cli_addr.sin_family=AF_INET;
	cli_addr.sin_port=htons(9877);
	if((he=gethostbyname(argv[1]))==NULL)
		printf("inet_pton error!\n");
	cli_addr.sin_addr=*((struct in_addr*)he->h_addr);
	if(connect(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
		printf("connect error!\n");
	printf("Enter your username : ");
	fgets(name,2048,stdin);
        send(sockfd,name,(strlen(name)),0);
	pthread_t tid;
	pthread_create(&tid,NULL,(void*)recv_thread,(void*)&sockfd);
	while(1)
    	{
		memset(sendbuf,0,sizeof(sendbuf));
		fgets(sendbuf,2048,stdin);
		send(sockfd,sendbuf,(strlen(sendbuf)),0);
	}
	close(sockfd);
	return 0;
}
