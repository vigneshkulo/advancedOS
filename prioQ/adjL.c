#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define	VISITED		1	
#define	NOT_VISITED	0	

#define TRUE		1
#define FALSE		0

typedef struct adjLL
{
	int msgId;
	int visited;
	int numOutgoing;
	int* outGoingEdgesList;
}adjLL;

void** adjList = NULL;
int numNodes = 0;
int foundMsg = FALSE;
int foundVertex = FALSE;
int* nodeArray = NULL;

int addNode(msgId, vertex)
{
	int i;
	adjLL* curPtr = NULL;
	if(NULL == adjList) adjList = (void**) malloc (sizeof(void*));
	{
		for(i = 0; i < numNodes; i++)
		{
			if(msgId == nodeArray[i])
			{
				foundMsg = TRUE;
			}
		}
		for(i = 0; i < numNodes; i++)
		{
			if(vertex == nodeArray[i])
			{
				foundVertex = TRUE;
			}
		}
		if(TRUE != foundMsg)
		{
			adjList[msgId] = (adjLL*) malloc (sizeof(adjLL));
			curPtr = adjList[msgId];
			curPtr->msgId = msgId;  
			curPtr->visited = NOT_VISITED;  
			curPtr->numOutgoing++;
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
			foundMsg = FALSE;
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
	//	if(NULL == adjList[vertex])
		if(TRUE != foundVertex)
		{
			adjList[vertex] = (adjLL*) malloc (sizeof(adjLL));
			curPtr = adjList[vertex];
			curPtr->msgId = msgId;  
			curPtr->visited = NOT_VISITED;  
			curPtr->numOutgoing = 0;
			curPtr->outGoingEdgesList = NULL; 
			foundVertex = FALSE;
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
	strcpy(tabA + strlen(tabA), "\t");
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
		}
	}
	return 0;
}

int main()
{
	int i;
	#if 0 
	addNode(501, 402);  
	addNode(501, 402);  
	addNode(501, 502);  
	addNode(501, 502);  
	addNode(501, 503);  
	addNode(501, 504);  
	addNode(501, 505);  
	addNode(502, 503);  
	addNode(401, 501);  
	addNode(401, 502);  
	addNode(401, 503);  
	display(501);
	display(502);
	display(401);

	#else 
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
	#endif

//	for(i = 0; i < numNodes; i++)
	//	printf("%d ", nodeArray[i]);

	if(0 == isCyclicCheck())
	{
		printf("\n* No Cycles Found\n");
	}

	#if 0 
	push(2);
	push(5);
	push(6);
	print();
	find(8);
	#endif

	return 0;
}
