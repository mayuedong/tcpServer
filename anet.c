#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "anet.h"

static void anetSetErr(char *err, char *fmt, ...)
{
	if (NULL == err || NULL == fmt)
	{
		return;
	}
	
	va_list ap = NULL;
	va_start(ap, fmt);
	vsnprintf(err, ANET_ERRLEN, fmt, ap);
	va_end(ap);
}

static int anetSetReuseAddr(char *err, int fd)
{
	int yes = 1;
	if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
	{
		anetSetErr(err, "anetSetReuseAddr error = %s", strerror(errno));
		return ERROR;
	}
}

static int anetSetBlock(char *err, int fd, int block)
{
    int flag = -1;
    if (-1 == (flag = fcntl(fd, F_GETFD)))
    {
        anetSetErr(err, "anetSetBlock fcntl F_GETFD error = %s", strerror(errno));
        return ERROR;
    }
	
    if (0 != block)
    {
    	flag &= ~O_NONBLOCK;
    }
    else
    {
		flag |= O_NONBLOCK;
    }
    	
    if (-1 == fcntl(fd, F_SETFD, flag))
    {
        anetSetErr(err, "anetSetBlock fcntl F_SETFD error = %s", strerror(errno));
        return ERROR;
    }
    return OK;
}

int anetNonBlock(char *err, int fd) 
{
    return anetSetBlock(err,fd,0);
}

int anetBlock(char *err, int fd) 
{
    return anetSetBlock(err,fd,1);
}

static int anetSetTcpNoDelay(char *err, int fd, int val)
{
    if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)))
    {
        anetSetErr(err, "anetSetTcpNoDelay error = %s", strerror(errno));
        return ERROR;
    }
    return OK;
}

int anetTcpNoDelay(char *err, int fd)
{
    return anetSetTcpNoDelay(err, fd, 1);
}

int anetTcpDelay(char *err, int fd)
{
    return anetSetTcpNoDelay(err, fd, 0);
}

int anetSetSendBuffer(char *err, int fd, int size)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) == -1)
    {
        anetSetErr(err, "anetSetSendBuffer error = %s", strerror(errno));
        return ERROR;
    }
    return OK;
}

int anetTcpKeepAlive(char *err, int fd)
{
    int yes = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)))
	{
        anetSetErr(err, "anetTcpKeepAlive error = %s", strerror(errno));
        return ERROR;
    }
    return OK;
}

int anetSendTimeout(char *err, int fd, long long ms) 
{
    struct timeval tv;
    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms%1000)*1000;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))) 
	{
        anetSetErr(err, "  anetSendTimeout error = %s", strerror(errno));
        return ERROR;
    }
    return OK;
}

static int anetCreateSocket(char *err, int doman)
{
	int fd = -1;
	if (-1 == (fd = socket(doman, SOCK_STREAM, 0)))
	{
		anetSetErr(err, "anetCreateSocket error = %s", strerror(errno));
		return ERROR;
	}

	if (ERROR == anetSetReuseAddr(err, fd))
	{
		close(fd);
		return ERROR;
	}
	return fd;
}

static int anetListen(char *err, int fd, struct sockaddr *addr, socklen_t len)
{
	if (-1 == bind(fd, addr, len))
	{
		anetSetErr(err, "anetListen bind error = %s", strerror(errno));
		close(fd);
		return ERROR;
	}
	if (-1 == listen(fd, SOMAXCONN))
	{
		anetSetErr(err, "anetListen listen error = %s", strerror(errno));
		close(fd);
		return ERROR;
	}
	return OK;
}

static int anetAccept(char *err, int fd, struct sockaddr *addr, socklen_t *len)
{
    int fdConn = -1;
    while (1)
    {
        if (-1 == (fdConn = accept(fd, addr, len)))
        {
            if (EINTR == errno)
            {
                continue;
            }
           anetSetErr(err, "anetAccept accept error = %s", strerror(errno));
		    return ERROR;
        }
        break;
    }
    return fdConn;
}

