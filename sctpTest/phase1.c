#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include<math.h>
#include<semaphore.h>
#include<sys/syscall.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>

#define INITIAL			0
#define PROPOSED		1
#define FINAL			2

#define MAX_MULTICAST_MSGS	20
#define MAX_MULTICAST_MEMBERS	10

#define MAX(a,b) (((a)>(b))?(a):(b))

sem_t semTimeStamp;
sem_t semSharedMem;

int usrPID;
int procID;
int numMcMsg;
int msgCount = 0;
int gTimeStamp = 0;
char sendFile[20];
char recvFile[20];
FILE *fpSend = NULL;
FILE *fpRecv = NULL;
int numMcMemb[MAX_MULTICAST_MSGS];
int mcMemb[MAX_MULTICAST_MSGS][50];
char msgData[MAX_MULTICAST_MSGS][50];

typedef struct
{
	int type;
	int msgId;
	int rcvId;
	int sendId;
	int timeStamp;
	int propTimeStamp;
}strMsg;

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

int m_send(int msgNum)
{
	int i;
	char str[10];
	int timeStamp;
	int propTS[MAX_MULTICAST_MEMBERS];
	if(msgNum > numMcMsg-1) return 0;

//	printf("* Number of Multicast Members: %d\n", numMcMemb[msgNum]);

//	sem_getvalue(&semTimeStamp, &semVal);
//	printf("* Send waiting for Semaphore: %d\n", semVal);
	sem_wait(&semTimeStamp);
	++gTimeStamp;
	timeStamp = gTimeStamp;
	sem_post(&semTimeStamp);
	
	msgCount++;

	sprintf(str, "%d ", (msgCount << 6) | usrPID);
	fwrite(str , 1 , strlen(str) , fpSend );

	for(i = 0; i < numMcMemb[msgNum]; i++)
	{
		sendMsg(INITIAL, mcMemb[msgNum][i], -1, timeStamp);
	}
	printf("* -------------------------------------------------------------------\n");

	for(i = 0; i < numMcMemb[msgNum]; i++)
	{
		sem_wait(&semSharedMem);
		propTS[i] = gMsgShare.propTimeStamp;
	//	printf("* m_send: Received Prop Time Stamp: %d\n", propTS[i]);
	}

	/* Bubble Sort */
        int j, swap = 0;
	for(i = 0; i < numMcMemb[msgNum] - 1; i++)
        {
                if (propTS[i] > propTS[i+1]) 
                {
                        swap       = propTS[i];
                        propTS[i]   = propTS[i+1];
                        propTS[i+1] = swap;
                }
        }
	printf("* msg_send: Proposed time stamps: ");
	for(i = 0; i < numMcMemb[msgNum]; i++)
		printf("%d ", propTS[i]);

	printf("\n* msg_send: Max Time Stamp: %d\n", propTS[numMcMemb[msgNum]-1]);
	
	sem_wait(&semTimeStamp);
	timeStamp = ++gTimeStamp;
	sem_post(&semTimeStamp);

	for(i = 0; i < numMcMemb[msgNum]; i++)
	{
		sendMsg(FINAL, mcMemb[msgNum][i], propTS[numMcMemb[msgNum]-1], timeStamp);
	}

	return 0;
}

