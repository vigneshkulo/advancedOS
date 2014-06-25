#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>

#define MAX_BUFFER	1024
#define MY_PORT_NUM	5000
#define LOCALTIME_STREAM        0
#define GMT_STREAM              1

int main()
{
	int connSock, ret, in, i, flags;
        time_t currentTime;
	struct sockaddr_in servaddr;
	struct sctp_sndrcvinfo sndrcvinfo;
	struct sctp_event_subscribe events;
	char buffer[MAX_BUFFER+1];


	for(i = 0; i < 2; i++)
	{
		/* Create an SCTP TCP-Style Socket */
		connSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

		/* Specify the peer endpoint to which we'll connect */
		bzero( (void *)&servaddr, sizeof(servaddr) );
		servaddr.sin_family = AF_INET;
		if(0 == i) servaddr.sin_port = htons(5000);
		if(0 == i) servaddr.sin_addr.s_addr = inet_addr( "10.176.67.64" );

		if(1 == i) servaddr.sin_port = htons(6000);
		if(1 == i) servaddr.sin_addr.s_addr = inet_addr( "10.176.67.66" );

		/* Connect to the server */
		ret = connect( connSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
		if(-1 == ret)
		{
			perror("* Client: Connection to Server Failed: ");
		}

		/* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
		memset( (void *)&events, 0, sizeof(events) );
		events.sctp_data_io_event = 1;
		setsockopt( connSock, SOL_SCTP, SCTP_EVENTS, (const void *)&events, sizeof(events) );

		#if 1
		/* New client socket has connected */

		/* Grab the current time */
		currentTime = time(NULL);

		if(0 == i)
		{
			/* Send local time on stream 0 (local time stream) */
			snprintf( buffer, MAX_BUFFER, "%s\n", ctime(&currentTime) );
			printf("* Client: Buffer: %s\n", buffer);

			ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );

			/* Expect reply message from the peer */
			memset( (void *)buffer, 0, sizeof(buffer) );
			in = sctp_recvmsg( connSock, (void *)buffer, sizeof(buffer), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );
			printf("* Client: Bytes: %d, Buffer: %s\n", in, buffer);
		}
		else if(1 == i)
		{
			/* Send GMT on stream 1 (GMT stream) */
			snprintf( buffer, MAX_BUFFER, "%s\n", asctime( gmtime( &currentTime ) ) );
			printf("* Client: Buffer: %s\n", buffer);

			ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, GMT_STREAM, 0, 0 );

			/* Expect reply message from the peer */
			memset( (void *)buffer, 0, sizeof(buffer) );
			in = sctp_recvmsg( connSock, (void *)buffer, sizeof(buffer), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );
			printf("* Client: Bytes: %d, Buffer: %s\n", in, buffer);
		}
		#endif

		/* Close our socket and exit */
		close(connSock);
	}
	return 0;
}
