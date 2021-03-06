/*
 * Submitted By    : Vignesh Kulothungan
 * Subject         : Advanced Operating Systems - Project 1
 * Module          : Test Module
 */

/*
 * Description:
 * Log Files created by the Application has Message Ids in their received order.
 * Ex: m1 m2 m3 m4
 *
 * Add all Message Ids from all Log files to the Directed Graph.
 *
 * Relation between Nodes (Message Ids):
 * If m1 is before m2 in Process 1's Log file, then m1 has an outgoind edge to m2.
 * If m2 is delivered before m1 in some other processes Log file, then
 * m2 will have an outgoing edge to m1. These edges will form a cycle.
 *
 * When a Cycle is detected, it violates the Total Order.
 * Hence from the Log files, message Ids are extracted and added to a directed 
 * graph. Then Depth First Search is used to find a Cycle.
 * If Cycle Exits, Total order is Violated.
 *
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define	VISITED		1	
#define	NOT_VISITED	0	

#define MAX_NODE_NUM	8000

#define MAX_MULTICAST_MSGS	500

int numMcMsg;
int notFound = 0;
int numMcMemb[400];
int lostMsg[MAX_MULTICAST_MSGS];
int mcMemb[MAX_MULTICAST_MSGS][50];

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

/* Add Node to the Adjacency List (Graph) */
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
int stack[1000];
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
			#ifdef PLOT
			printf("* %d Found at %d\n", num, i);
			#endif
			return 1;
		}
	}
	return 0;
}
char tabA[1000];
int isCyclic(int msgId)
{
	int i;
	adjLL* curPtr;
	curPtr = adjList[msgId];
	
	#ifdef PLOT
	printf("\n%s%d [%d]-> ", tabA, msgId, curPtr->numOutgoing);
	#endif

	if(VISITED == curPtr->visited) 
	{
		if(1 == find(msgId)) 
		{
			printf("\n* -----------------------------------------------\n");
			printf("* %d: Already Visited and In Stack -> Cycle Found\n", msgId);
			printf("* -----------------------------------------------\n");
			printf("* Stack Trace: \n");
			return -1;
		}
		#ifdef PLOT
		printf(" Already Visited & Not in stack.");
		#endif
		return 0;
	}

	curPtr->visited = VISITED;
	if(0 == curPtr->numOutgoing)
	{
		#ifdef PLOT
		printf(" No Outgoing Edges.");
		#endif
		return 0;
	}
	strcpy(tabA + strlen(tabA), " ");
	for(i = 0; i < curPtr->numOutgoing; i++)
	{
		msgId = curPtr->outGoingEdgesList[i];
		push(curPtr->msgId);
		if (-1 == isCyclic(msgId)) 
		{
			printf("%d <- ", msgId);
			fflush(stdout);
			return -1;
		}
		pop(curPtr->msgId);
	}
	tabA[strlen(tabA)-1] = '\0';
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

			#ifdef PLOT
			printf("\n* Starting from Another Node\n");
			#endif
		}
	}
	return 0;
}

int check(int msgId)
{
	int i;
	FILE* fpRecv = NULL;
        char recvFile[20];
        char line[100];
        char msgIdStr[20];
	int found = 0;
	int msgNum = (msgId >> 6) - 1;

	#ifdef DEBUG
	printf("* %d Found in: ", msgId);	
	#endif

	sprintf(msgIdStr , "%05d", msgId);
	for(i = 0; i < numMcMemb[msgNum]; i++)
	{
                sprintf(recvFile , "RecvOrder_%d", mcMemb[msgNum][i]);
                fpRecv = fopen( recvFile, "r");
                if(NULL == fpRecv)
                {
			printf("* Opening File: %s\n", recvFile);
                        perror("* Error opening Recv File: ");
                        exit(-1);
                }
                while(NULL != fgets(line, 70, fpRecv))
                {
			if(strstr(line, msgIdStr) != NULL)
			{
				#ifdef DEBUG
				printf("%d, ", mcMemb[msgNum][i]);
				#endif

				found = 1;
				break;
			}
		}
		if(0 == found)
		{
			#ifdef DEBUG
			printf("[%d], ", mcMemb[msgNum][i]);
			#endif
			lostMsg[notFound] = msgId;
			notFound++;
		}
		found = 0;
		fclose(fpRecv);
	}
	#ifdef DEBUG
	printf("\n");
	#endif
	return 0;
}
int extract(int usrPID)
{
        int memSet = 0;
	char mcGrp[50];
        int portN;
        int procID;
        char* cPtr;
        char* sPtr;
        char* ePtr;
        char pidStr[5];
        char line[100];
        int chkID;
        int i, j;
        FILE *fp = NULL;
        char hostName[30];
        int hostNameLen;

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
                        cPtr = mcGrp;
                        cPtr++;
			#ifdef DEBUG
                        printf("* Multicast Mem: ");
			#endif
                        while(1)
                        {
                                sscanf(cPtr, "%d", &mcMemb[i][j]);
                                sscanf(cPtr, "%*d%s", cPtr);
				#ifdef DEBUG
                                printf("%d ", mcMemb[i][j]);
				#endif
				numMcMemb[i]++;
                                if(!strncmp(">", cPtr, 1)) break;
                                cPtr++;
                                j++;
                        }
			#ifdef DEBUG
                        printf("\n");
			#endif

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
					//	memcpy(msgData[i], ++cPtr, ePtr-sPtr);
					//	msgData[i][ePtr-sPtr] = '\0';
						#ifdef DEBUG
					//	printf("* %d: Message: %s\n", numMcMsg, msgData[i]);
						#endif
					}
				}
                        }
			numMcMsg++;
                        i++;
                        j = 0;
			if(numMcMsg == MAX_MULTICAST_MSGS)
			{
				#ifdef DEBUG
				printf("* Max multicast message limit reached: %d\n", numMcMsg);
				#endif
				break;
			}
                }
                else
                {
                        if(strstr(line, pidStr) != NULL)
                        {
                                memSet++;
                        }
                }
	}
}

