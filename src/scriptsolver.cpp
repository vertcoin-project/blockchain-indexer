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
#include "scriptsolver.h"
#include "blockchaintypes.h"
#include "utility.h"
#include <iostream>
#include <sstream>
#include "leveldb/db.h"
//#include "hashing.h"
#include <memory>
#include <iomanip>

using namespace std;
VtcBlockIndexer::ScriptSolver::ScriptSolver() {

}

vector<string> VtcBlockIndexer::ScriptSolver::getAddressesFromScript(vector<unsigned char> script) {
    vector<string> addresses;
    uint64_t scriptSize = script.size();
    bool parsed = false;

    // The most common output script type that pays to hash160(pubKey)
    if(
        
            (25==scriptSize                  &&
            0x76==script.at(0)              &&  // OP_DUP
            0xA9==script.at(1)              &&  // OP_HASH160
              20==script.at(2)              &&  // OP_PUSHDATA(20)
            
            (0x88==script.at(scriptSize-2)   &&  // OP_EQUALVERIFY
            0xAC==script.at(scriptSize-1)))      // OP_CHECKSIG

            ||
// scripts appended with OP_NOP1. Since OP_NOP1 does nothing, this should still be valid.

            (25==scriptSize                  &&
                0x76==script.at(0)              &&  // OP_DUP
                0xA9==script.at(1)              &&  // OP_HASH160
                  20==script.at(2)              &&  // OP_PUSHDATA(20)
                
                (0x88==script.at(scriptSize-2)   &&  // OP_EQUALVERIFY
                0xB0==script.at(scriptSize-1)))      // OP_NOP1
    
                ||

            // scripts appended with OP_NOP. Since OP_NOP does nothing, this should still be valid.

            (26==scriptSize                  &&
            0x76==script.at(0)              &&  // OP_DUP
            0xA9==script.at(1)              &&  // OP_HASH160
              20==script.at(2)              &&  // OP_PUSHDATA(20)

            
            (0x88==script.at(scriptSize-3)   &&  // OP_EQUALVERIFY
            0xAC==script.at(scriptSize-2)   &&  // OP_CHECKSIG
            0x61==script.at(scriptSize-1)))     // OP_NOP
            
              
        
    ) { 
        addresses.push_back(VtcBlockIndexer::Utility::ripeMD160ToP2PKAddress(vector<unsigned char>(&script[3], &script[23])));
        parsed = true;
    }

    // Output script commonly found in block reward TX, that pays to an explicit pubKey
    if(
            67==scriptSize                &&  
              65==script.at(0)            &&  // OP_PUSHDATA(65)
            0xAC==script.at(scriptSize-1) // OP_CHECKSIG
              
    ) {
        addresses.push_back(VtcBlockIndexer::Utility::publicKeyToAddress(vector<unsigned char>(&script[1], &script[66])));
        parsed = true;
    }

    // Pay to compressed pubkey script
    if(
        35==scriptSize                &&
        0x21==script.at(0)            &&  // OP_PUSHDATA(33)
        0xAC==script.at(scriptSize-1)     // OP_CHECKSIG
    ) {
        addresses.push_back(VtcBlockIndexer::Utility::publicKeyToAddress(vector<unsigned char>(&script[1], &script[34])));
        parsed = true;
    }
    
    // P2WSH
    if(
        22 == scriptSize            &&
        0x00 == script.at(0)        &&  
        0x14 == script.at(1)        
    ) {
        addresses.push_back(VtcBlockIndexer::Utility::bech32Address(vector<unsigned char>(&script[2], &script[22])));
        parsed = true;
    }

    // P2WPKH
    if(
        34 == scriptSize            &&
        0x00 == script.at(0)        &&  
        0x20 == script.at(1)        
    ) {
        addresses.push_back(VtcBlockIndexer::Utility::bech32Address(vector<unsigned char>(&script[2], &script[34])));
        parsed = true;
    }

    // NULLDATA script. starts with OP_RETURN followed by arbitrary data and no further opcodes. Not needed to be stored in UTXO database. So Ignore.
    if(
        scriptSize > 0        && 
        0x6A == script.at(0)    
    ) {
        uint32_t pos = 1;
        
        bool foundOpcodes = false;
        while(pos < script.size()) {
            if(script.at(pos) >= 0x01 && script.at(pos) <= 0x4B) { 
                pos += script.at(pos) + 1;
            } else {
                foundOpcodes = true;
                break;
            } 
        }

        if(!foundOpcodes)
            parsed = true;
    }

    // A modern output script type, that pays to hash160(script)
    if(
        23 == scriptSize                &&
            0xA9==script.at(0)             &&  // OP_HASH160
              20==script.at(1)             &&  // OP_PUSHDATA(20)
            0x87==script.at(scriptSize-1)      // OP_EQUAL
              
        
    ) {
        addresses.push_back(VtcBlockIndexer::Utility::ripeMD160ToP2SHAddress(vector<unsigned char>(&script[2], &script[22])));
        parsed = true;
    }

    // OP_PUSHDATA(32) + data only (Found in litecoin chain - nonstandard script)
    // Public block explorers show these as "unknown" (https://bchain.info/LTC/tx/265278e51d1b29cdce906a858251b7ce15e2dab09de7dede0acb4c629f780b91)
    if(
        
            33==scriptSize                  &&
            0x20==script.at(0)             
            
        
    ) {
        
        parsed = true;
    }

    // OP_PUSHDATA(36) + data only (Found in litecoin chain - nonstandard script)
    // Public block explorers show these as "unknown" (http://explorer.litecoin.net/tx/936e8ed1cfca736320fdced61c2d03886b232497ea975e41d101f1d83bb74c44)
    if(
        
            37==scriptSize                  &&
            0x24==script.at(0)             
            
        
    ) {
        
        parsed = true;
    }

    // OP_PUSHDATA(20) + data only. Unparseable (BTC)
    // Public block explorers show these as "unknown" (https://blockchain.info/tx/b8fd633e7713a43d5ac87266adc78444669b987a56b3a65fb92d58c2c4b0e84d)
    if(
        
            24==scriptSize                  &&
            0x14==script.at(0)             
            
        
    ) {
        
        parsed = true;
    }


    // Unknown (seems malformed) output script found on Litecoin in p2pool blocks
    // For example https://bchain.info/LTC/tx/8f1220670b5d4ade8f9c6a82fde3d88a28d2e1c290f2edc6d7a7a13aa0352fc7 
    if(
        6 == scriptSize                &&
            0x73==script.at(0)             &&  // OP_IFDUP
            0x63==script.at(1)             &&  // OP_IF
            0x72==script.at(2)             &&  // OP_2SWAP
            0x69==script.at(3)             &&  // OP_VERIFY
            0x70==script.at(4)             &&  // OP_2OVER
            0x74==script.at(5)              // OP_DEPTH
            
        
    ) {

        parsed = true;
    }

    if(isMultiSig(script)) {
        uint32_t pos = 1;
        while(pos < script.size()-2) {
            if(script.at(pos) == 0x21) {
                addresses.push_back(VtcBlockIndexer::Utility::publicKeyToAddress(vector<unsigned char>(&script[pos+1], &script[pos+34]))); 
                pos += 34;
                parsed = true;
            }
            else if(script.at(pos) == 0x41) {
                addresses.push_back(VtcBlockIndexer::Utility::publicKeyToAddress(vector<unsigned char>(&script[pos+1], &script[pos+66])));
                pos += 66;
                parsed = true;
            }
            else
            {
                pos = script.size();
            }
        }
        
    }

    // A challenge: anyone who can find X such that 0==RIPEMD160(X) stands to earn a bunch of coins
    
    if(
        0x76==script[0] &&                  // OP_DUP
        0xA9==script[1] &&                  // OP_HASH160
        0x00==script[2] &&                  // OP_0
        0x88==script[3] &&                  // OP_EQUALVERIFY
        0xAC==script[4]                     // OP_CHECKSIG
    ) {
        parsed = true;
    }




    if(!parsed) {
        stringstream ssScript;
        for(uint64_t i = 0; i < scriptSize; i++)
        {
            ssScript << hex << setw(2) << setfill('0') << static_cast<int>(script.at(i));
        }
        cout << "Unrecognized script : [" << ssScript.str() << "]" << endl;
    }

    return addresses;
/*
    

    int m = 0;
    int n = 0;
    std::vector<uint160_t> addresses;
    if(
        isMultiSig(
            m,
            n,
            addresses,
            script,
            scriptSize
        )
    ) {
        packMultiSig(pubKeyHash, addresses, m, n);
        addrType[0] = 8;
        return 4;
    }

    // Broken output scripts that were created by p2pool for a while
    if(
        0x73==script[0] &&                  // OP_IFDUP
        0x63==script[1] &&                  // OP_IF
        0x72==script[2] &&                  // OP_2SWAP
        0x69==script[3] &&                  // OP_VERIFY
        0x70==script[4] &&                  // OP_2OVER
        0x74==script[5]                     // OP_DEPTH
    ) {
        return -2;
    }

    // A non-functional "comment" script
    if(isCommentScript(script, scriptSize)) {
        return -3;
    }

    
    return addresses;*/
}

bool VtcBlockIndexer::ScriptSolver::isMultiSig(vector<unsigned char> script) {
    if(script.size() == 0) return false;
    return (script.at(script.size()-1) == 0xAE);
}

int VtcBlockIndexer::ScriptSolver::requiredSignatures(vector<unsigned char> script) {
    if(!isMultiSig(script)) return -1;

    return (int)script.at(0);
}