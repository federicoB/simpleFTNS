/*
* Created by Federico Bertani on 16/11/16.
* Copyright (c) 2016 Federico Bertani, Riccardo Maffei
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

#include <malloc.h>
#include <errno.h>
#include <string.h>
#include "setter_increment.h"

int main(int argc, char **argv)
{
    //declare a variable for user selection in menu
    unsigned int response;
    //print menu until option 4 is chosen
    do {
        printf("\n Choose an option\n");
        printf("1-Set a new variable\n");
        printf("2-Increase current variable\n");
        printf("3-Get the value of a variable\n");
        printf("4-Program exit\n");
        scanf("%u", &response);
        switch (response) {
            case 1: { //brackets are for adding a scope
                uint32_t name, value;
                printf("Insert the name of the new variable\n");
                //read unsigned int variable name
                scanf("%u", &name);
                printf("Insert the value of the new variable\n");
                //read unsigned int variable value
                scanf("%u", &value);
                //execute the set operation
                if (!set(name, value)) {
                    //if an error occurred
                    printf("Error\n");
                }
                break;
            }
            case 2: { //brackets are for adding a scope
                uint32_t name,value;
                printf("Insert the name of the variable to increase\n");
                //read unsigned int variable name
                scanf("%u", &name);
                printf("Insert the amount to add\n");
                //read unsigned int for increase value
                scanf("%u", &value);
                //execute the increment operation
                if (!increment(name,value)) {
                    //if an error occurred
                    printf("Error\n");
                }
                break;
            }
            case 3: { //brackets are for adding a scope
                uint32_t name,value;
                printf("Insert the name of the variable\n");
                //read unsigned int variable name
                scanf("%u", &name);
                //execute get operation
                if (!get(name,&value)) {
                    //if an error occurred
                    printf("Error\n");
                } else printf("The value of the variable %u is %u\n",name,value);
            }
                break;
            case 4: {
                printf("Closing demo client\n");
                break;
            }
            default:
                printf("Undefined operation \n");
        }
    }while (response!=4);
    return 0;
}