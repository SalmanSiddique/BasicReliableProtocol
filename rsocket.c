/******************************************************************************
* rsocket.c : File contaion various function to createe basic reliable protocol
*	      API.
******************************************************************************/

#include "rsocket.h"
#define DEBUG		(0)
#define TEST		(1)
#define DROP_VALUE	(0.4)

struct msg_struct unack_msg[NO_OF_MESSAGE]={0},recv_msg[NO_OF_MESSAGE]={0};
int iRecMsgCount = 0, iUnackMsgCount = 0, globalstop = 0;
pthread_mutex_t sync_mutex;
pthread_t RthreadId=0,SthreadId=0;

/******************************************************************************
*
* @ function	: r_socket
* @ Parameter	: domain - specify a communication domain; 
*		  	   this selects the protocol family,
*			   which will be used for communication.
*		  type   - specifies the communication semantics.
*		  Protocol - specifies a particular protocol to be used with the
*			     socket	
* @ Return	: int socketid of socket
* @ Description	: Function create socket and two thread r and s.
* 
******************************************************************************/

int r_socket(int domain, int type, int protocol)
{
	int iSocketId = 0;

	iSocketId=socket(domain, type, protocol);
	if(iSocketId < 0)
	{
		printf("Failed to create Socket\n");
		return FALSE;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	if (setsockopt(iSocketId, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
	{
		perror("Error");
	}

	pthread_mutex_init(&sync_mutex, NULL);

	if(pthread_create(&RthreadId, NULL, RThread, (void *)iSocketId) != TRUE)
	{
		printf("R-thread creatation failed\n");
		return FALSE;
	}

	if(pthread_create(&SthreadId, NULL, SThread, (void *)iSocketId) != TRUE)
	{
		printf("S-thread creatation failed\n");
		return FALSE;
	}

	return iSocketId;	
}

/******************************************************************************
*
* @ function	: r_bind
* @ Parameter	: int iSocketId - socket id of socket
*		  struct sockaddr *server_address - Pointer to address where 
*				  we want to bind our socket
*		  socklen_t addrlen - length of sockaddr structure
* @ Return	: int - 0 on success.
*			-1 on failure.
* @ Description	: Function bind our socket.
* 
******************************************************************************/

int r_bind(int iSocketfd, struct sockaddr *server_address,socklen_t addrlen)
{
	if(bind(iSocketfd, server_address, addrlen) != TRUE)
	{
		perror("BIND");
		return FALSE;
	}

	return TRUE;
}

/******************************************************************************
*
* @ function	: r_close
* @ Parameter	: int iSocketId - socket id of socket
* @ Return	: int - 0 on success.
*			-1 on failure.
* @ Description	: Function wait for other thread to join and close the socket.
* 
******************************************************************************/

int r_close(int iSocketId)
{

	while(iUnackMsgCount)
	{
		sleep(2*T);
	}

	globalstop = 1;	
	pthread_join(RthreadId, NULL);
	pthread_join(SthreadId, NULL);

	if(iSocketId > 0)
	{
		close(iSocketId);
	}

	return TRUE;
}

/******************************************************************************
*
* @ function	: r_recvfrom
* @ Parameter	: int iSocketId - socket id of socket
*		  void *buff - use to store received message.
*		  size_t len - len of the received message.
*		  int flags - use to set various option.
*		  struct sockaddr_in * src_address - To store the receiver 
*							address.
*		  socklen_t * sock_len - Used to store src_address length.
* @ Return	: No of character sucessfully received in bytes.
* @ Description	: Function cheack Received message table and if found message
*		  Print it on screen or wait for retry. 
* 
******************************************************************************/

ssize_t r_recvfrom(int iSocketId, void *buff, size_t len, int flags, struct sockaddr *src_address, socklen_t *sock_len)
{
	int iRetstatus = 0, iCount = 0, iIndex = 0;

	while(globalstop == TRUE)
	{
		if(iRecMsgCount != 0)	
		{
			pthread_mutex_lock(&sync_mutex);
			iRetstatus = snprintf((char *)buff, strlen(recv_msg[iCount].data.chBuffer) + 1, "%s", recv_msg[iCount].data.chBuffer);
#if DEBUG
			printf("r_recvform : strlen(recv_msg[iCount].data.chBuffer) + 1 = %d\n",strlen(recv_msg[iCount].data.chBuffer) + 1);
			printf("r_recvform : recv_msg[iCount].data.chBuffer = %s\n", recv_msg[iCount].data.chBuffer);
			printf("r_recvform : buff = %s\n", (char *)buff);
#endif
			for(iIndex = 0; iIndex < iRecMsgCount; iIndex++)
			{
				memcpy(&recv_msg[iIndex],&recv_msg[iIndex+1],sizeof(recv_msg[iIndex]));
			}

			iRecMsgCount--;
			pthread_mutex_unlock(&sync_mutex);
			break;	
		}
		else
		{
			sleep(1);
		}
	}
	return iRetstatus;	
}

/******************************************************************************
*
* @ function	: r_sendto
* @ Parameter	: int iSocketId - socket id of socket
*		  void *buff - use to store the message we want to sent.
*		  size_t len - len of the message.
*		  int flags - use to set various option.
*		  struct sockaddr_in * dst_address - The destination address
*					where we want to send data.
*		  socklen_t * sock_len - Used to store dst_address length.
* @ Return	: No of character sucessfully sent in bytes.
* @ Description	: Function sent the message and store it into the Unacknoledge
*		  message table. 
* 
******************************************************************************/

ssize_t r_sendto(int iSocketId, const void *buff, size_t len, int flags, struct sockaddr *dst_address, socklen_t sock_len)
{
	while(iUnackMsgCount)
	{
		sleep(1);
	}

	int iRetStatus = 0;
	static iCount, iMsgSeqNo = 0;
	struct timeval *tv;
	struct msg_struct tmp_struct;

	pthread_mutex_lock(&sync_mutex);
	unack_msg[iUnackMsgCount].iMsgSeqNo = iMsgSeqNo;
	snprintf(unack_msg[iUnackMsgCount].data.chBuffer, len + 1,"%s", (char *)buff);	

	unack_msg[iUnackMsgCount].data.Type = 0;		/* 0-for data */

#if DEBUG
	printf("---------------------------------------------------------------------------------------------------");
	printf("r_sendto : buff = %s\n", (char *)buff);
	printf("r_sendto : iMsgSeqNo = %d\n", iMsgSeqNo);
#endif

	memcpy(&unack_msg[iUnackMsgCount].address, dst_address, sizeof(unack_msg[iUnackMsgCount].address));
	gettimeofday(&unack_msg[iUnackMsgCount].tv,NULL);
	memcpy(&tmp_struct, &unack_msg[iUnackMsgCount], sizeof(unack_msg[iUnackMsgCount]));
	pthread_mutex_unlock(&sync_mutex);

	//printf("Before sendto, tmp_struct.data.chBuffer = %s\n", tmp_struct.data.chBuffer);

	iRetStatus = sendto(iSocketId, &tmp_struct, sizeof(tmp_struct), flags, (struct sockaddr *)dst_address, sock_len);
	if(iRetStatus == FALSE)
	{
		perror("sendto");
		return iRetStatus;
	}	

	pthread_mutex_lock(&sync_mutex);
	iUnackMsgCount ++;
	pthread_mutex_unlock(&sync_mutex);
	iMsgSeqNo ++;
	return iRetStatus;
}

/******************************************************************************
*
* @ function	: RThread
* @ Parameter	: void *arg - use to store socketId
* @ Return	: void *
* @ Description	: Received the message and ckeck whether it is data or 
*		  acknowledge and action accordingly.
*		   
******************************************************************************/

void* RThread(void *arg)
{
	int iSocketId = (int )arg;
	struct sockaddr_in src_address;
	struct msg_struct tmp_struct;
	int sock_len = sizeof(struct sockaddr_in);
	int iCount = 0, iIndex = 0,iIndex1=0;

	while(!globalstop)
	{
		if(recvfrom(iSocketId, &tmp_struct, sizeof(tmp_struct), 0, (struct sockaddr *)&src_address, &sock_len) == -1)
		{
			continue;
			//printf("Receive failed \n");
		}
#if DEBUG
		printf("RThread : tmp_struct.data.chBuffer = %s\n", tmp_struct.data.chBuffer);
		printf("RThread : tmp_struct.iMsgSeqNo = %d\n", tmp_struct.iMsgSeqNo);
#endif
#if TEST
		if(dropMessage(DROP_VALUE))
			continue;
#endif
		if(tmp_struct.data.Type == 0)
		{
			pthread_mutex_lock(&sync_mutex);
			memcpy(&recv_msg[iRecMsgCount], &tmp_struct, sizeof(tmp_struct));
#if DEBUG
			printf("RThread DATA Section: recv_msg.data.chBuffer = %s\n", recv_msg->data.chBuffer);
#endif
			src_address.sin_addr.s_addr = inet_addr(inet_ntoa(src_address.sin_addr));
			src_address.sin_port = src_address.sin_port;
#if DEBUG
			printf("%s:%d\n",inet_ntoa(src_address.sin_addr),ntohs(src_address.sin_port));
#endif
			tmp_struct.data.Type=1;  /* send ACK */
			sendto(iSocketId, &tmp_struct, sizeof(tmp_struct), 0, (struct sockaddr *)&src_address, sock_len);
			iRecMsgCount ++;
			pthread_mutex_unlock(&sync_mutex);
		}

		else if(tmp_struct.data.Type == 1)
		{
//#if DEBUG
			printf("RThread ACK Section: ACK received\n");
			printf("RThread ACK Section: tmp mes seq no = %d\n",tmp_struct.iMsgSeqNo);
//#endif
			tmp_struct.data.Type = 0; /* made memcpy easy*/
			pthread_mutex_lock(&sync_mutex);
			for(iIndex = 0; iIndex < iUnackMsgCount; iIndex++)
			{
				if(tmp_struct.iMsgSeqNo == unack_msg[iIndex].iMsgSeqNo)
				{
					for(iIndex1 = iIndex; iIndex1 < iUnackMsgCount; iIndex1 ++)
					{
						memcpy(&unack_msg[iIndex1], &unack_msg[iIndex1+1], sizeof(unack_msg[iIndex1]));
					}
					iUnackMsgCount--;
#if DEBUG
					printf("RThread ACK Section: iUnackMsgCount = %d\n",iUnackMsgCount);
#endif
					break;
				}
			}
			pthread_mutex_unlock(&sync_mutex);
		}
	}
	pthread_exit(NULL);
}

/******************************************************************************
*
* @ function	: SThread
* @ Parameter	: void *arg - use to store socketId
* @ Return	: void *
* @ Description	: Check for message time and resend it. 
*		   
******************************************************************************/

void* SThread(void *arg)
{
	int iSocketId = ((int )arg);
	struct timeval tv;
	int iIndex = 0,iRetStatus = 0, iCount = 0;
	struct msg_struct tmp_struct;	

	while(!globalstop)
	{
		gettimeofday(&tv,NULL);
		pthread_mutex_lock(&sync_mutex);
		for(iIndex=0; iIndex < iUnackMsgCount; iIndex++)
		{
			if((tv.tv_sec - unack_msg[iIndex].tv.tv_sec) > (2*T))
			{
//#if DEBUG
				//printf("Time out\n");
//#endif					
				memcpy(&tmp_struct, &unack_msg[iIndex], sizeof(unack_msg[iIndex]));
				iRetStatus = sendto(iSocketId, &tmp_struct, sizeof(tmp_struct), 0, 
						(struct sockaddr *)&tmp_struct.address, sizeof(tmp_struct.address));
				gettimeofday(&unack_msg[iIndex].tv,NULL);
				iCount ++;
				printf("SThread : Timeout occured for tmp_struct.data.chBuffer = %s and Retransmission Count = %d\n", tmp_struct.data.chBuffer,iCount);
			}
		}
		pthread_mutex_unlock(&sync_mutex);
		sleep(T);
	}
	printf("iCount = %d\n",iCount);
	pthread_exit(NULL);
}

/******************************************************************************
*
* @ function	: SThread
* @ Parameter	: float p: any number float number
* @ Return	: int : 0 if p < random number
*			Otherwise, 1
* @ Description	: Used fr testing.
*		   
******************************************************************************/

int dropMessage(float p)
{ 
	float tmp;
	time_t tmp_time;
//	srand(time(&tmp_time));
	srand(time(NULL));
	tmp=(float)(rand()%100)/100;

	if(tmp < p)
		return 1; 
	else	
		return 0;

}
