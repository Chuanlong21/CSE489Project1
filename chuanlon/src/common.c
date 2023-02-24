//
// Created by chuanlon on 2023/2/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../include/logger.h"

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