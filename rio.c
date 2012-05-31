#include"rio.h"
/*
*****************************************************************
*	The rio package -robust I/O functions
*****************************************************************
*/
/* rio_read-This is a wrapper for the UNIX read() function that
 * transfers min(n,rio_cnt) bytes from an internal buffer to a user
 * buffer,where n is the number of bytes requested by the user and
 * rio_cnt is the number of unread bytes in the internal buffer.
 * On entry,rio_read() refills the internal buffer via a call to
 * read() if the internal buffer is empty.
 */

/*
 * rio_readn-robustly read n bytes(unbuffered)
 */
/* $begin rio_readn */

ssize_t rio_readn(int fd,void *usrbuf,size_t n)
{
	size_t nleft=n;
	ssize_t nread;
	char *bufp=(char *)usrbuf;

	while(nleft>0)
	{
		if((nread=read(fd,bufp,nleft))<0)
		{
			if(errno==EINTR) /* interrupted by sig handler return */
				nread=0;	/* and call read() again */
			else
				return -1;	/* errno set by read() */
		}
		else if(nread==0)
			break;			/* EOF */
		nleft-=nread;
		bufp+=nread;
	}
	return (n-nleft);		/* return>=0 */
}
/* $end rio_readn */

/*
 * rio_writen-robustly write n bytes(unbuffered)
 */
ssize_t rio_writen(int fd,void *usrbuf,size_t n)
{
	size_t nleft=n;
	ssize_t nwritten;
	char *bufp=usrbuf;
	while(nleft>0)
	{
		if((nwritten=write(fd,bufp,nleft))<=0)
		{
			if(errno==EINTR)	/* interrupted by sig handler return */
				nwritten=0;		/* and call write() again */
			else
				return -1;		/* errno set by write() */
		}
		nleft-=nwritten;
		bufp+=nwritten;
	}
	return n;	/* never return not n */
}
/* $end rio_writen */

/*
 * rio_readinitb - Associate a descriptor with a read buffer and reset buffer
 */
/* $begin rio_readinitb */
void rio_readinitb(rio_t *rp,int fd)
{
	rp->rio_fd=fd;
	rp->rio_cnt=0;
	rp->rio_bufptr=rp->rio_buf;
}
/* $end rio_readinitb */

/* $begin rio_read */
static ssize_t rio_read(rio_t *rp,char *usrbuf,size_t n)
{
	int cnt;
	while(rp->rio_cnt<=0)		/* refill if buf is empty */
	{
		rp->rio_cnt=read(rp->rio_fd,rp->rio_buf,sizeof(rp->rio_buf));
		if(rp->rio_cnt<0)
		{
			if(errno!=EINTR)	/* interrupted by sig handler return */
				return -1;
		}
		else if(rp->rio_cnt==0)	/* EOF */
			return 0;
		else
			rp->rio_bufptr=rp->rio_buf; /* reset buffer ptr */
	}
	cnt=n;
	if(rp->rio_cnt<n)
		cnt=rp->rio_cnt;
	memcpy((void *)usrbuf,(const void *)rp->rio_bufptr,cnt);
	rp->rio_bufptr+=cnt;
	rp->rio_cnt-=cnt;
	return cnt;
}
/* $end rio_read */

/*
 * rio_readnb-robustly read n bytes(buffered)
 * /
/* $begin rio_readnb */
ssize_t rio_readnb(rio_t *rp,void *usrbuf,size_t n)
{
	size_t nleft=n;
	ssize_t nread;
	char *bufp=(char *)usrbuf;
	
	while(nleft>0)
	{
		if((nread=rio_read(rp,bufp,nleft))<0)
		{
			if(errno==EINTR) /* interrupted by sig handler return*/
				nread=0;	/*call read() again */
			else
				return -1;	/* errno set by read() */
		}
		else if(nread==0)
			break;			/* EOF */
		nleft-=nread;
		bufp+=nread;
	}
	return (n-nleft);		/* return>=0 */
}
/* $end rio_readnb */

/*
 * rio_readlineb-robustly read a text line (niffered)
 */
/* $begin rio_readlineb */
ssize_t rio_readlineb(rio_t *rp,void *usrbuf,size_t maxlen)
{
	int n,rc;
	char c,*bufp=(char *)usrbuf;

	for(n=1;n<maxlen;++n)
	{
		if((rc=rio_read(rp,&c,1))==1)
		{
			*bufp++=c;
			if(c=='\n')
				break;
		}
		else if(rc==0)
		{
			if(n==1)
				return 0;	/* EOF,no data read */
			else
				break;		/* EOF,some data was read */
		}
		else
			return -1;		/* error */
	}
	*bufp=0;
	return n;
}
/* $end rio_readlineb */

