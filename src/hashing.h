/*  VTC Blockindexer - A utility to build additional indexes to the 
    Vertcoin blockchain by scanning and indexing the blockfiles
    downloaded by Vertcoin Core.
    
    Copyright (C) 2017  Gert-Jaap Glasbergen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <ck/ckmath.h>

/** Calculates a double SHA-256 hash over the input and returns it as string
 * @param input the string to convert to hex
 */   
std::string doubleSha256(unsigned char *input, int inputLen);

/** Calculates a single SHA-256 hash over the input and returns it as string
 * 
 * @param input the string to hash
 */
std::string sha256(unsigned char *input, int inputLen);
