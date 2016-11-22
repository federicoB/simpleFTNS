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
#include <unistd.h>
#include "simpleProtocol.h"

#define PORT 1088
#define ADDRESS "127.0.0.1"
#define TOKEN_FILE_NAME "token"

int client_Socket; //this is global for a reason. It is the shared socket for all the RPC calls.

int socketConnected(int *clientSocket);
int initializeToken(uint32_t *token);
int saveTokenToFile(uint32_t token);
int sendPacket(SimpleProtocolPacket* packet);
int waitResponse(SimpleProtocolPacket *responsePacket);
int checkSuccess(SimpleProtocolPacket *responsePacket);
int establishSession(int *clientSocket);

int set(uint32_t name, uint32_t value) {
    //declare a variable for keeping correctness status of program
    int shouldContinue;
    //check if a socket is connected, connect if not. Check if a token file is present. Make syn call.
    if ((shouldContinue=establishSession(&client_Socket))){
        //if no errors occurred establishing the session
        //create a packet
        SimpleProtocolPacket packet;
        //initialize the packet
        SPPINIT(packet);
        //set the packet to a "SET" packet
        SETSET(packet);
        //insert the variable name into the packet
        SETVAR(packet,name);
        //insert the variable value into the packet
        SETVAL(packet,value);
        //send the packet
        if ((shouldContinue=sendPacket(&packet)))
            //If errors have not occurred
            //reuse request packet for response. Wait response and check that it's a "SUCCESS" packet.
            shouldContinue=checkSuccess(&packet);
    }
    return shouldContinue;
}

int increment(uint32_t name, uint32_t value) {
    //declare a variable for keeping correctness status of program
    int shoulContinue;
    //check if a socket is connected, connect if not. Check if a token file is present. Make syn call.
    if ((shoulContinue=establishSession(&client_Socket))){
        //send increment packet
        //create a packet
        SimpleProtocolPacket packet;
        //initialize the packet
        SPPINIT(packet);
        //set the packet to a "INC" packet
        SETINC(packet);
        //insert the variable name into the packet
        SETVAR(packet,name);
        //insert the variable value into the packet
        SETVAL(packet,value);
        //send the packet
        if ((shoulContinue=sendPacket(&packet)))
            //If errors have not occurred
            //reuse packet for response. Wait response and check that it's a "SUCCESS" packet.
            shoulContinue= checkSuccess(&packet);
    }
    //return correctness indicator variable
    return shoulContinue;
}

int get(uint32_t name,uint32_t* value) {
    //declare a variable for keeping correctness status of program
    int shouldContinue;
    //check if a socket is connected, connect if not. Check if a token file is present. Make syn call.
    if ((shouldContinue=establishSession(&client_Socket))){
        //send a get packet
        //create a packet
        SimpleProtocolPacket packet;
        //initialize the packet
        SPPINIT(packet);
        //set the packet to a "GET" packet
        SETGET(packet);
        //insert the variable name into the packet
        SETVAR(packet,name);
        //send the packet
        if ((shouldContinue=sendPacket(&packet))) {
            //If errors have not occurred
            //reuse packet for response. Wait response and check that it's a "SUCCESS" packet.
            if ((shouldContinue=checkSuccess(&packet))) {
                //if it is a "SUCCESS" packet get the value of the variable from it and return to the caller through pointer.
                *value = (uint32_t) GETVAL(packet);
            }
        }
    }
    //return correctness indicator variable
    return shouldContinue;
}

