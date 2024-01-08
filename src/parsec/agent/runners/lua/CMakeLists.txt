project(lua_runner)

add_library(lua_runner impl.cpp
                       server.cpp)

target_link_libraries(lua_runner tc_primitives ${FF_LIBRARY} ${GMP_LIBRARY})
