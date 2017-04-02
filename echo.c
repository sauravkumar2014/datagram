#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "4950"	// the port users will be connecting to
#define RECSIZE 30000	//Maximum receiveing buffer size

struct pkt {		//struct to store pkt
  int a;
  struct timeval tm;
  int rt;
};

int main(int argc, char *argv[])
{
	if (argc >= 2 ) {
		if(strcmp(argv[1],"-h")==0){				//help instructions
			printf("USAGE: \n");
			printf("\t STEP 1:make\n");
			printf("\t STEP 2:serverexecommand\n");
			printf("\t \t example: ./echo.out\n");
			exit(1);
		}
	}
	
	int sockfd,rv;				//sockfd:socketdescriptor		rv:value returned while getting add info
	struct addrinfo hints, *servinfo;	//hints:info for setting up servinfo		servinfo:contains serverinfo
	struct sockaddr_storage their_addr;	//address of the client
	socklen_t addr_len;			//length of address of client
	const int pktsize=sizeof(struct pkt);	//size of struct packet

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 			// set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;			//use socket datagram
	hints.ai_flags = AI_PASSIVE; 			// use my IP
	hints.ai_protocol=0;			


	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {		//get server's own address info
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	//get socket descriptor
		if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol)) == -1) {
			perror("listener: socket");
		}
	//bind server to the specified port
		if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
		}

	if (servinfo == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	//server listening on the port
	printf("listener: waiting to recvfrom...\n");

	while(1){
		addr_len = sizeof their_addr;
		struct pkt buf;
		char packet[RECSIZE];
		//recieve packet
			if (recvfrom(sockfd, &packet, RECSIZE , 0, (struct sockaddr *)&their_addr, &addr_len) == -1) {
				perror("recvfrom");
				exit(1);
			}

		memcpy(&buf,&packet,pktsize);		//byte to struct
		buf.rt--;				//decrement RC value
		memcpy(&packet,&buf,pktsize);		//struct to byte
		//printf("sent");
		//send packet back to client
			if (sendto(sockfd, &packet , RECSIZE , 0, (struct sockaddr *)&their_addr, addr_len) == -1) 
			{
				perror("talker: sendto");
				exit(1);
			}	
	}
	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}
