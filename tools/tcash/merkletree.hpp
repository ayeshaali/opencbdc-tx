// Copyright (c) 2021 MIT Digital Currency Initiative,
//                    Federal Reserve Bank of Boston
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "parsec/agent/client.hpp"
#include "parsec/broker/interface.hpp"
#include "util/common/keys.hpp"
#include "util/common/logging.hpp"
#include "mimc.hpp"
#include <string>

namespace cbdc::parsec {
    class merkle_tree {
      public:
        /// Constructor.
        merkle_tree(std::shared_ptr<logging::log> log, std::string leaves, uint64_t num_leaves);
        
        /// Insert a new leaf into the Merkle Tree
        /// \param leaf new leaf to insert
        /// \return the value of the new root
        auto insert(std::string leaf) -> std::string;
        
        static const size_t TREE_DEPTH = 2;
        static const size_t LEAF_LENGTH = 32;

      private:
        std::shared_ptr<logging::log> m_log;

        std::vector<std::string> m_leaves;

        auto zeros(int i) -> std::string;

        mimc _hasher;

        inline static std::unordered_map<int, std::string> zeros_map = {
          {0, "2fe54c60d3acabf3343a35b6eba15db4821b340f76e741e2249685ed4899af6c"},
          {1, "256a6135777eee2fd26f54b8b7037a25439d5235caee224154186d2b8a52e31d"}
        };

        // zeros_map[1] = "\x256a6135777eee2fd26f54b8b7037a25439d5235caee224154186d2b8a52e31d";
        // zeros_map[2] = "\x1151949895e82ab19924de92c40a3d6f7bcb60d92b00504b8199613683f0c200";
        // zeros_map[3] = "\x20121ee811489ff8d61f09fb89e313f14959a0f28bb428a20dba6b0b068b3bdb";
        // zeros_map[4] = "\x0a89ca6ffa14cc462cfedb842c30ed221a50a3d6bf022a6a57dc82ab24c157c9";
        // zeros_map[5] = "\x24ca05c2b5cd42e890d6be94c68d0689f4f21c9cec9c0f13fe41d566dfb54959";
        // zeros_map[6] = "\x1ccb97c932565a92c60156bdba2d08f3bf1377464e025cee765679e604a7315c";
        // zeros_map[7] = "\x19156fbd7d1a8bf5cba8909367de1b624534ebab4f0f79e003bccdd1b182bdb4";
        // zeros_map[8] = "\x261af8c1f0912e465744641409f622d466c3920ac6e5ff37e36604cb11dfff80";
        // zeros_map[9] = "\x0058459724ff6ca5a1652fcbc3e82b93895cf08e975b19beab3f54c217d1c007";
        // zeros_map[10] = "\x1f04ef20dee48d39984d8eabe768a70eafa6310ad20849d4573c3c40c2ad1e30";
        // zeros_map[11] = "\x1bea3dec5dab51567ce7e200a30f7ba6d4276aeaa53e2686f962a46c66d511e5";
        // zeros_map[12] = "\x0ee0f941e2da4b9e31c3ca97a40d8fa9ce68d97c084177071b3cb46cd3372f0f";
        // zeros_map[13] = "\x1ca9503e8935884501bbaf20be14eb4c46b89772c97b96e3b2ebf3a36a948bbd";
        // zeros_map[14] = "\x133a80e30697cd55d8f7d4b0965b7be24057ba5dc3da898ee2187232446cb108";
        // zeros_map[15] = "\x13e6d8fc88839ed76e182c2a779af5b2c0da9dd18c90427a644f7e148a6253b6";
        // zeros_map[16] = "\x1eb16b057a477f4bc8f572ea6bee39561098f78f15bfb3699dcbb7bd8db61854";
        // zeros_map[17] = "\x0da2cb16a1ceaabf1c16b838f7a9e3f2a3a3088d9e0a6debaa748114620696ea";
        // zeros_map[18] = "\x24a3b3d822420b14b5d8cb6c28a574f01e98ea9e940551d2ebd75cee12649f9d";
        // zeros_map[19] = "\x198622acbd783d1b0d9064105b1fc8e4d8889de95c4c519b3f635809fe6afc05";
        // zeros_map[20] = "\x29d7ed391256ccc3ea596c86e933b89ff339d25ea8ddced975ae2fe30b5296d4";
        // zeros_map[21] = "\x19be59f2f0413ce78c0c3703a3a5451b1d7f39629fa33abd11548a76065b2967";
        // zeros_map[22] = "\x1ff3f61797e538b70e619310d33f2a063e7eb59104e112e95738da1254dc3453";
        // zeros_map[23] = "\x10c16ae9959cf8358980d9dd9616e48228737310a10e2b6b731c1a548f036c48";
        // zeros_map[24] = "\x0ba433a63174a90ac20992e75e3095496812b652685b5e1a2eae0b1bf4e8fcd1";
        // zeros_map[25] = "\x019ddb9df2bc98d987d0dfeca9d2b643deafab8f7036562e627c3667266a044c";
        // zeros_map[26] = "\x2d3c88b23175c5a5565db928414c66d1912b11acf974b2e644caaac04739ce99";
        // zeros_map[27] = "\x2eab55f6ae4e66e32c5189eed5c470840863445760f5ed7e7b69b2a62600f354";
        // zeros_map[28] = "\x002df37a2642621802383cf952bf4dd1f32e05433beeb1fd41031fb7eace979d";
        // zeros_map[29] = "\x104aeb41435db66c3e62feccc1d6f5d98d0a0ed75d1374db457cf462e3a1f427";
        // zeros_map[30] = "\x1f3c6fd858e9a7d4b0d1f38e256a09d81d5a5e3c963987e2d4b814cfab7c6ebb";
        // zeros_map[31] = "\x2c7a07d20dff79d01fecedc1134284a8d08436606c93693b67e333f671bf69cc";
    };
}

