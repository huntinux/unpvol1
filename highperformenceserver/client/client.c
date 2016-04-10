/**
 *	测试服务器的客户端程序
 *	启动多个线程，主线程控制子线程何时开始
 *	计划使用互斥锁和条件变量 
 */
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>  
#include <stdlib.h>		// exit
#include <netinet/in.h> // socketaddr_in 
#include <strings.h>	// bzero
#include <arpa/inet.h>	// inet_pton
#include <unistd.h>		// read
#include <string.h>


int create_and_conncect(const char *ip, const char *port)
{
	int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	   perror("socket err");
	   return -1;
    }

	struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(atoi(port));  
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <=0) {// 将ip地址由字符串，转换位2进制
        fprintf(stderr, "inet_pton error for: %s", ip);
		return -1;
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error:");
		return -1;
	}
    
	return sockfd;
}

volatile int g_continue = 0;


void* thread_proc(void *arg)
{
	while(g_continue == 0)
		;

	printf("thread: %lu start.\n", pthread_self());

	int sfd = create_and_conncect("127.0.0.1", "9090");

	char *buff = "hello world.";
	int count = 10;
	while(count--)
		send(sfd, buff, strlen(buff) + 1, 0);

	close(sfd);

	//pthread_mutex_lock(&mutex);
	//pthread_cond_wait(&cond);

	/* connect to server and send some data */

	//pthread_mutex_unlock(&mutex);
}

/* mutex lock and condition variable */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Usage: %s [thread_num]\n", argv[0]);
		return -1;
	}

	int thread_num = atoi(argv[1]);
	printf("Begin to create %d threads.\n", thread_num);

	int i;
	for(i = 0; i < thread_num; i++)
	{
		pthread_t tid;
		pthread_create(&tid, NULL, thread_proc, NULL);
	}

	g_continue = 1;
	while(1)
	{
		;
	}
	return 0;
}
