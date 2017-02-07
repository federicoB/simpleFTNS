/*
 * File:   server.c
 * Author: Riccardo Maffei, Federico Bertani
 *
 * Created on 8 novembre 2016, 13.36
 * Copyright (c) 2016 Federico Bertani, Riccardo Maffei
 * This file is part of simpleFTNS.
 * simpleFTNS is free software: you can redistribute it and/or modify
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "simpleProtocol.h"
#include <sqlite3.h>
#include <pthread.h>

void* connectionHandler(void* socket);
int countVar(sqlite3* db, uint32_t token, uint32_t varName, char** errMsg);
int callbackOne32UInteger(void* intPointer, int argc, char** argv, char** colNames);
int callbackOneInteger(void* intPointer, int argc, char** argv, char** colNames);
void sendPacket(int socketfd, SimpleProtocolPacket packet);

#define SERVERPORT 60010        //the port number this server will listen to
#define SERVERADDR "127.0.0.1"  //the ip address this server will bind to (not used yet)
#define MAXCONN 50              //the maximum number of incoming connection

/*
DB structure

CREATE TABLE `Sessions` (
	`ID`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	`NOTUSEDYET`	TEXT
);
CREATE TABLE `Variables` (
	`SESSID`	INTEGER NOT NULL,
	`Name`	INTEGER NOT NULL,
	`Value`	INTEGER NOT NULL,
	PRIMARY KEY(SESSID,Name),
	FOREIGN KEY(`SESSID`) REFERENCES Sessions ( ID )
);
 */



/*
 * Starts this simple server RPC-like
 */
