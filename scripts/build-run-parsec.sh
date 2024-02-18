#!/bin/bash
set -e 

REBUILD=false

for arg in "$@"; do
    if [[ "$arg" == "--rebuild"* ]]; then
        if [[ "$arg" == "--rebuild=true" ]]; then
            REBUILD=true
        fi
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

./scripts/parsec-run-local.sh --loglevel=TRACE --runner_type=lua
