// Copyright (c) 2021 MIT Digital Currency Initiative,
//                    Federal Reserve Bank of Boston
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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

auto main(int argc, char** argv) -> int {
    auto log
        = std::make_shared<cbdc::logging::log>(cbdc::logging::log_level::trace);

    auto sha2_impl = SHA256AutoDetect();
    log->trace("using sha2: ", sha2_impl);
   
    if(argc < 2) {
        log->error("Not enough arguments");
        return 1;
    }

    auto cfg = cbdc::parsec::read_config(argc - 2, argv);
    if(!cfg.has_value()) {
        log->error("Error parsing options");
        return 1;
    }

    auto args = cbdc::config::get_args(argc, argv);
    auto n_wallets = std::stoull(args.back());
    if(n_wallets < 1) {
        log->error("Must be at least one wallet");
        return 1;
    }

    log->trace("Connecting to shards");
    auto shards = std::vector<
        std::shared_ptr<cbdc::parsec::runtime_locking_shard::interface>>();
    for(const auto& shard_ep : cfg->m_shard_endpoints) {
        auto client = std::make_shared<
            cbdc::parsec::runtime_locking_shard::rpc::client>(
            std::vector<cbdc::network::endpoint_t>{shard_ep});
        if(!client->init()) {
            log->error("Error connecting to shard");
            return 1;
        }
        shards.emplace_back(client);
    }
    log->trace("Connected to shards");

    log->trace("Connecting to ticket machine");
    auto ticketer
        = std::make_shared<cbdc::parsec::ticket_machine::rpc::client>(
            std::vector<cbdc::network::endpoint_t>{
                cfg->m_ticket_machine_endpoints});
    if(!ticketer->init()) {
        log->error("Error connecting to ticket machine");
        return 1;
    }
    log->trace("Connected to ticket machine");

    auto directory
        = std::make_shared<cbdc::parsec::directory::impl>(shards.size());
    auto broker = std::make_shared<cbdc::parsec::broker::impl>(
        std::numeric_limits<size_t>::max(),
        shards,
        ticketer,
        directory,
        log);

    auto contract_file = args[args.size() - 2];
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dofile(L, contract_file.c_str());
    lua_getglobal(L, "gen_bytecode");
    if(lua_pcall(L, 0, 2, 0) != 0) {
        log->error("Contract bytecode generation failed, with error:",
                   lua_tostring(L, -1));
        return 1;
    }
    auto tc_deposit_contract = cbdc::buffer();
    auto tc_withdraw_contract = cbdc::buffer();
    tc_deposit_contract = cbdc::buffer::from_hex(lua_tostring(L, -2)).value();
    tc_withdraw_contract = cbdc::buffer::from_hex(lua_tostring(L, -1)).value();
    auto tc_deposit_contract_key = cbdc::buffer();
    auto tc_withdraw_contract_key = cbdc::buffer();
    tc_deposit_contract_key.append("tc_deposit_contract", 11);
    tc_withdraw_contract_key.append("tc_withdraw_contract", 11);

    log->trace("Inserting TC contract");
    auto init_error = std::atomic_bool{false};
    auto ret = cbdc::parsec::put_row(
            broker,
            tc_deposit_contract_key,
            tc_deposit_contract,
            [&](bool res) {
                if(!res) {
                    init_error = true;
                } else {
                    log->info("Inserted TC deposit contract");
                }
            });
    if(!ret || init_error) {
        log->error("Error adding deposit contract");
        return 2;
    }

    init_error = false;
    ret = cbdc::parsec::put_row(
            broker,
            tc_withdraw_contract_key,
            tc_withdraw_contract,
            [&](bool res) {
                if(!res) {
                    init_error = true;
                } else {
                    log->info("Inserted TC withdraw contract");
                }
            });
    if(!ret || init_error) {
        log->error("Error adding withdraw contract");
        return 2;
    }

    auto agents
        = std::vector<std::shared_ptr<cbdc::parsec::agent::rpc::client>>();
    for(auto& a : cfg->m_agent_endpoints) {
        auto agent = std::make_shared<cbdc::parsec::agent::rpc::client>(
            std::vector<cbdc::network::endpoint_t>{a});
        if(!agent->init()) {
            log->error("Error connecting to agent");
            return 1;
        }
        agents.emplace_back(agent);
    }

    auto wallets = std::vector<cbdc::parsec::account_wallet>();
    for(size_t i = 0; i < n_wallets; i++) {
        auto agent_idx = i % agents.size();
        wallets.emplace_back(log, broker, agents[agent_idx], tc_deposit_contract_key, tc_withdraw_contract_key);
    }

    constexpr auto init_balance = 10000;
    auto init_count = std::atomic<size_t>();
    init_error = false;
    for(size_t i = 0; i < n_wallets; i++) {
        ret = wallets[i].init(init_balance, [&](bool res) {
            if(!res) {
                init_error = true;
            } else {
                init_count++;
            }
        });
        if(!ret) {
            init_error = true;
            break;
        }
    }
    
    constexpr uint64_t timeout = 300;
    constexpr auto wait_time = std::chrono::seconds(1);
    for(size_t count = 0;
        init_count < n_wallets && !init_error && count < timeout;
        count++) {
        std::this_thread::sleep_for(wait_time);
    }

    if(init_count < n_wallets || init_error) {
        log->error("Error initializing accounts");
        return 1;
    }

    log->trace("Added new accounts");
    
    std::fstream myfile;
    myfile.open("sample_sequence.txt", std::ios::in);

    if (myfile.is_open()) {
        std::string action;
        while (getline(myfile, action)) {
            std::vector<std::string> act = split(action, ",");
            std::string op = act[0];
            int wallet_index = stoi(act[1]);
            if (!op.compare("0")) {
                log->trace(op);
                log->trace(wallet_index);
                wallets[wallet_index].deposit(
                    act[2],
                    [&, wallet_index](bool res) {
                        if(!res) {
                            log->fatal("Deposit request error");
                        }
                        log->trace(wallet_index, "done depositing");
                    }
                );
            } 
            // else {
            //     wallets[wallet_index].withdraw(
            //         act[2],
            //         act[3],
            //         act[4],
            //         act[5],
            //         act[6],
            //         act[7],
            //         act[8],
            //         [&, wallet_index](bool res) {
            //             if(!res) {
            //                 log->fatal("Withdraw request error");
            //             }
            //             log->trace(wallet_index, "done withdrawing");
            //         }
            //     );
            // }
        }
        myfile.close();
    }


    // log->trace("Depositing phase");
    // for(size_t i = 0; i < n_wallets; i++) {
    //     log->trace(i, " depositing");
    //     ret = wallets[i].deposit(
    //         100,
    //         [&, i](bool res) {
    //             if(!res) {
    //                 log->fatal("Deposit request error");
    //             }
    //             log->trace(i, " done depositing");
    //         });
    //     if(!ret) {
    //         log->fatal("Deposit request failed");
    //     }
    // }

    // log->trace("Withdrawing phase");
    // for(size_t i = 0; i < n_wallets; i++) {
    //     log->trace(i, " depositing");
    //     auto res = wallets[i].withdraw(
    //         100,
    //         [&, i](bool ret) {
    //             if(!ret) {
    //                 log->fatal("Deposit request error");
    //             }
    //             log->trace(i, " done depositing");
    //         });
    //     if(!res) {
    //         log->fatal("Deposit request failed");
    //     }
    // }
    
    // log->trace("Checking balances");

    // auto tot = std::atomic<uint64_t>{};
    // init_count = 0;
    // init_error = false;
    // for(size_t i = 0; i < n_wallets; i++) {
    //     auto res = wallets[i].update_balance([&, i](bool ret) {
    //         if(!ret) {
    //             init_error = true;
    //         } else {
    //             tot += wallets[i].get_balance();
    //             init_count++;
    //         }
    //     });
    //     if(!res) {
    //         init_error = true;
    //         break;
    //     }
    // }

    // for(size_t count = 0;
    //     init_count < n_wallets && !init_error && count < timeout;
    //     count++) {
    //     std::this_thread::sleep_for(wait_time);
    // }

    // if(init_count < n_wallets || init_error) {
    //     log->error("Error updating balances");
    //     return 2;
    // }

    // if(tot != init_balance * n_wallets) {
    //     return 3;
    // }

    // log->trace("Checked balances");

    return 0;
}