int main(int argc, char** argv) { 
    //thread array
    pthread_t thread; //dummy. TODO: implement
    //the socket file descriptor
    int socketd;
    //server socket address struct
    struct sockaddr_in serverAddress;     
    //create TCP socket
    socketd = socket(AF_INET, SOCK_STREAM, 0);
    //if the socket is < 1 (error)
    if (socketd < 0){
        //print an error
        printf("Error opening socket\n");
        //exit
        exit(1);
    }
    //clear struct
    memset(&serverAddress, 0, sizeof(serverAddress));
    //set address family as Internet
    serverAddress.sin_family = AF_INET;
    //set the server address (after conversion)
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(SERVERADDR);
    //set server port (after conversion)
    serverAddress.sin_port = htons(SERVERPORT);
    //try to bind and if fail (result < 0)
    if (bind(socketd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        //print an error
        printf("Error on binding\n");
        //exit
        exit(1);
    }
    //invoke listen
    listen(socketd, MAXCONN);
    //the client file descriptor
    int client = -1;
    //print debug info
    printf("Listening for connections\n");
    //while no error is signaled, accept connections
    while (client = accept(socketd, (struct sockaddr *) NULL, NULL)){
        //print message
        printf("Connection established\n");
        //create a thread which calls the connection handler. if fails
        if(pthread_create(&thread, NULL,  connectionHandler, (void*) &client) < 0){
            //print an error
            printf("Error creating thread\n");
            //exit
            exit(1);
        }
    }
    //TODO: implement join (need safe way to store threads pointers)
    while(1){
        sleep(10);
    }
    //close the server socket
    close(socketd);
    return (EXIT_SUCCESS);
}

/**
 * Handles the connection between this server and the given client socket.
 * @param socket void*: the client socket
 * @return void*: nothing! just for signature compliance.
 */
void* connectionHandler(void* socket){
    //lets use a separate db connection for each thread
    //on https://dev.yorhel.nl/doc/sqlaccess Yoran Heling published a good article about pros and cons 
    //of using sqlite in this (and other) way.
    //Remember to compile with flag: 
    
    //the db connection
    sqlite3* db;
    //the error message
    char* errMsg = 0;
    //the result code
    int rc = 0;
    
    //open the db
    rc = sqlite3_open("mainDB.sqlite", &db);
    //if error occurred
    if(rc){
        //print an error and exit
        printf("Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
    //else
    else{
        //print debug info
        printf("Database successfully opened\n");
   }
    //the inbound and outbound packets
    SimpleProtocolPacket inPacket, outPacket;
    //init (clear) the packets
    SPPINIT(inPacket);
    SPPINIT(outPacket);
    //the token
    uint32_t token;
    //cast the socket descriptor
    int socketDescriptor = *(int*)socket;
    //wait for SYN (receive one packet), if received
    if(recv(socketDescriptor, &inPacket, sizeof(SimpleProtocolPacket), 0) > 0){
        //convert the packet to network version (just in case this is a little endian machine)
        inPacket = SPPTOHST(inPacket);
        //if the packet is a SYN
        if(GETSYN(inPacket)){
            //should continue? (no fatal error occurred)
            int shouldContinue = 1;
            //get token
            token = GETTKN(inPacket);
            //if token is zero
            if(token == 0){
                //Create an entry in db (with new token)
                char sql[512] = "INSERT INTO Sessions (NOTUSEDYET) VALUES (NULL);";
                //execute query
                int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
                //if the query has bee successful
                if(rc == SQLITE_OK){
                    //since Sessions has an int, pk, ai column, it actually is an alias for rowid
                    snprintf(sql, 512, "SELECT last_insert_rowid();");
                    //execute the query with the callback which saves the last token
                    //ignore the rc. TODO: improve errore handling structure
                    sqlite3_exec(db, sql, callbackOne32UInteger, &token, &errMsg);
                    //leave the should continue as true
                    //print debug info
                    printf("New session created with id:%u\n", token);
                }
                //else (error)
                else{
                    //set should continue as false
                    shouldContinue = 0;
                    //print an error
                    printf("Error while inserting new session\n");
                    //set the response as err
                    SETERR(outPacket);
                    //TODO: implement error code (Internal server error)
                    //SETERC(...)
                    //send the packet
                    sendPacket(socketDescriptor, outPacket);
                }
            }
            //else, session token given
            else{
                //result
                int result = 0;
                //the sql buffer
                char sql[512];
                //create the query that counts (check if the session exists)
                snprintf(sql, 512,
                        "SELECT COUNT(*) FROM Sessions WHERE "
                        "ID = %u;", token);
                //execute the query
                int rc = sqlite3_exec(db, sql, callbackOneInteger, &result, &errMsg);
                //print debug info
//                printf("EXECUTED SQL: %s\n", sql);
                //if the query has bee successful
                if(rc == SQLITE_OK){
                    //if not exists
                    if(!(result > 0)){
                        //set should continue as false
                        shouldContinue = 0;
                        //print an error
                        printf("Error: invalid session id received\n");
                        //set the response as err
                        SETERR(outPacket);
                        //TODO: implement error code
                        //SETERC(...)
                        //send the packet
                        sendPacket(socketDescriptor, outPacket);
                    }
                    //else
                    else{
                        //leave the should continue as true
                        //print debug info
                        printf("Session restored with id:%u\n", token);
                    }
                }
                //else (error)
                else{
                    //set should continue as false
                    shouldContinue = 0;
                    //print an error
                    printf("Error restoring session %u\n", token);
                    //set the response as err
                    SETERR(outPacket);
                    //TODO: implement error code (Internal server error)
                    //SETERC(...)
                    //send the packet
                    sendPacket(socketDescriptor, outPacket);
                }
            }
            //if should continue
            if(shouldContinue){
                //create a SYN SUC response with the token
                SETSYN(outPacket);
                SETSUC(outPacket);
                SETTKN(outPacket, token);
                //send the packet
                sendPacket(socketDescriptor, outPacket);
                //print debug info
                printf("SYN SUC session restored. id:%u\n", token);
                //init (clear) the packets
                SPPINIT(inPacket);
                SPPINIT(outPacket);
                //should run?
                int shouldRun = 1;
                //should send?
                int shouldSend = 1;
                //while not finished (connection alive and no FIN received)
                while((shouldRun) && (recv(socketDescriptor, &inPacket, sizeof(SimpleProtocolPacket), 0) > 0)){
                    //set should send as true
                    shouldSend = 1;
                    //convert the packet to network version (just in case this is a little endian machine)
                    inPacket = SPPTOHST(inPacket);
                    //if is a set packet
                    if(GETSET(inPacket)){
                        //print debug info
                        printf("SET invoked from session %u\n", token);
                        //the var name (actually only 25bits)
                        uint32_t varName = 0;
                        //the var value
                        uint32_t varValue = 0;
                        //get the var name
                        varName = GETVAR(inPacket);
                        //get the var value
                        varValue = GETVAL(inPacket);
                        //the sql buffer
                        char sql[512];
                        //create the "insert or replace" query
                        snprintf(sql, 512,
                                "INSERT OR REPLACE INTO Variables (SESSID, Name, Value) "
                                "VALUES (%u, %u, %u);", token, varName, varValue);
                        //execute the query
                        int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
                        //print debug info
//                        printf("EXECUTED SQL: %s\n", sql);
                        //if query worked
                        if(rc == SQLITE_OK){
                            //set the name to outbound packet
                            SETVAR(outPacket, varName);
                            //set the new value to outbound packet
                            SETVAL(outPacket, varValue);
                            //set the response packet as SUC
                            SETSUC(outPacket);
                            //print debug info
                            printf("Var %u for session %u set to %u\n", varName, token, varValue);
                        }
                        //else (internal server error because the query should never fail)
                        else{
                            //set the response as err
                            SETERR(outPacket);
                            //TODO: implement error code (Internal server error)
                            //SETERC(...)
                            //print debug info
                            printf("Error in SET invoked from session %u (query failed)\n", token);
                        }
                    }
                    //else if is a inc packet
                    else if (GETINC(inPacket)) {
                        //print debug info
                        printf("INC invoked from session %u\n", token);
                        //the var name (actually only 25bits)
                        uint32_t varName = 0;
                        //the var value
                        uint32_t varValue = 0;
                        //get the var name
                        varName = GETVAR(inPacket);
                        //get the var value
                        varValue = GETVAL(inPacket);
                        //the number of found vars
                        int nVars = countVar(db, token, varName, &errMsg);
                        //if the var exists
                        if(nVars > 0){
                            //the sql buffer
                            char sql[512];
                            //create the "UPDATE and SELECT" query
                            snprintf(sql, 512,
                                    "UPDATE Variables SET "
                                    "Value = Value + %u WHERE "
                                    "SESSID = %u AND Name = %u;"
                                    "SELECT (Value) FROM Variables WHERE "
                                    "SESSID = %u AND "
                                    "Name = %u;", varValue, token, varName, token, varName);
                            //execute the query
                            int rc = sqlite3_exec(db, sql, callbackOne32UInteger, &varValue, &errMsg);
                            //print debug info
//                            printf("EXECUTED SQL: %s\n", sql);
                            //if query worked
                            if(rc == SQLITE_OK){
                                //set the name to outbound packet
                                SETVAR(outPacket, varName);
                                //set the new value to outbound packet
                                SETVAL(outPacket, varValue);
                                //set the response packet as SUC
                                SETSUC(outPacket);
                                //print debug info
                                printf("Var %u for session %u incremented to %u\n", varName, token, varValue);
                            }
                            //else (internal server error because the query should never fail)
                            else{
                                //print debug info
                                printf("Error while incrementing var %u in session %u (query failed)\n", varName, token);
                                //set the response as err
                                SETERR(outPacket);
                                //TODO: implement error code (Internal server error)
                                //SETERC(...)
                            }
                        }
                        //else
                        else{
                            //print debug info
                            printf("Error while incrementing var %u in session %u (var not found)\n", varName, token);
                            //set the response as err
                            SETERR(outPacket);
                            //TODO: implement error code
                            //SETERC(...)
                        }
                    }
                    //else if is a GET packet
                    else if (GETGET(inPacket)) {
                        //print debug info
                        printf("GET invoked from session %u\n", token);
                        //the var name (actually only 25bits)
                        uint32_t varName = 0;
                        //the var value
                        uint32_t varValue = 0;
                        //get the var name
                        varName = GETVAR(inPacket);
                        //is the var found? (count)
                        int nVars = countVar(db, token, varName, &errMsg);
                        //if the var exists
                        if(nVars > 0){
                            //the sql buffer
                            char sql[512];
                            //create the SELECT query
                            snprintf(sql, 512,
                                "SELECT (Value) FROM Variables WHERE "
                                "SESSID = %u AND "
                                "Name = %u;", token, varName);
                            //execute the query
                            int rc = sqlite3_exec(db, sql, callbackOneInteger, &varValue, &errMsg);
                            //print debug info
//                            printf("EXECUTED SQL: %s\n", sql);
                            //if query worked
                            if(rc == SQLITE_OK){
                                //print debug info
                                printf("Var %u got from session %u (value: %u)\n", varName, token, varValue);
                                //set the name to outbound packet
                                SETVAR(outPacket, varName);
                                //set the new value to outbound packet
                                SETVAL(outPacket, varValue);
                                //set the response packet as SUC
                                SETSUC(outPacket);
                            }
                            //else (internal server error because the query should never fail)
                            else{
                                //print debug info
                                printf("Error while getting var %u for session %u (query failed)\n", varName, token);
                                //set the response as err
                                SETERR(outPacket);
                                //TODO: implement error code (Internal server error)
                                //SETERC(...)
                            }
                        }
                        //else
                        else{
                            //print debug info
                            printf("Error while incrementing var %u in session %u (var not found)\n", varName, token);
                            //set the response as err
                            SETERR(outPacket);
                            //TODO: implement error code
                            //SETERC(...)
                        }
                    }
                    //else if is a FIN packet
                    else if(GETFIN(inPacket)){
                        //print debug info
                        printf("FIN invoked from session %u\n", token);
                        //set should run as false
                        shouldRun = 0;
                        //set should send
                        shouldSend = 0;
                    }
                    //else (SUC | ERR | FIN | SYN | <invalid>)
                    else{
                        //set the response as err
                        SETERR(outPacket);
                        //TODO: implement error code
                        //SETERC(...)
                    }
                    //if should send
                    if (shouldSend) {
                        //send the packet
                        sendPacket(socketDescriptor, outPacket);
                    }
                    //init (clear) the packets
                    SPPINIT(inPacket);
                    SPPINIT(outPacket);
                }
            }
            //close the connection
            close(socketDescriptor);
        }
        //else if the packet is a FIN
        else if(GETFIN(inPacket)){
            //close the connection
            close(socketDescriptor);
        }
        //else
        else{
            //send a FIN
            SETFIN(inPacket);
            //close the connection
            close(socketDescriptor);
        }
    }
    //close the db connection
    sqlite3_close(db);
    //auto close thread
    //print debug info
    printf("Connection to client closed\n");
}

/**
 * Callback which saves one integer to the given pointer.
 * Warning: if called multiple times the integer is overwritten.
 * @param intPointer void*: integer pointer
 * @param argc int: number of columns
 * @param argv char**: columns values
 * @param colNames char**: columns names
 */
int callbackOneInteger(void* intPointer, int argc, char** argv, char** colNames){
    //cast the pointer
    int* integer = (int*) intPointer;
    //convert to int and save
    *integer = atoi(argv[0]);
    //return ok
    return 0;
}

/**
 * Callback which saves one unsigned 32 bit integer to the given pointer.
 * Warning: if called multiple times the integer is overwritten.
 * @param intPointer void*: integer pointer
 * @param argc int: number of columns
 * @param argv char**: columns values
 * @param colNames char**: columns names
 */
int callbackOne32UInteger(void* intPointer, int argc, char** argv, char** colNames){
    //cast the pointer
    uint32_t* integer = (uint32_t*) intPointer;
    //convert to unsigned int 32 and save
    *integer = (uint32_t) strtoul(argv[0], NULL, 10);
    //return ok
    return 0;
}

/**
 * Counts vars with the given name and token. Should be 0 or 1 for table constraints
 * @param db sqlite3*: the db
 * @param token uint32_t: the token
 * @param varName uint32_t: the name of the var
 * @param varName char**: pointer to a string for error storage
 * @return int: number of vars
 */
int countVar(sqlite3* db, uint32_t token, uint32_t varName, char** errMsg){
    //result
    int result = 0;
    //the sql buffer
    char sql[512];
    //create the query that counts (check if the var exists)
    snprintf(sql, 512,
            "SELECT COUNT(*) FROM Variables WHERE "
            "SESSID = %u AND "
            "Name = %u;", token, varName);
    //execute the query
    sqlite3_exec(db, sql, callbackOneInteger, &result, errMsg);
    //print debug info
//    printf("EXECUTED SQL: %s\n", sql);
    //return the result
    return result;
}

/**
 * Send the given packet through the given socket
 * @param socketfd int: socket file descriptor
 * @param packet SimpleProtocolPacket: the packet
 */
void sendPacket(int socketfd, SimpleProtocolPacket packet){
    //convert the packet to network version (just in case thih is a little endian machine)
    SimpleProtocolPacket readyToSend = SPPTONTW(packet);
    //send the packet
    send(socketfd, &readyToSend, sizeof(readyToSend), 0);
}
