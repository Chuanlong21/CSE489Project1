//
// Created by chuanlon on 2023/2/22.
//
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <winsock2.h>
//#include <sys/socket.h>

#include "../include/common.h"
#include "../include/logger.h"

#define BACKLOG 50
int c_socks[256];
HANDLE hThread[256];
int count = 0;

int show_ip(){

}

int show_point(int port){
    cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
    cse4589_print_and_log("PORT:%d\n",port);
    cse4589_print_and_log("[%s:END]\n", "PORT");
    return 1;
}


int s_startUp(char * port){
    struct sockaddr_in server_addr;
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);//创建套接字

    if(server_socket < 0) {
        perror("socket");
        return -1;//创建失败的错误处理
    }

    memset(&server_addr,0,sizeof (server_addr));
    int addrLen = sizeof (server_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(port));

    printf("Port: %d:", ntohs(server_addr.sin_port));

    if (bind(server_socket,(struct  sockaddr *)&server_addr, sizeof server_addr) < 0){
        perror("socket");
        return -1;//创建失败的错误处理
    }
    printf("bind..........\n");

    if (listen(server_socket,BACKLOG) < 0){
        perror("socket");
        return -1;//创建失败的错误处理
    }
    printf("listen............\n");

    DWORD dwThreadID; //线程
    SOCKET sock;
    while(TRUE){
        printf("等待新连接\n");
        sock = accept(server_socket,(SOCKADDR*)&server_addr, &addrLen);
        if(sock < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        c_socks[count] = sock;


    }
}