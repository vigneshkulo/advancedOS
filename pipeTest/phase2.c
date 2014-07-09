#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

#define	SUCCESS		1
#define	FAILURE		-1
#define	MAX_SEND	-2
#define	MAX_RECEIVE	-3

#define SEND            1
#define RECEIVE         2

int usrPID;
FILE *fpRecv = NULL;
char sendFile[20];
char recvFile[20];
int sendMsgCount = 0;
int recvMsgCount = 0;
int skeenR, skeenW, parentR, parentW;

typedef struct
{
        int type;
        int msgId;
        int rcvId;
        int sendId;
        int timeStamp;
        int propTimeStamp;
}strMsg;

typedef struct
{
	int type;
	int sendMsgNum;
	strMsg recvMsg;
} strPipeMsg;

int m_send()
{
	int status = 0;
	strPipeMsg pipeMsg;	


	printf("* m_send: Sending Message: %d\n", sendMsgCount);
	pipeMsg.type = SEND;
	pipeMsg.sendMsgNum = sendMsgCount;

	write (parentW, &pipeMsg, sizeof(strPipeMsg));
	sendMsgCount++;
	read  (parentR, &status, sizeof(int));
	return status;
}

int m_receive()
{
        char str[10];
	strPipeMsg pipeMsg;	

	pipeMsg.type = RECEIVE;
	pipeMsg.sendMsgNum = -1;

	write (parentW, &pipeMsg, sizeof(strPipeMsg));
	read  (parentR, &pipeMsg, sizeof(strPipeMsg));
	printf("* m_receive: Received Message: %d\n", pipeMsg.recvMsg.msgId);
	if(MAX_RECEIVE == pipeMsg.recvMsg.type) return MAX_RECEIVE; 

	sprintf(str, "%05d ", pipeMsg.recvMsg.msgId);
	fwrite(str , 1 , strlen(str) , fpRecv );
	recvMsgCount++;

	if((recvMsgCount % 10) == 0) fwrite("\n" , 1, 1, fpRecv );

	return 0;
}

int main()
{
	int choice;
        int skeenPid = 0;
        int fds1[2], fds2[2];

        pipe (fds1);
        pipe (fds2);

        skeenR = fds1[0];
        skeenW = fds2[1];
        parentR = fds2[0];
        parentW = fds1[1];

        skeenPid = fork();
	if(-1 == skeenPid)
	{
		perror("* Application: Fork Failed: ");
		exit(-1);
	}
        if (skeenPid > 0)
        {
		printf("* Application: Skeen Module created\n");
//		while(1)
		{
			close(skeenR);
			close(skeenW);

			char* cPtr;
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

			sprintf(recvFile , "RecvOrder_%d", usrPID);

			fpRecv = fopen( recvFile, "w+");
			if(NULL == fpRecv)
			{
				perror("* Error opening Recv File: ");
				exit(-1);
			}

			int random_number;
			srand ( time(NULL) );
			while(1)
			{
				random_number = rand() % 5 + 1;
				printf(" %d\n", random_number);

				if(MAX_SEND == m_send())
				{
					printf("* No More Messages to Send\n");
					break;
				}
				sleep(random_number);
			}

			while(1)
			{
				if(MAX_RECEIVE == m_receive())
				{
					printf("* No More Messages to Receive\n");
					break;
				}
			}
		}
	}
	else
	{
		close( parentR);
		close( parentW);

		/* Assigning Read to 66 and Write to 99 */
		dup2(skeenR, 66);
		dup2(skeenW, 99);

		execv("skeen", NULL);
		perror("* Exec Failed: ");
	}
	return 0;	
}
