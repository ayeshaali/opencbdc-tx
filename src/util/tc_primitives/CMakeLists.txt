project(tc_primitives)

include_directories(../../common ../../3rdparty)

add_library(tc_primitives merkletree.cpp
                          mimc.cpp
                          verifier.cpp
                          blind_sig.cpp)

target_link_libraries(tc_primitives ${FF_LIBRARY}
                                    ${GMP_LIBRARY}
                                    common
                                    crypto)