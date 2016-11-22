/*
* Created by Federico Bertani on 17/11/16.
* Copyright (c) 2016 Federico Bertani, Riccardo Maffei
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
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "simpleProtocol.h"

#define PORT 1088
#define ADDRESS "127.0.0.1"
#define TOKEN_FILE_NAME "token"

int client_Socket; //this is global for a reason
struct sockaddr_in serverAddress;
uint32_t token;

/*int getMACaddress(uint8_t * MACaddress) {
    int status = 1;
    char buf[256];
    FILE *fp = fopen("/sys/class/net/eth0/address", "rt");
    memset(buf, 0, 256);
    if (fp) {
        if (fgets(buf, sizeof buf, fp) > 0) {
            sscanf(buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &MACaddress[0],
                   &MACaddress[1], &MACaddress[2], &MACaddress[3], &MACaddress[4], &MACaddress[5]);
            status = 0;
        }
        fclose(fp);
    }
    return status;
}*/

/**
 * Check if a given socket is already connected to a server or not.
 * @param clientSocket int*: the socket descriptor to check
 * @return 1 if already connected 0 if not
 */
int socketConnected(int *clientSocket) {
    //declare a variable for keeping getSockOpt error
    int error_code = 0;
    //get the size of error code variable
    socklen_t error_code_size = sizeof(error_code);
    //getting socket option of a not connected socket return -1
    int returnValue = getsockopt(*clientSocket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
    //1 if already connected 0 if not (!-1 is 0)
    return (returnValue==-1);
}

int initializeToken(uint32_t *token) {
    *token = 0;
    int result = 1;
    FILE* filetoken;
    if (access(TOKEN_FILE_NAME, F_OK)==0) {
        filetoken = fopen(TOKEN_FILE_NAME, "r");
        if (filetoken != NULL) {
            //read the token from file
            if (fread(token, sizeof(*token), 1, filetoken) == 0) {
                //if an error occur (fread return value == 0)
                result = 0;
            }
            fclose(filetoken);
        }
    }
    return result;
}

int saveTokenToFile(uint32_t token) {
    int result = 1;
    FILE* filetoken;
        if ((filetoken=fopen(TOKEN_FILE_NAME,"w"))!=NULL) {
            if (fwrite(&token,sizeof(token),1,filetoken)==0) {
                    //if an error occur (fwrite return value == 0)
                    result = 0;
                }
            } else result = 0;
    return result;
}

int sendPacket(SimpleProtocolPacket* packet) {
    int result=1;
    *packet = SPPTONTW(*packet);
    if (send(client_Socket,packet,sizeof(SimpleProtocolPacket),0)==-1) {
        result = 0;
    }
    return result;
}

int waitResponse(SimpleProtocolPacket *responsePacket) {
    int result=1;
    if (recv(client_Socket,responsePacket,sizeof(SimpleProtocolPacket),0)!=-1) {
        *responsePacket = SPPTOHST(*responsePacket);
    } else result = 0;
    return result;
}

int checkSuccess(SimpleProtocolPacket *responsePacket) {
    int result=1;
    if (waitResponse(responsePacket)) {
        if (!GETSUC(*responsePacket)) result=0;
    }
    return result;
}

int establishSession(int *clientSocket) {
    int result = 1;
    //check if an existing open connection not exist
    if (socketConnected(clientSocket) != 0) {
        if ((*clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            result = -2;
        } else {
            memset(&serverAddress, 0, sizeof(serverAddress));
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(PORT);
            if (inet_pton(AF_INET, ADDRESS, &serverAddress.sin_addr) <= 0) {
                result = -3;
            } else if (connect(*clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
                result = -5;
            }
        }
        if (result == 1) {
            //check if an existing token file is present. If present load it
            if ((result=initializeToken(&token))) {
                //sent Syn packet
                SimpleProtocolPacket packet;
                SPPINIT(packet);
                SETSYN(packet);
                SETTKN(packet, token);
                if ((result=sendPacket(&packet))) {
                    //reuse sending packet for response
                    SPPINIT(packet);
                    if ((result = checkSuccess(&packet))) {
                        if (token==0) {
                            result = saveTokenToFile((uint32_t )GETTKN(packet));
                        }
                    }
                }
            }
        }
    }
    return result;
}

int set(uint32_t name, uint32_t value) {
    int result = 1;
    //TODO check error checking
    if ((result=establishSession(&client_Socket))){
        //send set packet
        SimpleProtocolPacket packet;
        SPPINIT(packet);
        SETSET(packet);
        SETVAR(packet,name);
        SETVAL(packet,value);
        if ((result=sendPacket(&packet)))
            //reuse packet for response
            result= checkSuccess(&packet);
    };
    return result;
}

int increment(uint32_t name, uint32_t value) {
    int result=1;
    if ((result=establishSession(&client_Socket))){
        //send increment packet
        SimpleProtocolPacket packet;
        SPPINIT(packet);
        SETINC(packet);
        SETVAR(packet,name);
        SETVAL(packet,value);
        if ((result=sendPacket(&packet)))
            //reuse packet for response
            result= checkSuccess(&packet);
    };
    return result;
}

int get(uint32_t name,uint32_t* value) {
    int result = 1;
    if ((result=establishSession(&client_Socket))){
        SimpleProtocolPacket packet;
        SPPINIT(packet);
        SETGET(packet);
        SETVAR(packet,name);
        if ((result=sendPacket(&packet))) {
            //reuse packet for response
            if ((result=checkSuccess(&packet))) {
                if (GETSUC(packet)) *value = (uint32_t) GETVAL(packet);
                else result=0;
            };
        }
    };
    return result;
}
