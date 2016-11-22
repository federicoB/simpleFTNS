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
#include "setter_increment.h"

int main(int argc, char **argv)
{
    set(12,1);
    increment(12,42);
    uint32_t * value = malloc(sizeof(uint32_t));
    get(12,value);
    printf("%u",*value);
    free(value);
    return 0;
}