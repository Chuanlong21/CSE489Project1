//
// Created by chuanlon on 2023/2/21.
//

#ifndef CSE489PROJECT1_COMMON_H
#define CSE489PROJECT1_COMMON_H

#endif //CSE489PROJECT1_COMMON_H

struct blocked{
    int port;
    char* IP;
    char* host_name;
};

struct client {
    int client_fd;
    char* IP;
    struct blocked *block_list;
    int block_count;
    int status; //0 for logout; 1 for login
    int mSend;
    int mRev;
    char* hostName;
    char** bufferList;
    int buffer_count;
};





int IPv4_verify(char *ip);

int validNumber(char *a);

void show_port(char* port);

void show_Author();

void show_ip(int server_socket);

void listing(int* fds, int count);

void error(char* cmd);

void client_list(int sock_index,int* fds, int count);