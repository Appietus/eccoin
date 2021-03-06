// This file is part of the Eccoin project
// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2014-2018 The Eccoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcclient.h"

#include "rpcprotocol.h"
#include "util/util.h"

#include <set>
#include <stdint.h>

#include <boost/algorithm/string/case_conv.hpp> // for to_lower()
#include <univalue.h>

class CRPCConvertParam
{
public:
    std::string methodName; //! method whose params want conversion
    int paramIdx; //! 0-based idx of param to convert
};

static const CRPCConvertParam vRPCConvertParams[] = {{"stop", 0}, {"setmocktime", 0}, {"getaddednodeinfo", 0},
    {"generate", 0}, {"generatepos", 0}, {"getnetworkhashps", 0}, {"getnetworkhashps", 1}, {"sendtoaddress", 1},
    {"sendtoaddress", 4}, {"settxfee", 0}, {"getreceivedbyaddress", 1}, {"listreceivedbyaddress", 0},
    {"listreceivedbyaddress", 1}, {"listreceivedbyaddress", 2}, {"getbalance", 1}, {"getbalance", 2},
    {"getblockhash", 0}, {"move", 2}, {"move", 3}, {"sendfrom", 2}, {"sendfrom", 3}, {"listtransactions", 0},
    {"listtransactions", 1}, {"listtransactions", 2}, {"walletpassphrase", 1}, {"walletpassphrase", 2},
    {"getblocktemplate", 0}, {"listsinceblock", 1}, {"listsinceblock", 2}, {"sendmany", 1}, {"sendmany", 2},
    {"sendmany", 4}, {"addmultisigaddress", 0}, {"addmultisigaddress", 1}, {"createmultisig", 0}, {"createmultisig", 1},
    {"listunspent", 0}, {"listunspent", 1}, {"listunspent", 2}, {"getblock", 1}, {"getblockheader", 1},
    {"gettransaction", 1}, {"getrawtransaction", 1}, {"createrawtransaction", 0}, {"createrawtransaction", 1},
    {"createrawtransaction", 2}, {"signrawtransaction", 1}, {"signrawtransaction", 2}, {"sendrawtransaction", 1},
    {"fundrawtransaction", 1}, {"gettxout", 1}, {"gettxout", 2}, {"gettxoutproof", 0}, {"lockunspent", 0},
    {"lockunspent", 1}, {"importprivkey", 2}, {"importaddress", 1}, {"importaddress", 2}, {"importpubkey", 2},
    {"verifychain", 0}, {"verifychain", 1}, {"keypoolrefill", 0}, {"getrawmempool", 0}, {"estimatefee", 0},
    {"prioritisetransaction", 1}, {"prioritisetransaction", 2}, {"setban", 2}, {"setban", 3}, {"generatetoaddress", 0},
    {"generatetoaddress", 2}};

class CRPCConvertTable
{
private:
    std::set<std::pair<std::string, int> > members;


public:
    CRPCConvertTable();

    bool convert(const std::string &method, int idx) { return (members.count(std::make_pair(method, idx)) > 0); }
};

CRPCConvertTable::CRPCConvertTable()
{
    const unsigned int n_elem = (sizeof(vRPCConvertParams) / sizeof(vRPCConvertParams[0]));

    for (unsigned int i = 0; i < n_elem; i++)
    {
        members.insert(std::make_pair(vRPCConvertParams[i].methodName, vRPCConvertParams[i].paramIdx));
    }
}

static CRPCConvertTable rpcCvtTable;

/** Non-RFC4627 JSON parser, accepts internal values (such as numbers, true, false, null)
 * as well as objects and arrays.
 */
UniValue ParseNonRFCJSONValue(const std::string &strVal)
{
    UniValue jVal;
    if (!jVal.read(std::string("[") + strVal + std::string("]")) || !jVal.isArray() || jVal.size() != 1)
        throw std::runtime_error(std::string("Error parsing JSON:") + strVal);
    return jVal[0];
}

/** Convert strings to command-specific RPC representation */
UniValue RPCConvertValues(const std::string &strMethod, const std::vector<std::string> &strParams)
{
    UniValue params(UniValue::VARR);

    for (unsigned int idx = 0; idx < strParams.size(); idx++)
    {
        const std::string &strVal = strParams[idx];

        if (!rpcCvtTable.convert(strMethod, idx))
        {
            // insert string value directly
            params.push_back(strVal);
        }
        else
        {
            // parse string as JSON, insert bool/number/object/etc. value
            params.push_back(ParseNonRFCJSONValue(strVal));
        }
    }

    return params;
}
