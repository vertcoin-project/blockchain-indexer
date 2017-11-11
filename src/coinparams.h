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

#include <vector>
#include <string>

using namespace std;

namespace VtcBlockIndexer {
    
    /**
     * The CoinParms class provides static access to the coin parameters and a method
     * to read them from a JSON file
     */
    
    class CoinParams {
        public:
            static void readFromFile(string fileName);
            static vector<unsigned char> magic;
            static string bech32Prefix;
            static unsigned char p2pkhVersion;
            static unsigned char p2shVersion;
            ~CoinParams();
            
        private:
            CoinParams() {}
    };
}
