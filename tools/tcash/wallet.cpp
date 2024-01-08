// Copyright (c) 2021 MIT Digital Currency Initiative,
//                    Federal Reserve Bank of Boston
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.hpp"

#include "parsec/util.hpp"
#include "util/common/config.hpp"
#include "util/common/random_source.hpp"
#include "util/serialization/format.hpp"

#include <future>
#include <secp256k1_schnorrsig.h>

namespace cbdc::parsec {
    account_wallet::account_wallet(std::shared_ptr<logging::log> log,
                                   std::shared_ptr<broker::interface> broker,
                                   std::shared_ptr<agent::rpc::client> agent,
                                   cbdc::buffer deposit_contract_key,
                                   cbdc::buffer withdraw_contract_key)
        : m_log(std::move(log)),
          m_agent(std::move(agent)),
          m_broker(std::move(broker)),
          m_TC_deposit_contract_key(std::move(deposit_contract_key)),
          m_TC_withdraw_contract_key(std::move(withdraw_contract_key)) {
        auto rnd = cbdc::random_source(cbdc::config::random_source);
        m_privkey = rnd.random_hash();
        m_pubkey = cbdc::pubkey_from_privkey(m_privkey, m_secp.get());
        constexpr auto account_prefix = "account_";
        m_account_key.append(account_prefix, std::strlen(account_prefix));
        m_account_key.append(m_pubkey.data(), m_pubkey.size());
    }

    auto account_wallet::init(uint64_t value,
                              const std::function<void(bool)>& result_callback)
        -> bool {
        auto init_account = cbdc::buffer();
        auto ser = cbdc::buffer_serializer(init_account);
        ser << value;
        auto res = put_row(m_broker,
                           m_account_key,
                           init_account,
                           [&, result_callback, value](bool ret) {
                               if(ret) {
                                   m_balance = value;
                               }
                               result_callback(ret);
                           });
        return res;
    }

    auto account_wallet::get_pubkey() const -> pubkey_t {
        return m_pubkey;
    }

    auto account_wallet::deposit(std::string note,
                             const std::function<void(bool)>& result_callback)
        -> bool {
        auto params = cbdc::buffer();
        params.append(m_pubkey.data(), m_pubkey.size());
        params.append(note.c_str(), note.size());
        m_log->trace(note, note.size());
        return execute_params(m_TC_deposit_contract_key, params, false, result_callback);
    }

    auto account_wallet::withdraw(std::string proof, 
                                  std::string _root, 
                                  std::string _nullifierHash, 
                                  std::string _recipient, 
                                  std::string _relayer, 
                                  std::string _fee,
                                  std::string _refund,
                                  const std::function<void(bool)>& result_callback)
        -> bool {
        auto params = cbdc::buffer();
        params.append(m_pubkey.data(), m_pubkey.size());
        params.append(proof.c_str(), proof.size());
        params.append(_root.c_str(), _root.size());
        params.append(_nullifierHash.c_str(), _nullifierHash.size());
        params.append(_recipient.c_str(), _recipient.size());
        params.append(_relayer.c_str(), _relayer.size());
        params.append(_fee.c_str(), _fee.size());
        params.append(_refund.c_str(), _refund.size());
        return execute_params(m_TC_withdraw_contract_key, params, false, result_callback);
    }

    auto account_wallet::execute_params(
        cbdc::buffer contract_key,
        cbdc::buffer params,
        bool dry_run,
        const std::function<void(bool)>& result_callback) -> bool {
        auto send_success = m_agent->exec(
            contract_key,
            std::move(params),
            dry_run,
            [&, result_callback](agent::interface::exec_return_type res) {
                auto success = std::holds_alternative<agent::return_type>(res);
                m_log->trace("resulted");
                // if(success) {
                //     auto updates = std::get<agent::return_type>(res);
                //     for (auto it : updates) 
                //         m_log->trace(it.first.c_str());
                // }
                result_callback(success);
            });
        return send_success;
    }

    auto account_wallet::get_balance() const -> uint64_t {
        return m_balance;
    }
}
