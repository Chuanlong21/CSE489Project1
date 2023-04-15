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
#include <arpa/inet.h>

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
int is_blocked(struct blocked* block_list, int block_count, char* client_ip);
void get_block_list(char* ip, struct client * c_lst, int connect_count);
int blocked_cmd(char* input);

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
    struct client clientList[100];
    for (int i = 0; i < 100; i++) {
        clientList[i].client_fd = -1; // or whatever default value you want
        clientList[i].IP = NULL;
        clientList[i].block_list = NULL;
        clientList[i].block_count = 0;
        clientList[i].status = 0;
        clientList[i].mSend = 0;
        clientList[i].mRev = 0;
        clientList[i].hostName = NULL;
        clientList[i].bufferList = NULL;
        clientList[i].buffer_count = 0;
    }

    // Maintain a list of connected clients
    // struct client *head = NULL;

    int connected_count = 0;
    int *client_fd;
  
    //  Memory allocates dynamically using malloc()
    client_fd = (int*)malloc(100 * sizeof(int));
    client_port = (int*)malloc(100 * sizeof(int));
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
                        }else if (blocked_cmd(cmd) == 0){
                            char* client_command;
                            char* client_ip;
                            client_command = strtok(cmd, " ");
                            client_ip = strtok(NULL, " ");
                            client_ip[strlen(client_ip) - 1] = '\0';
                            printf("Passed in client IP: %s\n", client_ip);
                            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                            get_block_list(client_ip, clientList, connected_count);
                            cse4589_print_and_log("[%s:END]\n", cmd);
                        }
