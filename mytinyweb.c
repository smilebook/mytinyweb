#include"mytinyweb.h"

int open_clientfd(char *hostname,int portno)
{
	int clientfd;
	struct sockaddr_in serveraddr;
	struct hostent *hp;

	if((clientfd=socket(AF_INET,SOCK_STREAM,0))<0)
		return -1;
	if((hp=gethostbyname(hostname))==NULL)
		return -2;
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	bcopy((char *)hp->h_addr,(char *)&serveraddr.sin_addr.s_addr,hp->h_length);
	serveraddr.sin_port=htons(portno);

	if(connect(clientfd,(SA *)&serveraddr,sizeof(serveraddr))<0)
		return -1;
	return clientfd;
}

int open_listenfd(int port)
{
	int listenfd,optval=1;
	struct sockaddr_in serveraddr;

	if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
		return -1;
	if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&optval,sizeof(int))<0)
		return -1;
	bzero((char *)&serveraddr,sizeof(serveraddr));

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons((unsigned short)port);
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);

	if(bind(listenfd,(SA *)&serveraddr,sizeof(serveraddr))<0)
		return -1;
	if(listen(listenfd,LISTENQ)<0)
		return -1;
	return listenfd;
}
void doit(int fd)
{
	rio_t rio;
	int is_static;
	char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],filename[MAXLINE],version[MAXLINE];
	char cgiargs[MAXLINE];
	struct stat sbuf;

	rio_readinitb(&rio,fd);
	rio_readlineb(&rio,buf,MAXLINE);
	sscanf(buf,"%s %s %s",method,uri,version);
	read_requesthdrs(&rio);

	is_static=parse_uri(uri,filename,cgiargs);
	if(stat(filename,&sbuf)<0)
	{
		clienterror(fd,filename,"404","Not found","Tiny couldn't found this file");
		return;
	}
	if(is_static)
	{
		if(!(S_ISREG(sbuf.st_mode))||!(sbuf.st_mode&S_IRUSR))
		{
			clienterror(fd,filename,"403","Forbidden","Tiny couldn't read the file");
			return;
		}
		serve_static(fd,filename,sbuf.st_size);
	}
	else
	{
		if(!(S_ISREG(sbuf.st_mode))||!(sbuf.st_mode&S_IRUSR))
		{
			clienterror(fd,filename,"403","Forbidden","Tiny couldn't read the file");
			return;
		}
		serve_dynamic(fd,filename,cgiargs);
	}
}
void read_requesthdrs(rio_t *rp)
{
	char buf[MAXLINE];
	rio_readlineb(rp,buf,MAXLINE);
	while(strcmp(buf,"\r\n"))
		rio_readlineb(rp,buf,MAXLINE);
	return;
}
int parse_uri(char *uri,char *filename,char *cgiargs)
{
	char *ptr;

	if(!strstr(uri,"cgi-bin"))
	{
		strcpy(cgiargs,"");
		strcpy(filename,".");
		strcat(filename,uri);
		if(uri[strlen(uri)-1]=='/')
			strcat(filename,"home.html");
		return 1;
	}
	else
	{
		ptr=index(uri,'?');
		if(ptr)
		{
			strcpy(cgiargs,ptr+1);
			*ptr='\0';
		}
		else
			strcpy(cgiargs,"");
		strcpy(filename,".");
		strcat(filename,uri);
		return 0;
	}
}
void serve_static(int fd,char *filename,int filesize)
{
	int srcfd;
	char *srcp,filetype[MAXLINE],buf[MAXBUF];
	get_filetype(filename,filetype);
	sprintf(buf,"HTTP/1.0 200 OK\r\n");
	sprintf(buf,"%sServer:Tiny Web Server\r\n",buf);
	sprintf(buf,"%sContent-length: %d\r\n",buf,filesize);
	sprintf(buf,"%sContent-type: %s\r\n\r\n",buf,filetype);
	rio_writen(fd,buf,strlen(buf));

	srcfd=open(filename,O_RDONLY,0);
	srcp=mmap(0,filesize,PROT_READ,MAP_PRIVATE,srcfd,0);
	close(srcfd);
	rio_writen(fd,srcp,filesize);
	munmap(srcp,filesize);
}
void get_filetype(char *filename,char *filetype)
{
	if(strstr(filename,".html"))
		strcpy(filetype,"text/html");
	else if(strstr(filename,".gif"))
		strcpy(filetype,"image/gif");
	else if(strstr(filename,".jpg"))
		strcpy(filetype,"image/jpg");
	else 
		strcpy(filetype,"text/plain");
}
void serve_dynamic(int fd,char *filename,char *cgiargs)
{
	char buf[MAXLINE],*emptylist[]={ NULL };

	sprintf(buf,"HTTP/1.0 200 OK\r\n");
	rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"Server: Tiny Web Server\r\n\r\n");
	rio_writen(fd,buf,strlen(buf));
	if(fork()==0)
	{
		setenv("QUERY_STRING",cgiargs,1);
		dup2(fd,STDOUT_FILENO);
		execve(filename,emptylist,environ);
	}
	wait(NULL);
}
void clienterror(int fd,char *cause,char *errnum,char *shortmsg,char *longmsg)
{
	char buf[MAXLINE],body[MAXBUF];

	sprintf(body,"<html><title>Tiny Error</title>");
	sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
	sprintf(body,"%s%s: %s\r\n",body,errnum,shortmsg);
	sprintf(body,"%s<p>%s: %s\r\n",body,longmsg,cause);
	sprintf(body,"%s<hr><em>The Tiny Web server</em>\r\n",body);

	sprintf(buf,"HTTP/1.0 %s %s \r\n",errnum,shortmsg);
	rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"Content-type: text/html\r\n");
	rio_writen(fd,buf,strlen(buf));
	sprintf(buf,"Content-length: %d\r\n\r\n",strlen(body));
	rio_writen(fd,buf,strlen(buf));
	rio_writen(fd,body,strlen(body));
}

