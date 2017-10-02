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

#include <unordered_map>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "blockscanner.h"
#include "blockchaintypes.h"

std::unordered_map<unsigned char *, VtcBlockIndexer::ScannedBlock> blocks;
int totalBlocks = 0;


void scanBlocks(std::string fileName) {
    VtcBlockIndexer::BlockScanner* blockScanner = new VtcBlockIndexer::BlockScanner(fileName);
    if(blockScanner->open())
    {
        while(blockScanner->moveNext()) {
            totalBlocks++;
            if(totalBlocks % 1000 == 0) {
                std::cout << "\rFound " << totalBlocks << " blocks";
            }
            VtcBlockIndexer::ScannedBlock block = blockScanner->scanNextBlock();
            blocks[block.previousBlockHash] = block;
        }
        blockScanner->close();
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Starting... \r\n";

    DIR *dir;
    dirent *ent;

    std::cout << "Scanning directory [" << argv[1] << "] for blocks.\r\n";

    dir = opendir(argv[1]);
    while ((ent = readdir(dir)) != NULL) {
        const std::string file_name = ent->d_name;

        std::stringstream ss;
        ss << argv[1] << "/" << file_name;
        const std::string full_file_name = ss.str();

        std::string prefix = "blk"; 
        if(strncmp(file_name.c_str(), prefix.c_str(), prefix.size()) == 0)
        {
            scanBlocks(full_file_name);
        }
    }
    closedir(dir);
   
    
    
}