//                        else if (strcmp("STATISTICS\n",cmd) == 0){
//                            for (int i = 0; i < connected_count; ++i) {
//                                char * logIN = "logged-in";
//                                if (clientList[i].status == 0){
//                                    logIN = "logged-out";
//                                }
//                                cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", i+1, hostname, clientList[i].mSend, clientList[i].mRev, logIN);
//                            }
//                        }
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
                        printf("fd for this client: %d\n", fdaccept);
                        char *c_ip = malloc(INET_ADDRSTRLEN); // stores the client side IP address
                        inet_ntop(AF_INET, &client.sin_addr, c_ip, INET_ADDRSTRLEN);
                        printf("ip for this client: %s\n", c_ip);

                        // Get hostname for client
                        char addr[sizeof(struct in_addr)];
                        inet_pton(AF_INET, inet_ntoa(client_addr.sin_addr), addr); 
                        struct hostent *gethost_rtval;
                        gethost_rtval = gethostbyaddr(&addr, sizeof(addr), AF_INET);
                        char * host_name = malloc((strlen(gethost_rtval->h_name) + 1)* sizeof(char));
                        strcpy(host_name, gethost_rtval->h_name);
                        struct blocked* newBlocked = malloc(sizeof (struct blocked) * 100);
                        printf("......printing the client list before updating client list......\n");
                        for (int i = 0; i < connected_count; i++){
                            printf("ip: %s\n", clientList[i].IP);
                            printf("fd: %d\n", clientList[i].client_fd);
                            printf("hostname: %s\n", clientList[i].hostName);
                            printf("status: %d\n", clientList[i].status);

                        }
                        printf("\n");
                        // update the client list
                        clientList[connected_count].client_fd = fdaccept;
                        clientList[connected_count].IP = c_ip ;
                        clientList[connected_count].status = 1; 
                        clientList[connected_count].hostName = host_name;
                        clientList[connected_count].block_list = newBlocked;

                        connected_count += 1;
                        printf("......printing the client ip list after update......\n");
                        for (int i = 0; i < connected_count; i++){
                            printf("ip: %s\n", clientList[i].IP);
                            printf("fd: %d\n", clientList[i].client_fd);
                            printf("hostname: %s\n", clientList[i].hostName);
                            printf("status: %d\n", clientList[i].status);
                        }
                        printf("\n");
                        printf(".....................................................");
                        printf("\n");


                        client_list(fdaccept,sort_fd, connected_count);


                    }
                        /* Read from existing clients */
                    else{
                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);


                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                            close(sock_index);
                        }
                        else {
                            char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);
                            memset(cmd, '\0', CMD_SIZE);
                            char* rev[3];
                            int count = 0;
                            char *pNext = strtok(buffer, " ");

                            while (pNext != NULL) {
                                if (count >= 3){
                                    break;
                                }
                                rev[count] = pNext;
                                printf("%s\n",pNext);
                                ++count;
                                pNext = strtok(NULL, " ");
                            }

                            strcpy(cmd, rev[0]);
                            //Process incoming data from existing clients here ...

                            if (strcmp("REFRESH",cmd) == 0){
                                client_list(sock_index,sort_fd, connected_count);
                            }
                            else if (strcmp("EXIT", cmd) == 0){
                                remove_sck(sort_fd, client_port, sock_index, connected_count);
                                connected_count -= 1;
                            }
                            else if(strcmp("SEND", cmd) == 0){ ///////// -----------
                                printf("right here\n");
                                printf("%s\n",rev[1]); // IP
                                printf("%s\n",rev[2]); // MSG
                                char* from;
                                int isValid = 1;
                                int to = -1;
                                int toIndex = -1;
                                int toStatus = 0;
                                for (int i = 0; i < connected_count; i++) {
                                    if (sock_index == clientList[i].client_fd){
                                        from = clientList[i].IP;
                                        clientList[i].mSend += 1; //不计较对面有没有block我，我就算我发了信息了
                                    }
                                }
                                for (int i = 0; i < connected_count; i++) {
                                    if (strcmp(rev[1],clientList[i].IP) == 0){
                                        toIndex = i;
                                        toStatus = clientList[i].status;
                                        to = clientList[i].client_fd;
                                        if (is_blocked(clientList[i].block_list,clientList[i].block_count,from) == 1){
                                            isValid = 0;
                                        }
                                    }
                                }
                                char result[6 + strlen(from) +strlen(rev[2])];
                                strcpy(result, "msg: ");
                                strcat(result,from);
                                strcat(result, " ");
                                strcat(result,rev[2]);
                                printf("result-> %s\n",result);

                                if (isValid == 1 && to != -1 && toIndex != -1 && toStatus == 1){
                                    //运行条件是：不能被block，存在to，并且他的状态为登入
                                    clientList[toIndex].mRev += 1; //只有我成功接收到了，才算接收
                                    cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
                                    cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", from, rev[1], rev[2]);
                                    cse4589_print_and_log("[%s:END]\n", "RELAYED");
                                    send(to,result, strlen(result),0);
                                } else if (isValid == 1 && to != -1 && toIndex != -1 && toStatus == 0){ //(待测)
                                    //如果他的是登出状态，就缓存消息给他
                                    printf("he log out");
                                    clientList[toIndex].bufferList[clientList[toIndex].buffer_count] = result;
                                    clientList[toIndex].buffer_count+=1;
                                }

                            }else if(strcmp("BROADCAST", cmd) == 0){ ///////// -----------
                                printf("%s\n",rev[1]); // MSG
                                char* from;
                                for (int i = 0; i < connected_count; i++) {
                                    if (sock_index == clientList[i].client_fd){
                                        from = clientList[i].IP;
                                        clientList[i].mSend += 1; //广播也算我发了信息
                                    }
                                }

                                char result[6 + strlen(from) +strlen(rev[1])];
                                strcpy(result, "bro: ");
                                strcat(result,from);
                                strcat(result, " ");
                                strcat(result,rev[1]);
                                printf("result-> %s\n",result);

                                cse4589_print_and_log("[%s:SUCCESS]\n", "RELAYED");
                                cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", from, "255.255.255.255", rev[1]);
                                cse4589_print_and_log("[%s:END]\n", "RELAYED");
                                for (int i = 0; i < connected_count; i++) {
                                    printf("status %d : %d\n", clientList[i].client_fd, clientList[i].status);
                                    if (sock_index == clientList[i].client_fd){
                                        continue;
                                    }
                                    int check = 1;
                                    for (int j = 0; j < clientList[i].block_count; j++) {
                                        printf("block : %s\n",clientList[i].block_list[j].IP);
                                        if (strcmp(from, clientList[i].block_list[j].IP) == 0){
                                            check = 0;
                                            break;
                                        }
                                    }
                                    if (check == 1){
                                        if (clientList[i].status == 0){ //缓存信息给不在线的用户(待测)
                                            clientList[i].bufferList[clientList[i].buffer_count] = result;
                                            clientList[i].buffer_count+=1;
                                        } else{
                                            clientList[i].mRev += 1; //收到广播就算接收
                                            send(clientList[i].client_fd, result, strlen(result), 0);
                                        }
                                    }
                                }
                            } else if(strcmp("BLOCK", cmd) == 0){
                                printf("%s\n",rev[1]); // MSG
                                int current_block_count;
                                char* ip_to_block = malloc(INET_ADDRSTRLEN);
                                ip_to_block = rev[1];
                                printf("ip to block: %s\n", ip_to_block);
                                printf("......Now printing the existing ip list......\n");
                                for (int i = 0; i < connected_count; i++){
                                    printf("client ip: %s\n", clientList[i].IP);
                                }
                                for(int i = 0; i < connected_count; i++){                                    
                                    if(clientList[i].client_fd == sock_index){
                                        current_block_count = clientList[i].block_count;
                                        // Check if the client(to block) is already being blocked
                                        if(is_blocked(clientList[i].block_list, current_block_count, ip_to_block) == 0){
                                            int b_fd;
                                            int b_port = 0;
   
                                            for (int k = 0; k < connected_count; k++){
                                                printf("current checking ip: %s\n", clientList[k].IP);
                                                if(strcmp(clientList[k].IP, ip_to_block) == 0){ //有问题呀
                                                    b_fd = clientList[k].client_fd;
                                                    printf("blocked client fd: %d\n", b_fd);
                                                    break;
                                                }
                                            }
                                            // Get port number for blocked client
                                            struct sockaddr_in client;
                                            socklen_t len = sizeof(struct sockaddr_in);                
                                            if (getpeername(b_fd, (struct sockaddr *)&client, &len) == 0){
                                                b_port = ntohs(client.sin_port);
                                                printf("blocked client port number: %d\n", b_port);
                                            }else{
                                                printf("gepteername failed\n");
                                            }
                                            // Get hostname for blocked client
                                            char addr[sizeof(struct in_addr)];
                                            inet_pton(AF_INET, inet_ntoa(client.sin_addr), addr);
                                            struct hostent *gethost_rtval;
                                            gethost_rtval = gethostbyaddr(&addr, sizeof(addr), AF_INET);
                                            char * b_hostname = malloc((strlen(gethost_rtval->h_name) + 1)* sizeof(char));
                                            strcpy(b_hostname, gethost_rtval->h_name);
                                            printf("blocked client hostname: %s\n", b_hostname);
                                            // Initialize blocked client
                                            struct blocked *b = malloc(sizeof(struct blocked));
                                            b->IP = ip_to_block; 
                                            b->host_name = b_hostname;
                                            b->port = b_port;
                                            clientList[i].block_list[current_block_count] = *b;
                                            clientList[i].block_count ++;
                                            printf("Blocked client with ip: %s\n", ip_to_block);
                                            // Notify client about finishing the blocking event
                                            // Notify client about finishing the blocking event
                                            send(sock_index, "YES", 3, 0);
                                            printf("sent yes to client");
                                        }else{
                                            printf("This ip is already in block list: %s\n", ip_to_block);
                                            send(sock_index, "NO", 2, 0);
                                            printf("sent no to client");
                                        }
                                        break;
                                    }
                                }
                            } else if (strcmp("UNBLOCK", cmd) == 0){
                                printf("%s\n",rev[1]); // MSG
                                send(sock_index,"YES",3,0);
                            } else if(strcmp("LOGOUT", cmd) == 0){
                                for (int i = 0; i < connected_count; i++) {
                                    if (sock_index == clientList[i].client_fd){
                                        clientList[i].status = 0;
                                    }
                                }
                            } else if (strcmp("RELOGIN", cmd) == 0){//(待测)
                                for (int i = 0; i < connected_count; i++) {
                                    if (sock_index == clientList[i].client_fd){
                                        printf("status -> %d\n", clientList[i].status);
                                        clientList[i].status = 1;
                                        if (clientList[i].buffer_count > 0){
                                            printf("buffer here !!!");
                                            for (int j = 0; j < clientList[i].buffer_count; j++) {//传缓存消息给对应用户
                                                char* pass = clientList[i].bufferList[j];
                                                printf("pass -> %s\n", pass);
                                                send(sock_index, pass, strlen(pass), 0);
                                                memset(pass, 0, strlen(pass));
                                            }
                                            clientList[i].buffer_count = 0;
                                        }
                                        printf("no buffer\n");
                                        break;
                                    }
                                }
                            }
                            printf("\nClient sent me: %s\n", cmd);

                        }

                        free(buffer);
                    }
                }
            }
        }
    }


    return 0;
}

