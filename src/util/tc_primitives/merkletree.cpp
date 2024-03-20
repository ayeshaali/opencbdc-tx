#include "merkletree.hpp"
#include <string>

namespace cbdc {
    merkle_tree::merkle_tree(std::shared_ptr<logging::log> log,
                             std::string subtrees, 
                             uint64_t num_leaves,
                             uint64_t levels)
                             : m_log(std::move(log)),
                             _hasher(m_log),
                             m_num_leaves(std::move(num_leaves)),
                             TREE_DEPTH(std::move(levels)) {
        for (size_t i = 0; i < TREE_DEPTH; i++) {
            m_subtrees.push_back(subtrees.substr(i*NODE_LENGTH*2, NODE_LENGTH*2));
        }
    }
    
    auto merkle_tree::insert(std::string leaf) -> std::string {
        std::vector<std::string> subtree_copy(m_subtrees); 

        uint64_t currentIndex = m_num_leaves;
        std::string currentLevelHash = leaf;
        std::string left;
        std::string right; 

        for (size_t i = 0; i < TREE_DEPTH; i++) {
            if (currentIndex % 2 == 0) {
                left = currentLevelHash;
                right = zeros(i+1);
                subtree_copy[i] = currentLevelHash;
            } else {
                left = subtree_copy[i];
                right = currentLevelHash;
            }
            currentLevelHash = _hasher.hash(left, right); 
            currentIndex /= 2;
        }

        std::string output;
        output.reserve(NODE_LENGTH*(TREE_DEPTH+1));
        output.append(currentLevelHash);
        for (size_t i = 0; i < TREE_DEPTH; i++) {
            output.append(subtree_copy[i]);
        }
        return output;
    }

    auto merkle_tree::root_from_leaves(std::string leaves, uint64_t num_insert) -> std::string {
        std::vector<std::string> m_leaves;
        for (size_t i = 0; i < num_insert; i++) {
            m_leaves.push_back(leaves.substr(i*NODE_LENGTH*2, NODE_LENGTH*2));
        }

        while (m_leaves.size() > 1) {
            std::vector<std::string> next_level;
            for (size_t j = 0; j < m_leaves.size(); j+=2) {
                std::string hash;
                hash = _hasher.hash(m_leaves[j], m_leaves[j+1]);
                next_level.push_back(hash);
            }
            m_leaves = next_level;
        }            

        return m_leaves[0];
    }

    auto merkle_tree::zeros(int i) -> std::string {
        return zeros_map[i];
    }
}
