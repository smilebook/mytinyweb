#ifndef _MY_TINY_WEB_H
#define _MY_TINY_WEB_H

#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/wait.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/mman.h>

#include"rio.h"

#define DEF_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define DEF_UMASK	S_IWGRP|S_IWOTH
typedef struct sockaddr SA;
extern char **environ;
#define MAXLINE 8192
#define MAXBUF 8192
#define LISTENQ 1024

int open_clientfd(char *hostname,int portno);
int open_listenfd(int portno);
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri,char *filename,char *cgiargs);
void serve_static(int fd,char *filename,int filesize);
void get_filetype(char *filename,char *filetype);
void serve_dynamic(int fd,char *filename,char *cgiargs);
void clienterror(int fd,char *cause,char *errum,char *shortmsg,char *longmsg);
#endif