int main()
{
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

	#ifdef DEBUG
	printf("* Send Order\n");
	#endif
        for(i = 1; i <= num; i++)
	{
		#ifdef DEBUG
		printf("\n* Process %d Send Log\n", i);
		#endif

		sprintf(sendFile , "SendOrder_%d", i);
		fpSend = fopen( sendFile, "r");
		if(NULL == fpSend)
		{
			perror("* Error opening Send File: ");
			exit(-1);
		}
		memset(&numMcMemb, 0, sizeof(numMcMemb));
		numMcMsg = 0;
		extract(i);

		#ifdef DEBUG
		printf("* %d: Messages\n", numMcMsg);
		#endif

                while(NULL != fgets(line, 70, fpSend))
                {
                        cPtr = line;
                        for(j = 0; j < strlen(line)/6; j++)
                        {
                                sendMsgNum++;
                                sendNodeArr = (int*) realloc (sendNodeArr, sizeof(int) * sendMsgNum);
                                while(isspace(*cPtr)) cPtr++;
                                sscanf(cPtr, "%d", &sendNodeArr[sendMsgNum - 1]);

				#ifdef DEBUG
				printf("%5d ", sendNodeArr[sendMsgNum - 1]);
				fflush(stdout);
				#endif

                                cPtr = strchr(cPtr, ' ');

				check(sendNodeArr[sendMsgNum - 1]);
                        }

                        memset(line, 0, sizeof(line));
                }

		#ifdef DEBUG
                printf("\n");
		#endif
	}
	
	#ifdef DEBUG
	printf("* Receive Order");
	#endif

        for(i = 1; i <= num; i++)
        {
                sprintf(recvFile , "RecvOrder_%d", i);
                fpRecv = fopen( recvFile, "r");
                if(NULL == fpRecv)
                {
                        perror("* Error opening Recv File: ");
                        exit(-1);
                }

		#ifdef DEBUG
		printf("\n* Process %d Receive Log\n", i);
		#endif

                while(NULL != fgets(line, 70, fpRecv))
                {
                        cPtr = line;
                        for(j = 0; j < strlen(line)/6; j++)
                        {
                                recvMsgNum++;
                                recvNodeArr = (int*) realloc (recvNodeArr, sizeof(int) * recvMsgNum);
                                while(isspace(*cPtr)) cPtr++;
                                sscanf(cPtr, "%d", &recvNodeArr[recvMsgNum - 1]);

				#ifdef DEBUG
                                printf("%5d ", recvNodeArr[recvMsgNum - 1]);
                                fflush(stdout);
				#endif

                                cPtr = strchr(cPtr, ' ');
                        }

                        memset(line, 0, sizeof(line));

			#ifdef DEBUG
                        printf("\n");
			#endif
                }

		/* Add All nodes to the Directed Graph */
		for(k = 0; k < recvMsgNum - 1; k++)
		{
			addNode(recvNodeArr[k], recvNodeArr[k+1]);
		}
		recvMsgNum = 0;
        }

	/* Check for a Integrity in the Log Files */
	if(0 == notFound)
	{
		printf("* ---------------------------------------------------\n");
		printf("* Integrity Check Complete: All Messages were Found\n");
		printf("* ---------------------------------------------------\n");
	}
	else if (0 < notFound)
	{
		printf("* ---------------------------------------------------\n");
		printf("* Integrity Check Complete: Messages were Lost: ");
		for(i = 0; i < notFound; i++)
		{
			printf("%d ", lostMsg[i]);
		}
		printf("\n* ---------------------------------------------------\n");
	}

	/* Check for a Cycle in the Graph */
	if(0 == isCyclicCheck())
	{
		printf("* --------------------\n");
		printf("* No Cycles Found\n");
		printf("* ----------------------\n");
	}
        return 0;
}
