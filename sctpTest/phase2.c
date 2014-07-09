#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

/* Global Definitions */
#define	SUCCESS		1
#define	FAILURE		-1
#define	MAX_SEND	-2
#define	MAX_RECEIVE	-3

#define SEND            1
#define RECEIVE         2
#define EXIT		3

/* Global Variables */
int usrPID;
char sendFile[20];
char recvFile[20];
char tmstFile[20];
FILE *fpRecv = NULL;
FILE *fpTmSt = NULL;
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

/* Message Send Handle */
int m_send()
{
	int ret = 0;
	int status = 0;
	strPipeMsg pipeMsg;	

//	printf("* m_send: Sending Message: %d\n", sendMsgCount);
	pipeMsg.type = SEND;
	pipeMsg.sendMsgNum = sendMsgCount;

	ret = write (parentW, &pipeMsg, sizeof(strPipeMsg));
	if(-1 == ret || 0 == ret)
	{
		perror("* Error in Write: ");
		fflush(stdout);
		return -1;
	}

	sendMsgCount++;

	ret = read  (parentR, &status, sizeof(int));
	if(-1 == ret || 0 == ret)
	{
		perror("* Error in Read: ");
		fflush(stdout);
		return -1;
	}

	return status;
}

/* Message Receive Handle */
int m_receive()
{
	int ret = 0;
        char str[10];
	strPipeMsg pipeMsg;	

	pipeMsg.type = RECEIVE;
	pipeMsg.sendMsgNum = -1;

	ret = write (parentW, &pipeMsg, sizeof(strPipeMsg));
	if(-1 == ret || 0 == ret)
	{
		perror("* Error in Write: ");
		fflush(stdout);
		return -1;
	}

	memset(&pipeMsg, 0, sizeof(strPipeMsg));

	ret = read(parentR, &pipeMsg, sizeof(strPipeMsg));
	if(-1 == ret || 0 == ret)
	{
		perror("* Error in Read: ");
		fflush(stdout);
		return -1;
	}

	if(MAX_RECEIVE == pipeMsg.type) 
	{
		pipeMsg.type = EXIT;
		write (parentW, &pipeMsg, sizeof(strPipeMsg));
		return MAX_RECEIVE; 
	}


	sprintf(str, "%05d ", pipeMsg.recvMsg.msgId);
	fwrite(str , 1 , strlen(str) , fpRecv );
	sprintf(str, "%05d ", pipeMsg.recvMsg.propTimeStamp);
	fwrite(str , 1 , strlen(str) , fpTmSt );

	recvMsgCount++;
//	printf("* m_receive: Received Message: %d, %d\n", pipeMsg.recvMsg.msgId, recvMsgCount);

	if((recvMsgCount % 10) == 0) fwrite("\n" , 1, 1, fpRecv );
	if((recvMsgCount % 10) == 0) fwrite("\n" , 1, 1, fpTmSt );

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
			sprintf(tmstFile , "TmStOrder_%d", usrPID);

			fpRecv = fopen( recvFile, "w+");
			if(NULL == fpRecv)
			{
				perror("* Error opening Recv File: ");
				exit(-1);
			}

			fpTmSt = fopen( tmstFile, "w+");
			if(NULL == fpTmSt)
			{
				perror("* Error opening Recv File: ");
				exit(-1);
			}
			signal(SIGPIPE, SIG_IGN);
			
			sleep(20);
			int random_number;
			srand ( time(NULL) );
			while(1)
			{
				random_number = rand() % 5 + 1;

				sleep(random_number);
				if(MAX_SEND == m_send())
				{
					printf("* No More Messages to Send\n");
					break;
				}
			}

			while(1)
			{
				int ret;
				ret = m_receive();
				if(MAX_RECEIVE == ret)
				{
					printf("* No More Messages to Receive\n");
					break;
				}
				else if(FAILURE == ret)
				{
					if(-1 == kill(skeenPid, 0)) 
					{	
						printf("* Skeen is Dead\n");
						break;
					}
					printf("* Error Receiving\n");
					sleep(10);
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
		dup2(skeenR, 10);
		dup2(skeenW, 11);

		execv("skeen", NULL);
		perror("* Exec Failed: ");
	}
	return 0;	
}
