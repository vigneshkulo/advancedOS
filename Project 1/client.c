#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
	int num1 = 143;
	int sockFd = 0;
	int portN = 2000 ;
	struct hostent *server;
	struct addrinfo hints, *servInfo;
	struct sockaddr_in servAddr, clientAddr;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((getaddrinfo("10.176.67.64", "2000", &hints, &servInfo)) != 0) 
	{
		perror("* Client : Error getting Socket Info");
		exit(1);
	}
	#if 0
	server = gethostbyname("localhost");
	if (NULL == server)
	{
		perror("* Client : Error: No Such Host");
		exit(0);
	}
	#endif

	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockFd)
	{
		perror("* Client : Error Opening the Socket");
		exit(0);
	}

	memset(&servAddr, sizeof(servAddr), 0);

	#if 0 
	servAddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&servAddr.sin_addr.s_addr, server->h_length);
	servAddr.sin_port = htons(portN);
	#endif

	if (-1 == connect(sockFd, servInfo->ai_addr, servInfo->ai_addrlen))
	{
		perror("* Client : Error: Connecting");
		exit(-1);
	}

	if( -1 == read (sockFd, &num1, sizeof(num1)))
	{
		perror("* Client : Error Reading from Server");
		exit(-1);
	}
	printf("* Client : Number: %d\n", num1);
	num1++;

	if( -1 == write (sockFd, &num1, sizeof(int)))
	{
		perror("* Client : Error Writing to Server");
		exit(-1);
	}

	return 0;
}
