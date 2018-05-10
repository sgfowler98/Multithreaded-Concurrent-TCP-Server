#
#Author: sgfowle, Sam Fowler
#Filename: Makefile
#Description: A file containing compilation and linking rules for the TCP server/client

LIBS = -lsocket -lnsl

CLAGS = -g -Wall

.c.o:	gcc -c -Wall ${CFLAG} $<

all: server client

server: server.o
	gcc -pthread -o server server.o

client: client.o
	gcc -o client client.o

clean:
	rm *.o client server
