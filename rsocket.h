#ifndef __RSOCKET_H
#define __RSOCKET_H

/*	HEADERS 	*/
#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

/*	MACROS		*/
#define TRUE	 	(0)
#define FALSE		(-1)
#define SOCK_BRP	(2) 	// same as SOCK_DGRAM from /usr/include/i386-linux-gnu/bits/socket.h 
#define MAX_BUFFER_SIZE	(1024)
#define PORT 		(50428)
#define T		(2)
#define NO_OF_MESSAGE	(50)

/*	STRUCTURE	*/
struct Data
{
	char Type;			/*o-data ; 1-ack*/
	char chBuffer[MAX_BUFFER_SIZE];
};

struct msg_struct {
	int iMsgSeqNo;
	struct Data data;
	struct sockaddr address;
	struct timeval tv;
};

/*	FUNCTION DECLARATION	*/
int r_socket(int , int, int);
int r_bind(int, struct sockaddr *,socklen_t);
ssize_t r_recvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *);
ssize_t r_sendto(int , const void *, size_t, int, struct sockaddr *, socklen_t);
int r_close(int);
void *RThread(void *);
void *SThread(void *);
int dropMessage(float );

#endif
