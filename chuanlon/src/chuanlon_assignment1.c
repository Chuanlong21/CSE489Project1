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
//#include <winsock2.h>
//#include <sys/socket.h>


#include "../include/common.h"
#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"
#include "../include/client.h"




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
//	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
//	fclose(fopen(LOGFILE, "w"));

    /*Start Here*/
    if (argc < 3 || (strcmp(argv[1],"c") != 0 && strcmp(argv[1],"s") != 0) || validNumber(argv[2]) != 1) {
        return -1; // 代表指令问题
    }

    char* port = argv[2];

    if (strcmp(argv[1],"s") == 0){
        s_startUp(port);
    } else if(strcmp(argv[1],"c") == 0){
        c_startUp(port);
    }

    return 0;
}
