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


    // �����׽���
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    // INADDR_ANY : ����������ж������ӿڣ���ô��������������������ӿ��Ͻ��ܿͻ�����
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT);

    // �����׽������ַ��
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    // �Ѹ��׽��֣���ɼ����׽���
    // ���������Կͻ����������ӾͿ����ڸ��׽��������ں˽���
    // socket, bind, listen ��TCP������׼���������׽��֡� ����������
    listen(listenfd, 5);

    for(;;) {
        // ������˯�ߣ��ȴ��ͻ�������
        // ���� TCP 3������
        // ����һ�� ����������������
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);

        // ����ʱ����ͻ���
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));

		write(connfd, buff, strlen(buff));
	//	for (i = 0; i<strlen(buff); i++)
	//		write(connfd, buff + i, 1);

        // �ر�����
        close(connfd);
    }


    return 0;
}
