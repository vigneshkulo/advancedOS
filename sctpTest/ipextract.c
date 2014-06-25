#include <stdio.h>
#include <stdlib.h>

int main()
{
	int portN;
	int usrPID;
	int procID;
	char* cPtr;
	char line[50];
	int chkID;
	FILE *fp = NULL;
	char ipAddr[50];
	
	printf("* Enter Process ID\n");
	scanf("%d", &usrPID);

	fp = fopen("ipconfig.dat", "r+");
	if(NULL == fp)  exit(-1);

	while(NULL != fgets(line, 50, fp))
	{
		sscanf(line, "%d", &chkID);
		printf("* Check ID: %d\n", chkID);

		if(chkID == usrPID)
		{
			cPtr = line;
			sscanf(cPtr, "%d %d %s", &procID, &portN, ipAddr);
			printf("* Process ID : %d\n", procID);
			printf("* Port Number: %d\n", portN);
			printf("* IP Address : %s\n", ipAddr);
			break;
		}
	}
	return 0;
}
