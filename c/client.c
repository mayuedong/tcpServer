#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>

char g_buf[512];

int main(void)
{
    int iFdListen = socket(AF_INET, SOCK_STREAM, 0);
    int iYes = 1;
    setsockopt(iFdListen, SOL_SOCKET, SO_REUSEADDR, &iYes, sizeof(iYes));

    struct sockaddr_in stListenAddr;
    stListenAddr.sin_family = AF_INET;
    stListenAddr.sin_port = htons(9527);
    //stListenAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    inet_pton(AF_INET, "127.0.0.1", &(stListenAddr.sin_addr));

    if (-1 == connect(iFdListen, (struct sockaddr *)&stListenAddr, sizeof(stListenAddr)))
    {
		perror("connect");return -1;
    }

    struct epoll_event *pstEvents = (struct epoll_event *)malloc(sizeof(struct epoll_event) * 1024);
    struct epoll_event event;

    int epfd = epoll_create1(EPOLL_CLOEXEC);
    
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = iFdListen;
    if (0 == epoll_ctl(epfd, EPOLL_CTL_ADD, iFdListen, &event))
    {
		printf("ctl ok\r\n");
    }

    int infd = fileno(stdin);
    event.events = EPOLLOUT | EPOLLET;
    event.data.fd = infd;
    if (!epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &event))
    {
		printf("ctl ok\r\n");
    }
    
    while (1)
    {
        int count = epoll_wait(epfd, pstEvents, 1024, -1);
        int i = 0;
        for (i = 0; i < count; i++)
        {
        	if (pstEvents[i].events & EPOLLIN)
            {
                memset(g_buf, 0, sizeof(g_buf));
                int ReadNum = read(iFdListen, g_buf, sizeof(g_buf));
                printf("buf[%d] = %s\r\n", strlen(g_buf), g_buf);
            }
            else if (pstEvents[i].events & EPOLLOUT)
            {
                memset(g_buf, 0, sizeof(g_buf));
                fgets(g_buf, 1024, stdin);
                write(iFdListen, g_buf, strlen(g_buf));
            }
           
        }
    }
    return 0;
}
