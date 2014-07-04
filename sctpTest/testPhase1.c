#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct strMsg
{
	int timeStamp;
	struct strMsg* next;
}strMsg;

strMsg* head;

int insert(int timeStamp)
{
	strMsg* curPtr = NULL;
	strMsg* prevPtr = NULL;
	strMsg* local = (strMsg*) malloc (sizeof(strMsg));
	local->timeStamp = timeStamp;

	if(NULL == head)
	{
		head = local;
		head->next = NULL;
	}
	else
	{
		if(local->timeStamp < head->timeStamp)
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
			if(local->timeStamp < curPtr->timeStamp)
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
int delete(int timeStamp)
{
	strMsg* curPtr = NULL;
	strMsg* prevPtr = NULL;
	curPtr = head;
	if(timeStamp == curPtr->timeStamp)
	{
		head = curPtr->next;
		free(curPtr);
		return 0;
	}

	prevPtr = curPtr;
	curPtr = curPtr->next;
	while(NULL != curPtr)
	{
		if(timeStamp == curPtr->timeStamp)
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

int deleteMin()
{
	strMsg* curPtr = head;
	head = head->next;
	free(curPtr);
	return 0;
}


void display()
{
	strMsg* curPtr = NULL;
	curPtr = head;
	printf("* The List is : ");
	while(NULL != curPtr)
	{
		printf("%d ", curPtr->timeStamp);
		curPtr = curPtr->next;
	}
	printf("\n");
}

int replace(int old, int new)
{
	delete(old);
	insert(new);
}

int main()
{
	char str[7];
	int node[10];
	char* cPtr;
	FILE* fpRecv;
	int i, j, num;
	int msgNum = 0;
	char line[100];
	char recvFile[20];
	int* nodeArr = NULL;

	printf("* Enter number of Receive Files\n");
	scanf("%d", &num);

	for(i = 1; i <= num; i++)
	{
		sprintf(recvFile , "RecvOrder_%d", i);
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
				msgNum++;
				nodeArr = (int*) realloc (nodeArr, sizeof(int) * msgNum);
				while(isspace(*cPtr)) cPtr++;
				sscanf(cPtr, "%d", &nodeArr[msgNum - 1]);
				printf("%5d ", nodeArr[msgNum - 1]);
				fflush(stdout);
				cPtr = strchr(cPtr, ' ');
			}

			memset(line, 0, sizeof(line));
			printf("\n");
		}
		printf("\n");
		#endif
	}

	return 0;

	insert(7);
	display();
	insert(8);
	display();
	insert(1);
	display();
	insert(3);
	display();
	insert(5);
	display();
	insert(6);
	display();

	replace(8, 9);
	display();

	replace(3, 10);
	display();
	#if 0
	deleteMin();
	display();
	deleteMin();
	display();
	deleteMin();
	display();
	#endif

	return 0; 
}
