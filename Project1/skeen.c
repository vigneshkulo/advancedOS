/*
 * Submitted By    : Vignesh Kulothungan
 * Subject         : Advanced Operating Systems - Project 1
 * Module          : Total Order Mulicast Module - Skeen 
 */

/* User Define Header Files */
#include "include.h"

/* System Header Files */
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/syscall.h>
#include <netinet/sctp.h>

/* -------- Static Definitions Start -------- */
#define RECEIVE_RETRY_TIME	5

#define READ_FD         	10
#define WRITE_FD        	11

#define INITIAL			0
#define PROPOSED		1
#define FINAL			2

#define MAX_MULTICAST_MSGS	500
#define MAX_MULTICAST_MEMBERS	45

#define MAX(a,b) (((a)>(b))?(a):(b))

/* -------- Static Definitions End -------- */

/* -------- Global Variables Start -------- */
sem_t semQueue;
sem_t semTimeStamp;
sem_t semSharedMem;
sem_t semSharedMem1;

int usrPID;
int procID;
int numMcMsg;
int memSet = 0;
int msgCount = 0;
int gTimeStamp = 0;
int recvMsgCount =  0;
char sendFile[20];
char recvFile[20];
FILE *fpSend = NULL;
int numMcMemb[MAX_MULTICAST_MSGS];
int mcMemb[MAX_MULTICAST_MSGS][MAX_MULTICAST_MEMBERS];
char msgData[MAX_MULTICAST_MSGS][MAX_MULTICAST_MEMBERS];

typedef struct strMsgQ
{
	int type;
	int msgId;
	int rcvId;
	int sendId;
	int timeStamp;
	int propTimeStamp;
	struct strMsgQ* next;
}strMsgQ;

strMsg gMsgShare;

/* -------- Global Variables End -------- */


/* -------- Function Definition Start -------- */
int msgSend(int);
int sendMsg(int , int , int , int );
/* -------- Function Definition End -------- */

/* Called by Send Thread (Main Process) 
 * Handles the Send part of Skeen Algorithm.
 * Sends both INITIAL and FINAL messages to the multicast group members 
 */
int msgSend(int msgNum)
{
	int i;
	char str[10];
	int timeStamp;
	int propTS[MAX_MULTICAST_MEMBERS];
	if(msgNum > numMcMsg-1) return 0;

	/* Use Semaphore to access the Lamport Logical Clock */
	sem_wait(&semTimeStamp);
	++gTimeStamp;
	timeStamp = gTimeStamp;
	sem_post(&semTimeStamp);
	
	/* Send Message Count */
	msgCount++;

	/* Write to File the Message Id being Sent */
	sprintf(str, "%5d ", (msgCount << 6) | usrPID);
	fwrite(str , 1 , strlen(str) , fpSend );

	if((msgCount % 10) == 0) fwrite("\n" , 1, 1, fpSend );

	/* Sending INITIAL Message */
	for(i = 0; i < numMcMemb[msgNum]; i++)
	{
		sendMsg(INITIAL, mcMemb[msgNum][i], -1, timeStamp);
	}
	
	/* Waiting for PROPOSED TimeStamps */
	for(i = 0; i < numMcMemb[msgNum]; i++)
	{
		sem_wait(&semSharedMem);
		propTS[i] = gMsgShare.propTimeStamp;
		sem_post(&semSharedMem1);
	}

	/* Bubble Sort to get MAX of Proposed TimeStamps */
        int swap = 0;
	for(i = 0; i < numMcMemb[msgNum] - 1; i++)
        {
                if (propTS[i] > propTS[i+1]) 
                {
                        swap       = propTS[i];
                        propTS[i]   = propTS[i+1];
                        propTS[i+1] = swap;
                }
        }

	#ifdef DEBUG
	printf("* msgSend: Proposed time stamps: ");
	for(i = 0; i < numMcMemb[msgNum]; i++)
	{
		printf("%d ", propTS[i]);
	}
	#endif

	#ifdef DEBUG
	printf("\n* msgSend: Max Time Stamp: %d\n", propTS[numMcMemb[msgNum]-1]);
	#endif
	
	/* Use Semaphore to access the Lamport Logical Clock */
	sem_wait(&semTimeStamp);
	timeStamp = ++gTimeStamp;
	sem_post(&semTimeStamp);

	/* Sending FINAL Message */
	for(i = 0; i < numMcMemb[msgNum]; i++)
	{
		sendMsg(FINAL, mcMemb[msgNum][i], propTS[numMcMemb[msgNum]-1], timeStamp);
	}

	printf("* -------------------------------------------------------------------\n");
	return 0;
}

