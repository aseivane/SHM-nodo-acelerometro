/*
 * tic-toc-sender.c
 *
 *  Created on: May 9, 2020
 *      Author: jaatadia@gmail.com
 */
#include "daemon.h"

#include <stdio.h>

#include "time.h"


#include "microtime.h"
#include "esp_timer.h"
#include <inttypes.h>

#define TICTOC_DAEMON_DEBUG

//#define DAEMON_SERVER

/*
//To use this enable `Component config->FreeRTOS->Enable FreeRTOS trace facility`
void printRemainingStack(const char* identifier){
	TaskStatus_t *pxTaskStatusArray;
	volatile UBaseType_t uxArraySize;
	unsigned long ulTotalRunTime;

	// Take a snapshot of the number of tasks in case it changes while this function is executing.
	uxArraySize = uxTaskGetNumberOfTasks();

	// Allocate a TaskStatus_t structure for each task.
	pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

	// Generate raw status information about each task.
	uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalRunTime );

	for(int i = 0; i < uxArraySize; i++) {
		if(strcmp("TicTocDaemon", pxTaskStatusArray[i].pcTaskName) == 0){
			printf("%s - task %s: remaining stack higher mark %u (bytes)\n", identifier, pxTaskStatusArray[i].pcTaskName, pxTaskStatusArray[i].usStackHighWaterMark);
			break;
		}
	}

	vPortFree( pxTaskStatusArray );
} */

/* **************************************************
*	
*                 Server Daemon
*	
**************************************************** */

int setupServerConnection() {
	int socketfd;
	if ( (socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("Failed to create socket"); 
		exit(EXIT_FAILURE); 
	} 

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(8080); 
	
	if (bind(socketfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) 
	{ 
		perror("Failed to bind socket"); 
		exit(EXIT_FAILURE); 
	} 
	return socketfd;
}

void receiveTimeStamps(void * parameter) {
	TicTocData* ticTocData = (TicTocData*) parameter;

	BufferData bufferData;
	int32_t* timestamps = bufferData.timestamps;
	size_t incomingSize = sizeof(int32_t) * 2 * 3;
	socklen_t cliadrrSize = sizeof(struct sockaddr_in);

	for(;;) {
		memset(&bufferData.cliaddr, 0, cliadrrSize);

		if(recvfrom(ticTocData->socketfd, (int32_t *) timestamps, incomingSize, MSG_WAITALL, (struct sockaddr*) &bufferData.cliaddr, &cliadrrSize) > 0){
			encodeEpochInMicros(esp_timer_get_time(), timestamps, 2);

			#ifdef TICTOC_DAEMON_DEBUG
			if(*(ticTocData->timeRequest) != -1) {
				printf("TicTocDaemon ---------------- timeRequested: %"PRId64"-------------------.\n", *(ticTocData->timeRequest));
				*(ticTocData->timeRequest) = -1;
			}
			#endif

			printf("TicTocDaemon - received t1: %"PRId64".\n", decodeEpochInMicros(timestamps,0));

			//char buffer[INET_ADDRSTRLEN];
			//inet_ntop(AF_INET, &bufferData.cliaddr, buffer, sizeof( buffer ));
			//printf("will send to %s\n", buffer);
			UBaseType_t res = xRingbufferSend(ticTocData->buf_handle, &bufferData, sizeof(bufferData), pdMS_TO_TICKS(1000));
			//if (res != pdTRUE) {
			//  printf("Failed to send item\n");
			//}
		}
	}
}


void provideTimeStamps(void * parameter) {
	TicTocData* ticTocData = (TicTocData*) parameter;
	size_t outGoingSize = sizeof(int32_t) * 2 * 3;
    size_t item_size = sizeof(BufferData);
	for(;;) {
	    BufferData * item = (BufferData *)xRingbufferReceive(ticTocData->buf_handle, &item_size, pdMS_TO_TICKS(100000));
	    if (item != NULL) {
	    	//char buffer[INET_ADDRSTRLEN];
	    	//inet_ntop(AF_INET, &item->cliaddr, buffer, sizeof( buffer ));
	    	//printf("Sending to %s\n", buffer);
	    	encodeEpochInMicros(esp_timer_get_time(), item->timestamps, 4);
	    	sendto(ticTocData->socketfd, (const int32_t *)item->timestamps, outGoingSize, 0, (const struct sockaddr *) &(item->cliaddr), sizeof(item->cliaddr));
	        vRingbufferReturnItem(ticTocData->buf_handle, (void *)item);
	    }
	}

}

/* **************************************************
*	
*                 Client Daemon
*	
**************************************************** */

void setupServerAddr(struct sockaddr_in * servaddr, const char * serverIp, int serverPort) {
	memset(servaddr, 0, sizeof(*servaddr));
	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(serverPort);
	//servaddr->sin_addr.s_addr = INADDR_ANY;
	inet_aton(serverIp, &(servaddr->sin_addr.s_addr));
}


int getStamps(int sock, const struct sockaddr * servaddr, size_t servaddrSize, int64_t* resultArray){
	size_t outGoingSize = sizeof(int32_t) * 2 * 3;
	size_t incomingSize = sizeof(int32_t) * 2 * 3;
	int32_t timestamps[2 * 4];
	int64_t t1 = esp_timer_get_time();
	encodeEpochInMicros(t1, timestamps, 0);

	//sending t1
	sendto(sock, (const int32_t *)timestamps, outGoingSize,  0, servaddr, servaddrSize);

	if(recv(sock, (int32_t *)timestamps, incomingSize, MSG_WAITALL) == incomingSize){
		resultArray[3]=esp_timer_get_time();
		for(int i = 0; i<3; i++) {
			resultArray[i]=decodeEpochInMicros(timestamps,2*i);
		}

		if(resultArray[0]!=t1){
			//receive old package discard it, and wait for the current one, that we discard due to contamination caused by the old package
			recv(sock, (int32_t *)timestamps, incomingSize, MSG_WAITALL);
			return 0;
		}
		return 1;
	} else {
		return 0;
	}
}



void getTimeStamps(void * parameter){
		int64_t loopCount = 0;
		TicTocData* ticTocData = (TicTocData*) parameter;


		struct sockaddr_in servaddr;
		setupServerAddr(&servaddr, ticTocData->serverIp, ticTocData->serverPort);
		socklen_t servaddrSize = sizeof(servaddr);

		int64_t resultArray[4];

		for(;;) {
			loopCount++;

			#ifdef TICTOC_DAEMON_DEBUG
			if(*(ticTocData->timeRequest) != -1) {
				printf("TicTocDaemon - timeRequested:%"PRId64" tictocTime:%"PRId64".\n",
						*(ticTocData->timeRequest),
						ticTocReady(ticTocData) ? sicTime(&ticTocData->sicdata, *(ticTocData->timeRequest)) : 0);
				*(ticTocData->timeRequest) = -1;
			}	
			#endif

			if(getStamps(ticTocData->sock, (const struct sockaddr *) &servaddr, servaddrSize, resultArray)){
				if (loopCount > 10) {
					#ifdef TICTOC_DAEMON_DEBUG
					printf("TicTocDaemon %lld - t1:%"PRId64" t2:%"PRId64" t3:%"PRId64" t4:%"PRId64"\n", loopCount, resultArray[0], resultArray[1], resultArray[2], resultArray[3]);
					#endif
					sicStep(&ticTocData->sicdata, resultArray[0], resultArray[1], resultArray[2], resultArray[3]);	
				}

			} else {
				#ifdef TICTOC_DAEMON_DEBUG
				printf("TicTocDaemon timeout\n");
				#endif
				sicStepTimeout(&ticTocData->sicdata);
			}

			// Pause the task for TIC_TOC_PERIOD ms
			vTaskDelay(TIC_TOC_PERIOD / portTICK_PERIOD_MS);
		}
}


void setupTicTocClient(TicTocData* ticToc, const char * serverIp, int serverPort){
	sicInit(&ticToc->sicdata);
	int sockfd;
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		return;
	}
	
	//read timeout
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 800000; // 0.8 seconds
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const struct timeval*)&tv, sizeof(tv));
	

	ticToc->sock=sockfd;
	ticToc->serverIp = serverIp;
	ticToc->serverPort = serverPort;

	xTaskCreatePinnedToCore(
			// Function that should be called
			getTimeStamps,
		    "TicTocDaemon",				// Name of the task (for debugging)
		    4000,						// Stack size (bytes)
		    ticToc,						// Parameter to pass
			TIC_TOC_DAEMON_PRIORITY,	// Task priority
		    NULL,             			// Task handle
			1						    // Execution Core
		  );
}

