
CC=gcc
CFLAG= -DHAVE_MSGHDR_MSG_CONTROL  -O2 -Wall

all:mycat openfile

mycat:mycat.c read_fd.c
	$(CC) $^ $(CFLAG) -o $@

openfile:openfile.c write_fd.c
	$(CC) $^ $(CFLAG) -o $@
