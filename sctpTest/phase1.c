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

int usrPID;
int procID;
int portN;
int numMcMsg;
int numMcMemb[MAX_MULTICAST_MSGS];
int mcMemb[MAX_MULTICAST_MSGS][50];
char msgData[MAX_MULTICAST_MSGS][50];
char ipAddr[MAX_MULTICAST_MEMBERS][50];

int m_send(int msgNum)
{
	int chkID;
	char* cPtr;
	int i;
        char line[100];
	FILE *fp = NULL;

	if(msgNum > numMcMsg-1) return 0;

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
	}
	#if 0
        int connSock, ret, in, i, flags;
        time_t currentTime;
        struct sockaddr_in servaddr;
        struct sctp_sndrcvinfo sndrcvinfo;
        struct sctp_event_subscribe events;
        char buffer[MAX_BUFFER+1];


        for(i = 0; i < 2; i++)
        {
                /* Create an SCTP TCP-Style Socket */
                connSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

                /* Specify the peer endpoint to which we'll connect */
                bzero( (void *)&servaddr, sizeof(servaddr) );
                servaddr.sin_family = AF_INET;
                if(0 == i) servaddr.sin_port = htons(5000);
                if(0 == i) servaddr.sin_addr.s_addr = inet_addr( "10.176.67.64" );

                if(1 == i) servaddr.sin_port = htons(6000);
                if(1 == i) servaddr.sin_addr.s_addr = inet_addr( "10.176.67.66" );

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

                #if 1
                /* New client socket has connected */

                /* Grab the current time */
                currentTime = time(NULL);

                if(0 == i)
                {
                        /* Send local time on stream 0 (local time stream) */
                        snprintf( buffer, MAX_BUFFER, "%s\n", ctime(&currentTime) );
                        printf("* Client: Buffer: %s\n", buffer);

                        ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );

                        /* Expect reply message from the peer */
                        memset( (void *)buffer, 0, sizeof(buffer) );
                        in = sctp_recvmsg( connSock, (void *)buffer, sizeof(buffer), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );
                        printf("* Client: Bytes: %d, Buffer: %s\n", in, buffer);
                }
                else if(1 == i)
                {
                        /* Send GMT on stream 1 (GMT stream) */
                        snprintf( buffer, MAX_BUFFER, "%s\n", asctime( gmtime( &currentTime ) ) );
                        printf("* Client: Buffer: %s\n", buffer);

                        ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, GMT_STREAM, 0, 0 );

                        /* Expect reply message from the peer */
                        memset( (void *)buffer, 0, sizeof(buffer) );
                        in = sctp_recvmsg( connSock, (void *)buffer, sizeof(buffer), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );
                        printf("* Client: Bytes: %d, Buffer: %s\n", in, buffer);
                }
                #endif

                /* Close our socket and exit */
                close(connSock);
        }
	#endif
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

        fp = fopen("msgconfig.dat", "r+");
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

	m_send(0);
	m_send(1);
	return 0;
}
