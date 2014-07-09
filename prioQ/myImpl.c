#include <stdio.h>
#include <stdlib.h>

typedef struct strMsg
{
	int pid;
	int timeStamp;
	struct strMsg* next;
}strMsg;

strMsg* head;

int insert(int timeStamp, int pid)
{
	strMsg* curPtr = NULL;
	strMsg* prevPtr = NULL;
	strMsg* local = (strMsg*) malloc (sizeof(strMsg));
	local->timeStamp = timeStamp;
	local->pid = pid;

	if(NULL == head)
	{
		head = local;
		head->next = NULL;
	}
	else
	{
		if(local->timeStamp == head->timeStamp)
		{
			if(local->pid < head->pid)
			{
				local->next = head;
				head = local;
				return 0;
			}
			else
			{
				prevPtr = head;
				curPtr = head->next;
				while(NULL != curPtr)
				{
					if(local->timeStamp == curPtr->timeStamp && local->pid > curPtr->pid)
					{
						prevPtr = curPtr;
						curPtr = curPtr->next;
						continue;
					}
					else
					{
						prevPtr->next = local;
						local->next = curPtr;
						break;
					}
				}
				if(NULL == curPtr)
				{
					prevPtr->next = local;
					local->next = NULL;
				}
				return 0;
			}
		}
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
			if(local->timeStamp == curPtr->timeStamp)
			{
				if(local->pid < curPtr->pid)
				{
					prevPtr->next = local;
					local->next = curPtr;
					return 0;
				}
				else
				{
					while(NULL != curPtr)
					{
						if(local->timeStamp == curPtr->timeStamp && local->pid > curPtr->pid)
						{
							prevPtr = curPtr;
							curPtr = curPtr->next;
							continue;
						}
						else
						{
							prevPtr->next = local;
							local->next = curPtr;
							break;
						}
					}
					if(NULL == curPtr)
					{
						prevPtr->next = local;
						local->next = NULL;
					}
					return 0;
				}
			}
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
int delete(int timeStamp, int pid)
{
	strMsg* curPtr = NULL;
	strMsg* prevPtr = NULL;
	curPtr = head;
	if(timeStamp == curPtr->timeStamp && pid == curPtr->pid)
	{
		head = curPtr->next;
		free(curPtr);
		return 0;
	}

	prevPtr = curPtr;
	curPtr = curPtr->next;
	while(NULL != curPtr)
	{
		if(timeStamp == curPtr->timeStamp && pid == curPtr->pid)
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
		printf("%d[%d] ", curPtr->timeStamp, curPtr->pid);
		curPtr = curPtr->next;
	}
	printf("\n");
}

int replace(int oldT, int newT, int pid)
{
	delete(oldT, pid);
	insert(newT, pid);
}
int main()
{
	int timeStamp, pid, choice;
	int timeStampOld, pidOld ;

while(1)
{
	printf("* Enter Choice 1. Insert, 2. Replace, 3. Delete Min\n");
	scanf("%d", &choice);
	switch(choice)
	{
		case 1:
			printf("* Enter Time Stamp and Pid\n");
			scanf("%d %d", &timeStamp, &pid);
			insert(timeStamp, pid);
			display();
			break;
		case 2:
			printf("* Enter Old, New Time Stamp and Pid\n");
			scanf("%d %d %d", &timeStampOld, &timeStamp, &pid);
			replace(timeStampOld, timeStamp, pid);
			display();
			break;
		case 3: deleteMin();
			display();
			break;
	}
}
	return 0;

	#if 0
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
	#endif
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
