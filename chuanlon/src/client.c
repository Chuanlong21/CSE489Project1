//
// Created by chuanlon on 2023/2/22.
//

/**
* @client
* @author  Swetank Kumar Saha <swetankk@buffalo.edu>, Shivang Aggarwal <shivanga@buffalo.edu>
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
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/select.h>

#include "../include/common.h"
#include "../include/logger.h"

#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define STDIN 0
#define CMD_SIZE 100


int connect_to_host(char *server_ip, char *server_port);

/**
* main function
*
* @param  argc Number of arguments
* @param  argv The argument list
* @return 0 EXIT_SUCCESS
*/



int c_startUp(char *port)
{
//    if(argc != 3) {
//        printf("Usage:%s [ip] [port]\n", argv[0]);
//        exit(-1);
//    }

    int client_socket,head_socket, selret, sock_index, connection;
    fd_set master_list, watch_list;
    struct sockaddr_in addr;
    char ip_buff[16];
    int p;
    sscanf(port, "%d", &p);
    printf("p: %d\n", p);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("8.8.8.8");
    addr.sin_port = htons(p);

    //create socket and connect to the addr for IP address
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_socket < 0){
        error("IP");
    }

    if(bind(client_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0){
        perror("Bind failed");
    }

    connection = connect(client_socket, (struct sockaddr*) &addr,sizeof (addr));
    if (connection < 0){
        error("IP");
    }
    socklen_t len = sizeof(addr);
    getsockname(client_socket, (struct sockaddr*) &addr, &len);
    inet_ntop(AF_INET, &addr.sin_addr, ip_buff, 16);

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);
    head_socket = STDIN;


    while(TRUE){

        memcpy(&watch_list, &master_list, sizeof(master_list));

        //printf("\n[PA1-Server@CSE489/589]$ ");
        //fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");

        if(selret > 0){
            for(sock_index=0; sock_index<=head_socket; sock_index+=1){
                if(FD_ISSET(sock_index, &watch_list)){
                    /* Check if new command on STDIN */
                    if (sock_index == STDIN){
                        char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);
                        memset(cmd, '\0', CMD_SIZE);
                        cmd[strlen(cmd) - 1] = 0;
                        //COMMAND
                        if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
                            exit(-1);

                        //Process PA1 commands here ...
                        if (strstr(cmd,"LOGIN")){

                            char* rev[3];
                            int count = 0;
                            char *pNext = strtok(cmd, " ");

                            while (pNext != NULL) {
                                rev[count] = pNext;
                                ++count;
                                pNext = strtok(NULL, " ");
                            }

                            if (count == 3){
                                rev[2][strlen(rev[2])  -1 ] = 0;
                                if (IPv4_verify(rev[1]) == 1 && validNumber(rev[2]) == 1){
                                    //when we have login command, we will need to connect to the host sever
                                    int server;
                                    server = connect_to_host(rev[1], rev[2]);
                                    if (server < 0){
                                        error("LOGIN");
                                    }
                                }
                            } else error("LOGIN");

                        }else if(strcmp("PORT\n", cmd) == 0){
                            show_port(port);
                        }else if (strcmp("AUTHOR\n", cmd) == 0){
                            show_Author();
                        }else if (strcmp("IP\n", cmd) == 0){
                            cse4589_print_and_log("[%s:SUCCESS]\n", "IP");
                            cse4589_print_and_log("IP:%s\n", ip_buff);
                            cse4589_print_and_log("[%s:END]\n", "IP");
                        }else if (strcmp("EXIT\n", cmd) == 0){
                            //删除了之后 服务端也要把watch list里它的socket给删除
                            exit(EXIT_SUCCESS);
                        }

                        free(cmd);
                    }
                }
            }
        }

//        char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
//        memset(msg, '\0', MSG_SIZE);
//        if(fgets(msg, MSG_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to msg
//            exit(-1);
//
//        printf("I got: %s(size:%d chars)", msg, strlen(msg));
//
//        printf("\nSENDing it to the remote server ... ");
//        if(send(server, msg, strlen(msg), 0) == strlen(msg))
//            printf("Done!\n");
//        fflush(stdout);
//
//        /* Initialize buffer to receieve response */
//        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
//        memset(buffer, '\0', BUFFER_SIZE);
//
//        if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
//            printf("Server responded: %s", buffer);
//            fflush(stdout);
//        }
    }
}

int connect_to_host(char *server_ip, char* server_port)
{
    int fdsocket;
    struct addrinfo hints, *res;

    /* Set up hints structure */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    /* Fill up address structures */
    if (getaddrinfo(server_ip, server_port, &hints, &res) != 0)
        perror("getaddrinfo failed");

    /* Socket */
    fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(fdsocket < 0)
        perror("Failed to create socket");

    /* Connect */
    if(connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0)
        perror("Connect failed");

    freeaddrinfo(res);

    return fdsocket;
}



