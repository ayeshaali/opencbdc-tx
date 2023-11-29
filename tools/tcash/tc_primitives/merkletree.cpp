#include "merkletree.hpp"
#include <string>

namespace cbdc::parsec {
    merkle_tree::merkle_tree(std::shared_ptr<logging::log> log,
                             std::string leaves, 
                             uint64_t num_leaves)
                             : m_log(std::move(log)),
                             _hasher(m_log) {
        for (size_t i = 0; i < num_leaves; i++) {
            m_leaves.push_back(leaves.substr(i*LEAF_LENGTH*2, LEAF_LENGTH*2));
        }
    }
    
    auto merkle_tree::insert(std::string leaf) -> std::string {
        std::vector<std::string> leaves_copy(m_leaves); 
        leaves_copy.push_back(leaf);

        m_log->trace(leaf);

        // Changed from using subtrees:
        // Starting with the leaves, hash up every pair of nodes into the next level of the MT
        // Use the zeros hashes for single nodes (that do not have pairs)
        for (size_t i = 0; i < TREE_DEPTH; i++) {
            size_t level_length = leaves_copy.size();
            std::vector<std::string> next_level;
            for (size_t j = 0; j < leaves_copy.size(); j+=2) {
                std::string hash;
                if (j+1 < level_length) {
                    hash = _hasher.hash(leaves_copy[j], leaves_copy[j+1]);
                } else {
                    hash = _hasher.hash(leaves_copy[j], zeros(i+1));
                }
                next_level.push_back(hash);
            }
            leaves_copy = next_level;
        }

        return leaves_copy[0];
    }

    auto merkle_tree::zeros(int i) -> std::string {
        return zeros_map[i];
    }
}
