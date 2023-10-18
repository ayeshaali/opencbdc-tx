// Copyright (c) 2021 MIT Digital Currency Initiative,
//                    Federal Reserve Bank of Boston
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "merkletree.hpp"
#include "crypto/sha256.h"
#include "parsec/agent/client.hpp"
#include "parsec/broker/impl.hpp"
#include "parsec/directory/impl.hpp"
#include "parsec/runtime_locking_shard/client.hpp"
#include "parsec/ticket_machine/client.hpp"
#include "parsec/util.hpp"
#include "wallet.hpp"

#include <lua.hpp>
#include <random>
#include <thread>

auto main() -> int {
    auto log
        = std::make_shared<cbdc::logging::log>(cbdc::logging::log_level::trace);

    // std::string leaves = "2258b49c14c69055e173816ac7750c7aee37a18d5e6fd740b78f0bfb4431e02928d97f1bbbb84705c60b288279eaf6900b5c46ee573673942d4bae9559539b4a";
    // log->trace("leaves: ", leaves);
    // cbdc::parsec::merkle_tree MT = cbdc::parsec::merkle_tree(log, leaves, 2);
    // log->trace("inserting");
    // std::string new_leaf = "076eba5021b964b931eaf7f0253c2eee8b3a8334f4ccf2251a547bb8f3878b10";
    // std::string hash = MT.insert(new_leaf);
    // log->trace("new_leaf: ", new_leaf);

    std::string left = "2258b49c14c69055e173816ac7750c7aee37a18d5e6fd740b78f0bfb4431e029";
    std::string right = "28d97f1bbbb84705c60b288279eaf6900b5c46ee573673942d4bae9559539b4a";
    log->trace("hashing");
    cbdc::parsec::mimc hasher =  cbdc::parsec::mimc(log);
    std::string hash = hasher.hash(left, right);
    log->trace(hash);
    return 0;
}
