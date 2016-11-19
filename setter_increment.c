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
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#define PORT 1088
#define ADDRESS "127.0.0.1"

int client_Socket; //this is global for a reason
struct sockaddr_in serverAddress;
int token;

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

int socketConnected(int *clientSocket) {
    int error_code = 0;
    int error_code_size = sizeof(error_code);
    int returnValue = getsockopt(*clientSocket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
    return (error_code | returnValue);
}

int initializeToken(int *token) {
    token = 0;
    int result = 1;
    char* currentWorkingDirectoryPath;
    DIR *currentWorkingDirectory;
    FILE* filetoken;
    //get current working directory, getcwd automatically malloc
    currentWorkingDirectoryPath = getcwd(NULL, PATH_MAX);
    if (currentWorkingDirectoryPath!=NULL) {
        currentWorkingDirectory = opendir(currentWorkingDirectoryPath);
        if (currentWorkingDirectory!=NULL) {
            //clean up currentWorkingDirectoryPath from memory
            free(currentWorkingDirectoryPath);
            //define a struct for containing current file
            struct dirent *directoryStruct;
            //while we haven't finish to read the files into the directory we count the number of files
            while ((directoryStruct = readdir(currentWorkingDirectory)) != NULL) {
                #define filename directoryStruct->d_name
                //it's ok to check only the first character
                if (*filename != '.') {
                    filetoken = fopen(filename, "r");
                    if (filetoken!=NULL) {
                        //read the token from file
                        if (fread(&token, sizeof(token), 1, filetoken)==0) {
                            //if an error occur (fread return value == 0)
                            result = errno;
                        }
                        fclose(filetoken);
                    }
                }
            }
        } else result=errno;
    } else result = errno;
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
            initializeToken(&token);
            //sent Syn packet
            //wait success
        }
    }
}

int set(uint32_t name, uint32_t value) {
    establishSession(&client_Socket);
//send setpacket
//wait success
}

int increment(uint32_t name, uint32_t value) {
    establishSession(&client_Socket);
    //send setpacket
    //wait success
}
