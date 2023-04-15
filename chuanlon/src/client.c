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
#include <unistd.h>

#include "../include/common.h"
#include "../include/logger.h"

#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define STDIN 0
#define CMD_SIZE 100
#define DES_SIZE 100


int connect_to_host(char *server_ip, char *server_port, char *port);

int stringToInt(int *arr, char *buff);

/**
* main function
*
* @param  argc Number of arguments
* @param  argv The argument list
* @return 0 EXIT_SUCCESS
*/



int c_startUp(char *port) {
//    if(argc != 3) {
//        printf("Usage:%s [ip] [port]\n", argv[0]);
//        exit(-1);
//    }

    int client_socket, head_socket, selret, sock_index;
    int server = -1;
    fd_set master_list, watch_list;
    struct addrinfo hints, *res;
    char hostName[1024];
    int *des = (int *) malloc(sizeof(int *) * DES_SIZE);
    int login = -1;

    /* Set up hints structure */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    gethostname(hostName, 1024);
    /* Fill up address structures */
    if (getaddrinfo(hostName, NULL, &hints, &res) != 0)
        perror("getaddrinfo failed");

    /* Socket */
    client_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (client_socket < 0)
        perror("Cannot create socket");

    /* Bind */
    if (bind(client_socket, res->ai_addr, res->ai_addrlen) < 0)
        perror("Bind failed");

    freeaddrinfo(res);

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);
    head_socket = STDIN;
    char *msg = (char *) malloc(1000 * sizeof(char));
    char *store = (char *) malloc(sizeof(char) * 1000);
    memset(store, '\0', 1000);
    int isStart = -1;
    int isEnd = -1;

    while (TRUE) {
        memcpy(&watch_list, &master_list, sizeof(master_list));

        //printf("\n[PA1-Server@CSE489/589]$ ");
        //fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if (selret < 0) {
            perror("select fail");
            exit(-1);
        } else {
            for (sock_index = 0; sock_index <= head_socket; sock_index += 1) {
                if (FD_ISSET(sock_index, &watch_list)) {
                    /* Check if new command on STDIN */
                    if (sock_index == STDIN) {
                        char *input = (char *) malloc(sizeof(char) * 1000);
                        memset(input, '\0', CMD_SIZE);
                        char *cmd = (char *) malloc(sizeof(char) * CMD_SIZE);
                        memset(cmd, '\0', CMD_SIZE);
                        if (fgets(input, 999, stdin) ==
                            NULL) //Mind the newline character that will be written to cmd
                        {
                            exit(-1);
                        }

                        char *rev[3];
                        int count = 0;
                        char *pNext = strtok(input, " ");
                        rev[count ++] = pNext;
                        pNext = strtok(NULL, " ");
                        if (pNext != NULL && count < 2) {
                            rev[count ++] = pNext;
                        }
                        pNext = strtok(NULL, "");
                        if (pNext != NULL) {
                            rev[count ++] = pNext;
                        }

                        if (count == 1) {
                            rev[0][strlen(rev[0]) - 1] = '\0';
                        }
                        strcpy(cmd, rev[0]);

//                        printf("right now %d\n", strcmp(cmd,"LOGIN"));
//                        Process PA1 commands here ...
                        if (strcmp(cmd, "LOGIN") == 0 && login == -1) {
                            if (count == 3) {
                                rev[2][strlen(rev[2]) - 1] = '\0';
                                printf("IP: %s\n", rev[1]);
                                printf("PORT: %s\n", rev[2]);
                                if (IPv4_verify(rev[1]) == 1 && validNumber(rev[2]) == 1) {
                                    //when we have login command, we will need to connect to the host sever
                                    server = connect_to_host(rev[1], rev[2], port);
                                    if (server > -1) {
                                        FD_SET(server, &master_list);
                                        if (server > head_socket) {
                                            head_socket = server;
                                        }
                                        login = 1;
                                        cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                                        cse4589_print_and_log("[%s:END]\n", cmd);
                                    } else {
                                        cse4589_print_and_log("[%s:ERROR]\n", cmd);
                                        cse4589_print_and_log("[%s:END]\n", cmd);
                                    }
//                                    char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                                    memset(msg, '\0', 1000);
                                    if (recv(server, msg, 1000, 0) >= 0) {
                                        fflush(stdout);
                                    }
//                                    if (stringToInt(des,buffer) < 0) error(rev[0]);
                                } else error(cmd);
                            } else error(cmd);
                        } else if (strcmp(cmd, "LOGIN") == 0 && login == 0) {
                            if (count == 3) {
                                rev[2][strlen(rev[2]) - 1] = '\0';
                                printf("IP: %s\n", rev[1]);
                                printf("PORT: %s\n", rev[2]);
                                if (IPv4_verify(rev[1]) == 1 && validNumber(rev[2]) == 1) {
                                    login = 1;
                                    cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                                    cse4589_print_and_log("[%s:END]\n", cmd);
                                    send(server,"RELOGIN", strlen("RELOGIN"), 0);
                                }
                            }
                        } else if (strcmp("PORT", cmd) == 0) {
                            show_port(port);
                        } else if (strcmp("AUTHOR", cmd) == 0) {
                            show_Author();
                        } else if (strcmp("IP", cmd) == 0) {
                            show_ip(client_socket);
                        } else if (strcmp("EXIT", cmd) == 0) {
                            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                            cse4589_print_and_log("[%s:END]\n", cmd);
                            exit(-1);
                        } else if (login == 1) {
                            if (strcmp("REFRESH", cmd) == 0) {
                                send(server, "REFRESH", strlen("REFRESH"), 0);
                                memset(msg, '\0', 1000);
                                if (recv(server, msg, 1000, 0) >= 0) {
                                    fflush(stdout);
                                }
//                                 printf("%s",msg);
                                cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                                cse4589_print_and_log("[%s:END]\n", cmd);

                            } else if (strcmp("EXIT", cmd) == 0) {
                                //删除了之后 服务端也要把watch list里它的socket给删除
                                if (server > 0) {
                                    send(server, "EXIT", strlen("EXIT"), 0);//发送exit给服务端，让他知道得把连接数组给删掉
                                    cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                                    cse4589_print_and_log("[%s:END]\n", cmd);
                                    close(client_socket);
                                    exit(0);
                                } else error(cmd);
                            } else if (strcmp("LIST", cmd) == 0) {
                                cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                                cse4589_print_and_log("%s", msg);
                                cse4589_print_and_log("[%s:END]\n", cmd);
                            } else if (strcmp("SEND", cmd) == 0) {
                                if (count == 3) {
                                    if (IPv4_verify(rev[1]) == 1 && strlen(rev[2]) <= 256) { // 257？
                                        rev[2][strlen(rev[2]) - 1] = '\0';
                                        char result[6 + strlen(rev[1]) + strlen(rev[2])];
                                        strcpy(result, "SEND ");
                                        strcat(result, rev[1]);
                                        strcat(result, " ");
                                        strcat(result, rev[2]);
                                        send(server, result, strlen(result), 0);
                                        //看一下是否send成功了, yes or no
                                        memset(msg, '\0', 1000);
                                        if (recv(server, msg, 1000, 0) >= 0) {
                                            fflush(stdout);
                                        }
                                        if (strcmp(msg, "YES") == 0) {
                                            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                                            cse4589_print_and_log("[%s:END]\n", cmd);
                                        } else error(cmd);
                                    } else error(cmd);
                                } else error(cmd);
                            } else if (strcmp("BROADCAST", cmd) == 0) {
                                rev[1][strlen(rev[1]) - 1] = '\0';
                                if (strlen(rev[1]) <= 256){
                                    char result[10 + strlen(rev[1])];
                                    strcpy(result, "BROADCAST ");
                                    strcat(result, rev[1]);
                                    send(server, result, strlen(result), 0);
                                } else error(cmd);
                            } else if (strcmp("BLOCK", cmd) == 0) {
                                if (count == 2) {
                                    rev[1][strlen(rev[1]) - 1] = '\0';
                                    if (IPv4_verify(rev[1]) == 1) {
                                        char result[6 + strlen(rev[1])];
                                        strcpy(result, "BLOCK ");
                                        strcat(result, rev[1]);
                                        send(server, result, strlen(result), 0);
                                        //看一下是否block成功了, yes or no
                                        memset(msg, '\0', 1000);
                                        if (recv(server, msg, 1000, 0) >= 0) {
                                            fflush(stdout);
                                        }
                                        if (strcmp(msg, "YES") == 0) {
                                            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                                            cse4589_print_and_log("[%s:END]\n", cmd);
                                        } else error(cmd);
                                    } else error(cmd);
                                } else error(cmd);
                            } else if (strcmp("UNBLOCK", cmd) == 0) {
                                if (count == 2) {
                                    rev[1][strlen(rev[1]) - 1] = '\0';
                                    if (IPv4_verify(rev[1]) == 1) {
                                        char result[8 + strlen(rev[1])];
                                        strcpy(result, "UNBLOCK ");
                                        strcat(result, rev[1]);
                                        send(server, result, strlen(result), 0);
                                        //看一下是否block成功了, yes or no
                                        memset(msg, '\0', 1000);
                                        if (recv(server, msg, 1000, 0) >= 0) {
                                            fflush(stdout);
                                        }
                                        if (strcmp(msg, "YES") == 0) {
                                            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                                            cse4589_print_and_log("[%s:END]\n", cmd);
                                        } else error(cmd);
                                    } else error(cmd);
                                } else error(cmd);
                            } else if (strcmp("LOGOUT", cmd) == 0) {
                                send(server, "LOGOUT", strlen("LOGOUT"), 0);
                                cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                                cse4589_print_and_log("[%s:END]\n", cmd);
                                login = 0;
                            }

                        } else {
                            error(cmd);
                        }
                        free(cmd);
                        free(input);
                    }
                    else if (sock_index == server){
                        size_t len_nbo;
                        recv(server, &len_nbo, sizeof(len_nbo), 0);
                        size_t len = ntohl(len_nbo);

                        char *buffer = malloc(len + 1);
                        memset(buffer, '\0', len + 1);
                        if (recv(server, buffer, len, 0) <= 0) {
                            close(server);
                        }
                        else {
                            buffer[len] = '\0';
                            printf("Received message: %s\n", buffer);

                            if (strstr(buffer, "msg: ") != NULL || strstr(buffer, "bro: ") != NULL) {
                                char *co[3];
                                int n = 0;
                                char *pN = strtok(buffer, " ");
                                co[n ++] = pN;
                                pN = strtok(NULL, " ");
                                if (pN != NULL && n < 2) {
                                    co[n ++] = pN;
                                }
                                pN = strtok(NULL, "");
                                if (pN != NULL) {
                                    co[n ++] = pN;
                                }
                                cse4589_print_and_log("[%s:SUCCESS]\n", "RECEIVED");
                                cse4589_print_and_log("msg from:%s\n[msg]:%s\n", co[1], co[2]);
                                cse4589_print_and_log("[%s:END]\n", "RECEIVED");
                            }
                        }
                        free(buffer);
                    }
                }
            }
        }

    }
}

int connect_to_host(char *server_ip, char *server_port, char *port) {
//    printf("connecting////");
    int fdsocket;
    struct sockaddr_in addr;
    struct addrinfo hints, *res;

    /* Set up hints structure */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(port));

    /* Fill up address structures */
    if (getaddrinfo(server_ip, server_port, &hints, &res) != 0)
        perror("getaddrinfo failed");

    /* Socket */
    fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fdsocket < 0) {
        perror("Failed to create socket");
        return -1;
    }

    if (bind(fdsocket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Bind fail");
        return -1;
    }
    /* Connect */
//    printf("%d\n", connect(fdsocket, res->ai_addr, res->ai_addrlen));
    if (connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Connect failed");
        return -1;
    }


    freeaddrinfo(res);
    return fdsocket;
}

int stringToInt(int *arr, char *buff) {
    int i = 0;
    char *token = strtok(buff, " ");
    while (token != NULL) {
        arr[i++] = atoi(token);
        token = strtok(NULL, " ");
    }
    return i;
}

