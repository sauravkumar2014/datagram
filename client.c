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
#include <stdbool.h>
#define SERVERPORT "4950"	// the port users will be connecting to
#define LOOP 50			//no of datapackets to send
#define RT atoi(argv[2])	//initial RC value
#define MAXSIZE atoi(argv[3])	//Packet Size
#define OUTPUTFILE argv[4]	//Output File
struct pkt {			//Single packet structure
  int a;
  struct timeval tm;
  int rt;
};

int main(int argc, char *argv[])
{
	if (argc < 2 ) {						//if wrong help/input
		fprintf(stderr,"wrong input\n");
		fprintf(stderr,"For help use \"clientexecommand -h\" \n");
		fprintf(stderr,"EX: \"./client.out -h\" \n");
		exit(1);
	}
	if(strcmp(argv[1],"-h")==0){				//help instructions
		printf("USAGE: \n");
		printf("\t STEP 1:make\n");
		printf("\t STEP 2:clientexecommand IPv4Address RCInitialValue PacketSizeP Filename\n");
		printf("\t \t example: ./client.out \"127.0.0.1\" 10 200 output.txt\n");
		exit(1);
	}
	if (argc != 5) {					//if wrong input
		fprintf(stderr,"wrong input\n");
		fprintf(stderr,"For help use \"client -h\" \n");
		exit(1);
	}

	int sockfd,i,rv;		//sockfd:socketdescriptor	i:for loop		rv:value returned while getting add info
	struct addrinfo hints, *servinfo;	//hints:info for setting up servinfo		servinfo:contains serverinfo
	FILE *fptr;				//ptr to write to file
	struct timeval delay,now;		//for packetloss
	const int pktsize=sizeof(struct pkt);	//size of 1 struct pkt

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;		//don't know if IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM;		//Socket Datagram sockettype

	//get address info using the info provided
	if ((rv=getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo))!= 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	//setup socket
	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol)) == -1) {
		perror("talker: socket");
	}
	//open file to write to
	fptr=fopen(OUTPUTFILE,"w");

	//loop to send 50 datapackets
	for(i=0;i<LOOP;i++){
		struct pkt t;			//packet in struct
		t.a=i+1;	gettimeofday(&t.tm, NULL);	t.rt=RT;
		char packet[MAXSIZE];		//packet in bytes
		bool drop=false;		//check for packetloss

		while(t.rt>0){			//unless RC is 0
			memcpy(&packet,&t,pktsize);		//struct to byte

			gettimeofday(&delay, NULL);		//starting time of sending of pkt

			//send packet
			if (sendto(sockfd, &packet , MAXSIZE, 0, servinfo->ai_addr, servinfo->ai_addrlen) == -1) 
			{
				perror("talker: sendto");
				exit(1);
			}
			//recieve packet
			if (recvfrom(sockfd, &packet, MAXSIZE , 0, servinfo->ai_addr, &servinfo->ai_addrlen) == -1) {
				perror("recvfrom");
				exit(1);
			}
			gettimeofday(&now, NULL);
			timersub(&now,&delay, &delay);			//subtract starting time of sending packet with present time
			if(delay.tv_usec>=800 && delay.tv_sec>0){				//if more than delay time drop
				printf("packet has been dropped \n");
				drop=true;
				break;
			}
			memcpy(&t,&packet,pktsize);			//byte to struct
			t.rt--;						//decrement RC
		}

		char c[pktsize];				//string of pkt excluding RC value
		if(drop==false){					//if not dropped
			gettimeofday(&now, NULL);
			timersub(&now,&t.tm, &t.tm);			//total time taken in reaching RC=0
			sprintf(c,"%d\t%lu.%lu\n",t.a,t.tm.tv_sec,t.tm.tv_usec);//fill in the string to be written to file
			fprintf(fptr,"%s",c);				//write to file
		}
		else{							//packet dropped
			//sprintf(c,"%d\tFAIL\n",t.a);			//fill in string to be written to file
			//fprintf(fptr,"%s",c);				//write to file
		}
	}
	freeaddrinfo(servinfo);				//delete servinfo
	fclose(fptr);					//close file ptr
	close(sockfd);					//close socket
	return 0;
}
