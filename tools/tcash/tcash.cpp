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
    if(lua_pcall(L, 0, 4, 0) != 0) {
        log->error("Contract bytecode generation failed, with error:",
                   lua_tostring(L, -1));
        return 1;
    }
    auto tc_deposit_contract = cbdc::buffer();
    auto tc_withdraw_contract = cbdc::buffer();
    tc_deposit_contract = cbdc::buffer::from_hex(lua_tostring(L, -4)).value();
    tc_withdraw_contract = cbdc::buffer::from_hex(lua_tostring(L, -3)).value();
    auto tc_deposit_contract_key = cbdc::buffer();
    auto tc_withdraw_contract_key = cbdc::buffer();
    tc_deposit_contract_key.append("tc_deposit_contract", 19);
    tc_withdraw_contract_key.append("tc_withdraw_contract", 20);

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
    log->trace("Initialized agents");

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
    constexpr auto wait_time = std::chrono::milliseconds(2000);
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
    myfile.open("tools/tcash/tcash_sequences/tcash_10k.txt", std::ios::in);

    std::mutex samples_mut;
    auto samples_file = std::ofstream(
        "tools/tcash/tcash_sequences/tx_samples_tcash_10k_" + std::to_string(cfg->m_component_id) + ".txt");
    if(!samples_file.good()) {
        log->error("Unable to open samples file");
        return 1;
    }

    auto total_transaction_queue = std::queue<std::vector<std::string>>();
    auto curr_transaction_queue = cbdc::blocking_queue<std::vector<std::string>>();;
    if (myfile.is_open()) {
        std::string action;
        while (getline(myfile, action)) {
            std::vector<std::string> act = split(action, ",");
            total_transaction_queue.push(act);
        }
        log->trace("finished reading file");
        myfile.close();
    }

    for (size_t i =0; i<5; i++) {
        std::vector<std::string> act = std::move(total_transaction_queue.front());
        curr_transaction_queue.push(act);
        total_transaction_queue.pop();
    }

    auto deposit_flight = std::atomic<size_t>();
    auto withdraw_flight = std::atomic<size_t>();

    auto thread_count = std::thread::hardware_concurrency();
    auto threads = std::vector<std::thread>();
    for(size_t i = 0; i < thread_count; i++) {
        auto t = std::thread([&]() {
            std::vector<std::string> act;
            while(curr_transaction_queue.pop(act)) {
                std::string op = act[0];
                if (!op.compare("0")) {
                    int wallet_index = stoi(act[1]);
                    int deposit_number = stoi(act[2]);
                    log->trace("start deposit", deposit_number, "for wallet", wallet_index);
                    deposit_flight++;
                    auto tx_start = std::chrono::high_resolution_clock::now();
                    auto res = wallets[wallet_index].deposit(
                        act[3],
                        [&, wallet_index, deposit_number, tx_start](bool ret) {
                            auto tx_end
                            = std::chrono::high_resolution_clock::now();
                            const auto tx_delay = tx_end - tx_start;
                            auto out_buf = std::stringstream();
                            out_buf << tx_end.time_since_epoch().count() << " "
                                    << tx_delay.count() << "\n";
                            auto out_str = out_buf.str();
                            {
                                std::unique_lock l(samples_mut);
                                samples_file << out_str;
                            }
                            if(!ret) {
                                log->fatal("Deposit request error");
                            }
                            log->trace("finished deposit", deposit_number, "for wallet", wallet_index);
                            deposit_flight--;
                            if (!total_transaction_queue.empty()) {
                                std::vector<std::string> new_deposit = std::move(total_transaction_queue.front());
                                curr_transaction_queue.push(new_deposit);
                                total_transaction_queue.pop();
                            }
                        }
                    );
                    if(!res) {
                        log->fatal("Deposit request failed");
                    }
                } else {
                    int wallet_index = stoi(act[1]);
                    int withdraw_number = stoi(act[2]);
                    withdraw_flight++;
                    log->trace("start withdraw", withdraw_number, "for wallet", wallet_index);
                    auto tx_start = std::chrono::high_resolution_clock::now();
                    auto res = wallets[wallet_index].withdraw(
                        act[4],
                        act[5],
                        act[6],
                        act[7],
                        act[8],
                        act[9],
                        act[10],
                        [&, wallet_index, withdraw_number, tx_start](bool ret) {
                            auto tx_end
                            = std::chrono::high_resolution_clock::now();
                            const auto tx_delay = tx_end - tx_start;
                            auto out_buf = std::stringstream();
                            out_buf << tx_end.time_since_epoch().count() << " "
                                    << tx_delay.count() << "\n";
                            auto out_str = out_buf.str();
                            {
                                std::unique_lock l(samples_mut);
                                samples_file << out_str;
                            }
                            if(!ret) {
                                log->fatal("Withdraw request error");
                            }
                            log->trace("finished withdraw", withdraw_number, "for wallet", wallet_index);
                            withdraw_flight--;
                            if (!total_transaction_queue.empty()) {
                                std::vector<std::string> new_deposit = std::move(total_transaction_queue.front());
                                curr_transaction_queue.push(new_deposit);
                                total_transaction_queue.pop();
                            }
                        }
                    );
                    if(!res) {
                        log->fatal("Withdraw request failed");
                    } 
                }
            }
        });
        threads.emplace_back(std::move(t));
    }
    
    std::this_thread::sleep_for(wait_time);
    while(withdraw_flight > 0 || deposit_flight > 0) {
        log->trace("deposits in flights (run loop):", deposit_flight);
        log->trace("withdraws in flights (run loop):", withdraw_flight);
        std::this_thread::sleep_for(wait_time);
    }

    log->trace("Joining thread");
    curr_transaction_queue.clear();
    for(auto& t : threads) {
        t.join();
    }

    log->trace("finished");
    return 0;
}

