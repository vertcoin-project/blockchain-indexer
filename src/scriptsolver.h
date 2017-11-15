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


#ifndef SCRIPTSOLVER_H_INCLUDED
#define SCRIPTSOLVER_H_INCLUDED

#include <iostream>
#include <fstream>

#include "blockchaintypes.h"
using namespace std;

namespace VtcBlockIndexer {

/**
 * The ScriptSolver class provides methods to parse the bitcoin script language used in
 * transaction outputs and determine the public keys / addresses that can spend it.
 */

class ScriptSolver {
public:
    /** Constructs a BlockIndexer instance using the given block data directory
     */
    ScriptSolver();

    /** Read addresses from script
     */
    vector<string> getAddressesFromScript(vector<unsigned char> scriptString);

    /** Returns if the script is multisig
     */
    bool isMultiSig(vector<unsigned char> scriptString);

    /** Returns the number of required signatures
     */
    int requiredSignatures(vector<unsigned char> scriptString);
};

}

#endif // SCRIPTSOLVER_H_INCLUDED