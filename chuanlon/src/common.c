//
// Created by chuanlon on 2023/2/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../include/logger.h"

int IPv4_verify(char *ip) {
    int a,b,c,d;
    char t;

    if (4 == sscanf(ip,"%d.%d.%d.%d%c",&a,&b,&c,&d,&t)){
        if (0<=a && a<=255
            && 0<=b && b<=255
            && 0<=c && c<=255
            && 0<=d && d<=255) {
            return 1;
        }
    }
    return 0;
}


int validNumber(char *a) {
    for (int i = 0; i < strlen(a); i++) {
        if (!isdigit(a[i])) {
            return 0;
        }
    }
    return 1;
}

void show_port(char *port){
    cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
    cse4589_print_and_log("PORT:%s\n",port);
    cse4589_print_and_log("[%s:END]\n", "PORT");
}

void show_ip(int server_socket){
    struct sockaddr_in addr_in;
    socklen_t  len = sizeof (addr_in);
    getsockname(server_socket,(struct sockaddr*)&addr_in, &len);
    if (addr_in.sin_family != AF_INET){
        return;
    }
    cse4589_print_and_log("[%s:SUCCESS]\n", "IP");
    cse4589_print_and_log("IP:%s\n", inet_ntoa(addr_in.sin_addr));
    cse4589_print_and_log("[%s:END]\n", "IP");
}

void show_Author(){
    char* msg = "I, chuanlon have read and understood the course academic integrity policy.";
    cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
    cse4589_print_and_log("AUTHOR:%s\n",msg);
    cse4589_print_and_log("[%s:END]\n", "AUTHOR");
}

void list_client(fd_set *clients, int max_fd){
    printf("Working on listing clients....\n");
    for (int e=0; e <= max_fd; e++){

        struct sockaddr_in client_addr;
        socklen_t len;

        if (FD_ISSET(e, clients)){
            printf("IM in if\n");

            if (getpeername(e, (struct sockaddr *)&client_addr, &len) == 0){
             printf("%-5d%-35s%-20s%-8d\n", e, hostname, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port))
                printf("Socket FD %i is connected to a peer at IP address %s\n", e, inet_ntoa(client_addr.sin_addr));
            }else{
                perror("getpeername");
            }
        }
        
    }

}

void listing(int* connectd, int count){
    for (int i = 0; i < count; i++){

        int fd = connectd[i];
        struct sockaddr_in client_addr;
        socklen_t len;

        if (getpeername(fd, (struct sockaddr *)&client_addr, &len) == 0){

            //printf("getpeername success\n");

            char ipv4addr[sizeof(struct in_addr)];
            inet_pton(AF_INET, inet_ntoa(client_addr.sin_addr), ipv4addr);  
            struct hostent *h_retval;
            h_retval = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
            if(h_retval){
                printf("%-5d%-35s%-20s%-8d\n", fd, h_retval->h_name, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            }
            else{
                printf("Error AHAHA:%s\n", hstrerror(h_errno));
            }
        }else{
            perror("getpeername");
        }
    }
}

void error(char* cmd){
    cse4589_print_and_log("[%s:ERROR]\n", cmd);
    cse4589_print_and_log("[%s:END]\n", cmd);
}

// void sortedInsert(struct client** head, struct client* newNode)
// {
//     struct client dummy;
//     struct client* current = &dummy;
//     dummy.next = *head;
 
//     while (current->next != NULL && current->next->port_num < newNode->port_num) {
//         current = current->next;
//     }
 
//     newNode->next = current->next;
//     current->next = newNode;
//     *head = dummy.next;
// }
