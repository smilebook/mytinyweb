#include"mytinyweb.h"

int main(int argc,char *argv[])
{
	int listenfd,connfd,port,clientlen;
	struct sockaddr_in clientaddr;

	if(argc!=2)
	{
		fprintf(stderr,"usage: %s <port>\n",argv[0]);
		return 0;
	}
	port=atoi(argv[1]);
	listenfd=open_listenfd(port);
	while(1)
	{
		clientlen=sizeof(clientaddr);
		connfd=accept(listenfd,(SA *)&clientaddr,&clientlen);
		doit(connfd);
		close(connfd);
	}
	return 0;
}

