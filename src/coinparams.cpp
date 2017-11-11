#include "coinparams.h"
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


#include <iostream>
#include <fstream>
#include "json.hpp"
#include "utility.h"
using json = nlohmann::json;
using namespace std;

vector<unsigned char> VtcBlockIndexer::CoinParams::magic;
string VtcBlockIndexer::CoinParams::bech32Prefix;
unsigned char VtcBlockIndexer::CoinParams::p2pkhVersion;
unsigned char VtcBlockIndexer::CoinParams::p2shVersion;

void VtcBlockIndexer::CoinParams::readFromFile(string fileName)
{
    cout << "Reading coin params from [" << fileName << "]" << endl;
    ifstream i(fileName);
    json j;
    i >> j;

    assert(j["magic"].is_string());
    assert(j["prefix_bech32"].is_string());
    assert(j["version_p2sh"].is_string());
    assert(j["version_p2pkh"].is_string());
    
    magic = VtcBlockIndexer::Utility::hexToBytes(j["magic"].get<string>());
    bech32Prefix = j["prefix_bech32"].get<string>();
    p2shVersion = (unsigned char) strtol(j["version_p2sh"].get<string>().c_str(), NULL, 16);
    p2pkhVersion = (unsigned char) strtol(j["version_p2pkh"].get<string>().c_str(), NULL, 16);
    
}
