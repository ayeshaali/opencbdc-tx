project(tc_primitives)

add_library(tc_primitives merkletree.cpp
                          mimc.cpp
                          verifier.cpp)

target_link_libraries(tc_primitives ${FF_LIBRARY}
                                    ${GMP_LIBRARY})