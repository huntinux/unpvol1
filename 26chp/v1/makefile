
CC=gcc
FLAGE= -lpthread -Wall -O2

all: server client


server: tcpserv01.c
	$(CC) $(FLAGE) $^ -o $@


client: tcpcli01.c
	$(CC) $(FLAGE) $^ -o $@


clean:
	rm server client
