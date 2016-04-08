#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int main()
{

	struct sockaddr_in address;
	bzero( &address, sizeof( address ) );
	address.sin_family = AF_INET;
	inet_pton( AF_INET, "127.0.0.1", &address.sin_addr );
	address.sin_port = htons( 9090 );

	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sfd >= 0);

	//int nReuseAddr = 1;
	//setsockopt( sfd, SOL_SOCKET, SO_OOBINLINE, &nReuseAddr, sizeof( nReuseAddr ) );

	if(-1 == connect(sfd, (struct sockaddr *)(&address), sizeof(struct sockaddr_in)))
	{
		perror("connect");
		return -1;
	}

	send(sfd, "hello", 6, 0);
	if(send(sfd, "OOB", 3, MSG_OOB) == -1) // OOB
	{
		perror("send OOB");
		return -2;
	}

	sleep(2);
	close(sfd);
	return 0;
}
