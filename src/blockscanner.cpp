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
#include "blockscanner.h"
#include <string.h>

const char magic[] = "\xfa\xbf\xb5\xda";

VtcBlockIndexer::BlockScanner::BlockScanner(const std::string blockFileName) {
    this->blockFileName = blockFileName;
}

bool VtcBlockIndexer::BlockScanner::open() {
    this->blockFileStream.open(this->blockFileName, std::ios_base::in | std::ios_base::binary);
    return this->blockFileStream.is_open();
}

bool VtcBlockIndexer::BlockScanner::close() {
    if(!this->blockFileStream.is_open()) return false;
    this->blockFileStream.close();
    return !this->blockFileStream.is_open();
}

bool VtcBlockIndexer::BlockScanner::moveNext() {
    char * buffer = new char[4];
    this->blockFileStream.read(buffer, 4);
    if(this->blockFileStream.eof() || this->blockFileStream.fail()) return false;

    return (memcmp(buffer, magic, 4) == 0);
}