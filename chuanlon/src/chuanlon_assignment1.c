/**
 * @chuanlon_assignment1
 * @author  chuanlong liu <chuanlon@buffalo.edu>
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
 * This contains the main function. Add further description here....
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <winsock2.h>

#include "../include/global.h"
#include "../include/logger.h"

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{

	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

    if (argc < 3 ||strcmp(argv[1],"c") != 0 || strcmp(argv[1],"s") != 0 ) {
        return -1; // 代表当前指令少于3个
    }
    struct sockaddr_in server_addr;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);//创建套接字
    if(server_socket < 0) {
        perror("socket");
        return -1;//创建失败的错误处理
    }
    memset(&server_addr,0,sizeof (server_addr));
    char* port = argv[2];

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(port));

    printf("Port: %d:", ntohs(server_addr.sin_port));

    if (bind(server_socket,(struct  sockaddr *)&server_addr, sizeof server_addr) < 0){
        perror("socket");
        return -1;//创建失败的错误处理
    }

    if (listen(server_socket,3) < 0){
        perror("socket");
        return -1;//创建失败的错误处理
    }


	/*Start Here*/

	return 0;
}
