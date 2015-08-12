#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for execl
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFSIZE 1024


/*
	create a child process A (openfile), 
	A will open a file 
	return the file descriptor of the file to Parent

*/


int my_open(const char *pathname, int mode)
{
	int			fd, sockfd[2], status;
	pid_t		childpid;
	char		c, argsockfd[10], argmode[10];

	// create socket pair , store in sockfd[]
	socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);

	if ( (childpid = fork()) == 0) {		/* child process */
		close(sockfd[0]);
		snprintf(argsockfd, sizeof(argsockfd), "%d", sockfd[1]);
		snprintf(argmode, sizeof(argmode), "%d", mode);
		execl("./openfile", "openfile", argsockfd, pathname, argmode, // run openfile
			  (char *) NULL);
		//err_sys("execl error");
	}

	/* parent process - wait for the child to terminate */
	close(sockfd[1]);			/* close the end we don't use */

	waitpid(childpid, &status, 0);
	if (WIFEXITED(status) == 0) {
		fprintf(stderr, "child did not terminate");
		exit(1);
	}
	if ( (status = WEXITSTATUS(status)) == 0)
		Read_fd(sockfd[0], &c, 1, &fd);
	else {
		errno = status;		/* set errno value from child's status */
		fd = -1;
	}

	close(sockfd[0]);
	return(fd);
}

int main(int argc, char **argv)
{
	int		fd, n;
	char	buff[BUFFSIZE];

	if (argc != 2) {
		fprintf(stderr, "usage: mycat <pathname>");
		exit(1);
	}

	if ( (fd = my_open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "cannot open %s", argv[1]);
		exit(1);
	}

	while ( (n = read(fd, buff, BUFFSIZE)) > 0)
		write(STDOUT_FILENO, buff, n);

	exit(0);
}