int anetTcpAccept(char *err, int fd, char *ip, int *port)
{
    struct sockaddr_in stSockAddr;
    memset(&stSockAddr, 0, sizeof(stSockAddr));
    socklen_t len = sizeof(struct sockaddr_in);
	int fdConn = -1;
    if (ERROR == (fdConn = anetAccept(err, fd, (struct sockaddr *)&stSockAddr, &len)))
    {
        return ERROR;
    }

	int iplen = strlen(ip);
    if (NULL != ip && iplen >= INET_ADDRSTRLEN)
    {
       inet_ntop(AF_INET, &stSockAddr.sin_addr, ip, iplen);	
    }

    if (NULL != port)
    {
        *port = ntohs(stSockAddr.sin_port); 
    }
    return fdConn;
}

int anetUnixAccept(char *err, int fd)
{
	struct sockaddr_un addr;
	socklen_t len = sizeof(addr);
	memset(&addr, 0, len);

	int fdConn = -1;
	if (ERROR == (fdConn = anetAccept(err, fd, (struct sockaddr *)&addr, &len)))
    {
        return ERROR;
    }
	return fdConn;
}

int anetTcpServer(char *err, char *hostname, int port)
{
	struct addrinfo hints, *info = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	char service[6] = {0};
	snprintf(service, 6, "%d", port);
	
	int result = 0;
	if (0 != (result = getaddrinfo(hostname, service, &hints, &info)))
	{
		anetSetErr(err, "anetTcpServer getaddrinfo error = %s", gai_strerror(result));
		return ERROR;
	}

	int fd = -1;
	struct addrinfo *p = NULL;
	for (p = info; NULL != p; p = p->ai_next)
	{
		if (-1 == (fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)))
		{
			continue;
		}
		
		if (ERROR == anetSetReuseAddr(err, fd))
		{
			goto ERR;
		}

		if (ERROR == anetListen(err, fd, p->ai_addr, p->ai_addrlen))
		{
			goto ERR;
		}
		goto END;
	}
	
	if (NULL == p)
	{
		anetSetErr(err, "anetTcpServer Createing error strerror(%d) =  %s", errno, strerror(errno));
    	goto ERR;
	}
	
ERR:
	if (-1 != fd)
	{
		close(fd);
		fd = -1;
	}
END:
	freeaddrinfo(info);		
	return fd;
}

int anetUnixServer(char *err, char *path, mode_t mode)
{
	if (NULL == path)
	{
		return ERROR;
	}
	
    int fd = anetCreateSocket(err, AF_LOCAL);
    if (ERROR == fd)
    {
        return ERROR; 
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
	socklen_t len = sizeof(addr);
	
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);	
		
	unlink((char *)&addr.sun_path);
    if (ERROR == anetListen(err, fd, (struct sockaddr *)&addr, len))
    {
        return ERROR;
    }

	if (0 != mode)
	{
		chmod(addr.sun_path, mode);
	}
    return fd;
}


static int anetTcpConnect(char *err, char *hostname, int port, int flag)
{
    struct addrinfo hints, *info = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    char service[6] = {0};
	snprintf(service, 6, "%d", port);

	int rest = -1;
	if (0 != (rest = getaddrinfo(hostname, service, &hints, &info)))
	{
		anetSetErr(err, "anetTcpConnect getaddrinfo error = %s", gai_strerror(rest));
		return ERROR;
	}
	
	int fd = -1;
	struct addrinfo *p = NULL;
	for(p = info; NULL != p; p = p->ai_next)
	{
		if (-1 == (fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)))
		{
			continue;
		}

		if (ERROR == anetSetReuseAddr(err, fd))
		{
			goto ERR;
		}
		
		if ((flag & _ANET_NONBLOCK_) && ERROR == anetNonBlock(err, fd))
		{
			goto ERR;
		}
		
		if (-1 == connect(fd, p->ai_addr, p->ai_addrlen))
		{
			if (EINPROGRESS == errno && (flag & _ANET_NONBLOCK_))
			{
				goto END;
			}
			close(fd);
			fd = -1;
			continue;
		}
		goto END;
	}
	if (NULL == p)
	{
		anetSetErr(err, "anetTcpConnect Createing error strerror(%d) = %s", errno, strerror(errno));
		goto ERR;
	}
ERR:
	if (-1 != fd)
	{
		close(fd);
		fd = -1;
	}
END:
	freeaddrinfo(info);
    return fd;
}

int anetTcpNonBlockConnect(char *err, char *ip, int port)
{
    return anetTcpConnect(err, ip, port, _ANET_NONBLOCK_);
}

