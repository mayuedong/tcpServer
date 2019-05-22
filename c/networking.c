#include "server.h"

void SendReplyToClient(EventLoop *loop, void *privdata, int fd, int mask)
{
	if (NULL == loop || NULL == privdata || 0 >= fd)
	{
		return;
	}
	CLIENT *pClient = (CLIENT *)privdata;
	int num = write(fd, pClient->replyBuf, sdsLength(pClient->replyBuf));
	if (-1 == num)
	{
		if (EAGAIN == errno)
		{
			return;
		}
		else
		{		
			Log(LOG_INFO, "send reply client closed");
			ReleaseClient(pClient);
			return;
		}
	}
	if (0 < num)
	{
		sdsMove(pClient->replyBuf, num, -1);
	}
}

int PrepareClientToWrite(CLIENT *pClient)
{
	if (0 >= pClient->fd)
	{
		return ERROR;
	}
	
	if (0 == sdsLength(pClient->replyBuf))
	{
		return aeCreateFileEvent(g_server.loop, pClient->fd, AE_WRITE, SendReplyToClient, pClient);	
	}
	return OK;
}

void AddReplyString(CLIENT *pClient, char *str)
{
	if (NULL == pClient || NULL == str)
	{
		return;
	}
	PrepareClientToWrite(pClient);
	pClient->replyBuf = sdsCatBuf(pClient->replyBuf, str);
}

void AddReplySds(CLIENT *pClient, sds str)
{
	if (NULL == pClient || NULL == str)
	{
		return;
	}

	PrepareClientToWrite(pClient);
	pClient->replyBuf = sdsCatSds(pClient->replyBuf, str);

}
int processInlineBuf(CLIENT *pClient)
{
	if (NULL == pClient)
	{
		return ERROR;
	}

	char *pos = strstr(pClient->queryBuf, "\n");
	if (NULL == pos)
	{
		if (0 < sdsLength(pClient->queryBuf))
		{
			AddReplyString(pClient, "query data too long\r\n");
		}
		else
		{
			AddReplyString(pClient, "query data is empty\r\n");
		}
		return ERROR;
	}

	sdsReleaseSplit(pClient->argv, pClient->argc);
	pClient->argc = 0;
	pClient->argv = sdsSplit((const char *)(pClient->queryBuf), pos - pClient->queryBuf, " ", 1, &(pClient->argc));
	sdsMove(pClient->queryBuf, pos - pClient->queryBuf + 1, -1);
	return OK;
}

int ProcessInputBuf(CLIENT*pClient)
{
	if (NULL == pClient)
	{
		return ERROR;
	}
	
	while (0 < sdsLength(pClient->queryBuf))
	{
		if (ERROR == processInlineBuf(pClient))
		{
			return ERROR;
		}
	}
	
	ProcessCommand(pClient);
	return OK;
}

void ReadQueryFromClient(EventLoop *loop, void *clientData, int fd, int mask)
{
	if (NULL == loop || NULL == clientData)
	{
		return;
	}
	CLIENT *pClient = (CLIENT *)clientData;
	int index = sdsLength(pClient->queryBuf);
	pClient->queryBuf = sdsAddRoom(pClient->queryBuf, QUERY_LEN);
	int num = read(fd, pClient->queryBuf + index, QUERY_LEN);

	if (-1 == num)
	{
		if (EAGAIN == errno)
		{
			return;
		}
		else
		{		
			Log(LOG_INFO, "reading from client %s", strerror(errno));
			ReleaseClient(pClient);
			return;
		}
	}
	else if (0 == num)
	{
		Log(LOG_INFO, "read query client closed");
		ReleaseClient(pClient);
		return;
	}

	if (0 < num)
	{
		sdsIncressLen(pClient->queryBuf, num);
		pClient->lastTime = g_server.localTime;
	}

	ProcessInputBuf(pClient);
}

void ReleaseClient(CLIENT *pClient)
{
	if (NULL == pClient)
	{
		return;
	}
	close(pClient->fd);
	sdsRelease(pClient->replyBuf);
	sdsRelease(pClient->queryBuf);
	aeDeleteFileEvent(g_server.loop, pClient->fd, AE_READ);
	aeDeleteFileEvent(g_server.loop, pClient->fd, AE_WRITE);
	sdsReleaseSplit(pClient->argv, pClient->argc);
	reFree(pClient);
}

CLIENT *CreateClient(int fd)
{
	CLIENT *pClient= (CLIENT *)reMalloc(sizeof(CLIENT));
	
	if (-1 != fd)
	{
		anetNonBlock(NULL, fd);
		anetTcpNoDelay(NULL, fd);
		if (0 < g_server.tcpKeepAlive)
		{
			anetTcpKeepAlive(NULL, fd);
		}
		
		if (ERROR == aeCreateFileEvent(g_server.loop, fd, AE_READ, ReadQueryFromClient, pClient))
		{
			close(fd);
			reFree(pClient);
			return NULL;
		}
	}
	pClient->fd = fd;
	pClient->db = g_server.db;
	pClient->replyBuf = sdsNewEmpty();
	pClient->queryBuf = sdsNewEmpty(); 
	pClient->argc = 0;
	pClient->argv = NULL;
	pClient->lastTime = g_server.localTime;
	listAddTail(g_server.clients, pClient);
	return pClient;
}

static void AcceptHandler(int fd)
{
	CLIENT *pClient= CreateClient(fd);
	if (NULL == pClient)
	{
		Log(LOG_WARNING," registering fd event for the new client: %s (fd=%d)", strerror(errno), fd);
		close(fd);
		return;
	}
	if (listLength(g_server.clients) > g_server.clientMaxNum)
	{
		char *err = "alot of clients reached\r\n";
		write(fd, err, strlen(err));
		ReleaseClient(pClient);
		g_server.clientRegectedNum++;
	}
	else
	{
		g_server.clientConnectedNum++;
	}
}

void AcceptTcpHandler(EventLoop *loop, void *clientData, int fd, int mask)
{
	char ip[128] = {0};
	int port;
	
	int cfd = anetTcpAccept(g_server.neterr, fd, ip, &port);

	if (ERROR == cfd)
	{
		Log(LOG_WARNING, "Accepting client connection %s", g_server.neterr);
	}
	else
	{
		Log(LOG_INFO, "Accepted ip = %s	port = %d", ip, port);
	}
	AcceptHandler(cfd);
}
