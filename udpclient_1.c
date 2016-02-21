/******************************************************************************
*
* @FILE USER2.c is read message using the r_receive messge.
*
******************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<fcntl.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "rsocket.h"

/* Macro */
#define CLIENT_ADDRESS	"127.0.0.1"
#define CLIENT_PORT	(50429)
#define BUFFERSIZE	(1024)
#define SUCCESS		(0)
#define FAILURE		(-1)

/******************************************************************************
*
* @Function	: main() is used to read string/charcter using r_recvfrom.
*
******************************************************************************/
int main()
{
	struct sockaddr_in client_address = {0};
	char chBuffer[BUFFERSIZE] = {0};
	int iSocketId = 0 , iLength_client = 0;
	int iIndex = 0;

	iSocketId = r_socket(AF_INET, SOCK_BRP, 0);
	if(iSocketId < 0)
	{
		perror("socket");
		return FAILURE;
	}
	printf("iSocketId = %d\n", iSocketId );
	
	/* Fill the sockaddr_in strecture for client */
	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(CLIENT_PORT);
	client_address.sin_addr.s_addr = INADDR_ANY;
	iLength_client = sizeof(client_address);

	/* Bind your socket using r_bind */
	if( r_bind(iSocketId , (struct sockaddr *)&client_address, iLength_client) != SUCCESS )
	{
		printf("r_bind failed\n");
		return FAILURE;
	}

	/* Receive the character using recvform */
	for(iIndex = 0; iIndex < 50; iIndex ++)
	{
		r_recvfrom(iSocketId, &chBuffer[iIndex], sizeof(chBuffer[iIndex]), 0, (struct sockaddr *)&client_address, &iLength_client);
		printf("%c\n",chBuffer[iIndex]);
	}

	/* Close the socket */
	if( r_close(iSocketId) != SUCCESS )
	{
		printf("r_close failed\n");
		return FAILURE;
	}
	printf("data=%s\n",chBuffer);

	return SUCCESS;
}


