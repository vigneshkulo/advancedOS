#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#define LOCALTIME_STREAM        0
#define GMT_STREAM              1
#define MAX_BUFFER      	1024
#define MY_PORT_NUM     	5000

int main()
{
	int listenSock, connSock, ret;
	struct sockaddr_in servaddr;
	char buffer[MAX_BUFFER+1];
	time_t currentTime;

        int in, i, flags;
        struct sctp_sndrcvinfo sndrcvinfo;
        struct sctp_event_subscribe events;

	/* Create SCTP TCP-Style Socket */
	listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );
	printf("* Server: Socket FD: %d\n", listenSock);

	/* Accept connections from any interface */
	bzero( (void *)&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
	servaddr.sin_port = htons(MY_PORT_NUM);

	/* Bind to the wildcard address (all) and MY_PORT_NUM */
	ret = bind( listenSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
	printf("* Server: Bind Return: %d\n", ret);

	/* Place the server socket into the listening state */
	listen( listenSock, 5 );

	/* Server loop... */
//	while( 1 ) 
	{

	/* Await a new client connection */
	connSock = accept( listenSock, (struct sockaddr *)NULL, (int *)NULL );
	printf("* Server: Connection Accepted: %d\n", connSock);

        /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
        memset( (void *)&events, 0, sizeof(events) );
        events.sctp_data_io_event = 1;
        setsockopt( connSock, SOL_SCTP, SCTP_EVENTS, (const void *)&events, sizeof(events) );

        /* Expect two messages from the peer */
//        for (i = 0 ; i < 2 ; i++) {

        in = sctp_recvmsg( connSock, (void *)buffer, sizeof(buffer), (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags );

        /* Null terminate the incoming string */
        buffer[in] = 0;

        if        (sndrcvinfo.sinfo_stream == LOCALTIME_STREAM) {
        printf("(Local) %s\n", buffer);
        } else if (sndrcvinfo.sinfo_stream == GMT_STREAM) {
        printf("(GMT  ) %s\n", buffer);
        }

 //       }

        snprintf( buffer, MAX_BUFFER, "%s\n", "Received");
        ret = sctp_sendmsg( connSock, (void *)buffer, (size_t)strlen(buffer), NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );
        printf("* Send Status: %d\n", ret);
        sleep(1);

	/* Close the client connection */
	close( connSock );

	}

	return 0;
}
