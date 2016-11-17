/*
* Created by Federico Bertani on 16/11/16.
* Copyright (c) 2016 Federico Bertani 
* This file is part of RestorableTCP.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 1088

int getMACaddress(uint8_t * MACaddress) {
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
}

int main(int argc, char **argv)
{
    int	clientSocket, numberOfCharacterRead;
    uint8_t MACaddress;
    struct sockaddr_in serverAddress;
    uint64_t* buffer = malloc(sizeof(uint64_t));

    if (argc != 2){
        printf("You need to specify an ip address\n");
        exit(-1);
    }

    if ( (clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("socket error\n");
        exit(-2);
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, argv[1], &serverAddress.sin_addr) <= 0){
        printf("inet_pton error for %s\n", argv[1]);
        exit(-3);
    }

    if (getMACaddress(MACaddress)) {
        printf("An error occurred while retrieving the MAC address");
        exit(-4);
    }

    if (connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0){
        printf("connect error\n");
        exit(-5);
    }

    //TODO send syn packet with MAC address

    while ( (numberOfCharacterRead = read(clientSocket, buffer,(ssize_t) sizeof(uint64_t))) > 0) {
        //TODO check response
    }

    return 0;
}