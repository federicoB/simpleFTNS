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

#ifndef SETTER_INCREMENT_H
#define SETTER_INCREMENT_H

#include <stdint.h>

/**
 * Set a variable with the value given.
 * @param name uint32_t: the name of the variable to set. The variable name will be truncated to 25 bit.
 * @param value uint32_t the value to set.
 * @return a value <0 if an error occurred 1 otherwise.
 */
int set(uint32_t name,uint32_t value);

/**
 * Increment of a given value the specified variable identified by name.
 * @param name uint32_t: the name of the variable to increase. The variable name will be truncated to 25 bit.
 * @param value uint32_t: the increase amount
 * @return a value <0 if an error occurred 1 otherwise.
 */
int increment(uint32_t name,uint32_t value);

/**
 * Get the value of the variable named "name"
 * @param name uint32_t: the name of the variable.The variable name will be truncated to 25 bit.
 * @param value uint32_t*: the obtained value
 * @return a value <0 if an error occurred 1 otherwise.
 */
int get(uint32_t name,uint32_t* value);

#endif