int establishSession(int *clientSocket) {
    //declare a variable for keeping correctness status of program
    int shouldContinue = 1;
    //check if an existing open connection not exist
    if (socketConnected(clientSocket)) {
        //if it not exist create a client socket for IPV4 and TCP
        if ((*clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            //in case of error block the program
            shouldContinue = 0;
        } else {
            // if no errors occurred
            //create a sockaddr_in struct for specifying the address of the server
            struct sockaddr_in serverAddress;
            //set all the struct to zero
            memset(&serverAddress, 0, sizeof(serverAddress));
            //set the server internet family to IPV4
            serverAddress.sin_family = AF_INET;
            //set the server port
            serverAddress.sin_port = htons(PORT);
            //convert the address from dotted character representation to binary representation
            if (inet_pton(AF_INET, ADDRESS, &serverAddress.sin_addr) <= 0) {
                //in an error occur block the program
                shouldContinue = 0;
            }
            //if the conversion is successful connect to the server
            else if (connect(*clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
                //in an error occur during connection to server block the program
                shouldContinue = 0;
            }
        }
        //if the program can continue
        if (shouldContinue == 1) {
            //declare a variable for storing the token
            uint32_t token;
            //check if an existing token file is present. If present load it. Default token is zero.
            if ((shouldContinue=initializeToken(&token))) {
                //sent Syn packet
                //declare packet
                SimpleProtocolPacket packet;
                //initialize the packet
                SPPINIT(packet);
                //set the packet to be a "SYN" packet
                SETSYN(packet);
                //insert the token into file
                SETTKN(packet, token);
                //send packet
                if ((shouldContinue=sendPacket(&packet))) {
                    //If errors have not occurred
                    //reuse packet for response. Wait response and check that it's a "SUCCESS" packet.
                    if ((shouldContinue = checkSuccess(&packet))) {
                        //if the packet is a success packet and the sent token was zero
                        if (token==0) {
                            //this means that is a new session
                            //save the token to a new file
                            shouldContinue = saveTokenToFile((uint32_t )GETTKN(packet));
                        }
                    }
                }
            }
        }
    }
    //return correctness indicator variable
    return shouldContinue;
}

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

int sendPacket(SimpleProtocolPacket* packet) {
    //declare a variable for keeping correctness status of program
    int shouldContinue=1;
    *packet = SPPTONTW(*packet);
    if (send(client_Socket,packet,sizeof(SimpleProtocolPacket),0)==-1) {
        shouldContinue = 0;
    }
    //return correctness indicator variable
    return shouldContinue;
}

int waitResponse(SimpleProtocolPacket *responsePacket) {
    //declare a variable for keeping correctness status of program
    int shouldContinue=1;
    if (recv(client_Socket,responsePacket,sizeof(SimpleProtocolPacket),0)!=-1) {
        *responsePacket = SPPTOHST(*responsePacket);
    } else shouldContinue = 0;
    //return correctness indicator variable
    return shouldContinue;
}

int checkSuccess(SimpleProtocolPacket *responsePacket) {
    //declare a variable for keeping correctness status of program
    int shouldContinue=1;
    if (waitResponse(responsePacket)) {
        if (!GETSUC(*responsePacket)) shouldContinue=0;
    }
    //return correctness indicator variable
    return shouldContinue;
}

int initializeToken(uint32_t *token) {
    *token = 0;
    //declare a variable for keeping correctness status of program
    int shouldContinue = 1;
    FILE* filetoken;
    if (access(TOKEN_FILE_NAME, F_OK)==0) {
        filetoken = fopen(TOKEN_FILE_NAME, "rb");
        if (filetoken != NULL) {
            //read the token from file
            if (fread(token, sizeof(*token), 1, filetoken) == 0) {
                //if an error occur (fread return value == 0)
                shouldContinue = 0;
            }
            fclose(filetoken);
        }
    }
    //return correctness indicator variable
    return shouldContinue;
}

int saveTokenToFile(uint32_t token) {
    //declare a variable for keeping correctness status of program
    int shouldContinue = 1;
    FILE* filetoken;
    if ((filetoken=fopen(TOKEN_FILE_NAME,"wb"))!=NULL) {
        if (fwrite(&token,sizeof(token),1,filetoken)==0) {
            //if an error occur (fwrite return value == 0)
            shouldContinue = 0;
        }
        fclose(filetoken);
    } else shouldContinue = 0;
    //return correctness indicator variable
    return shouldContinue;
}

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