int sendMsg(int type, int rcvId, int propTimeStamp, int timeStamp)
{
	int portN;
	int chkID;
	char* cPtr;
        char line[100];
	char ipAddr[20];
	FILE *fp = NULL;

	fp = fopen("ipconfig.dat", "r+");
	if(NULL == fp)  exit(-1);

	while(NULL != fgets(line, 50, fp))
	{
		sscanf(line, "%d", &chkID);

		if(chkID == rcvId)
		{
			cPtr = line;
			sscanf(cPtr, "%*d %d %s", &portN, ipAddr);
			printf("* sendMsg: Sending Msg to %d [%d, %s]\n", rcvId, portN, ipAddr);
			break;
		}
	}

	int semVal;
	strMsg msgBuf;
	int connSock, ret, in, flags;
	time_t currentTime;
	struct sockaddr_in servaddr;
	struct sctp_sndrcvinfo sndrcvinfo;
	struct sctp_event_subscribe events;

	/* Create an SCTP TCP-Style Socket */
	connSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

	/* Specify the peer endpoint to which we'll connect */
	bzero( (void *)&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(portN);
	servaddr.sin_addr.s_addr = inet_addr(ipAddr);

	/* Connect to the server */
	ret = connect( connSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
	if(-1 == ret)
	{
		perror("* sendMsg: Connection to Server Failed: ");
	}

	/* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
	memset( (void *)&events, 0, sizeof(events) );
	events.sctp_data_io_event = 1;
	setsockopt( connSock, SOL_SCTP, SCTP_EVENTS, (const void *)&events, sizeof(events) );

	/* New client socket has connected */

	msgBuf.type = type;
	msgBuf.msgId = (msgCount << 6) | usrPID;
	msgBuf.sendId = usrPID;
	msgBuf.rcvId = rcvId;
	msgBuf.propTimeStamp = propTimeStamp;
	msgBuf.timeStamp = timeStamp;

	ret = sctp_sendmsg( connSock, (void *)&msgBuf, (size_t)sizeof(strMsg), NULL, 0, 0, 0, 0, 0, 0 );

	/* Close our socket and exit */
	close(connSock);
}

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

int getMin(strMsg *argMsg)
{
	if(NULL == head) return -1;
        strMsgQ* curPtr = head;
        head = head->next;

	argMsg->type = curPtr->type;
	argMsg->msgId = curPtr->msgId;
	argMsg->rcvId = curPtr->rcvId;
	argMsg->sendId = curPtr->sendId;
        argMsg->timeStamp = curPtr->timeStamp;
	argMsg->propTimeStamp = curPtr->propTimeStamp;

        free(curPtr);
        return 0;
}


void display()
{
        strMsgQ* curPtr = NULL;
        curPtr = head;
        printf("* The List is : ");
        while(NULL != curPtr)
        {
                printf("[%d %d %s], ", curPtr->msgId, curPtr->propTimeStamp, (curPtr->type == FINAL) ? "FINAL" : "INITIAL");
                curPtr = curPtr->next;
        }
        printf("\n");
}

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
				printf("****** No Change ******\n");
				return 0;
			}
			else
			{
				delete(argMsg.msgId);
				insert(argMsg);
				printf("****** Changed ******\n");
				return 0;
			}
		}
		curPtr = curPtr->next;
	}
}