int is_blocked(struct blocked* block_list, int block_count, char* client_ip){
    if (block_count == 0){
        return 0;
    }
    for (int i = 0; i < block_count; i++){
        if(strcmp(client_ip, block_list[i].IP) == 0){
            return 1;
        }
    }
    return 0;
}

int blocked_cmd(char* input){
    char substr[8];
    strncpy(substr, &input[0], 7);
    substr[7] = '\0';
    // printf("user input: %s\n", substr);
    return strcmp("BLOCKED", substr);
}

int compareByPort(const void* a, const void* b) {
    const struct blocked* blockedA = (const struct blocked*) a;
    const struct blocked* blockedB = (const struct blocked*) b;
    return blockedA->port - blockedB->port;
}

void get_block_list(char* ip, struct client * c_lst, int connect_count){

    // Get block list for this client
    int block_count;
    struct blocked *blc;
    printf("connected count: %d\n", connect_count);
    for(int i = 0; i < connect_count; i++){
        printf("checking ip: %s\n", c_lst[i].IP);
        if(strcmp(c_lst[i].IP, ip) == 0){
            block_count = c_lst[i].block_count;
            blc = c_lst[i].block_list;
            printf("Get block count: %d\n", block_count);
            break;
        }
    }
    // Sort blocked list by increasing port number
    qsort(blc, block_count, sizeof(struct blocked), compareByPort);
    printf("Finishing sorting\n");
    for(int i = 0; i < block_count; i++){
        struct blocked blocked_client = blc[i];
        cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i+1, blocked_client.host_name, blocked_client.IP, blocked_client.port);
    }

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



