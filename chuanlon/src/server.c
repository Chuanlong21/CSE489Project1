//
// Created by chuanlon on 2023/2/22.
//
/**
* @server
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
* This file contains the server init and main while loop for tha application.
* Uses the select() API to multiplex between network I/O and STDIN.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>


#include <../include/logger.h>
#include <../include/common.h>

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256




/**
* main function
*
* @param  argc Number of arguments
* @param  argv The argument list
* @return 0 EXIT_SUCCESS
*/

int *client_port;
int compare (const void * a, const void * b);
void intArrToString(char* str,int count, int arr[]);
void remove_sck(int fds[100], int pts[100], int sck_idx, int count);

int s_startUp(char *port)
{
//    if(argc != 2) {
//        printf("Usage:%s [port]\n", argv[0]);
//        exit(-1);
//    }

    int server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
    struct sockaddr_in client_addr;
    struct addrinfo hints, *res;
    fd_set master_list, watch_list;
    char hostName[1024];

    // Initialization for storing client socket information
    struct client {
        int client_fd;
        char* IP;
        char** block_list;
    };

    struct client clientList[100];

    int connected_count = 0;
    int *client_fd;
    char *client_ip;
  
    //  Memory allocates dynamically using malloc()
    client_fd = (int*)malloc(100 * sizeof(int));
    client_port = (int*)malloc(100 * sizeof(int));
    client_ip = (char*)malloc(100 * sizeof(int));
    int sort_fd[100];

    memset(sort_fd,0,100);
    int *sorted_fd = sort_fd;
    // Checking for memory allocation
    if (client_fd == NULL) {
        printf("Client fd memory not allocated.\n");
    }
    if (client_port == NULL) {
        printf("Client port memory not allocated.\n");
    }

    /* Set up hints structure */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    gethostname(hostName,1024);
    /* Fill up address structures */
    if (getaddrinfo(hostName, port, &hints, &res) != 0)
        perror("getaddrinfo failed");

    /* Socket */
    server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(server_socket < 0)
        perror("Cannot create socket");

    /* Bind */
    if(bind(server_socket, res->ai_addr, res->ai_addrlen) < 0 )
        perror("Bind failed");

    freeaddrinfo(res);

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

        //printf("\n[PA1-Server@CSE489/589]$ ");
        //fflush(stdout);

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

                        //COMMAND
                        if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
                            exit(-1);

                        //Process PA1 commands here ...
                        if(strcmp("PORT\n", cmd) == 0){
                            show_port(port);
                        }else if (strcmp("AUTHOR\n", cmd) == 0){
                            show_Author();
                        }else if (strcmp("IP\n", cmd) == 0){
                            show_ip(server_socket);
                        }else if (strcmp("LIST\n", cmd) == 0){
                            cmd[strlen(cmd) - 1] ='\0';
                            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                            listing(sorted_fd, connected_count);
                            cse4589_print_and_log("[%s:END]\n", cmd);
                        }

                        free(cmd);
                    }
                        /* Check if new client is requesting connection */
                    else if(sock_index == server_socket){
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                        if(fdaccept < 0)
                            perror("Accept failed.");

//                        printf("\nRemote Host connected!\n");
//                        printf("fd: %d\n", fdaccept);

                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if(fdaccept > head_socket) head_socket = fdaccept;

                        // add to array storage
//                        printf("Client Count: %d\n", connected_count);
                        client_fd[connected_count] = fdaccept;
                        // printf(fdaccept);
                        // printf(client_fd[0]);
                        struct sockaddr_in client;
                        socklen_t len = sizeof(struct sockaddr_in);                   
                        if (getpeername(fdaccept, (struct sockaddr *)&client, &len) == 0){
//                            printf("first getpeername success\n");
                            client_port[connected_count] = ntohs(client_addr.sin_port);

                        }
                        int perm[connected_count + 1], i;

                        for (i = 0 ; i < connected_count + 1; i++) {
                            perm[i] = i;
                        }
                        qsort (perm, connected_count + 1, sizeof(int), compare);
                        for (i = 0 ; i < connected_count + 1; i++) {
                            sort_fd[i] = client_fd[perm[i]];
                        }

                        // add to client struct list
                        char c_ip[INET_ADDRSTRLEN]; // stores the client side IP address
                        
                        struct client c = {.client_fd = fdaccept, .IP = inet_ntop(AF_INET, &client.sin_addr, c_ip, sizeof(c_ip))};
                        clientList[connected_count] = c;
                        printf("added ip: %s\n", c_ip);

                        connected_count += 1;

                        // for (i = 0 ; i < connected_count; i++) {
                        //     printf("get IP for client %d\n", i);
                        //     struct sockaddr_in client;
                        //     socklen_t len = sizeof(struct sockaddr_in);                   
                        //     if (getpeername(client_fd[i], (struct sockaddr *)&client, &len) == 0){
                        //         printf("getpeername succeeded\n");
                        //         client_ip[i] = inet_ntoa(client.sin_addr);
                        //         printf("ip: %s\n", client_ip[i]);
                        //     }
                        // }
                        // printf("ip sorted\n");
                        // for (i = 0 ; i < connected_count; i++){
                        //     printf("i: \n", i);
                        //     printf(client_ip[i]);
                        // }
//                        printf("Updated Client Count: %d\n", connected_count);


                        client_list(fdaccept,sort_fd, connected_count);
                                                            
                    }
                        /* Read from existing clients */
                    else{
                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                            close(sock_index);
//                            printf("Remote Host terminated connection!\n");
                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
//                            printf("New Connected Count: %d\n", connected_count);
//                            for(int k=0; k < connected_count; k++){
//                                printf("Active Client fd: %d", sort_fd[k]);
//                                printf("  Port: %d\n", client_port[k]);
//
//                            }
                        }
                        else {
                            
                            //Process incoming data from existing clients here ...
                            if (strcmp("REFRESH",buffer) == 0){
//                                char str[connected_count * 2 + 1];
//                                intArrToString(str,connected_count,sort_fd);
//                                if(send(sock_index, str, strlen(str), 0) == strlen(str)){
//                                    printf("Done sending clients list!\n");
//                                    fflush(stdout);
//                                }
                                client_list(sock_index,sort_fd, connected_count);
                            }
//                            else if (strcmp("LIST",buffer) == 0){
//                                client_list(sock_index,sort_fd, connected_count);
//                            }
                            else if (strcmp("EXIT", buffer) == 0){
                                remove_sck(sort_fd, client_port, sock_index, connected_count);
                                connected_count -= 1;
                            }
                            else if(send_command(buffer) == 0){
                                // Parse received data
                                char* client_command;
                                char* ip_to_sent;
                                char* msg;
                                client_command = strtok(buffer, " ");
                                ip_to_sent =  strtok(NULL, " ");
                                printf("ip to sent: %s\n", ip_to_sent);
                                printf("size: %d\n", sizeof(ip_to_sent));
                                msg = strtok(NULL, " ");
                                printf("msg: %s\n", msg);

                                // Find fd based on ip to sent
                                int fd_to_sent;
                                printf("connected count: %d\n", connected_count);
                                for(int k=0; k < connected_count; k++){
                                    printf("index: %d\n", k);
                                    printf("current ip size: %s\n", sizeof(clientList[k].IP));
                                    if(strcmp(clientList[k].IP, ip_to_sent) == 0){
                                        printf("found matching ip%s\n", clientList[k].IP);
                                        fd_to_sent = clientList[k].client_fd;
                                        printf("fd found: %d\n", fd_to_sent);
                                        break;
                                    }
                                }
                                printf("ready to sent\n");
                                int sending_result = send(fd_to_sent, msg, strlen(msg), 0);

                                if(sending_result < 0){
                                    printf("sent failed\n");
                                }
                            }
//                            printf("\nClient sent me: %s\n", buffer);
//                            printf("ECHOing it back to the remote host ... ");
//                            if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
//                                printf("Done!\n");
//                            fflush(stdout);
                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    return 0;
}

int compare (const void * a, const void * b) {
    int diff = client_port[*(int*)a] - client_port[*(int*)b];
    return  (0 < diff) - (diff < 0);
}

void intArrToString(char* str,int count, int arr[]){
//    char str[count * 2 + 1];
    int x, y = 0;
    for (x = 0; x < count; x++) {
        y += sprintf(str + y, "%d ", arr[x]);
    }
}

int send_command(char* received){
    char substr[5];
    strncpy(substr, &received[0], 4);
    substr[4] = '\0';
    printf("command from client: %s\n", substr);
    return strcmp("SEND", substr);
}

void remove_sck(int fds[100], int pts[100], int sck_idx, int count){
    int pos = 0;
    for (int i = 0; i < count; i++){
        if (fds[i] == sck_idx){
            pos = i;
            break;
        }
    }
    for (int j = pos; j < count - 1; j++){
        fds[j] = fds[j + 1];
        pts[j] = pts[j + 1];
    }
}