/* 
 * Sends messages to a given Destination (rcvid) 
 * Used by both msgSend and msgReceive
 */
int sendMsg(int type, int rcvId, int propTimeStamp, int timeStamp)
{
	int portN;
	int chkID;
	char* cPtr;
        char line[100];
	char ipAddr[20];
	FILE *fp = NULL;

	/* Open "ipconfig.dat" file to read the destination IP and Port Number */
	fp = fopen("ipconfig.dat", "r+");
	if(NULL == fp)  exit(-1);

	while(NULL != fgets(line, 50, fp))
	{
		sscanf(line, "%d", &chkID);

		if(chkID == rcvId)
		{
			cPtr = line;
			sscanf(cPtr, "%*d %d %s", &portN, ipAddr);
			#ifdef DEBUG
			printf("* sendMsg: Sending Msg to %d [%d, %s]\n", rcvId, portN, ipAddr);
			#endif
			break;
		}
	}

	strMsg msgBuf;
	int connSock, ret;
	struct sockaddr_in servaddr;
	struct sctp_event_subscribe events;

	/* Create an SCTP TCP-Style Socket */
	connSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );
	if(-1 == connSock)
	{
		perror("* Error creating Socket Object: ");
		exit(-1);
	}	

	/* Specify the peer endpoint to which we'll connect */
	bzero( (void *)&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(portN);
	servaddr.sin_addr.s_addr = inet_addr(ipAddr);

	/* Connect to the server */
	ret = connect( connSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
	if(-1 == ret)
	{
		usleep(1000);
		ret = connect( connSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
		if(-1 == ret)
		{
			printf("* sendMsg: Sending %d to %s\n", type, ipAddr);
			perror("* sendMsg: Connection to Server Failed: ");
			return -1;
		}
	}

	/* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
	memset( (void *)&events, 0, sizeof(events) );
	events.sctp_data_io_event = 1;
	setsockopt( connSock, SOL_SCTP, SCTP_EVENTS, (const void *)&events, sizeof(events) );

	/* Fill the message to be Sent */
	msgBuf.type = type;

	if(PROPOSED != type) msgBuf.msgId = (msgCount << 6) | usrPID;
	else msgBuf.msgId = 0;

	msgBuf.sendId = usrPID;
	msgBuf.rcvId = rcvId;
	msgBuf.propTimeStamp = propTimeStamp;
	msgBuf.timeStamp = timeStamp;

	printf("* msgSend: Sending %d, %d to %s\n", msgBuf.type, msgBuf.msgId, ipAddr);

	/* SCTP Send Call */
	ret = sctp_sendmsg( connSock, (void *)&msgBuf, (size_t)sizeof(strMsg), NULL, 0, 0, 0, 0, 0, 0 );
	if(-1 == ret)
	{
		printf("************* Error Sending %d to %s: ", type, ipAddr);
		perror("");
	}

	/* Close socket */
	close(connSock);
	return 0;
}


/* ------------------------ PRIORITY QUEUE IMPLEMENTATION START ------------------------ */
/* 
 * INSERT function - To insert into Priority Queue 
 */
strMsgQ* head = NULL;
int insert(strMsg argMsg)
{
        strMsgQ* curPtr = NULL;
        strMsgQ* prevPtr = NULL;
        strMsgQ* local = (strMsgQ*) malloc (sizeof(strMsgQ));

	local->type = argMsg.type;
	local->msgId = argMsg.msgId;
	local->rcvId = argMsg.rcvId;
	local->sendId = argMsg.sendId;
        local->timeStamp = argMsg.timeStamp;
	local->propTimeStamp = argMsg.propTimeStamp;

        if(NULL == head)
        {
                head = local;
                head->next = NULL;
		printf("* Inserted Head: %d\n", local->propTimeStamp);
        }
        else
        {
                if(local->propTimeStamp == head->propTimeStamp)
                {
                        if(local->sendId < head->sendId)
                        {
                                local->next = head;
                                head = local;
                                return 0;
                        }
                        else
                        {
                                prevPtr = head;
                                curPtr = head->next;
                                while(NULL != curPtr)
                                {
                                        if(local->propTimeStamp == curPtr->propTimeStamp && local->sendId > curPtr->sendId)
                                        {
                                                prevPtr = curPtr;
                                                curPtr = curPtr->next;
                                                continue;
                                        }
                                        else
                                        {
                                                prevPtr->next = local;
                                                local->next = curPtr;
                                                break;
                                        }
                                }
                                if(NULL == curPtr)
                                {
                                        prevPtr->next = local;
                                        local->next = NULL;
                                }
                                return 0;
                        }
                }

                if(local->propTimeStamp < head->propTimeStamp)
                {
                        local->next = head;
                        head = local;
                        return 0;
                }

                curPtr = head;
                while(NULL != curPtr->next)
                {
                        prevPtr = curPtr;
                        curPtr = curPtr->next;
                        if(local->propTimeStamp == curPtr->propTimeStamp)
                        {
                                if(local->sendId < curPtr->sendId)
                                {
                                        prevPtr->next = local;
                                        local->next = curPtr;
                                        return 0;
                                }
                                else
                                {
                                        while(NULL != curPtr)
                                        {
                                                if(local->propTimeStamp == curPtr->propTimeStamp && local->sendId > curPtr->sendId)
                                                {
                                                        prevPtr = curPtr;
                                                        curPtr = curPtr->next;
                                                        continue;
                                                }
                                                else
                                                {
                                                        prevPtr->next = local;
                                                        local->next = curPtr;
                                                        break;
                                                }
                                        }
                                        if(NULL == curPtr)
                                        {
                                                prevPtr->next = local;
                                                local->next = NULL;
                                        }
                                        return 0;
                                }
                        }
                        if(local->propTimeStamp < curPtr->propTimeStamp)
                        {
                                        prevPtr->next = local;
                                        local->next = curPtr;
                                        return 0;
                        }
                }
                if(NULL == curPtr->next)
                {
                        curPtr->next = local;
                        local->next = NULL;
                }
        }
        return 0;
}

/* 
 * DELETE function - To delete node from Priority Queue using Unique MsgId.
 */
int delete(int msgId)
{
        strMsgQ* curPtr = NULL;
        strMsgQ* prevPtr = NULL;
        curPtr = head;
        if(msgId == curPtr->msgId)
        {
                head = curPtr->next;
                free(curPtr);
                return 0;
        }

        prevPtr = curPtr;
        curPtr = curPtr->next;
        while(NULL != curPtr)
        {
                if(msgId == curPtr->msgId)
                {
                        prevPtr->next = curPtr->next;
                        free(curPtr);
                        break;
                }
                prevPtr = curPtr;
                curPtr = curPtr->next;
        }
        return 0;
}

/* 
 * GETMIN function - Returns the First Entry and removes it from Priority Queue if it is "Deliverable" 
 */
int getMin(strMsg *argMsg)
{
	if(recvMsgCount >= memSet) 
	{
		printf("* Max Reached: Received %d, Max %d\n", recvMsgCount, memSet);
		return -1;
	}
	if(NULL == head)
	{
		return -2;
	}
	/* If Type is INITIAL, then it is not deliverable, so wait */
	else if(INITIAL == head->type)
	{
		printf("* ---------------\n");
		printf("* INITIAL at HEAD\n");
		printf("* ---------------\n");
		return -2;
	}
        strMsgQ* curPtr = head;
        head = head->next;

	argMsg->type = curPtr->type;
	argMsg->msgId = curPtr->msgId;
	argMsg->rcvId = curPtr->rcvId;
	argMsg->sendId = curPtr->sendId;
        argMsg->timeStamp = curPtr->timeStamp;
	argMsg->propTimeStamp = curPtr->propTimeStamp;

        free(curPtr);
	recvMsgCount++;
        return 0;
}

/* 
 * DISPLAY function - Prints the current nodes in Priority Queue 
 */
void display()
{
        strMsgQ* curPtr = NULL;
        curPtr = head;
        printf("* The List is : ");
        while(NULL != curPtr)
        {
                printf("[%d %d %d], ", curPtr->propTimeStamp, curPtr->msgId, curPtr->type);
                curPtr = curPtr->next;
        }
        printf("\n");
}

/* 
 * REPLACE function - Replace a node if its proposed time stamp value changes or just change 
 * the type from INITIAL to FINAL.
 */
int replace(strMsg argMsg)
{
        strMsgQ* curPtr = NULL;
        curPtr = head;
        while(NULL != curPtr)
        {
		if(argMsg.msgId == curPtr->msgId)
		{
			if(argMsg.propTimeStamp == curPtr->propTimeStamp)
			{
				curPtr->type = argMsg.type;
				return 0;
			}
			else
			{
				delete(argMsg.msgId);
				insert(argMsg);
				return 0;
			}
		}
		curPtr = curPtr->next;
	}
	return 0;
}
/* ------------------------ PRIORITY QUEUE IMPLEMENTATION END ------------------------ */

/* 
 * Implements Receive thread
 * Handles the receive part of Skeen' Algorithm
 * Receives INITIAL, PROPOSED and FINAL Messages.
 * Owns the Socket binded to the IP and Port Number.
 */
void* msgReceive()
{
        int portN;
        int chkID;
        char* cPtr;
        int in, flags;
	strMsg msgBuf;
        char line[50];
        FILE *fp = NULL;
	int timeStamp = 0;
	int propTimeStamp;
	struct sockaddr_in servaddr;
	int listenSock, connSock, ret;
        struct sctp_sndrcvinfo sndrcvinfo;
        struct sctp_event_subscribe events;

	/* Reads IP and Port Num to bind it to the Socket */
        fp = fopen("ipconfig.dat", "r");
        if(NULL == fp)  exit(-1);

        while(NULL != fgets(line, 50, fp))
        {
                sscanf(line, "%d", &chkID);
                if(chkID == usrPID)
                {
                        cPtr = line;
                        sscanf(cPtr, "%*d %d", &portN);
			#ifdef DEBUG
                        printf("* msgReceive: My Port Number: %d\n", portN);
			#endif
                        break;
                }
        }
	fclose(fp);
	fp = NULL;

	/* Create SCTP TCP-Style Socket */
	listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );
	if(-1 == listenSock)
	{
		perror("* Error creating Socket Object: ");
		exit(-1);
	}	
	#ifdef DEBUG
	printf("* msgReceive: Socket FD: %d\n", listenSock);
	#endif

	/* Accept connections from any interface */
	bzero( (void *)&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	servaddr.sin_port = htons(portN);

	/* Bind to the wildcard address (all) and Port Number */
	ret = bind( listenSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
	if(-1 == ret)
	{
		perror("* msgReceive: Error Binding Socket: ");
		exit(-1);
	}
	#ifdef DEBUG
	printf("* msgReceive: Bind Return: %d\n", ret);
	#endif

	/* Place Socket into the listening state */
	listen( listenSock, 128 );

	while( 1 ) 
	{
		/* Await a new client connection */
		connSock = accept( listenSock, (struct sockaddr *)NULL, NULL );
		if(-1 == connSock)
		{
			perror("* msgReceive: Accepting Connection Failed: ");
			continue;
		}

		/* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
		memset( (void *)&events, 0, sizeof(events) );
		events.sctp_data_io_event = 1;
		setsockopt( connSock, SOL_SCTP, SCTP_EVENTS, (const void *)&events, sizeof(events) );
		flags = 0;
		
		memset(&msgBuf, 0, sizeof(strMsg));

		/* SCTP Receive call */
		in = sctp_recvmsg( connSock, (void *)&msgBuf, sizeof(strMsg), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );
		if(-1 == in)
		{
			perror("* Error Receiving: ");
			continue;
		}

		/* Access Lamport Logical Clock via Semaphore */
		sem_wait(&semTimeStamp);
		gTimeStamp = MAX(gTimeStamp, msgBuf.timeStamp) + 1;
		sem_post(&semTimeStamp);

		printf("* msgReceive: Receiving %d, %d from %d\n", msgBuf.type, msgBuf.propTimeStamp, msgBuf.sendId);

		/* Handles INITIAL Message */
		if(INITIAL == msgBuf.type)
		{
			sem_wait(&semTimeStamp);
			propTimeStamp = gTimeStamp;
			++gTimeStamp;
			timeStamp = gTimeStamp;
			sem_post(&semTimeStamp);

			#ifdef DEBUG
			printf("* msgReceive INITIAL: Proposing Time Stamp: %d\n", propTimeStamp);	
			#endif

			msgBuf.propTimeStamp = propTimeStamp;

			sem_wait(&semQueue);
			insert(msgBuf);
			sem_post(&semQueue);

			sendMsg(PROPOSED, msgBuf.sendId, propTimeStamp, timeStamp);
		}
		/* Handles PROPOSED Message */
		else if(PROPOSED == msgBuf.type)
		{
			#ifdef DEBUG
			printf("* msgReceive PROPOSED: Recevied Proposed Time Stamp: %d\n", msgBuf.propTimeStamp);	
			#endif
			sem_wait(&semSharedMem1);
			gMsgShare.propTimeStamp = msgBuf.propTimeStamp;
			sem_post(&semSharedMem);
		}
		/* Handles FINAL Message */
		else if(FINAL == msgBuf.type)
		{
			sem_wait(&semQueue);
			#ifdef DEBUG
			printf("* msgReceive FINAL: Recevied Final Proposed TS: %d\n", msgBuf.propTimeStamp);	
			#endif

			replace(msgBuf);

			sem_post(&semQueue);
			printf("* ---------------------------------------\n");
		}
		#ifdef DEBUG
		printf("* -------------------------------------------------------------------\n");
		#endif

		/* Close the connection */
		close( connSock );
	}
	return NULL;
}

/* 
 * Skeen Module Main Function 
 * Does Send part and creates a Receive Thread to handle the receive part of Skeen's Algorithm
 */
int main()
{
        int i, j;
	int chkID;
	char* cPtr;
        char* sPtr;
        char* ePtr;
        char pidStr[5];
        char mcGrp[50];
        char line[100];
	FILE *fp = NULL;
        char hostName[30];

	/* Semaphore Initialization */
        sem_init(&semTimeStamp, 0, 1);
        sem_init(&semSharedMem, 0, 0);
        sem_init(&semSharedMem1, 0, 1);
        sem_init(&semQueue, 0, 1);

	/* Get Process ID Ex. Net01 -> 01, Net02 -> 02. */
        if(-1 == gethostname(hostName, sizeof(hostName)))
        {
                printf("* Enter Process ID\n");
                scanf("%d", &usrPID);
        }
	else
	{
		#ifdef DEBUG
		printf("* Host: %s\n", hostName);
		#endif
		cPtr = hostName;
		cPtr = cPtr+3;
		sscanf(cPtr, "%2d", &usrPID);
	}
	#ifdef DEBUG
        printf("* Process Id: %d\n", usrPID);
	#endif

        sprintf(pidStr, "%03d", usrPID);
	#ifdef DEBUG
        printf("* Pid String is %s\n", pidStr);	
	#endif

	/* Creates the Send Log File */
	sprintf(sendFile, "SendOrder_%d", usrPID);
        fpSend = fopen( sendFile, "w+");
        if(NULL == fpSend)  
	{
		perror("* Error opening Send File: ");
		exit(-1);
	}
	
	/* Read Input Information from "msgconfig.dat" */
        fp = fopen("msgconfig.dat", "r");
        if(NULL == fp)  exit(-1);

        i = 0;
        j = 0;
        while(NULL != fgets(line, 100, fp))
        {
                sscanf(line, "%*d %d", &chkID);

                if(chkID == usrPID)
                {
                        cPtr = line;
                        sscanf(cPtr, "%*d %d %s", &procID, mcGrp);
			#ifdef DEBUG
                        printf("* Multicast Grp: %s\n", mcGrp);
			#endif
                        cPtr = mcGrp;
                        cPtr++;
			#ifdef DEBUG
                        printf("* Multicast Mem: ");
			#endif
                        while(1)
                        {
                                sscanf(cPtr, "%d", &mcMemb[i][j]);
                                sscanf(cPtr, "%*d%s", cPtr);
				#ifdef DEBUG
                                printf("%d ", mcMemb[i][j]);
				#endif
				numMcMemb[i]++;
                                if(!strncmp(">", cPtr, 1)) break;
                                cPtr++;
                                j++;
                        }
			#ifdef DEBUG
                        printf("\n");
			#endif

                        cPtr = strchr(line, '<');
                        cPtr++;
                        cPtr = strchr(cPtr, '<');
                        if(NULL != strchr(cPtr, '<'))
                        {
                                sPtr = strchr(cPtr, '<');
                                sPtr++;
                                if(NULL != strchr(cPtr, '>'))
                                {
					sPtr = strchr(cPtr, '<');
					sPtr++;
					if(NULL != strchr(cPtr, '>'))
					{
						ePtr = strchr(cPtr, '>');
						memcpy(msgData[i], ++cPtr, ePtr-sPtr);
						msgData[i][ePtr-sPtr] = '\0';
						#ifdef DEBUG
						printf("* %d: Message: %s\n", numMcMsg, msgData[i]);
						#endif
					}
				}
                        }
			numMcMsg++;
                        i++;
                        j = 0;
			#ifdef DEBUG
                        printf("* --------------------------------------------- \n");
			#endif
			if(numMcMsg == MAX_MULTICAST_MSGS)
			{
				#ifdef DEBUG
				printf("* Max multicast message limit reached: %d\n", numMcMsg);
				#endif
				break;
			}
                }
                else
                {
                        if(strstr(line, pidStr) != NULL)
                        {
                                memSet++;
                        }
                }
	}
	fclose(fp); 
	fp = NULL;

	/* If no messages to Send or Receive Exit */
	if(0 == numMcMsg && 0 == memSet) 
	{
		printf("* No Messages to Send or Receive\n");
		exit(0);
	}

	/* Create Receive Thread */
	pthread_t receiveThread;
	pthread_create(&receiveThread, NULL, msgReceive, NULL);

	int ret;
        int status = 0;
	strMsg msgLocal;
        strPipeMsg pipeMsg;

        printf("* Skeen Algorithm Running\n");
        while(1)
        {
		memset(&pipeMsg, 0, sizeof(strPipeMsg));

		/* Read from Pipe to receive command */
                ret = read(READ_FD, &pipeMsg, sizeof(strPipeMsg));
		if(-1 == ret)
		{
			perror("* Error Reading From Pipe: ");
			continue;
		}
                if(SEND == pipeMsg.type)
                {
			#ifdef DEBUG
                        printf("* Skeen: Sending Msg: %d\n", pipeMsg.sendMsgNum);
			#endif
			if (pipeMsg.sendMsgNum < numMcMsg) 
			{
				msgSend(pipeMsg.sendMsgNum);
				status = SUCCESS;
				ret = write(WRITE_FD, &status, sizeof(int));
				if(-1 == ret)
				{
					perror("* Error Writing to Pipe: ");
					continue;
				}
			}
			else
			{
				/* Reached Send Limit, Cant send anymore */
				status = MAX_SEND;
				ret = write(WRITE_FD, &status, sizeof(int));	
				if(-1 == ret)
				{
					perror("* Error Writing to Pipe: ");
					continue;
				}
			}
                }
                else if(RECEIVE == pipeMsg.type)
                {
			sem_wait(&semQueue);
			ret = getMin(&msgLocal); 
			sem_post(&semQueue);
			if(-1 == ret)	
			{
				/* Reached Receive Limit, Cant Receive Anymore */
				printf("* Skeen: Received All Messages\n");
				memset(&pipeMsg, 0, sizeof(strPipeMsg));
				pipeMsg.type = MAX_RECEIVE;
				ret = write(WRITE_FD, &pipeMsg, sizeof(strPipeMsg));
				if(-1 == ret)
				{
					perror("* Error Writing to Pipe: ");
					continue;
				}
			}
			else if(-2 == ret)
			{
				/* No Deliverable Messages Now, so Retry Later */
				while(1)
				{
					sleep(RECEIVE_RETRY_TIME);
					sem_wait(&semQueue);
					display();
					ret = getMin(&msgLocal); 
					sem_post(&semQueue);
					if(-1 != ret && -2 != ret) break;
				}
			}
			if(-1 != ret && -2 != ret)
			{
				#ifdef DEBUG
				printf("* Skeen: Receiving %d Msg in Queue\n", recvMsgCount);
				#endif
				pipeMsg.type = SUCCESS;
				memcpy(&(pipeMsg.recvMsg), &msgLocal, sizeof(strMsg));
				ret = write(WRITE_FD, &pipeMsg, sizeof(strPipeMsg));
				if(-1 == ret)
				{
					perror("* Error Writing to Pipe: ");
					continue;
				}
			}
                }
		else  if(EXIT == pipeMsg.type)
		{
			printf("* EXIT message Received\n");
			break;
		}
        }
        printf("* Skeen: Exiting\n");

	fclose(fpSend);
	return 0;
}
