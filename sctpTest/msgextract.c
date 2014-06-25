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
	int i, j;
	int memID[50][50];
	FILE *fp = NULL;
	char hostName[30];
	int hostNameLen;
	char ipAddr[50];
	char mcGrp[50];
	char msgData[20][50];
	
	if(-1 == gethostname(hostName, sizeof(hostName)))
	{
		printf("* Enter Process ID\n");
		scanf("%d", &usrPID);
	}

	printf("* Host: %s\n", hostName);
	cPtr = hostName;
	cPtr = cPtr+3;
	printf("* Host: %s\n", cPtr);
	sscanf(cPtr, "%2d", &usrPID);
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
					printf("* Message: %s\n", msgData[i]);
				}
			}
			i++;
			j = 0;
			printf("* --------------------------------------------- \n");
		}
	}
	return 0;
}
