/**
 * @client
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
 * This file contains the client.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"

#define TRUE 1
#define MSG_SIZE 512
#define BUFFER_SIZE 256
#define STDIN 0

int loginstatus = 0;

int isValidPort(char *port);
void changeBlockStatus(int block, char *IPaddress, peerSideList** head);
int isAlreadyBlocked(char *IPaddress, peerSideList** head);
void freeMyList(peerSideList** head_ref);
int connect_to_host(char *server_ip, int myport, int server_port);
void addToClientList(char buffer[256], peerSideList** head);
void printIP();
int propSend(char *message, int file_des);
char* propReceive(int file_des);
int isInClientList(char IPAddress[50], peerSideList** head);
int isValidIP(char *ip_str);

 /**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int mainClient(int port)
{
	char* ubitname = "shadulla";
	int server;
	peerSideList *myClientList = NULL;
	//server = connect_to_host(argv[1], atoi(argv[2]));
	//server = connect_to_host("localhost", port);
	int head_socket, selret, sock_index, fdaccept=0, caddr_len;
	fd_set master_list, watch_list;
	FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    FD_SET(STDIN, &master_list);
	head_socket = STDIN;
	while(TRUE){
		memcpy(&watch_list, &master_list, sizeof(master_list));
		
		printf("\n[PA1-Client@CSE489/589]$ ");
		fflush(stdout);
		selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");
            
        if(selret > 0)
        {
			for(sock_index=0; sock_index<=head_socket; sock_index+=1){
				
					if(FD_ISSET(sock_index, &watch_list))
					{
							if (sock_index == STDIN) {
									char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
									memset(msg, '\0', MSG_SIZE);
									if(fgets(msg, MSG_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to msg
										exit(-1);
									if(msg[strlen(msg)-1] == '\n')
									{
										msg[strlen(msg)-1] = '\0';
									}
									printf("I got: %s (size:%d chars) \n", msg, strlen(msg));
									char *messageToSend = (char*) malloc(sizeof(char)*MSG_SIZE);
									memset(messageToSend, '\0', MSG_SIZE);
									strncpy(messageToSend, msg, strlen(msg));
									printf("message at client side: %s \n", messageToSend);
									if(strlen(msg) == 0)
									{
										continue;
									}
									
									char* token = strtok(msg, " ");
									int argc = 0;
									char* argv[1000];
									memset(argv, 0, sizeof(argv));
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
										peerSideList *temp = myClientList;
										int list_id = 1;
										while(temp != NULL)
										{
											cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, temp->hostname, temp->IPaddress, temp->port);
											temp = temp->next;
											list_id += 1;
										}
										cse4589_print_and_log("[%s:END]\n", argv[0]);
									}
									else if(strcmp(argv[0], "LOGIN") == 0)
									{
										cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
										cse4589_print_and_log("[%s:END]\n", argv[0]);
										char *cpy = (char*) malloc(sizeof(char)*strlen(argv[1]));
										strncpy(cpy, argv[1], strlen(argv[1]));
										if(isValidIP(cpy) && isValidPort(argv[2]))
										{ 
											server = connect_to_host(argv[1], port, atoi(argv[2]));
											char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
											memset(buffer, '\0', BUFFER_SIZE);
											myClientList == NULL;
											
											if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
												printf("Server responded: %s", buffer);
												loginstatus = 1;
												addToClientList(buffer, &myClientList);
												fflush(stdout);
											} 
											FD_SET(server, &master_list);
											if(server > head_socket) head_socket = server;
										}
										else {
											cse4589_print_and_log("[%s:ERROR]\n", argv[0]);
											cse4589_print_and_log("[%s:END]\n", argv[0]);
										} 
									}
									else if(strcmp(argv[0], "BROADCAST") == 0)
									{
										if(send(server, messageToSend, strlen(messageToSend), 0) == strlen(messageToSend))
											printf("Done!\n");
										fflush(stdout);
									} 
									else if(strcmp(argv[0], "SEND") == 0)
									{
										char *cpy = (char*) malloc(sizeof(char)*strlen(argv[1]));
										strncpy(cpy, argv[1], strlen(argv[1]));
										if(isValidIP(cpy) && isInClientList(argv[1], &myClientList))
										{
											if(send(server, messageToSend, strlen(messageToSend), 0) == strlen(messageToSend))
											printf("Done!\n");
											
											cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
											cse4589_print_and_log("[%s:END]\n", argv[0]);
										}
										else 
										{
											cse4589_print_and_log("[%s:ERROR]\n", argv[0]);
											cse4589_print_and_log("[%s:END]\n", argv[0]);
										}
										fflush(stdout); 
									} 
									else if(strcmp(argv[0], "R") == 0)
									{
										char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
										memset(buffer, '\0', BUFFER_SIZE);
										if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
											printf("Server responded: %s", buffer);
											loginstatus = 1;
											addToClientList(buffer, &myClientList);
											fflush(stdout);
										} 
									}
									else if(strcmp(argv[0], "REFRESH") == 0)
									{
										if(send(server, messageToSend, strlen(messageToSend), 0) == strlen(messageToSend))
											printf("Done!\n");
										char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
										memset(buffer, '\0', BUFFER_SIZE);
										
										if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
											printf("Server responded: %s", buffer);
											freeMyList(&myClientList);
											if(myClientList != NULL)
											{
												printf("\nList is not empty\n");
											}
											addToClientList(buffer, &myClientList);
											fflush(stdout);
										} 
									}
									else if(strcmp(argv[0], "BLOCK") == 0)
									{
										char *cpy = (char*) malloc(sizeof(char)*strlen(argv[1]));
										strncpy(cpy, argv[1], strlen(argv[1]));
										if(isValidIP(cpy) && isInClientList(argv[1], &myClientList) && isAlreadyBlocked(argv[1], &myClientList) == 0)
										{
											if(send(server, messageToSend, strlen(messageToSend), 0) == strlen(messageToSend))
												printf("Done!\n");
											changeBlockStatus(1, argv[1], &myClientList);
											cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
											cse4589_print_and_log("[%s:END]\n", argv[0]);
									   }
									   else
									   {
										   cse4589_print_and_log("[%s:ERROR]\n", argv[0]);
										   cse4589_print_and_log("[%s:END]\n", argv[0]);
									   }
									}
									else if(strcmp(argv[0], "UNBLOCK") == 0)
									{
										char *cpy = (char*) malloc(sizeof(char)*strlen(argv[1]));
										strncpy(cpy, argv[1], strlen(argv[1]));
										if(isValidIP(cpy) && isInClientList(argv[1], &myClientList) && isAlreadyBlocked(argv[1], &myClientList) == 1)
										{
											if(send(server, messageToSend, strlen(messageToSend), 0) == strlen(messageToSend))
												printf("Done!\n");
											changeBlockStatus(0, argv[1], &myClientList);
											cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
											cse4589_print_and_log("[%s:END]\n", argv[0]);
										}
										else
										{
										   cse4589_print_and_log("[%s:ERROR]\n", argv[0]);
										   cse4589_print_and_log("[%s:END]\n", argv[0]);
										}
										
									}
									
							}
							else
							{
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
									//Process incoming data from existing server here ...
									char delimiter[] = " ";
									char *firstWord, *secondWord, *remainder, *context;
									printf("\nServer sent me: %s\n", buffer);
									firstWord = strtok_r (buffer, delimiter, &context);
									secondWord = strtok_r (NULL, delimiter, &context);
									remainder = context;
									printf("first %s\n", firstWord);
									printf("second %s\n", secondWord);
									printf("remainder %s\n", remainder);
									cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
									cse4589_print_and_log("msg from:%s\n[msg]:%s\n", secondWord, remainder);
									cse4589_print_and_log("[RECEIVED:END]\n"); 
									
								}

								free(buffer);
							}
				}
			}
		}
	}
}

int connect_to_host(char *server_ip, int myport, int server_port)
{
    int fdsocket, len;
    struct sockaddr_in remote_server_addr;

    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(fdsocket < 0)
       perror("Failed to create socket");

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(server_port);

    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
        perror("Connect failed");
    char portNo[100];
    memset(portNo, 0, sizeof(portNo));
    sprintf(portNo, "%d", myport);
    if(send(fdsocket, portNo, strlen(portNo), 0) == strlen(portNo))
			printf("Done!\n");
	fflush(stdout);
    cse4589_print_and_log((char *)"[%s:SUCCESS]\n", "LOGIN");
    cse4589_print_and_log((char *)"[%s:END]\n", "LOGIN");

    return fdsocket;
}


int valid_digit(char *ip_str)
{
	//Code for valid IP address taken from geeksforgeeks http://www.geeksforgeeks.org/program-to-validate-an-ip-address/
    while (*ip_str) {
        if (*ip_str >= '0' && *ip_str <= '9')
            ++ip_str;
        else
            return 0;
    }
    return 1;
}

int isValidIP(char *ip_str)
{
	//Code for valid IP address taken from geeksforgeeks http://www.geeksforgeeks.org/program-to-validate-an-ip-address/
    int i, num, dots = 0;
    char *ptr;
	printf("\ninside is valid\n");
    if (ip_str == NULL)
        return 0;
 
    // See following link for strtok()
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/strtok_r.html
    ptr = strtok(ip_str, ".");
 
    if (ptr == NULL)
        return 0;
 
    while (ptr) {
 
        /* after parsing string, it must contain only digits */
        if (!valid_digit(ptr))
            return 0;
 
        num = atoi(ptr);
 
        /* check for valid IP */
        if (num >= 0 && num <= 255) {
            /* parse remaining string */
            ptr = strtok(NULL, ".");
            if (ptr != NULL)
                ++dots;
        } else
            return 0;
    }
 
    /* valid IP string must contain 3 dots */
    if (dots != 3)
        return 0;
    return 1;
}

int isValidPort(char *port)
{
	 while (*port) {
        if (isdigit(*port++) == 0) return 0;
    }
    return 1;
}


