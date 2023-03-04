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
    // struct client{
    //     int port_num;
    //     int sck_fd;

    //     struct client *next;
    // };
    // Maintain a list of connected clients
    // struct client *head = NULL;

    int connected_count = 0;
    int *client_fd;
    int *client_port;
    //  Memory allocates dynamically using malloc()
    client_fd = (int*)malloc(100 * sizeof(int));
    client_port = (int*)malloc(100 * sizeof(int));
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
                            listing(client_fd, client_port, connected_count);
                        }else if (strcmp("EXIT\n", cmd) == 0){
                            exit(EXIT_SUCCESS);
                        }

                        printf("\nI got: %s\n", cmd);

                        free(cmd);
                    }
                        /* Check if new client is requesting connection */
                    else if(sock_index == server_socket){
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                        if(fdaccept < 0)
                            perror("Accept failed.");

                        printf("\nRemote Host connected!\n");

                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if(fdaccept > head_socket) head_socket = fdaccept;

                        // add to array storage
                        client_fd[connected_count] = fdaccept;
                        struct sockaddr_in client_addr;
                        socklen_t len = sizeof(client_addr);                   
                        if (getpeername(fdaccept, (struct sockaddr *)&client_addr, &len) == 0){
                            printf("getpeername success\n");
                            printf(ntohs(client_addr.sin_port));
                            client_port[connected_count] = ntohs(client_addr.sin_port);
                        }
                        // printf("Client fd: ", client_fd, "\n");
                        // printf("Client Ports: ", client_port, "\n");
                        connected_count += 1;

                        // // add to linked list storage   
                        // struct sockaddr_in client_addr;
                        // socklen_t len;                   
                        // if (getpeername(fdaccept, (struct sockaddr *)&client_addr, &len) == 0){
                        //     if (head == NULL){
                        //         head -> port_num = ntohs(client_addr.sin_port);
                        //         head -> sck_fd = fdaccept;
                        //     }else{
                        //         struct client* new_client = (struct client*) malloc(sizeof(struct client));
                        //         new_clients -> port_num = ntohs(client_addr.sin_port);
                        //         new_clients -> sck_fd = fdaccept;                                
                        //         sortedInsert(clients, new_client)
                        //     }
                        // }else{
                        //     perror("getpeername");
                        // }                                                    
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
                            printf("ECHOing it back to the remote host ... ");
                            if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
                                printf("Done!\n");
                            fflush(stdout);
                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    return 0;
}






