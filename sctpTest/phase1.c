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

#define LOCALTIME_STREAM        0
#define GMT_STREAM              1

#define MAX_MULTICAST_MSGS	20
#define MAX_MULTICAST_MEMBERS	10

#define MAX_BUFFER      	1024
#define MY_PORT_NUM     	6000

#define MAX(a,b) (((a)>(b))?(a):(b))

sem_t semTimeStamp;

int usrPID;
int procID;
int numMcMsg;
int gTimeStamp = 0;
int numMcMemb[MAX_MULTICAST_MSGS];
int mcMemb[MAX_MULTICAST_MSGS][50];
char msgData[MAX_MULTICAST_MSGS][50];

typedef struct
{
	int rcvId;
	int sendId;
	int timeStamp;
}strMsg;

int m_send(int msgNum)
{
	int portN;
	char ipAddr[50];
	int chkID;
	char* cPtr;
	int i;
        char line[100];
	FILE *fp = NULL;

	if(msgNum > numMcMsg-1) return 0;

	++gTimeStamp;
	printf("* Number of Multicast Members: %d\n", numMcMemb[msgNum]);
	for(i = 0; i < numMcMemb[msgNum]; i++)
	{
		fp = fopen("ipconfig.dat", "r+");
		if(NULL == fp)  exit(-1);

		while(NULL != fgets(line, 50, fp))
		{
			sscanf(line, "%d", &chkID);

			if(chkID == mcMemb[msgNum][i])
			{
				cPtr = line;
				sscanf(cPtr, "%d %d %s", &procID, &portN, ipAddr);
				printf("* Process ID : %d, PortN: %d, IP Address: %s\n", procID, portN, ipAddr);
				break;
			}
		}

		strMsg msgBuf;
		int connSock, ret, in, flags;
		time_t currentTime;
		struct sockaddr_in servaddr;
		struct sctp_sndrcvinfo sndrcvinfo;
		struct sctp_event_subscribe events;
		char buffer[MAX_BUFFER+1];

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
                        perror("* Client: Connection to Server Failed: ");
                }

                /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
                memset( (void *)&events, 0, sizeof(events) );
                events.sctp_data_io_event = 1;
                setsockopt( connSock, SOL_SCTP, SCTP_EVENTS, (const void *)&events, sizeof(events) );

                /* New client socket has connected */

                /* Grab the current time */
                currentTime = time(NULL);

	//	if(0 == i%2)
		if(1)
                {
                        /* Send local time on stream 0 (local time stream) */
                        snprintf( buffer, MAX_BUFFER, "%s\n", ctime(&currentTime) );
                        printf("* Client: Buffer: %s\n", buffer);

			msgBuf.sendId = usrPID;
			msgBuf.rcvId = mcMemb[msgNum][i];

			sem_wait(&semTimeStamp);
			msgBuf.timeStamp = gTimeStamp;
			sem_post(&semTimeStamp);

		//	ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );
			ret = sctp_sendmsg( connSock, (void *)&msgBuf, (size_t)sizeof(strMsg), NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );
                }
                else 
                {
                        /* Send GMT on stream 1 (GMT stream) */
                        snprintf( buffer, MAX_BUFFER, "%s\n", asctime( gmtime( &currentTime ) ) );
                        printf("* Client: Buffer: %s\n", buffer);

                        ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, GMT_STREAM, 0, 0 );
                }

                /* Close our socket and exit */
                close(connSock);
        }
}

int m_receive()
{
	strMsg msgBuf;
	time_t currentTime;
	char buffer[MAX_BUFFER+1];
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

        fp = fopen("ipconfig.dat", "r");
        if(NULL == fp)  exit(-1);

        while(NULL != fgets(line, 50, fp))
        {
                sscanf(line, "%d", &chkID);
                if(chkID == usrPID)
                {
                        cPtr = line;
                        sscanf(cPtr, "%*d %d", &portN);
                        printf("* My Port Number: %d\n", portN);
                        break;
                }
        }
	fclose(fp);

	/* Create SCTP TCP-Style Socket */
	listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );
	printf("* Server: Socket FD: %d\n", listenSock);

	/* Accept connections from any interface */
	bzero( (void *)&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	servaddr.sin_port = htons(portN);

	/* Bind to the wildcard address (all) and MY_PORT_NUM */
	ret = bind( listenSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
	if(-1 == ret)
	{
		perror("* Error Binding Socket: ");
	}
	printf("* Server: Bind Return: %d\n", ret);

	/* Place the server socket into the listening state */
	listen( listenSock, 5 );

	/* Server loop... */
	while( 1 ) 
	{

		/* Await a new client connection */
		connSock = accept( listenSock, (struct sockaddr *)NULL, (int *)NULL );
		printf("* Server: Connection Accepted: %d\n", connSock);

		/* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
		memset( (void *)&events, 0, sizeof(events) );
		events.sctp_data_io_event = 1;
		setsockopt( connSock, SOL_SCTP, SCTP_EVENTS, (const void *)&events, sizeof(events) );

	//	in = sctp_recvmsg( connSock, (void *)buffer, sizeof(buffer), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );
		in = sctp_recvmsg( connSock, (void *)&msgBuf, sizeof(strMsg), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );
	
		sem_wait(&semTimeStamp);
		gTimeStamp = MAX(gTimeStamp, msgBuf.timeStamp) + 1;
		printf("* ----------------------------------------------------------------- \n");
		printf("* <%d> Sender: %d, Rcv: %d, TimeStamp: %d\n", gTimeStamp, msgBuf.sendId, msgBuf.rcvId, msgBuf.timeStamp);
		printf("* ----------------------------------------------------------------- \n");
		sem_post(&semTimeStamp);

		#if 0
		/* Null terminate the incoming string */
		buffer[in] = 0;

		if        (sndrcvinfo.sinfo_stream == LOCALTIME_STREAM) {
		printf("(Local) %s\n", buffer);
		} else if (sndrcvinfo.sinfo_stream == GMT_STREAM) {
		printf("(GMT  ) %s\n", buffer);
		}
		#endif

	}
	
	#if 0
        snprintf( buffer, MAX_BUFFER, "%s\n", "Received");
        ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );
        printf("* Send Status: %d\n", ret);
        sleep(1);
	#endif

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

	printf("* Press Enter to Send\n");
	scanf("%d", &i);
	m_send(0);
	return 0;
}
