#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>  
#include <stdlib.h>     // exit
#include <netinet/in.h> // socketaddr_in 
#include <strings.h>    // bzero
#include <arpa/inet.h>  // inet_pton
#include <unistd.h>     // read
#include <time.h>       // time
#include <string.h>     // strlen

#define	MAXLINE		4096	/* max text line length */
#define PORT    8888

int main(int argc, char ** argv)
{
    struct sockaddr_in servaddr;
    int listenfd, connfd;
    char buff[MAXLINE];
    time_t ticks;
	int i;


    // 创建套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    // INADDR_ANY : 如果服务器有多个网络接口，那么服务器可以在任意网络接口上接受客户连接
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT);

    // 将该套接字与地址绑定
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    // 把该套接字，变成监听套接字
    // 这样，来自客户到外来连接就可以在该套接字上由内核接受
    // socket, bind, listen 是TCP服务器准备“监听套接字” 的正常步骤
    listen(listenfd, 5);

    for(;;) {
        // 服务器睡眠，等待客户端连接
        // 经过 TCP 3次握手
        // 返回一个 “已连接描述符”
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

        // 发送时间给客户端
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));

		write(connfd, buff, strlen(buff));
	//	for (i = 0; i<strlen(buff); i++)
	//		write(connfd, buff + i, 1);

        // 关闭连接
        close(connfd);
    }


    return 0;
}