int m_receive()
{
	strMsg msgBuf;
	char ipAddr[20];
	time_t currentTime;
	struct sockaddr_in servaddr;
	int listenSock, connSock, ret;

        int in, i, flags;
        struct sctp_sndrcvinfo sndrcvinfo;
        struct sctp_event_subscribe events;

        int portN;
        int chkID;
        char* cPtr;
        char line[50];
        FILE *fp = NULL;
	int propTimeStamp;

        fp = fopen("ipconfig.dat", "r");
        if(NULL == fp)  exit(-1);

        while(NULL != fgets(line, 50, fp))
        {
                sscanf(line, "%d", &chkID);
                if(chkID == usrPID)
                {
                        cPtr = line;
                        sscanf(cPtr, "%*d %d", &portN);
                        printf("* m_receive: My Port Number: %d\n", portN);
                        break;
                }
        }
	fclose(fp);
	fp = NULL;

	/* Create SCTP TCP-Style Socket */
	listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );
	printf("* m_receive: Socket FD: %d\n", listenSock);

	/* Accept connections from any interface */
	bzero( (void *)&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	servaddr.sin_port = htons(portN);

	/* Bind to the wildcard address (all) and MY_PORT_NUM */
	ret = bind( listenSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
	if(-1 == ret)
	{
		perror("* m_receive: Error Binding Socket: ");
	}
	printf("* m_receive: Bind Return: %d\n", ret);

	/* Place the server socket into the listening state */
	listen( listenSock, 5 );

	/* Server loop... */
	while( 1 ) 
	{
		/* Await a new client connection */
		connSock = accept( listenSock, (struct sockaddr *)NULL, (int *)NULL );
		if(-1 == connSock)
		{
			perror("* m_receive: Accepting Connection Failed: ");
		}
	//	printf("* m_receive: Connection Accepted: %d\n", connSock);

		/* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
		memset( (void *)&events, 0, sizeof(events) );
		events.sctp_data_io_event = 1;
		setsockopt( connSock, SOL_SCTP, SCTP_EVENTS, (const void *)&events, sizeof(events) );

		in = sctp_recvmsg( connSock, (void *)&msgBuf, sizeof(strMsg), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );
	
		sem_wait(&semTimeStamp);
		gTimeStamp = MAX(gTimeStamp, msgBuf.timeStamp) + 1;
		printf("* m_receive: Receiving from %d\n", msgBuf.sendId);
		sem_post(&semTimeStamp);

		if(INITIAL == msgBuf.type)
		{
			propTimeStamp = gTimeStamp;
			++gTimeStamp;
			printf("* m_receive INITIAL: Proposing Time Stamp: %d\n", propTimeStamp);	
			msgBuf.propTimeStamp = propTimeStamp;
			insert(msgBuf);
			sendMsg(PROPOSED, msgBuf.sendId, propTimeStamp, gTimeStamp);
		}
		else if(PROPOSED == msgBuf.type)
		{
			printf("* m_receive PROPOSED: Recevied Proposed Time Stamp: %d\n", msgBuf.propTimeStamp);	
			gMsgShare.propTimeStamp = msgBuf.propTimeStamp;
			sem_post(&semSharedMem);
		}
		else if(FINAL == msgBuf.type)
		{
			display();
			printf("* m_receive FINAL: Recevied Final Proposed TS: %d\n", msgBuf.propTimeStamp);	
			replace(msgBuf);
			display();
		}
		printf("* -------------------------------------------------------------------\n");
	}
	
	/* Close the client connection */
	close( connSock );

	return 0;
}
int main()
{
        int i, j;
	int chkID;
	char* cPtr;
        char* sPtr;
        char* ePtr;
        char mcGrp[50];
        char line[100];
	FILE *fp = NULL;
        char hostName[30];

        sem_init(&semTimeStamp, 0, 1);
        sem_init(&semSharedMem, 0, 0);

        if(-1 == gethostname(hostName, sizeof(hostName)))
        {
                printf("* Enter Process ID\n");
                scanf("%d", &usrPID);
        }
	else
	{
		printf("* Host: %s\n", hostName);
		cPtr = hostName;
		cPtr = cPtr+3;
		sscanf(cPtr, "%2d", &usrPID);
	}
        printf("* Process Id: %d\n", usrPID);
	
	sprintf(sendFile, "SendOrder_%d", usrPID);
	sprintf(recvFile , "RecvOrder_%d", usrPID);

        fpSend = fopen( sendFile, "w+");
        if(NULL == fpSend)  
	{
		perror("* Error opening Send File: ");
		exit(-1);
	}
	
        fpRecv = fopen( recvFile, "w+");
        if(NULL == fpRecv)  
	{
		perror("* Error opening Recv File: ");
		exit(-1);
	}

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
                        printf("* Multicast Grp: %s\n", mcGrp);
                        cPtr = mcGrp;
                        cPtr++;
                        printf("* Multicast Mem: ");
                        while(1)
                        {
                                sscanf(cPtr, "%d", &mcMemb[i][j]);
                                sscanf(cPtr, "%*d%s", cPtr);
                                printf("%d ", mcMemb[i][j]);
				numMcMemb[i]++;
                                if(!strncmp(">", cPtr, 1)) break;
                                cPtr++;
                                j++;
                        }
                        printf("\n");

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
					//      printf("* Bytes: [%x - %x] = %d\n", *ePtr, *sPtr, ePtr-sPtr);
						memcpy(msgData[i], ++cPtr, ePtr-sPtr);
						msgData[i][ePtr-sPtr] = '\0';
						printf("* Message: %s\n", msgData[i]);
					}
				}
                        }
			numMcMsg++;
                        i++;
                        j = 0;
                        printf("* --------------------------------------------- \n");
                }
	}
	fclose(fp); 
	fp = NULL;

	pthread_t receiveThread;
	pthread_create(&receiveThread, NULL, m_receive, NULL);

	printf("* Enter a number to Start\n");
	scanf("%d", &i);

	int random_number;
	srand ( time(NULL) );
	for(i = 0; i < numMcMsg; i++)
	{
		random_number = rand() % 5 + 1;
		printf(" %d\n", random_number);

		m_send(i);

		sleep(random_number);
	}
	
	printf("* Enter a number to Exit\n");
	scanf("%d", &i);
	
	char str[10];
	strMsg msgLocal;

	while(1)
	{
		if(-1 == getMin(&msgLocal)) break;

		sprintf(str, "%d ", msgLocal.msgId);
		fwrite(str , 1 , strlen(str) , fpRecv );
	}
	return 0;
}