auto sequential_check(std::shared_ptr<cbdc::logging::log> log, 
                      std::vector<cbdc::parsec::account_wallet> wallets) -> int {

    std::fstream myfile;
    myfile.open("sample_sequence.txt", std::ios::in);

    auto total_deposit_queue = std::queue<std::vector<std::string>>();
    auto total_withdraw_queue = cbdc::blocking_queue<std::vector<std::string>>();
    auto curr_deposit_queue = cbdc::blocking_queue<std::vector<std::string>>();

    if (myfile.is_open()) {
        std::string action;
        while (getline(myfile, action)) {
            std::vector<std::string> act = split(action, ",");
            std::string op = act[0];
            if (!op.compare("0")) {
                total_deposit_queue.push(act);
            } else {
                total_withdraw_queue.push(act);
            }
        }
        log->trace("finished reading file");
        myfile.close();
    }

    for (size_t i =0; i<5; i++) {
        std::vector<std::string> act = std::move(total_deposit_queue.front());
        curr_deposit_queue.push(act);
        total_deposit_queue.pop();
    }

    auto deposit_flight = std::atomic<size_t>();
    auto withdraw_flight = std::atomic<size_t>();
    auto count = std::atomic<size_t>();
    auto threads = std::vector<std::thread>();

    for(size_t i = 0; i < 1; i++) {
        auto t = std::thread([&]() {
            std::vector<std::string> act;
            while(curr_deposit_queue.pop(act)) {
                std::string op = act[0];
                int wallet_index = stoi(act[1]);
                int deposit_number = stoi(act[2]);
                log->trace("start deposit", deposit_number, "for wallet", wallet_index);
                deposit_flight++;
                auto res = wallets[wallet_index].deposit(
                    act[3],
                    [&, wallet_index, deposit_number](bool ret) {
                        if(!ret) {
                            log->fatal("Deposit request error");
                        }
                        log->trace("finished deposit", deposit_number, "for wallet", wallet_index);
                        deposit_flight--;
                        count++;
                        if (!total_deposit_queue.empty()) {
                            std::vector<std::string> new_deposit = std::move(total_deposit_queue.front());
                            curr_deposit_queue.push(new_deposit);
                            total_deposit_queue.pop();
                            std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        }
                    }
                );
                if(!res) {
                    log->fatal("Deposit request failed");
                }
            }
        });
        threads.emplace_back(std::move(t));
    }

    for(size_t i = 0; i < 1; i++) {
        auto t = std::thread([&]() {
            std::vector<std::string> act;
            while(total_withdraw_queue.pop(act)) {   
                std::string op = act[0];
                int wallet_index = stoi(act[1]);
                int withdraw_number = stoi(act[2]);
                int curr_deposit = stoi(act[3]);
                while (count < (unsigned long) curr_deposit) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }   
                withdraw_flight++;
                log->trace("current deposit count: ", count);
                log->trace("deposit after which to withdraw", curr_deposit);
                log->trace("start withdraw", withdraw_number, "for wallet", wallet_index);
                log->trace("withdraws in flights:", withdraw_flight);
                auto res = wallets[wallet_index].withdraw(
                    act[4],
                    act[5],
                    act[6],
                    act[7],
                    act[8],
                    act[9],
                    act[10],
                    [&, wallet_index, withdraw_number](bool ret) {
                        if(!ret) {
                            log->fatal("Withdraw request error");
                        }
                        log->trace("finished withdraw", withdraw_number, "for wallet", wallet_index);
                        withdraw_flight--;
                    }
                );
                if(!res) {
                    log->fatal("Withdraw request failed");
                } 
            }
        });
        threads.emplace_back(std::move(t));
    }

    constexpr auto wait_time = std::chrono::milliseconds(2000);
    std::this_thread::sleep_for(wait_time);
    while(withdraw_flight > 0 || deposit_flight > 0) {
        log->trace("deposits in flights (run loop):", deposit_flight);
        log->trace("withdraws in flights (run loop):", withdraw_flight);
        std::this_thread::sleep_for(wait_time);
    }

    log->trace("Joining thread");
    curr_deposit_queue.clear();
    total_withdraw_queue.clear();
    for(auto& t : threads) {
        t.join();
    }

    log->trace("finished");
    return 0;
}
