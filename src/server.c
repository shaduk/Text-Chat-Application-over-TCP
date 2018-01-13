/**
 * @server
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This file contains the server init and main while loop for tha application.
 * Uses the select() API to multiplex between network I/O and STDIN.
 */
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>


#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 512
#define MSG_SIZE 512

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"

int isValidIP(char *ip_str);
int isInServerList(char *IPaddress, clientList** head);
void displayBlockedList(char ip[100], clientList** list);
void sendToIP(char ipaddress[256], clientList** list, char messageToSend[256]);
void addToServerList(char host[200], int clientlistenport, int s, clientList** list);
void displayPeerList(clientList** list);
const char *inet_ntop(int af, const void *src,
                      char *dst, socklen_t size);


void sendServerList(int socket, clientList** list);
void printIP();

 /**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int mainServer(int port)
{

	int server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
	struct sockaddr_in server_addr, client_addr;
	fd_set master_list, watch_list;
	char *ubitname = "shadulla";
	clientList *myServerList = NULL;

	/* Socket */
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
		perror("Cannot create socket");

	/* Fill up sockaddr_in struct */
	bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    /* Bind */
    if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
    	perror("Bind failed");

    /* Listen */
    if(listen(server_socket, BACKLOG) < 0)
    	perror("Unable to listen on port");

    /* ---------------------------------------------------------------------------- */

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    
    /* Register the listening socket */
    FD_SET(server_socket, &master_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);

    head_socket = server_socket;

    while(TRUE){
        memcpy(&watch_list, &master_list, sizeof(master_list));

        printf("\n[PA1-Server@CSE489/589]$ ");
		fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if(selret > 0){
            /* Loop through socket descriptors to check which ones are ready */
            for(sock_index=0; sock_index<=head_socket; sock_index+=1){

                if(FD_ISSET(sock_index, &watch_list)){

                    /* Check if new command on STDIN */
                    if (sock_index == STDIN){
                    	char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);

                    	memset(cmd, '\0', CMD_SIZE);
						if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
							exit(-1);
							
						if(cmd[strlen(cmd)-1] == '\n')
						{
							cmd[strlen(cmd)-1] = '\0';
						}

						printf("\nI got: %s\n", cmd);
						
                   
						//Process PA1 commands here ...
                        char* token = strtok(cmd, " ");
						int argc = 0;
						char* argv[1000];
						//parse cmd and args from input
						while(token) {
							argv[argc] = malloc(strlen(token) + 1);
							strcpy(argv[argc], token);
							argc += 1;
							token = strtok(NULL, " ");
						}
						
						
						if(strcmp(argv[0], "AUTHOR") == 0)
						{
							cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
							cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", ubitname);
							cse4589_print_and_log("[%s:END]\n", argv[0]);
						} 
						else if(strcmp(argv[0], "IP") == 0)
						{
							cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
							printIP();
							cse4589_print_and_log("[%s:END]\n", argv[0]);
						} 
						else if (strcmp(argv[0], "PORT") == 0)
						{
							cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
							cse4589_print_and_log("PORT:%d\n", port);
							cse4589_print_and_log("[%s:END]\n", argv[0]);
						}
						else if(strcmp(argv[0], "LIST") == 0)
						{
							cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
							displayPeerList(&myServerList);
							cse4589_print_and_log("[%s:END]\n", argv[0]);
						} 
						else if(strcmp(argv[0], "BLOCKED") == 0)
						{
								char *cpy = (char*) malloc(sizeof(char)*strlen(argv[1]));
								strncpy(cpy, argv[1], strlen(argv[1]));
								if(isValidIP(cpy) && isInServerList(argv[1], &myServerList))
								{
									cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
									displayBlockedList(argv[1], &myServerList);
									cse4589_print_and_log("[%s:END]\n", argv[0]);
								}
								else
								{
									cse4589_print_and_log("[%s:ERROR]\n", argv[0]);
									cse4589_print_and_log("[%s:END]\n", argv[0]);
								}
						}
						
						free(cmd);
                    }
                    /* Check if new client is requesting connection */
                    else if(sock_index == server_socket){
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                        if(fdaccept < 0)
                            perror("Accept failed.");

						printf("\nRemote Host connected!\n");    
						char host[1024];
						char service[20];
						// pretend sa is full of good information about the host and port...

						getnameinfo(&client_addr, sizeof client_addr, host, sizeof host, service, sizeof service, 0);
						char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);
                        
						if(recv(fdaccept, buffer, BUFFER_SIZE, 0) >= 0)
						{
							printf("Client sent me listen port %s \n", buffer);
						}
						int clientListenPort =  atoi(buffer);
						addToServerList(host, clientListenPort, fdaccept, &myServerList);
						sendServerList(fdaccept, &myServerList);
						
                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if(fdaccept > head_socket) head_socket = fdaccept;
                    }
                    /* Read from existing clients */
                    else{
                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                            close(sock_index);
                            printf("Remote Host terminated connection!\n");
							
                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else {
                        	//Process incoming data from existing clients here ...

                        	printf("\nClient sent me: %s\n", buffer);
                        	int isBlocked = 0;
                        	char fromclientip[50];
                        	memset(fromclientip, 0, sizeof fromclientip);
                        	clientList *temp = myServerList;
                        	
							while(temp != NULL)
							{
								printf("I am here in temp\n");
								if(temp->id == sock_index)
								{
									strncpy(fromclientip, temp->IPaddress, sizeof(temp->IPaddress));
									isBlocked = temp->isBlocked;
								}
								temp = temp->next;
							}
							free(temp);
                        	char *messageToSend = (char*) calloc(strlen(buffer) + 1, sizeof(char));;
							strncpy(messageToSend, buffer, strlen(buffer));
                      		char* token = strtok(buffer, " ");
							
							char* argv[300];
							int argc = 0;
							//parse cmd and args from input
							while(token) {
								argv[argc] = malloc(strlen(token) + 1);
								strcpy(argv[argc], token);
								argc += 1;
								token = strtok(NULL, " ");
							}
							
							if(strcmp(argv[0], "BROADCAST") == 0)
							{
								char forwardMessage[256];
								memset(forwardMessage, 0, sizeof forwardMessage);
								printf("ECHOing it back to the remote host and other connected devices... ");
							
								char delimiter[] = " ";
								char *firstWord, *remainder, *context;
								firstWord = strtok_r (messageToSend, delimiter, &context);
								remainder = context;
								printf("\nMessage for broadcast1 : %s", remainder);
								strcat(forwardMessage, "RCV ");
								strcat(forwardMessage, fromclientip);
								strcat(forwardMessage, " ");
								strcat(forwardMessage, remainder); 
								for(int i = 0; i <= head_socket; i++) 
								{
									if(FD_ISSET(i, &master_list))
									{
										if(i != 0 && i != server_socket && i != sock_index)
										{
											printf("%d", i);
											if(send(i, forwardMessage, strlen(forwardMessage), 0) == strlen(forwardMessage))
											{
												printf("Done!\n");
												cse4589_print_and_log("[RELAYED:SUCCESS]\n");
												cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", fromclientip, "255.255.255.255", remainder);
												cse4589_print_and_log("[RELAYED:END]\n");
											}
												fflush(stdout);
										}	
																	
									}
								} 
							}
							else if(strcmp(argv[0], "SEND") == 0)
							{
								char forwardMessage[256];
								memset(forwardMessage, 0, sizeof forwardMessage);
								//example from stack overflow https://stackoverflow.com/questions/1556616/how-to-extract-words-from-a-sentence-efficiently-in-c
								if(isBlocked == 0)
								{
									char delimiter[] = " ";
									printf("whole message: %s\n", messageToSend);
									char *firstWord, *secondWord, *remainder, *context;
									firstWord = strtok_r (messageToSend, delimiter, &context);
									secondWord = strtok_r (NULL, delimiter, &context);
									remainder = context;
									strcat(forwardMessage, "RCV ");
									strcat(forwardMessage, fromclientip);
									strcat(forwardMessage, " ");
									strcat(forwardMessage, remainder);
									cse4589_print_and_log("[RELAYED:SUCCESS]\n");
									cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", fromclientip, secondWord, remainder);
									cse4589_print_and_log("[RELAYED:END]\n");
									sendToIP(secondWord, &myServerList, forwardMessage);
								}
								
							}
							else if(strcmp(argv[0], "REFRESH") == 0)
							{
								clientList *temp = myServerList;
								while(temp != NULL)
								{
									if(temp->id == sock_index)
									{
										sendServerList(sock_index, &myServerList);
										break;
									}
									temp = temp->next;
								}
							}
							else if(strcmp(argv[0], "BLOCK") == 0)
							{
								printf("I am here in block1\n");
								char delimiter[] = " ";
								char *firstWord, *remainder, *context;
								printf("I am here in block2\n");
								firstWord = strtok_r (messageToSend, delimiter, &context);
								printf("I am here in block3\n");
								remainder = context;
								printf("I am here in block4\n");
								clientList *temp1 = myServerList;
								printf("Blocked by %s\n", fromclientip);
								while(temp1 != NULL)
								{
									if(strcmp(temp1->IPaddress, remainder) == 0)
									{
										printf("This is true\n");
										strcpy(temp1->blockedby, fromclientip);
										printf("hence\n");
										temp1->isBlocked = 1;
									}
									temp1 = temp1->next;
								}
							}
							else if(strcmp(argv[0], "UNBLOCK") == 0)
							{
								char delimiter[] = " ";
								char *firstWord, *remainder, *context;
								firstWord = strtok_r (messageToSend, delimiter, &context);
								remainder = context;
								clientList *temp = myServerList;
								while(temp != NULL)
								{
									if(strcmp(temp->IPaddress, remainder) == 0)
									{
										temp->isBlocked = 0;
									}
									temp = temp->next;
								}
							}
                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    return 0;
}

void printIP()
{
	struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];
	char hostname[128];
	gethostname(hostname, sizeof hostname);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    }

    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        cse4589_print_and_log("IP:%s\n", ipstr); 
    }

    freeaddrinfo(res); // free the linked list
}

