#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<math.h>
#include<errno.h>
#include<pthread.h>
#include<semaphore.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netinet/sctp.h>


#define MAX_BUFFER 1024
#define MY_PORT_NUM 8000 
#define INPUT_STREAM 1

pthread_t tid[2];
sem_t semaphore1;

int no_of_messages=0;
int no_of_destinations[500];
FILE *fp1=NULL;
FILE *fp2=NULL;
int message_counter=0;

struct message_structure
{
        char process_id[5];
        char message[200];
        int destination_set[45];
}msg[100];


void * recieve_thread(void *arg)
{
	int t = (intptr_t) arg;
 printf("running recieve thread \n");
 int listenSock, recvSock, ret,flags;
  struct sockaddr_in servaddr;
  struct sctp_event_subscribe events;
  char buffer[MAX_BUFFER+1];
  time_t currentTime;
  struct sctp_sndrcvinfo sndrcvinfo;
  /* Create SCTP TCP-Style Socket */
  listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

  /* Accept connections from any interface */
  bzero( (void *)&servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
  servaddr.sin_port = htons(MY_PORT_NUM);

  
	ret = bind( listenSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
	if(-1 == ret) 
	{
		perror("********** Error Binding: ");
		exit(-1);
	}
 
  listen( listenSock, 5 );


// printf("running recieve thread \n");
	while( 1 ) 
	{
		printf("* Ready to Accept connections\n");
		recvSock = accept( listenSock,(struct sockaddr *)NULL, (int *)NULL );
		if(-1 == recvSock) 
		{
			perror("********** Error Accepting: ");
			exit(-1);
		}

		printf("* Socket Fd: %d \n", recvSock);

		memset( (void *)&events, 0, sizeof(events) );
		events.sctp_data_io_event = 1;
		setsockopt( recvSock, SOL_SCTP, SCTP_EVENTS,(const void *)&events, sizeof(events) ); 
		flags = 0;

		ret = sctp_recvmsg( recvSock, (void *)buffer, sizeof(buffer),(struct sockaddr *)NULL, 0,&sndrcvinfo, &flags );
		if(ret == -1)
		{
		   perror("******** Error Receiving: ");
		   if(errno == EAGAIN)
			   printf("* It Will Block: %d\n", flags);
		}
		printf("* Received Bytes: %d \n",ret);
		buffer[ret] = 0;

		printf(" %s \n",buffer);
		/* Close the client connection */
	}
	close( recvSock );
}

void * send_thread(void *arg)
{
	int ret;
  int connSock, in, ij, flags;
  struct sockaddr_in servaddr;
  struct sctp_sndrcvinfo sndrcvinfo;
  struct sctp_event_subscribe events;
  char buffer[MAX_BUFFER+1]= "Test message";
  char ip_address[20];
  char line_reader[30];
  FILE *ipfp = NULL;
  int temp_dest; 
  int dest_counter = 0;

/* Create an SCTP TCP-Style Socket */

while(1)
{
	printf("* Send thread runnnning \n");
sem_wait(&semaphore1);
printf("the no of destinations is %d",no_of_destinations[dest_counter]);
for (ij = 0 ; ij<no_of_destinations[dest_counter];ij++)
{ printf("entering send thread \n");
  //sleep(3);
  connSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );
  ipfp = fopen("ipconfig.dat","r+");
//  printf("the value of destination set is %d \n",msg[1].destination_set[0]);
  while(fgets(line_reader,100,ipfp)!=NULL)
  {
	sscanf(line_reader,"%d ",&temp_dest);
//	printf("the  value of ij and message ccounter is %d %d \n",ij,message_counter);
//	printf("the value of temp_dest %d and destination_set is %d \n",temp_dest,msg[message_counter].destination_set[ij]);
	if(temp_dest == msg[message_counter].destination_set[ij])
	{
		printf("going to read ip \n");
	sscanf(line_reader,"%*s %*s %s",ip_address); 
	}
  }

  printf("the ipaddress is %s \n",ip_address);
  /* Specify the peer endpoint to which we'll connect */
  bzero( (void *)&servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(MY_PORT_NUM);
  servaddr.sin_addr.s_addr = inet_addr(ip_address);

  /* Connect to the server */

  ret = connect( connSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
  if(-1 == ret)
  {
  	perror("********* Error Connecting: ");
	continue;
  }
  printf("connection established \n");
  /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
  memset( (void *)&events, 0, sizeof(events) );
  events.sctp_data_io_event = 1;
  setsockopt( connSock, SOL_SCTP, SCTP_EVENTS,
               (const void *)&events, sizeof(events) );
  /* Expect two messages from the peer */
   in  = sctp_sendmsg( connSock,(void *)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, 0, 0, 0 );
   if(-1 == in)
   {
  	perror("********* Error Sending: ");
	continue;
   }
   printf("* Sent %d bytes\n", in);
   printf("* -------------------------------\n");

  close(connSock);

}
dest_counter++;
message_counter++;
}

  /* Close our socket and exit */
}

void main()
{
	int pipe_1[2],pipe_2[2];
	pipe(pipe_1);
	sem_init(&semaphore1,0,0);
	char choice[10];

	if(fork() == 0)
	{
		while(1)
		{
			printf("enter the command m_send or m_recieve \n");
			fflush(stdin);
			scanf("%s",choice);
			if(strncmp(choice,"m_send",sizeof(choice))==0)
			{
				write(pipe_1[1],choice,sizeof(choice));
			}
		}
	}
	else
	{
	char choice_recieved[10];
	char hostname[6];
	char processname[6];
	char line[200];
	char destination[200];
	char *startpointer=NULL;
	char *endpointer=NULL;
	char *destin=NULL;
	int msgsize;
	int i=0,j=0;
	char *token;
	char s[2]=":";
	char temp_process_id[30];
	startpointer = line;
	destin = destination;
	gethostname(hostname,6);
	printf("the hostname is %s \n",hostname);
	strncpy(processname,hostname,5);
	processname[6]='\0';
	printf("The processname is %s \n",processname);

	fp1 = fopen("msgconfig.dat","r+");
	while(fgets(line,200,fp1)!=NULL)
	{
	startpointer = strchr(line,'<');
	sscanf(line,"%s ",temp_process_id);
	if(strcmp(temp_process_id,processname)==0)
	{
	endpointer = strchr(line,'>');
	msgsize = endpointer - startpointer;
	memcpy(msg[i].message,startpointer+1,msgsize-1);
	printf("the line is %s\n",line);
	printf("the message is %s \n",msg[i].message);
	sscanf(line," %s %s",msg[i].process_id,destination);
	printf("The message process id is : %s and destination is %s \n",msg[i].process_id,destination);
	token = strtok(destination,s);
	while(token != NULL)
	{

	sscanf(token,"%d ",&msg[i].destination_set[j]);
	printf("the value of i is %d and j is %d \n",i,j);
	printf("the detination set is %d\n",msg[i].destination_set[j]);
	token = strtok(NULL,s);
	j++;
	no_of_destinations[i]=j;
	}
	i++;
	no_of_messages=i;
	}
	j=0;
	}

	pthread_create( &tid[0], NULL , recieve_thread , NULL);
	pthread_create( &tid[1] , NULL , send_thread , NULL);
		while(1)
		{
			read(pipe_1[0],choice_recieved,sizeof(choice_recieved)); 
			printf("read complete \n");
			printf("the no of messages is %d \n",no_of_messages);
			printf("the message counter is %d \n",message_counter);
			if(strncmp(choice_recieved,"m_send",sizeof(choice))==0)
			{    
				if(message_counter<no_of_messages)
				{
					printf("the semaphore is going to be posted \n");
					sem_post(&semaphore1);
					printf("semaphore is posteed \n");
				}
				else
				{
					printf("all messages have been sent \n");
					sleep(5);
					exit(0);
				}
			}
		}
	}
}