void setupTicTocServer(TicTocData* ticToc){

	 //Create ring buffer
	ticToc->buf_handle = xRingbufferCreate(sizeof(BufferData) * 20, RINGBUF_TYPE_NOSPLIT);
	if (ticToc->buf_handle == NULL) {
		printf("Failed to create ring buffer\n");
	}

	ticToc->socketfd = setupServerConnection();

	xTaskCreatePinnedToCore(
		// Function that should be called
		receiveTimeStamps,
		"TicTocDaemonServerListener",// Name of the task (for debugging)
		4000,						// Stack size (bytes)
		ticToc,						// Parameter to pass
		TIC_TOC_DAEMON_PRIORITY,	// Task priority
		NULL,             			// Task handle
		1						    // Execution Core
	);

	xTaskCreatePinnedToCore(
		// Function that should be called
		provideTimeStamps,
		"TicTocDaemonServerListener",// Name of the task (for debugging)
		4000,						// Stack size (bytes)
		ticToc,						// Parameter to pass
		TIC_TOC_DAEMON_PRIORITY,	// Task priority
		NULL,             			// Task handle
		0						    // Execution Core
	);
}

void setupTicToc(TicTocData* ticToc, const char * serverIp, int serverPort)
{
	#ifdef DAEMON_SERVER
	setupTicTocServer(ticToc);
	#else
	setupTicTocClient(ticToc, serverIp, serverPort);
	#endif
}

int IRAM_ATTR ticTocReady(TicTocData * ticTocData){
	return sicTimeAvailable(&ticTocData->sicdata);
}

int64_t IRAM_ATTR ticTocTime(TicTocData * ticTocData){
	return sicTime(&ticTocData->sicdata, esp_timer_get_time());
}
