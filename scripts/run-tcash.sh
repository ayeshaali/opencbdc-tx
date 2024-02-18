#!/bin/bash
set -e 

if [ -z "$*" ]; then 
    echo "add '--run=' argument with either ToT, tcash, or lua_bench"; 
fi

for arg in "$@"; do
    if [[ "$arg" == "--run"* ]]; then
        if [[ "$arg" == "--run=ToT" ]]; then
           ./build/tools/tcash/ToT --component_id=0 --ticket_machine0_endpoint=localhost:7777 --ticket_machine_count=1 \
            --shard_count=1 --shard0_count=1 --shard00_endpoint=localhost:5556 \
            --agent_count=1 --agent0_endpoint=localhost:8000 \
            --loglevel=TRACE tools/tcash/gen_bytecode.lua 5 > logs/tcash.log & 
        elif [[ "$arg" == "--run=tcash" ]]; then 
            ./build/tools/tcash/tcash --component_id=0 --ticket_machine0_endpoint=localhost:7777 --ticket_machine_count=1 \
            --shard_count=1 --shard0_count=1 --shard00_endpoint=localhost:5556 \
            --agent_count=1 --agent0_endpoint=localhost:8000 \
            --loglevel=TRACE tools/tcash/gen_bytecode.lua 5 > logs/tcash.log & 
        elif [[ "$arg" == "--run=lua_bench" ]]; then 
            ./build/tools/bench/parsec/lua/lua_bench --component_id=0 --ticket_machine0_endpoint=localhost:7777 --ticket_machine_count=1 \
            --shard_count=1 --shard0_count=1 --shard00_endpoint=localhost:5556 \
            --agent_count=1 --agent0_endpoint=localhost:8000 \
            --loglevel=TRACE scripts/gen_bytecode.lua 5 > logs/test.log & 
        else 
            echo "run with either ToT, tcash, or lua_bench"
        fi
    fi
done
