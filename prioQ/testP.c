#include<stdio.h>
#include<stdlib.h>

#define	VISITED		1	
#define	NOT_VISITED	0	

typedef struct adjLL
{
	int msgId;
	int visited;
	struct adjLL* next;
}adjLL;

void** adjList = NULL;

int addNode(msgId, vertex)
{
	adjLL* curPtr = NULL;
	if(NULL == adjList) adjList = (void**) malloc (sizeof(void*));
	{
		if(NULL == adjList[msgId])
		{
			adjList[msgId] = (adjLL*) malloc (sizeof(adjLL));
			curPtr = adjList[msgId];
			curPtr->msgId = vertex;  
			curPtr->next = NULL;  
			curPtr->visited = NOT_VISITED;  
			return 0;
		}
		curPtr = adjList[msgId];
		if(vertex == curPtr->msgId) return 0; 

		while(NULL != curPtr->next) 
		{
			curPtr = curPtr->next;
			if(vertex == curPtr->msgId) return 0; 
		}
		curPtr->next = (adjLL*) malloc (sizeof(adjLL));
		curPtr = curPtr->next;
		curPtr->msgId = vertex;  
		curPtr->visited = NOT_VISITED;  
		curPtr->next = NULL;  
	}
	return 0; 
}

void display(int msgId)
{
	adjLL* curPtr;
	curPtr = adjList[msgId];
	printf("* Adj List of %d: ", msgId);
	while(NULL != curPtr)
	{
		printf("%d -> ", curPtr->msgId);
		curPtr = curPtr->next;
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
			printf("* Found\n");
			return 1;
		}
	}
	printf("* Not Found\n");
	return 0;
}

int isCyclic()
{
	printf("* Checking for Cycle\n");
	int msgId = 401;
	adjLL* curPtr;
	curPtr = adjList[msgId];
	while(NULL != curPtr)	
	{
		#if 0
		if(VISITED == curPtr->visited) 
		{
			if(1 == find(msgId)) 
			{
				printf("* Cycle Found\n");
				return -1;
			}
			msgId = pop();
			continue;
		}
		#endif
	//	push(msgId);
	//	curPtr->visited = VISITED;
		msgId = curPtr->msgId;
		printf("%d -> ", msgId);
		fflush(stdout);
		curPtr = adjList[msgId];
	}
	print();
	printf("\n");
	return 0;	
}

int main()
{
	#if 1 
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
	addNode(503, 401);  
	display(501);
	display(502);
	display(401);
	#endif

	isCyclic(401);

	#if 0 
	push(2);
	push(5);
	push(6);
	print();
	find(8);
	#endif

	return 0;
}
