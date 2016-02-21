/******************************************************************************
*
* @File : USER1.c is send sting charcater by characterr using r_send api.
*
******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "rsocket.h"

/* Macro */
#define SERVER_PORT	(50428)
#define CLIENT_PORT	(50429)
#define CLIENT_ADDRESS	"127.0.0.1"
#define BUFFERSIZE	(1024)
#define SUCCESS		(0)
#define FAILURE		(-1)

/******************************************************************************
*
* @Function	: main() is used to take the string from user and send to
*		  Client one by one.
*
******************************************************************************/
int main()
{
	struct sockaddr_in server_address = {0}, client_address = {0};
	char chBuffer[BUFFERSIZE] = {0};
	int iSocketId = 0, iLength_server = 0, iLength_client = 0;
	int iIndex = 0;

	iSocketId = r_socket(AF_INET, SOCK_BRP, 0);
	if(iSocketId < 0)
	{
		perror("socket");
		return FAILURE;
	}
	printf("SocketId = %d\n", iSocketId);

	/* Fill the sockaddr_in structure for server */
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;
	iLength_server = sizeof(server_address);

	/* Bind your socket using r_bind */
	if( r_bind(iSocketId, (struct sockaddr *)&server_address, iLength_server) != SUCCESS )
	{	
		printf("r_bind faild\n");
		return FAILURE;
	}

	/* Fill the sockaddr_in structure for client.
	   so, we can send message using this structure */
	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(CLIENT_PORT);
	client_address.sin_addr.s_addr = inet_addr(CLIENT_ADDRESS);
	iLength_client = sizeof(client_address);

	printf("Enter your string Here\n");
	scanf("%s",chBuffer);
	printf("The string you enter is = %s\n", chBuffer);

	/* Send each character of string one by one */
	for(iIndex = 0; iIndex < strlen(chBuffer); iIndex ++)
	{
		r_sendto(iSocketId, &chBuffer[iIndex], sizeof(chBuffer[iIndex]), 0, (struct sockaddr *)&client_address, iLength_client);
	}

	/* close the socket */
	if( r_close(iSocketId) != SUCCESS )
	{
		printf("r_close failed\n");
		return FAILURE;
	}
	
	printf("data=%s\n",chBuffer);
	return SUCCESS;
}

