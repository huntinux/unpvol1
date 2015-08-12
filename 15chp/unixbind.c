#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
	int sfd;
	int len;
	struct sockaddr_un addr1, addr2;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s pathname\n", argv[0]);
		exit(1);
	}
	sfd = socket(AF_LOCAL, SOCK_STREAM, 0);

	unlink(argv[1]);

	// 将一个路径与该Unix域套接字进行绑定
	bzero(&addr1, sizeof(addr1));
	addr1.sun_family = AF_LOCAL;
	strncpy(addr1.sun_path, argv[1], sizeof(addr1.sun_path) - 1); // 使用strncpy, 避免溢出, 因为前面bzero,可以保证字符串的正常结尾 \0
	bind(sfd, (struct sockaddr *)&addr1, sizeof(addr1));

	// 获得与套接字绑定的地址
	len = sizeof(addr2);
	getsockname(sfd, (struct sockaddr *)&addr2, &len);
	printf("bound name = %s, len = %d\n", addr2.sun_path, len);
	return 0;
}
