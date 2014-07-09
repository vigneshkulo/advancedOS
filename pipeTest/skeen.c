#include<stdio.h>
#include<stdlib.h>

#define READ_FD		66
#define WRITE_FD	99

#define SEND		1
#define RECEIVE		2

#define SUCCESS		1	

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

int main()
{
	int status = 0;
	strPipeMsg pipeMsg;

	printf("* Skeen Algorithm Running\n");
	while(1)
	{
		read(READ_FD, &pipeMsg, sizeof(strPipeMsg));
		if(SEND == pipeMsg.type)
		{
			printf("* Skeen: Sending Msg: %d\n", pipeMsg.sendMsgNum);
			status = SUCCESS;
			write(WRITE_FD, &status, sizeof(int));
		}
		else if(RECEIVE == pipeMsg.type)
		{
			printf("* Skeen: Receiving first Msg in Queue\n");
			status = SUCCESS;
			pipeMsg.recvMsg.msgId = 1;
			write(WRITE_FD, &pipeMsg, sizeof(strPipeMsg));
		}
	}
	printf("* Skeen: Exiting\n");
	return 0;
}
