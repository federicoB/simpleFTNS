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

#ifndef SETTER_INCREMENT_H
#define SETTER_INCREMENT_H

/**
 * Set a variable with the value given.
 * TODO add truncating description.
 * @param name uint32_t: the name of the variable to set
 * @param value uint32_t the value to set.
 * @return -1 if an error occurred 0 otherwise.
 */
int set(uint32_t name,uint32_t value);

/**
 * Increment of a given value the specified variable identified by name.
 * @param name uint32_t: the name of the variable to increase
 * @param value uint32_t: the increase amount
 * @return -1 if an error occurred 0 otherwise.
 */
int increment(uint32_t name,uint32_t value);

#endif
