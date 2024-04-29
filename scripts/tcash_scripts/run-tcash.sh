#!/bin/bash
set -e 

if [ -z "$*" ]; then 
    echo "add '--run=' argument with either ToT, tcash, or lua_bench"; 
fi

RUNNER_TYPE="lua_bench"
TREES="2"
FILENAME="ToT_2_10k_ordered"
UPDATE=1
NUM_AGENTS=1
P=100
WALLETS=5
PROB=0

for arg in "$@"; do
    if [[ "$arg" == "--run"* ]]; then
        RUNNER_TYPE="${arg#--run=}"
    elif [[ "$arg" == "--trees"* ]]; then
        TREES="${arg#--trees=}"
    elif [[ "$arg" == "--file"* ]]; then 
        FILENAME="${arg#--file=}"
    elif [[ "$arg" == "--update"* ]]; then 
        UPDATE="${arg#--update=}"
    elif [[ "$arg" == "--agents"* ]]; then 
        NUM_AGENTS="${arg#--agents=}"
    elif [[ "$arg" == "--prob"* ]]; then 
        PROB="${arg#--prob=}"
    elif [[ "$arg" == "--p"* ]]; then 
        P="${arg#--p=}"
    elif [[ "$arg" == "--w"* ]]; then 
        WALLETS="${arg#--w=}"
    fi
done

agent_endpoints=""
for ((i=0;i<$NUM_AGENTS;i++)); do
    agent_endpoints+=" --agent"$i"_endpoint=localhost:800"$i
done 

if [[ $RUNNER_TYPE == "ToT" ]]; then
    echo Running ToT with $TREES trees, $NUM_AGENTS agents, updating with factor $UPDATE, with $PROB tree probability
    ./build/tools/tcash/ToT --component_id=0 --ticket_machine0_endpoint=localhost:7777 --ticket_machine_count=1 \
    --shard_count=1 --shard0_count=1 --shard00_endpoint=localhost:5556 \
    --agent_count=$NUM_AGENTS $agent_endpoints \
    --loglevel=TRACE tools/tcash/merkletree.lua $PROB $UPDATE $WALLETS $TREES $FILENAME > logs/tcash.log 
elif [[ $RUNNER_TYPE == "tcash" ]]; then 
    echo Running tcash with $NUM_AGENTS agents
    ./build/tools/tcash/tcash --component_id=0 --ticket_machine0_endpoint=localhost:7777 --ticket_machine_count=1 \
    --shard_count=1 --shard0_count=1 --shard00_endpoint=localhost:5556 \
    --agent_count=$NUM_AGENTS $agent_endpoints \
    --loglevel=TRACE tools/tcash/merkletree.lua $P $WALLETS > logs/tcash.log 
elif [[ $RUNNER_TYPE == "ecash" ]]; then 
    echo Running ecash with $NUM_AGENTS agents, verify factor $P/1000
    ./build/tools/tcash/ecash --component_id=0 --ticket_machine0_endpoint=localhost:7777 --ticket_machine_count=1 \
    --shard_count=1 --shard0_count=1 --shard00_endpoint=localhost:5556 \
    --agent_count=$NUM_AGENTS $agent_endpoints \
    --loglevel=TRACE tools/tcash/ecash.lua $P $WALLETS > logs/tcash.log 
elif [[ $RUNNER_TYPE == "lua_bench" ]]; then 
    echo Running lua_bench with $NUM_AGENTS agents
    ./build/tools/bench/parsec/lua/lua_bench --component_id=0 --ticket_machine0_endpoint=localhost:7777 --ticket_machine_count=1 \
    --shard_count=1 --shard0_count=1 --shard00_endpoint=localhost:5556 \
    --agent_count=$NUM_AGENTS $agent_endpoints \
    --loglevel=TRACE scripts/gen_bytecode.lua $WALLETS > logs/test.log 
else 
    echo "run with either ToT, tcash, or lua_bench"
fi

