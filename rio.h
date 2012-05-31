#ifndef _ROBUSTIO_H
#define _ROBUSTI0_H
/* Persistent state for the robust I/O(Rio) package */
/* From the book named CSAPP */

#include<errno.h>
#include<sys/types.h>
/* $begin rio_t */
#define RIO_BUFSIZE 8192
typedef struct
{
	int rio_fd;			/* decriptor for this internal buf */
	int rio_cnt;		/* unread bytes in internal buf */
	char *rio_bufptr;	/* next unread byte in internal buf */
	char rio_buf[RIO_BUFSIZE];	/* internal buffer */
}rio_t;
/* $end rio_t */

/* rio packeage */
ssize_t rio_readn(int fd,void *usrbuf,size_t n);
ssize_t rio_writen(int fd,void *usrbuf,size_t n);
void rio_readinitb(rio_t *rp,int fd);
ssize_t rio_readnb(rio_t *rp,void *usrbuf,size_t n);
ssize_t rio_readlineb(rio_t *rp,void *usrbuf,size_t maxlen);

#endif
