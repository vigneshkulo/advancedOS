/*
 *	Submitted By	: Vignesh Kulothungan
 *	Subject		: Advanced Operating Systems - Project 1
 */

/* Header Files*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(int argc, char *argv[])
{
	int num1 = 0;
	int sockFd;
        int clientLen = 0;
	int parentFd;
	struct sockaddr_in servAddr, clientAddr;

	int buffer;

	int portN = 0;
	char line[50];
	char chkStr[5];
	char hostIP[20];
	char *cPtr = NULL;
	FILE* fp = NULL;
	
	#if 0
	fp = fopen("IPinfo.dat", "r+");
	if(NULL == fp)  exit(-1);

	fgets(line, 50, fp);
        sscanf(line, "%s", chkStr);
	printf("* IP is: %s\n", chkStr);

        if(!strcmp(chkStr, "mainp"))
        {
		cPtr = line;
		cPtr += 6;
                sscanf(cPtr, "%*s %d", &parPortN);
        }
        printf("* Server : Parent Port Number: %d\n", parPortN);

	fgets(line, 50, fp);
	fgets(line, 50, fp);

	sscanf(line, "%s", chkStr);
	chkStr[5] = '\0';

	if(!strcmp(chkStr, "facto"))
	{
		cPtr = line;
		cPtr += 6;
		sscanf(cPtr, "%s", hostName);
		cPtr += strlen(hostName);
		sscanf(cPtr, "%d", &portN);
	}
	printf("* Server : Port Number: %d\n", portN);

	fgets(line, 50, fp);
	fgets(line, 50, fp);
	sscanf(line, "%d %d", &maxN, &minN);
	printf("* Server : Max and Min Limit: %d, %d\n", maxN, minN);
	#endif

	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd < 0) 
		perror("* Server : Error opening Socket");

	memset(&servAddr, sizeof(servAddr), 0);
	portN = 2000;
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = INADDR_ANY;
        servAddr.sin_port = htons(portN);

        if (bind(sockFd, (const struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
        {
                perror("* Server : Error Binding the Socket");
        }

        listen(sockFd, 5);
        clientLen = sizeof(clientAddr);
        parentFd = accept(sockFd, (struct sockaddr*) &clientAddr, (socklen_t *) &clientLen);
        if (-1 == parentFd)
        {
                perror("* Server : Error Accepting the Socket 1");
        }

	buffer = 200;
	if( -1 == write(parentFd, &buffer, sizeof(buffer)))
	{
		perror("* Server : Error writing to Client");
		exit(-1);
	}

	if( -1 == read(parentFd, &num1, sizeof(int)))
	{
		perror("* Server : Error reading from Client");
		exit(-1);
	}

	printf("* Server Exiting: %d\n", num1);

	return 0;
}
