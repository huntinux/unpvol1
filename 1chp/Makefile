CC = gcc
FLAGS = -Wall -O2

PROGS = daytimetcpsrv daytimetcpcli

all: ${PROGS}

daytimetcpsrv : daytimetcpsrv.c
	${CC} ${FLAGS} -o $@ $^

daytimetcpcli : daytimetcpcli.c
	${CC} ${FLAGS} -o $@ $^



.PHONY : clean 
clean:
	rm  ${PROGS}
