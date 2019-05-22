#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "server.h"

struct SERVER g_server;

unsigned int dictSdsCaseHash(void *key)
{
	return dictGenCaseHashFunction((unsigned char*)key, sdsLength((char*)key));
}

int dictSdsKeyCaseCompare(void *privdata, void *key1, void *key2)
{
	return strcasecmp(key1, key2);
}

void dictSdsDestructor(void *privdata, void *val)
{
    sdsRelease(val);
}

dictType CommandDictType = 
{
	NULL,
	NULL,
	dictSdsDestructor,
	NULL,
	dictSdsKeyCaseCompare,
	dictSdsCaseHash
};

int dictSdsKeyCompare(void *privdata, void *key1, void *key2)
{
    int len1 = sdsLength((sds)key1);
    int len2 = sdsLength((sds)key2);
    if (len1 != len2)
			return 0;
    return memcmp(key1, key2, len1) == 0;
}

unsigned int dictSdsHash(void *key) 
{
    return dictGenHashFunction(key, sdsLength((sds)key));
}

dictType DatabaseDictType = 
{
	NULL,
	NULL,
	NULL,
	NULL,
	dictSdsKeyCompare,
	dictSdsHash
};

struct Command g_commandTable[] = 
{
		{"get", GetCommand},
		{"set", SetCommand}
};

void GetCommand(CLIENT *pClient)
{
	printf("GetCommand\r\n");
}

void SetCommand(CLIENT *pClient)
{
	printf("SetCommand\r\n");
}

int ProcessCommand(CLIENT *pClient)
{
	if (NULL == pClient)
	{
		return ERROR;
	}
	int i = 0;
	for (i = 0; i < pClient->argc; ++i)
	{
		dictEntry *entry = dictFind(g_server.commands, pClient->argv[i]);	
		struct Command *pCommand = (struct Command *)dictGetVal(entry);
	
		if (NULL == pCommand)	
		{
			AddReplyString(pClient, "unknown command\r\n");
			return OK;
		}
	
		if (NULL != pCommand->proc)
		{
			pCommand->proc(pClient);
			return OK;
		}
	}
	return ERROR;
}

void PopulateCommandTable(void)
{
	int count = sizeof(g_commandTable) / sizeof(struct Command);
	int i = 0;
	for (i = 0; i < count; ++i)
	{
		struct Command *pCommand = g_commandTable + i;
		sds key = sdsNew(g_commandTable[i].name);
		dictAdd(g_server.commands, key, pCommand);
	}
}

void InitServerConfig()
{
	g_server.ipfd = -1;
	g_server.ip = INADDR_ANY;
	g_server.port = 9527;
	g_server.tcpKeepAlive = 1;
	/*int unixfd;
	char *path;
	mode_t mode;*/
	memset(&g_server.neterr, 0, sizeof(g_server.neterr));
	
	g_server.clients = NULL;
	g_server.clientRegectedNum = 0;
	g_server.clientConnectedNum = 0;
	g_server.clientMaxNum = 1024;
	
	g_server.localTime = time(NULL);
	g_server.db = NULL;
	g_server.loop = NULL;
	
	g_server.commands = dictCreate(&CommandDictType, NULL);
	PopulateCommandTable();
}

void InitServer()
{
	if (0 != g_server.port)
	{
		g_server.ipfd = anetTcpServer(g_server.neterr, g_server.ip, g_server.port);
		if (ERROR == g_server.ipfd)
		{
			Log(LOG_WARNING,  "Opening port %d: %s", g_server.port, g_server.neterr);
			exit(1);
		}
	}
	
	g_server.clients = listCreate();	
	g_server.db = dictCreate(&DatabaseDictType, NULL);
	g_server.loop = aeCreate(1024);
		
	if (ERROR == aeCreateFileEvent(g_server.loop, g_server.ipfd, AE_READ, AcceptTcpHandler, NULL))
	{
		Log(LOG_WARNING, "Unrecoverable error creating server.ipfd file event.");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	//signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	srand(time(NULL)^getpid());

	InitServerConfig();
	InitServer();
	aeMain(g_server.loop);
	return 0;
}

