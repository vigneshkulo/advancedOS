#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	int portN;
	int usrPID;
	int procID;
	char* cPtr;
	char* sPtr;
	char* ePtr;
	char line[100];
	int chkID;
	int i, j, numMcMsg;
	int memID[50][50];
	FILE *fp = NULL;
	char hostName[30];
	int hostNameLen;
	char ipAddr[50];
	char mcGrp[50];
	char msgData[20][50];
	int numMcMemb[20];
	
//	if(-1 == gethostname(hostName, sizeof(hostName)))
	if(1)
	{
		printf("* Enter Process ID\n");
		scanf("%d", &usrPID);
	}
	else
	{
		printf("* Host: %s\n", hostName);
		cPtr = hostName;
		cPtr = cPtr+3;
		printf("* Host: %s\n", cPtr);
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
				sscanf(cPtr, "%d", &memID[i][j]);
				sscanf(cPtr, "%*d%s", cPtr);
				printf("%d ", memID[i][j]);
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
					ePtr = strchr(cPtr, '>');
				//	printf("* Bytes: [%x - %x] = %d\n", *ePtr, *sPtr, ePtr-sPtr);
					memcpy(msgData[i], ++cPtr, ePtr-sPtr);
					msgData[i][ePtr-sPtr] = '\0';
					printf("* Message %d: %s\n", numMcMsg, msgData[i]);
				}
			}
			numMcMsg++;
			i++;
			j = 0;
			printf("* --------------------------------------------- \n");
		}
	}
	
	#if 1
	char str[7];
        int node[10];
        FILE* fpSend;
        FILE* fpRecv;
        int num, k, l;
        int recvMsgNum = 0;
        int sendMsgNum = 0;
        char sendFile[20];
        char recvFile[20];
        int* sendNodeArr = NULL;
        int* recvNodeArr = NULL;

//        printf("* Enter number of Receive Files\n");
//        scanf("%d", &num);

        printf("* Send Order\n");
        for(i = 1; i <= 1; i++)
        {
                sprintf(sendFile , "SendOrder_%d", i);
                fpSend = fopen( sendFile, "r");
                if(NULL == fpSend)
                {
                        perror("* Error opening Send File: ");
                        exit(-1);
                }

                while(NULL != fgets(line, 70, fpSend))
                {
                        cPtr = line;
                        printf("* %d: ", strlen(line));
                        fflush(stdout);

                        for(j = 0; j < strlen(line)/6; j++)
                        {
                                sendMsgNum++;
                                sendNodeArr = (int*) realloc (sendNodeArr, sizeof(int) * sendMsgNum);
                                while(isspace(*cPtr)) cPtr++;
                                sscanf(cPtr, "%d", &sendNodeArr[sendMsgNum - 1]);
                                printf("%5d ", sendNodeArr[sendMsgNum - 1]);
                                fflush(stdout);
                                cPtr = strchr(cPtr, ' ');
                        }

                        memset(line, 0, sizeof(line));
                        printf("\n");
                }
                printf("\n");
        }

        printf("* Recv Order\n");
        for(i = 0; i < numMcMsg; i++)
	{
	for(l = 0; l < numMcMemb[i]; l++)
        {
		printf("* Current Recv: %d\n", memID[i][l]);
                sprintf(recvFile , "RecvOrder_%d", memID[i][l]);
                fpRecv = fopen( recvFile, "r");
                if(NULL == fpRecv)
                {
                        perror("* Error opening Recv File: ");
                        exit(-1);
                }

                #if 1
                while(NULL != fgets(line, 70, fpRecv))
                {
                        cPtr = line;
                        printf("* %d: ", strlen(line));
                        fflush(stdout);

                        for(j = 0; j < strlen(line)/6; j++)
                        {
                                recvMsgNum++;
                                recvNodeArr = (int*) realloc (recvNodeArr, sizeof(int) * recvMsgNum);
                                while(isspace(*cPtr)) cPtr++;
                                sscanf(cPtr, "%d", &recvNodeArr[recvMsgNum - 1]);
                                printf("%5d ", recvNodeArr[recvMsgNum - 1]);
                                fflush(stdout);
                                cPtr = strchr(cPtr, ' ');
                        }

                        memset(line, 0, sizeof(line));
                        printf("\n");
                }
                printf("\n");
                #endif
		
		for(k = 0; k < recvMsgNum; k++)
		{
		}
        }
	}
        #endif

	return 0;
}
