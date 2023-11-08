project(tcash)

include_directories(../../src ../../3rdparty ../../3rdparty/secp256k1/include)

add_executable(tcash tcash.cpp
                     wallet.cpp
                     merkletree.cpp
                     mimc.cpp)
add_executable(test test.cpp
                     merkletree.cpp
                     mimc.cpp
                     verifier.cpp)
target_link_libraries(tcash broker
                                directory
                                runtime_locking_shard
                                ticket_machine
                                parsec
                                agent
                                rpc
                                network
                                common
                                serialization
                                crypto
                                secp256k1
                                ${FF_LIBRARY}
                                ${GMP_LIBRARY}
                                ${LUA_LIBRARY}
                                ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(test broker
                                directory
                                runtime_locking_shard
                                ticket_machine
                                parsec
                                agent
                                rpc
                                network
                                common
                                serialization
                                crypto
                                secp256k1
                                ${FF_LIBRARY}
                                ${GMP_LIBRARY}
                                ${LUA_LIBRARY}
                                ${CMAKE_THREAD_LIBS_INIT})