#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define	VISITED		1	
#define	NOT_VISITED	0	

#define MAX_NODE_NUM	8000

typedef struct adjLL
{
	int msgId;
	int visited;
	int numOutgoing;
	int* outGoingEdgesList;
}adjLL;

adjLL* adjList[MAX_NODE_NUM];
int numNodes = 0;
int* nodeArray = NULL;

int addNode(msgId, vertex)
{
	int i;
	adjLL* curPtr = NULL;
	{
		if(NULL == adjList[msgId])
		{
			adjList[msgId] = (adjLL*) malloc (sizeof(adjLL));
			curPtr = adjList[msgId];
			curPtr->msgId = msgId;  
			curPtr->visited = NOT_VISITED;  
			curPtr->numOutgoing = 1;
			curPtr->outGoingEdgesList = (int*) malloc(sizeof(int));
			curPtr->outGoingEdgesList[0] = vertex;

			if(NULL == nodeArray) 
			{
				nodeArray = (int*) malloc (sizeof(int));
				nodeArray[0] = msgId;
				numNodes++;
			}
			else
			{
				numNodes++;
				nodeArray = (int*) realloc (nodeArray, sizeof(int) * numNodes);
				nodeArray[numNodes - 1] = msgId;
			}
		}
		else
		{
			curPtr = adjList[msgId];
			for(i = 0; i < curPtr->numOutgoing; i++)
			{
				if(vertex == curPtr->outGoingEdgesList[i]) break;
			}
			if(i >= curPtr->numOutgoing)
			{
				curPtr->numOutgoing++;
				curPtr->outGoingEdgesList = (int*) realloc(curPtr->outGoingEdgesList, sizeof(int) * curPtr->numOutgoing);
				curPtr->outGoingEdgesList[curPtr->numOutgoing - 1] = vertex;

				numNodes++;
				nodeArray = (int*) realloc (nodeArray, sizeof(int) * numNodes);
				nodeArray[numNodes - 1] = msgId;
			}
		}
		if(NULL == adjList[vertex])
		{
			adjList[vertex] = (adjLL*) malloc (sizeof(adjLL));
			curPtr = adjList[vertex];
			curPtr->msgId = msgId;  
			curPtr->visited = NOT_VISITED;  
			curPtr->numOutgoing = 0;
			curPtr->outGoingEdgesList = NULL; 
		}
	}
	return 0; 
}

void display(int msgId)
{
	int i;
	adjLL* curPtr;
	curPtr = adjList[msgId];
	printf("* Adj List of %d: ", msgId);
	for(i = 0; i < curPtr->numOutgoing; i++)
	{
		printf("%d -> ", curPtr->outGoingEdgesList[i]);
	}
	printf("\n");
	printf("* ------------------------------------\n");
}
int tail = 0; 
int stack[100];
int push(int num)
{
	stack[tail] = num;
	tail++;
}
int pop()
{
	if(0 == tail) return;
	tail--;
	return stack[tail];
}

int print()
{
	int i;
	for(i = 0; i < tail; i++)
	{
		printf("%d ", stack[i]);
	}
	printf("\n");
	return 0;
}
int find(int num)
{
	int i;
	for(i = 0; i < tail; i++)
	{
		if(num == stack[i])
		{
	//		printf("* Found\n");
			return 1;
		}
	}
	return 0;
}
char tabA[100];
int isCyclic(int msgId)
{
	int i;
	adjLL* curPtr;
	curPtr = adjList[msgId];

	printf("\n%s%d [%d]-> ", tabA, msgId, curPtr->numOutgoing);

	if(VISITED == curPtr->visited) 
	{
		if(1 == find(msgId)) 
		{
			printf("* Already Visited and In Stack -> Cycle Found\n");
			return -1;
		}
		printf(" Already Visited & Not in stack.");
		return 0;
	}

	curPtr->visited = VISITED;
	if(0 == curPtr->numOutgoing)
	{
//		printf("\n%s%d [%d]-> ", tabA, msgId, curPtr->numOutgoing);
		printf(" No Outgoing Edges.");
		return 0;
	}
	strcpy(tabA + strlen(tabA), " ");
	for(i = 0; i < curPtr->numOutgoing; i++)
	{
		msgId = curPtr->outGoingEdgesList[i];
		push(curPtr->msgId);
		if (-1 == isCyclic(msgId)) return -1;
		pop(curPtr->msgId);
	}
	tabA[strlen(tabA)-1] = '\0';
//	strncpy(tabA, tabA, strlen(tabA)-1);
	return 0;	
}

int isCyclicCheck()
{
	int i;
	adjLL* curPtr;
	for(i = 0; i < numNodes; i++)
	{
		curPtr = adjList[nodeArray[i]];
		if(VISITED != curPtr->visited)
		{
			if(-1 == isCyclic(nodeArray[i])) return -1;
			printf("* Starting from Another Node\n");
		}
	}
	return 0;
}

int main()
{
	#if  0
        addNode(401, 503);
        addNode(504, 503);
        addNode(402, 503);
        addNode(503, 502);
        addNode(503, 506);
        addNode(503, 501);
        addNode(503, 505);
        addNode(505, 507);
        addNode(505, 404);
        addNode(404, 504);

        if(0 == isCyclicCheck())
        {
                printf("\n* No Cycles Found\n");
        }

	return 0;
	#endif

        char str[7];
        int node[10];
        char* cPtr;
        FILE* fpSend;
        FILE* fpRecv;
        int i, j, k, num;
        int recvMsgNum = 0;
        int sendMsgNum = 0;
        char line[100];
        char sendFile[20];
        char recvFile[20];
        int* sendNodeArr = NULL;
        int* recvNodeArr = NULL;

	for(i = 0; i < MAX_NODE_NUM; i++)
	{
		adjList[i] = NULL;
	}

        printf("* Enter number of Receive Files\n");
        scanf("%d", &num);

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
        for(i = 1; i <= num; i++)
        {
                sprintf(recvFile , "RecvOrder_%d", i);
                fpRecv = fopen( recvFile, "r");
                if(NULL == fpRecv)
                {
                        perror("* Error opening Recv File: ");
                        exit(-1);
                }

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
		for(k = 0; k < recvMsgNum - 1; k++)
		{
			printf("* Adding %d -> %d\n", recvNodeArr[k], recvNodeArr[k+1]);
			addNode(recvNodeArr[k], recvNodeArr[k+1]);
		}
		recvMsgNum = 0;
                printf("\n");
        }


	if(0 == isCyclicCheck())
	{
		printf("\n* No Cycles Found\n");
	}
        return 0;
}
