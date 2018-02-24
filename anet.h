#ifndef _ANET_H_
#define _ANET_H_

#include "common.h"

#define _ANET_BLOCK_    0
#define _ANET_NONBLOCK_ 1

#define _ANET_NODELAY_  0
#define _ANET_DELAY_    1

#define ANET_ERRLEN		256


int anetNonBlock(char *err, int fd);
int anetBlock(char *err, int fd);
int anetTcpNoDelay(char *err, int fd);
int anetTcpDelay(char *err, int fd);
int anetSetSendBuffer(char *err, int fd, int size);
int anetTcpKeepAlive(char *err, int fd);
int anetSendTimeout(char *err, int fd, long long ms); 

int anetTcpAccept(char *err, int fd, char *ip, int *port);
int anetUnixAccept(char *err, int fd);

int anetTcpServer(char *err, char *hostname, int port);
int anetUnixServer(char *err, char *path, mode_t mode);

int anetTcpNonBlockConnect(char *err, char *ip, int port);
int anetTcpBlockConnect(char *err, char *ip, int port);
int anetUnixNonBlockConnect(char *err, char *path);
int anetUnixBlockConnect(char *err, char *path);

int anetRead(char *err, int fd, void *buf, int len);
int anetWrite(char *err, int fd, void *buf, int len);

int anetResolve(char *err, char *host, char *ip, char len);
int anetSockName(char *err, int fd, int *port, char *ip, int len);
int anetPeerName(char *err, int fd, int *port, char *ip, int len);


#endif//_ANET_H_
