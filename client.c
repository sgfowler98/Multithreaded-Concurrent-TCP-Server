/*
** Author: sgfowle, Sam Fowler
** Filename: client.c
** Description: this file provides the implementation of a TCP client that
**		sends data line-by-line to a TCP server, while waiting for
**		ACK messages
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int sk, n, temp, len, ack, filelen;
	struct sockaddr_in skaddr;
	char *input;
	char line[100000], filename[100];
	FILE *file;

	//check for valid argc
	if (argc != 4) {
		printf("Usage: ./client <server address> <server port> <input file>");
		exit(0);
	}

	//create socket
	if ((sk = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error encountered while creating socket");
		exit(1);
	}

	//fill in addr struct
	skaddr.sin_family = AF_INET;
	if (inet_aton(argv[1], &skaddr.sin_addr) == 0) {
		printf("Invalid IP address: %s\n", argv[1]);
		exit(1);
	}
	skaddr.sin_port = htons(atoi(argv[2]));

	//open input file
	input = argv[3];
	file = fopen(input, "r");

	//connect
	if (connect(sk, (struct sockaddr *) &skaddr, sizeof(skaddr)) < 0) {
		printf("Error encountered while connecting socket");
		exit(1);
	}

	//send filename, first sending len then the name itself
	len = strlen(input);
	temp = htonl(len);
	if ((n = write(sk, (char *) &temp, sizeof(temp))) < 0) {
		printf("Error encountered while writing filename size");
		exit(1);
	}
	if ((n = write(sk, input, len)) < 0) {
		printf("Error encountered while writing filename");
		exit(1);
	}

	//read unique filename
	if ((n = read(sk, (char *) &filelen, sizeof(filelen))) < 0 ) {
		printf("Error encountered while reading unique filename size");
		exit(1);
	}
	filelen = ntohl(filelen);
	if ((n = read(sk, &filename, filelen)) < 0) {
		printf("Error encountered while reading unique filename");
		exit(1);
	}
	printf("\nUnique output filename: %s\n", filename);
	if (file != NULL) {
		//send file line-by-line
		while (fgets(line, sizeof(line), file) != NULL) {
			len = strlen(line);
			temp = htonl(len);
			if ((n = write(sk, (char *) &temp, sizeof(temp))) < 0) {
				printf("Error encountered while writing linesize");
				exit(1);
			}
			if ((n = write(sk, line, len)) < 0) {
				printf("Error encountered while writing message");
				exit(1);
			}
		}
		fclose(file);
	}
	else {
		printf("%s is empty.", input);
		exit(0);
	}

	return 0;
}

