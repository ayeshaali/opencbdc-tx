// Copyright (c) 2021 MIT Digital Currency Initiative,
//                    Federal Reserve Bank of Boston
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef OPENCBDC_TX_SRC_PARSEC_TRANSACTIONS_ACCOUNT_WALLET_H_
#define OPENCBDC_TX_SRC_PARSEC_TRANSACTIONS_ACCOUNT_WALLET_H_

#include "parsec/agent/client.hpp"
#include "parsec/broker/interface.hpp"
#include "util/common/keys.hpp"
#include "util/common/logging.hpp"
#include <secp256k1.h>

struct Deposit {   
  int nullifier;
  int secret;           
  cbdc::buffer preimage;      
  cbdc::buffer commitment;
  cbdc::buffer nullifierHash;      
};

namespace cbdc::parsec {
    /// Manages an account-based wallet.
    class account_wallet {
      public:
        /// Constructor.
        /// \param log log instance.
        /// \param broker broker instance to use for initializing account.
        /// \param agent agent instance to use for making pay requests and
        ///              updating the account balance.
        /// \param pay_contract_key key where pay contract bytecode is located.
        account_wallet(std::shared_ptr<logging::log> log,
                       std::shared_ptr<broker::interface> broker,
                       std::shared_ptr<agent::rpc::client> agent,
                       cbdc::buffer deposit_contract_key,
                       cbdc::buffer withdraw_contract_key);

        /// Initializes the account by generating a new public/private key pair
        /// and inserting a new account with the given initial balance.
        /// \param value initial balance of the new account.
        /// \param result_callback function to call with initialization result.
        /// \return true if the new account was created successfully.
        auto init(uint64_t value, const std::function<void(bool)>& result_callback) -> bool;
        
        /// Deposit the given amount from the account managed by this wallet to the
        /// TC contract. Blocks until the contract execution has completed.
        /// Updates the internal account balance with the most recent balance.
        /// \param note deposit note
        /// \param result_callback function to call with pay result.
        /// \return true if the transaction was successful.
        auto deposit(std::string note,
                 const std::function<void(bool)>& result_callback) -> bool;
        
        /// Deposit the given amount from the account managed by this wallet to the
        /// ToT contract. Blocks until the contract execution has completed.
        /// Updates the internal account balance with the most recent balance.
        /// \param MT_index deposit note 
        /// \param note index of MT to deposit to
        /// \param result_callback function to call with pay result.
        /// \return true if the transaction was successful.
        auto deposit_ToT(std::string MT_index, std::string note,
                 const std::function<void(bool)>& result_callback) -> bool;
        
        /// Withdraw the given bank note from the TC contract to this wallet to the
        /// TC contract. Blocks until the contract execution has completed.
        /// Updates the internal account balance with the most recent balance.
        /// \param note preimage of commitment (nullifier + secret)
        /// \param result_callback function to call with pay result.
        /// \return true if the transaction was successful.
        auto withdraw(std::string proof, 
                      std::string _root, 
                      std::string _nullifierHash, 
                      std::string _recipient, 
                      std::string _relayer, 
                      std::string _fee,
                      std::string _refund,
                      const std::function<void(bool)>& result_callback) -> bool;

        /// Return the public key associated with this account.
        /// \return public key.
        [[nodiscard]] auto get_pubkey() const -> pubkey_t;

        /// Return the balance held in this account as of the most recent
        /// balance update.
        /// \return account balance.
        [[nodiscard]] auto get_balance() const -> uint64_t;

      private:
        privkey_t m_privkey{};
        pubkey_t m_pubkey{};
        uint64_t m_balance{};

        std::shared_ptr<logging::log> m_log;
        std::shared_ptr<agent::rpc::client> m_agent;
        std::shared_ptr<broker::interface> m_broker;
        cbdc::buffer m_TC_deposit_contract_key;
        cbdc::buffer m_TC_withdraw_contract_key;
        cbdc::buffer m_account_key;

        std::unique_ptr<secp256k1_context,
                        decltype(&secp256k1_context_destroy)>
            m_secp{secp256k1_context_create(SECP256K1_CONTEXT_SIGN),
                   &secp256k1_context_destroy};
        
        auto execute_params(cbdc::buffer contract_key,
                            cbdc::buffer params,
                            bool dry_run,
                            const std::function<void(bool)>& result_callback)
            -> bool;
    };
}

#endif