int anetTcpBlockConnect(char *err, char *ip, int port)
{
    return anetTcpConnect(err, ip, port, _ANET_BLOCK_);
}

static int anetUnixConnect(char *err, char *path, int flag)
{
	int fd = -1;
	if (ERROR == (fd = anetCreateSocket(err, AF_LOCAL)))
	{
		return ERROR;


	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_LOCAL;
	strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

	if (_ANET_NONBLOCK_ & flag)
	{
		if (ERROR == anetNonBlock(err, fd))
		{
			close(fd);
			return ERROR;
		}
	}

	if (-1 == connect(fd, (struct sockaddr *)&addr, sizeof(addr)))
	{
		if (EINPROGRESS == errno && (_ANET_NONBLOCK_ & flag))
		{
			return fd;
		}
		else
		{
			anetSetErr(err, "anetUnixConnect connect error = %s", strerror(errno));
			close(fd);
			return ERROR;
		}
	}
	return fd;
}

int anetUnixNonBlockConnect(char *err, char *path)
{
    return anetUnixConnect(err, path, _ANET_NONBLOCK_);
}

int anetUnixBlockConnect(char *err, char *path)
{
    return anetUnixConnect(err, path,  _ANET_BLOCK_);
}

int anetRead(char *err, int fd, void *buf, int len)
{
	if (NULL == buf || 0 >= len)
	{
		return ERROR;
	}
	
    int sum = 0, count = -1;
    while (sum < len)
    {
		count = read(fd, (char *)buf + sum, len - sum);
		
		if (-1 == count)
		{
			anetSetErr(err, "anetRead error = %s	return = %d", strerror(errno), count);
			return -1;
		}
		else if(0 == count)
		{
			anetSetErr(err, "anetRead error = %s	return = %d", strerror(errno), count);
			break;
		}	
		sum += count;
	}
    return sum;
}

int anetWrite(char *err, int fd, void *buf, int len)
{
	if (NULL == buf || 0 >= len)
	{
		return ERROR;
	}
	
   	 int sum = 0, count = -1;
    while (sum < len)
    {
		count = write(fd, (char *)buf + sum, len - sum);
		
		if (-1 == count)
		{
			anetSetErr(err, "anetWrite error = %s	return = %d", strerror(errno), count);
			return -1;
		}
		else if(0 == count)
		{
			anetSetErr(err, "anetWrite error = %s	return = %d", strerror(errno), count);
			break;
		}
		sum += count;
	}
    return sum;
}

int anetResolve(char *err, char *host, char *ip, char len)
{
 	if (NULL == ip || len < INET_ADDRSTRLEN)
    {
      return ERROR;
    }
	
	struct addrinfo hints, *info = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	int rest;
	if (0 != (rest = getaddrinfo(host, NULL, &hints, &info)))
	{
		anetSetErr(err, "anetResolve getaddrinfo error = %s", gai_strerror(rest));
		return ERROR;
	}

	inet_ntop(info->ai_family, info->ai_addr, ip, len);
	freeaddrinfo(info);
	return OK;
}

int anetSockName(char *err, int fd, int *port, char *ip, int len)
{
	if (NULL == ip || len < INET_ADDRSTRLEN || NULL == port)
    {
      return ERROR;
    }
	
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	memset(&addr, 0, addrlen);
	
	if (-1 == getsockname(fd, (struct sockaddr *)&addr, &addrlen))
	{
		anetSetErr(err, "anetSockName getsockname error = %s", strerror(errno));
		return ERROR;
	}
	
	inet_ntop(AF_INET, &addr.sin_addr, ip, len);
	*port = ntohs(addr.sin_port);
	return OK;
}
int anetPeerName(char *err, int fd, int *port, char *ip, int len)
{
	if (NULL == ip || len < INET_ADDRSTRLEN || NULL == port)
    {
      return ERROR;
    }
	
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	memset(&addr, 0, addrlen);
	
	if (-1 == getpeername(fd, (struct sockaddr *)&addr, &addrlen))
	{
		anetSetErr(err, "anetPeerName getpeername error = %s", strerror(errno));
		return ERROR;
	}
	
	inet_ntop(AF_INET, &addr.sin_addr, ip, len);
	*port = ntohs(addr.sin_port);
	return OK;
}
