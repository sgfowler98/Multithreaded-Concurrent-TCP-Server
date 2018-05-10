/*
** Author: sgfowle, Sam Fowler
** Filename: server.c
** Description: this file provides the implementation of a multi-client, concurrent
**		server that is sent files line-by-line, reverses them, and prints
**		them to specified output files.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

void * doStuff();

int ld, sd, addrlen, n, i, k, strlength, filelen, socketno, pid, ack;
struct sockaddr_in skaddr, from;
char buffer[100000], filename[100], timestr[10];
char temp;
FILE *file;
time_t rawtime;
struct tm *timeinfo;
pthread_t tidA;


int main(int argc, char *argv[]) {
	//create socket
	if ((ld = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error encountered while creating socket");
		exit(1);
	}

	//set address
	skaddr.sin_family = AF_INET;
	skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (atoi(argv[1]) < 0 || atoi(argv[1]) > 65535) { //valid port numbers
		printf("Port number must be between 0 and 65535");
		exit(1);
	}
	skaddr.sin_port = htons(atoi(argv[1]));

	//bind address
	if (bind(ld, (struct sockaddr *) &skaddr, sizeof(skaddr)) < 0) {
		printf("Error encountered while binding");
		exit(1);
	}

	//put socket into passive move
	if (listen(ld, 5) < 0) {
		printf("Error encountered while listening");
		exit(1);
	}

	//accepts connections
	printf("Ready for connections...\n");
	while (1) {
		addrlen = sizeof(skaddr);
		sd = accept(ld, (struct sockaddr *) &from, &addrlen);

		//create threads
		if ((pthread_create(&tidA, NULL, doStuff, NULL)) > 0) {
			printf("Error returned by pthread_create");
			exit(1);
		}
		else {
			if ((pthread_join(tidA, NULL)) > 0) {
				printf("Error returned by pthread_join");
				exit(1);
			}
		fclose(file);
		printf("Connection finished - closing socket #%d\n", sd);
		close(sd);
		}
	}
}

void * doStuff() {
	addrlen = sizeof(skaddr);
	if ((getsockname(ld, (struct sockaddr *) &skaddr, &addrlen)) < 0) {
		printf("Error encountered during getsockname");
		exit(1);
	}
	if ((n = read(sd, (char *) &filelen, sizeof(filelen))) < 0) {
		printf("Error encountered while reading filename size");
		exit(1);
	}
	filelen = ntohl(filelen);
	if ((n = read(sd, &filename, filelen)) < 0) {
		printf("Error encountered while reading filename");
		exit(1);
	}

	//generate unique filename
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timestr, 10, "-%I-%M-%S", timeinfo);
	strcat(filename, timestr);
	file = fopen(filename, "w+");

	filelen = strlen(filename);
	if ((n = write(sd, (char *) &filelen, sizeof(filelen))) < 0) {
		printf("Error writing unique filename size");
		exit(1);
	}
	if ((n = write(sd, filename, filelen)) < 0) {
		printf("Error writing unique filename");
		exit(1);
	}

	//get data line by line and print to file in reverse
	n = 1;
	while ((n = read(sd, (char *) &strlength, sizeof(strlength))) > 0) {
		if (n < 0) {
			printf("Error encountered while reading linesize");
			exit(1);
		}
		strlength = htonl(strlength);
		if (strlength > 100000){
			printf("All input strings must be length <= 100000");
			exit(1);
		}
		if ((n = read(sd, &buffer, strlength)) < 0) {
			printf("Error encountered while reading line");
			exit(1);
		}
		for (i = 0, k = strlength - 1; i < strlength/2; i++, k--) {
			temp = buffer[k];
			buffer[k] = buffer[i];
			buffer[i] = temp;
		}
		fputs(buffer, file);
		memset(&buffer, 0 , sizeof(buffer));
		memset(&filename, 0, sizeof(filename));
		memset(&timestr, 0, sizeof(timestr));
	}
}
