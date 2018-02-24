#ifndef _SERVER_H_
#define _SERVER_H_

#include <errno.h>
#include <strings.h>
#include "sds.h"
#include "list.h"
#include "dict.h"
#include "anet.h"
#include "ae.h"

#define REPLY_LEN	(1024 * 16)
#define QUERY_LEN	(1024 * 16)

typedef struct DB
{
	dict *database;
}DB;

typedef struct CLIENT
{
	int fd;
	sds queryBuf;
	sds replyBuf;
	dict *db;
	int argc;
	sds *argv;
	time_t lastTime;
}CLIENT;

typedef void CommandProc(CLIENT *pClient);
struct Command
{
	char *name;
	CommandProc *proc;
};

void GetCommand(CLIENT *pClient);
void SetCommand(CLIENT *pClient);



struct SERVER
{
	int ipfd;
	int port;
	char *ip;
	int tcpKeepAlive;

	int unixfd;
	char *path;
	mode_t mode;
	char neterr[ANET_ERRLEN];

	EventLoop *loop;
	
	list *clients;
	int clientMaxNum;
	int clientRegectedNum;
	int clientConnectedNum;

	time_t localTime;

	dict *db;
	dict *commands;
};

void PopulateCommandTable(void);
void InitServerConfig();
void InitServer();



void AddReplyString(CLIENT *pClient, char *str);
void AddReplySds(CLIENT *pClient, sds str);

int ProcessCommand(CLIENT *pClient);
int PrepareClientToWrite(CLIENT *pClient);

void SendReplyToClient(EventLoop *loop, void *privdata, int fd, int mask);
int ProcessInputBuf(CLIENT*pClient);
void ReadQueryFromClient(EventLoop *loop, void *clientData, int fd, int mask);
CLIENT *CreateClient(int fd);
void ReleaseClient(CLIENT *pClient);
void AcceptTcpHandler(EventLoop *loop, void *clientData, int fd, int mask);

extern struct SERVER g_server;
#endif//_SERVER_H_
