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

void printerror();

int main(int argc, char **argv)
{
    unsigned int response;
    do {
        printf("Choose an option\n");
        printf("1-Set a new variable\n");
        printf("2-Increase current variable\n");
        printf("3-Get the value of a variable\n");
        printf("4-Program exit\n");
        scanf("%u", &response);
        switch (response) {
            case 1: {
                uint32_t name, value;
                printf("Insert the name of the new variable\n");
                scanf("%u", &name);
                printf("Insert the value of the new variable\n");
                scanf("%u", &value);
                if (!set(name, value)) {
                    printerror();
                }
                break;
            }
            case 2: {
                uint32_t name,value;
                printf("Insert the name of the variable to increase\n");
                scanf("%u", &name);
                printf("Insert how much to increase\n");
                scanf("%u", &value);
                if (!increment(name,value)) {
                    printerror();
                }
                break;
            }
            case 3: {
                uint32_t name,value;
                printf("Insert the name of the variable\n");
                scanf("%u", &name);
                if (!get(name,&value)) {
                    printerror();
                }
                printf("The value of the variable %u is %u\n",name,value);
            }
        }
    }while (response!=4);
    return 0;
}

void printerror() {
    printf("Insert the name of the variable\n");
}