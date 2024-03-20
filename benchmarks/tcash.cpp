// Copyright (c) 2021 MIT Digital Currency Initiative,
//                    Federal Reserve Bank of Boston
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// Note: Contains call to BENCHMARK_MAIN

#include "util/tc_primitives/merkletree.hpp"
#include "util/tc_primitives/verifier.hpp"
#include "util/tc_primitives/blind_sig.hpp"
#include "crypto/sha256.h"
#include <string>
#include <fstream>

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

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

class tcash : public ::benchmark::Fixture {
  protected:
    void SetUp(const ::benchmark::State&) override {
    }
    std::string leaves[5] = {"305abbd6d482f95ee1a66db2917aca912c58c78efe84582cca7a68a65de4153d",
                             "2b423d0d3935da94689486f7cbecd2cf7eeca4114ca1a3913b94d8d05a18817e",
                             "09d04f1abe72de5354f7e543cbffaae9e03e6cf192d8f20c9e6fab83724b0b5e",
                             "067619b0c4a5a3c977bfa973315bcd51167312e263e508985f590e72e77b1821",
                             "244c10e039e786b5a109641b35f3aa03ad9ef5e370e55c4db929bcf85e4afc8a" };
    
    std::string roots[5] = {"2bc0a0302b277a7c4c462848d39114e402b397913533f9236059052e3bda54f4",
                            "1fe8a6df0e437e1a314f648e4193f415cc400a4be8780378eb50a02905f6080b",
                            "1efe0339dbf40f64842d5a855990054ba8156c174cf32ad46c50696a45ce53b0",
                            "05c3fb846a9f6c4ef8d6fdefbdbd60bc0a7df8cab2ac0d3059d5a9438b0fc58f",
                            "0044e42f425b76804b90aa4f82d6c9c13bf6e7c79cd6f7cd964fa41f64669413" };
    
    std::string subtrees = "2fe54c60d3acabf3343a35b6eba15db4821b340f76e741e2249685ed4899af6c256a6135777eee2fd26f54b8b7037a25439d5235caee224154186d2b8a52e31d1151949895e82ab19924de92c40a3d6f7bcb60d92b00504b8199613683f0c20020121ee811489ff8d61f09fb89e313f14959a0f28bb428a20dba6b0b068b3bdb0a89ca6ffa14cc462cfedb842c30ed221a50a3d6bf022a6a57dc82ab24c157c924ca05c2b5cd42e890d6be94c68d0689f4f21c9cec9c0f13fe41d566dfb549591ccb97c932565a92c60156bdba2d08f3bf1377464e025cee765679e604a7315c19156fbd7d1a8bf5cba8909367de1b624534ebab4f0f79e003bccdd1b182bdb4261af8c1f0912e465744641409f622d466c3920ac6e5ff37e36604cb11dfff800058459724ff6ca5a1652fcbc3e82b93895cf08e975b19beab3f54c217d1c0071f04ef20dee48d39984d8eabe768a70eafa6310ad20849d4573c3c40c2ad1e301bea3dec5dab51567ce7e200a30f7ba6d4276aeaa53e2686f962a46c66d511e50ee0f941e2da4b9e31c3ca97a40d8fa9ce68d97c084177071b3cb46cd3372f0f1ca9503e8935884501bbaf20be14eb4c46b89772c97b96e3b2ebf3a36a948bbd133a80e30697cd55d8f7d4b0965b7be24057ba5dc3da898ee2187232446cb10813e6d8fc88839ed76e182c2a779af5b2c0da9dd18c90427a644f7e148a6253b61eb16b057a477f4bc8f572ea6bee39561098f78f15bfb3699dcbb7bd8db618540da2cb16a1ceaabf1c16b838f7a9e3f2a3a3088d9e0a6debaa748114620696ea24a3b3d822420b14b5d8cb6c28a574f01e98ea9e940551d2ebd75cee12649f9d198622acbd783d1b0d9064105b1fc8e4d8889de95c4c519b3f635809fe6afc0529d7ed391256ccc3ea596c86e933b89ff339d25ea8ddced975ae2fe30b5296d4";
};

// verify proof
BENCHMARK_F(tcash, verify_proof)(benchmark::State& state) {
    std::fstream myfile;
    myfile.open("sample_proofs.txt", std::ios::in);
    std::vector<std::string> pd;
    if (myfile.is_open()) {
        std::string proof;
        getline(myfile, proof);
        pd = split(proof, ",");
        myfile.close();
    }

    for(auto _ : state) {
        auto log
            = std::make_shared<cbdc::logging::log>(cbdc::logging::log_level::trace);
        cbdc::verifier v =  cbdc::verifier(log);
        state.ResumeTiming();
        bool result = v.verifyProof(pd[0], pd[1], pd[2], pd[3], pd[4], stoi(pd[5]), stoi(pd[6]));
        state.PauseTiming();
        ASSERT_EQ(1, result);
    }
}

// insert into merkle tree
BENCHMARK_F(tcash, insert_tree)(benchmark::State& state) {
    for(auto _ : state) {
        auto log
            = std::make_shared<cbdc::logging::log>(cbdc::logging::log_level::trace);
        cbdc::merkle_tree MT = cbdc::merkle_tree(log, subtrees, 0);
        state.ResumeTiming();
        std::string hash = MT.insert(leaves[0]);
        state.PauseTiming();
        ASSERT_EQ(roots[0], hash.substr(0,64));
    }
}