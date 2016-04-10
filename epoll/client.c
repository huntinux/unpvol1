
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>  
#include <stdlib.h>		// exit
#include <netinet/in.h> // socketaddr_in 
#include <strings.h>	// bzero
#include <arpa/inet.h>	// inet_pton
#include <unistd.h>		// read
#include <string.h>

#define	MAXLINE		4096	/* max text line length */
//#define PORT 8888

int main(int argc, char ** argv)
{
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;
	int cnt = 0;
	pid_t clipid;

    if (argc != 4) {
       fprintf(stderr, "Usage: %s IP port Message\n", argv[0]);
       exit(1);
    }

	// 创建套接字 
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	   perror("socket err\n");
       exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(atoi(argv[2]));  // 端口号使用网络字节序
    //inet_pton(AF_INET, argv[2], &servaddr.sin_port);  // 端口号使用网络字节序
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <=0) {// 将ip地址由字符串，转换位2进制
        fprintf(stderr, "inet_pton error for: %s", argv[1]);
        exit(1);
    }

	// 连接服务器, 后续使用该套接字进行通信
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect error: %s\n", argv[1]);
        perror("connect error:");
        exit(1);
    }

	// 与服务器进行通信
	snprintf(recvline, MAXLINE, "Hello, I am client with pid: %d.My message is:%s", getpid(), argv[3]);
	n = write(sockfd, recvline, strlen(recvline) + 1);
	/*
    while( (n = read(sockfd, recvline, MAXLINE)) > 0) {
		cnt++;
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error\n");
            exit(1);
        }
    }
	*/

    if(n < 0) {
        fprintf(stderr, "write error\n");
		close(sockfd);
        exit(1);
    }    

	close(sockfd);
    return 0;
}
