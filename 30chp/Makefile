
allserver=serv0 serv1 serv2 serv3 serv4 serv6 serv7 serv8 

all: $(allserver) client

serv0:serv00.c
	gcc $^ -o $@ 

serv1:serv01.c
	gcc $^ -o $@

serv2:serv02.c
	gcc $^ -o $@

serv3:serv03.c
	gcc $^ -o $@ -lpthread

serv4:serv04.c
	gcc $^ -o $@ -lpthread

serv6:serv06.c
	gcc $^ -o $@ -lpthread

serv7:serv07.c
	gcc $^ -o $@ -lpthread

serv8:serv08.c
	gcc $^ -o $@ -lpthread

client:client.c
	gcc $^ -o $@ 

clean:
	rm $(allserver) client
