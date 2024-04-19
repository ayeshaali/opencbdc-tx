#!/bin/bash
set -e 

REBUILD=false
RUN_PARSEC=true

IP="localhost"
PORT="8000"
RUNNER_TYPE="lua"
LOGLEVEL="TRACE"
NUM_AGENTS=1

for arg in "$@"; do
    if [[ "$arg" == "--rebuild"* ]]; then
        REBUILD=true
    elif [[ "$arg" == "--agents"* ]]; then 
        NUM_AGENTS="${arg#--agents=}"
    elif [[ "$arg" == "--kill"* ]]; then 
        RUN_PARSEC=false
    fi
done

echo "build and run" 

if lsof -i:5556 -t > /dev/null; then 
    echo $(lsof -i:5556 -t)
    $(kill -9 $( lsof -i:5556 -t )) || echo "didn't work"
    echo "kill runtime"
fi 

if lsof -i:7777 -t > /dev/null; then 
    echo $(lsof -i:7777 -t)
    $(kill -9 $( lsof -i:7777 -t  )) || echo "didn't work"
    echo "kill ticket machine"
fi 

if lsof -i:5557 -t > /dev/null; then 
    echo $(lsof -i:5557 -t)
    $(kill -9 $( lsof -i:5557 -t  )) || echo "didn't work"
    echo "kill run time again ?"
fi 

if lsof -i:5557 -t > /dev/null; then 
    echo $(lsof -i:5557 -t)
    $(kill -9 $( lsof -i:5557 -t  )) || echo "didn't work"
    echo "kill run time again ?"
fi 

rm -rf runtime_locking_shard0_raft_log_0
rm -rf ticket_machine_raft_log_0

if $REBUILD; then
    rm -rf build/tools/tcash
    echo "removed folder"
    ./scripts/build.sh
fi


if $RUN_PARSEC; then
    mkdir -p logs
    echo Log level = $LOGLEVEL
    echo Runner type = $RUNNER_TYPE

    ./build/src/parsec/runtime_locking_shard/runtime_locking_shardd --shard_count=1 \
        --shard0_count=1 --shard00_endpoint=$IP:5556 \
        --shard00_raft_endpoint=$IP:5557 --node_id=0 --component_id=0 \
        --agent_count=1 --agent0_endpoint=$IP:6666 --ticket_machine_count=1 \
        --ticket_machine0_endpoint=$IP:7777 --loglevel=$LOGLEVEL \
        > logs/shardd.log &
    sleep 1

    ./scripts/wait-for-it.sh -s $IP:5556 -t 60 -- \
        ./build/src/parsec/ticket_machine/ticket_machined --shard_count=1 \
        --shard0_count=1 --shard00_endpoint=$IP:5556 --node_id=0 \
        --component_id=0 --agent_count=1 --agent0_endpoint=$IP:6666 \
        --ticket_machine_count=1 --ticket_machine0_endpoint=$IP:7777 \
        --loglevel=$LOGLEVEL > logs/ticket_machined.log &
    sleep 1

    for ((i=0;i<$NUM_AGENTS;i++)); do
        ./scripts/wait-for-it.sh -s $IP:7777 -t 60  -- ./scripts/wait-for-it.sh -s \
        $IP:5556 -t 60 -- ./scripts/tcash_scripts/spawn-agent.sh --id=$i --port=800$i --runner_type=lua
        sleep 1
    done 
fi
