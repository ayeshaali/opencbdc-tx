// Copyright (c) 2021 MIT Digital Currency Initiative,
//                    Federal Reserve Bank of Boston
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tc_primitives/merkletree.hpp"
#include "tc_primitives/verifier.hpp"

#include <lua.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <thread>

std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

auto main() -> int {
    auto log
        = std::make_shared<cbdc::logging::log>(cbdc::logging::log_level::trace);

    // std::string leaves = "2258b49c14c69055e173816ac7750c7aee37a18d5e6fd740b78f0bfb4431e02928d97f1bbbb84705c60b288279eaf6900b5c46ee573673942d4bae9559539b4a";
    // cbdc::parsec::merkle_tree MT = cbdc::parsec::merkle_tree(log, leaves, 2);
    // log->trace("inserting");
    // std::string new_leaf = "076eba5021b964b931eaf7f0253c2eee8b3a8334f4ccf2251a547bb8f3878b10";
    // std::string hash = MT.insert(new_leaf);
    // log->trace("new root: ", hash);

    // std::string hash = "2fe54c60d3acabf3343a35b6eba15db4821b340f76e741e2249685ed4899af6c";
    // log->trace("hashing");
    // cbdc::parsec::mimc hasher =  cbdc::parsec::mimc(log);
    // std::string hash = hasher.hash(left, right);
    // log->trace(hash);

    cbdc::parsec::verifier v =  cbdc::parsec::verifier(log);
    std::fstream myfile;
    myfile.open("sample_proofs.txt", std::ios::in);

    if (myfile.is_open()) {
        std::string proof;
        while (getline(myfile, proof)) {
            std::vector<std::string> pd = split(proof, ",");
            bool result = v.verifyProof(pd[0], pd[1], pd[2], pd[3], pd[4], stoi(pd[5]), stoi(pd[6]));
            log->trace("Proof Result: ");
            log->trace(result);
        }
        myfile.close();
    }

    return 0;
}
