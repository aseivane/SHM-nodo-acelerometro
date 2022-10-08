// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include "microtime.h"

#define SERVER "127.0.0.1" 
#define PORT 8080 

int setupConnection() {
	int sockfd; 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000; // 0.1 seconds
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const struct timeval*)&tv, sizeof(tv));

	return sockfd;
}

void setupServerAddr(struct sockaddr_in * servaddr) {
	memset(servaddr, 0, sizeof(*servaddr)); 
	servaddr->sin_family = AF_INET; 
	servaddr->sin_port = htons(PORT); 
	//servaddr->sin_addr.s_addr = INADDR_ANY; 
	inet_aton(SERVER, &(servaddr->sin_addr.s_addr));
}

int main() { 
	int sockfd = setupConnection();	
	int32_t timestamps[2 * 4]; //TODO remove magic constants
	
	struct sockaddr_in servaddr;
	setupServerAddr(&servaddr); 
	
	size_t outGoingSize = sizeof(int32_t) * 2; 
	size_t incomingSize = sizeof(int32_t) * 2 * 3;
	socklen_t servaddrSize = sizeof(servaddr);

	long long timestamp = epochInMicros();
	encodeEpochInMicros(timestamp, &timestamps, 0);

	printf("Sending t1: %lld.\n", timestamp); 
	sendto(sockfd, (const int32_t *)timestamps, outGoingSize,  0, (const struct sockaddr *) &servaddr, servaddrSize); 
	printf("Awaiting response...\n"); 
	
	// TODO add verification of server identity	
	if(recv(sockfd, (int32_t *)timestamps, incomingSize, MSG_WAITALL) == incomingSize){
		encodeEpochInMicros(epochInMicros(), &timestamps, 3*2);
		printf("Received timestamps\n"); 	
		for(int i = 0; i<4; i++) {
			printf("t%d: %lld\n", i + 1, decodeEpochInMicros(&timestamps, i*2)); 	
		}	
	} else {
		printf("timeout\n");
	}
	
	close(sockfd); 
	return 0; 
} 
