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

	// ��һ��·�����Unix���׽��ֽ��а�
	bzero(&addr1, sizeof(addr1));
	addr1.sun_family = AF_LOCAL;
	strncpy(addr1.sun_path, argv[1], sizeof(addr1.sun_path) - 1); // ʹ��strncpy, �������, ��Ϊǰ��bzero,���Ա�֤�ַ�����������β \0
	bind(sfd, (struct sockaddr *)&addr1, sizeof(addr1));

	// ������׽��ְ󶨵ĵ�ַ
	len = sizeof(addr2);
	getsockname(sfd, (struct sockaddr *)&addr2, &len);
	printf("bound name = %s, len = %d\n", addr2.sun_path, len);
	return 0;
}
