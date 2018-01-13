#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"

#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256


int highestPort = -1;

int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, size_t hostlen,
                char *serv, size_t servlen, int flags);

void addToServerList(char host[200], int clientlistenport, int s, clientList** head)
{
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port;

	len = sizeof addr;
	getpeername(s, (struct sockaddr*)&addr, &len);

	// deal with both IPv4 and IPv6:
	if (addr.ss_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		//getnameinfo(&sadd, sizeof sadd, host, sizeof host, service, sizeof service, 0);
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	} else { // AF_INET6
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = ntohs(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
	}
	
	clientList *new = (clientList*)malloc(sizeof(clientList));
	clientList *current = *head;
	new->id = s;
	strncpy(new->IPaddress,ipstr, strlen(ipstr));
	strncpy(new->hostname, host, strlen(host));
	new->port = clientlistenport;
	new->isBlocked = 0;
	new->logStatus = 1;
	new->next = NULL;
	
	if(*head == NULL || (*head)->port > new->port) 
	{
		new->next = *head;
		*head = new;
	}
	else
	{
		while(current->next != NULL && current->next->port < new->port)
		{
			current = current->next;
		}
		new->next = current->next;
		current->next = new;
	}
	printf("Peer IP address: %s\n", ipstr);
	printf("Peer Listening port      : %d\n", clientlistenport);
	printf("Host Name      : %s\n", host);
}


void sendToIP(char ipaddress[256], clientList** list, char messageToSend[512])
{
	clientList *temp = *list;
	while(temp != NULL)
	{
		if(strcmp(temp->IPaddress, ipaddress) == 0)
		{
			if(send(temp->id, messageToSend, strlen(messageToSend), 0) == strlen(messageToSend))
				printf("Done!\n");
			fflush(stdout);
		}
		temp = temp->next;
	}
}

void displayPeerList(clientList** list)
{
	clientList *temp = *list;
	int list_id = 1;
	while(temp != NULL)
	{
		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, temp->hostname, temp->IPaddress, temp->port);
		/*
		printf("Peer IP address: %s\n", temp->IPaddress);
		printf("Peer port      : %d\n", temp->port);
		printf("Host Name      : %s\n", temp->hostname);
		 */
		list_id += 1;
		temp = temp->next;
	}
}

void displayBlockedList(char ip[100], clientList** list)
{
	clientList *temp = *list;
	int list_id = 1;
	while(temp != NULL)
	{
		if(temp->isBlocked == 1 && strcmp(temp->blockedby, ip) == 0)
		{
			cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, temp->hostname, temp->IPaddress, temp->port);
			/*
			printf("Peer IP address: %s\n", temp->IPaddress);
			printf("Peer port      : %d\n", temp->port);
			printf("Host Name      : %s\n", temp->hostname);
			 */
			list_id += 1;
		}
		temp = temp->next;
	}
}

void sendServerList(int socket, clientList** list)
{
	clientList *temp = *list;
	char buff[100];
	char listc[1000];
	memset(listc, 0, sizeof(listc));
	memset(buff, 0, sizeof(buff));
	while(temp != NULL)
	{
		printf("%d \n", temp->port);

		sprintf(buff, "%d-%s-%s+",temp->port,temp->IPaddress, temp->hostname);
		strcat(listc, buff);
		fflush(stdout);
		temp = temp->next;
	}
	if(send(socket, listc, strlen(listc), 0) == strlen(listc))
				printf("Done!\n");
}

void addToClientList(char buffer[256], peerSideList** head)
{
	// code taken from stack overflow https://stackoverflow.com/questions/4693884/nested-strtok-function-problem-in-c
	  
	  char *line;
	  char *token;
	  char buf[256];
	  for (line = strtok (buffer, "+"); line != NULL;
		   line = strtok (line + strlen (line) + 1, "+"))
		{
		  strncpy (buf, line, sizeof (buf));
		  int i = 1;
		  peerSideList *last = *head;
		  peerSideList *new = (peerSideList*)malloc(sizeof(peerSideList));
		  
		  for (token = strtok (buf, "-"); token != NULL;
		   token = strtok (token + strlen (token) + 1, "-"))
			{
				if(i == 1)
				{
					new->port = atoi(token);
					i += 1;
				}
				else if(i == 2)
				{
					memset(new->IPaddress, 0, 100);
					strncpy(new->IPaddress, token, strlen(token));
					i += 1;
				}
				else if(i == 3)
				{
					memset(new->hostname, 0, 256);
					strncpy(new->hostname, token, strlen(token));
					i += 1;
				}
			}
			new->isBlocked = 0;
			printf("\nPort is %d\n", new->port);
			printf("\nIP address %s\n", new->IPaddress);
			printf("\nHost Is %s\n", new->hostname);
			new->next = NULL;
			if(*head == NULL) 
			{
				*head = new;
			}
			else
			{
				while(last->next != NULL)
				{
					last = last->next;
				}
				last->next = new;
			}	
		}		
}

//code referred from geeks for geeks  http://www.geeksforgeeks.org/write-a-function-to-delete-a-linked-list/

void freeMyList(peerSideList** head_ref)
{
   /* deref head_ref to get the real head */
   peerSideList* current = *head_ref;
   peerSideList* next;
 
   while (current != NULL) 
   {
       next = current->next;
       free(current);
       current = next;
   }
   
   /* deref head_ref to affect the real head back
      in the caller. */
   *head_ref = NULL;
}


int isInClientList(char *IPaddress, peerSideList** head)
{
	printf("\nInside is client list function\n");
	peerSideList *temp = *head;
	while(temp != NULL)
	{	
		printf("string 1: %s string 2: %s\n", temp->IPaddress, IPaddress);
		printf("comparison %d\n", strcmp(temp->IPaddress, IPaddress));
		if(strcmp(temp->IPaddress, IPaddress) == 0)
		{
			printf("I am returned 1\n");
			return 1;
		}
		temp = temp->next;
	}
	printf("I am returned 0\n");
	return 0;
}

int isAlreadyBlocked(char *IPaddress, peerSideList** head)
{
	peerSideList *temp = *head;
	while(temp != NULL)
	{	
		printf("string 1: %s string 2: %s\n", temp->IPaddress, IPaddress);
		printf("comparison %d\n", strcmp(temp->IPaddress, IPaddress));
		if(strcmp(temp->IPaddress, IPaddress) == 0)
		{
			if(temp->isBlocked == 1)
			return 1;
		}
		temp = temp->next;
	}
	return 0;
}

void changeBlockStatus(int block, char *IPaddress, peerSideList** head)
{
	peerSideList *temp = *head;
	while(temp != NULL)
	{	
		printf("string 1: %s string 2: %s\n", temp->IPaddress, IPaddress);
		printf("comparison %d\n", strcmp(temp->IPaddress, IPaddress));
		if(strcmp(temp->IPaddress, IPaddress) == 0)
		{
			temp->isBlocked = block;
		}
		temp = temp->next;
	}
}

int isInServerList(char *IPaddress, clientList** head)
{
	printf("\nInside is client list function\n");
	clientList *temp = *head;
	while(temp != NULL)
	{	
		printf("string 1: %s string 2: %s\n", temp->IPaddress, IPaddress);
		printf("comparison %d\n", strcmp(temp->IPaddress, IPaddress));
		if(strcmp(temp->IPaddress, IPaddress) == 0)
		{
			printf("I am returned 1\n");
			return 1;
		}
		temp = temp->next;
	}
	printf("I am returned 0\n");
	return 0;
}

