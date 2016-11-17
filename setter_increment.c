/*
* Created by Federico Bertani on 17/11/16.
* Copyright (c) 2016 Federico Bertani 
* This file is part of FaultTolerantClientServer.
* FaultTolerantClientServer is free software: you can redistribute it and/or modify
*    it under the terms of the GNU Affero General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "setter_increment.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 1088
#define ADDRESS "127.0.0.1"

int clientSocket;
struct sockaddr_in serverAddress;


int socketConnected(int *socket) {
    int error_code = 0;
    int error_code_size = sizeof(error_code);
    int returnValue = getsockopt(*socket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
    return (error_code | returnValue);
}

int set(uint32_t name, uint32_t value) {
    //check if an existing open connection not exist
    if (socketConnected(&clientSocket)!=0) {
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("socket error\n");
            exit(-2);
        } else {
            memset(&serverAddress, 0, sizeof(serverAddress));
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(PORT);
            if (inet_pton(AF_INET, ADDRESS, &serverAddress.sin_addr) <= 0) {
                printf("inet_pton error for %s\n", ADDRESS);
                exit(-3);
            } else if (connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
                printf("connect error\n");
                exit(-5);
            }
        }
        //check if an existing token file is present. If present load it
        //create connection
        //sent Syn packet
        //wait success
    }
//send setpacket
//wait success
}

int increment(uint32_t name, uint32_t value) {
    //check if an existing open not connection exist
    //check if an existing token file is present
    //load it
    //create connection
    //sent Syn packet
    //wait success
    //send setpacket
    //wait success
}
