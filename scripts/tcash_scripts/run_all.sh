#!/bin/bash
set -e 

## CRYPTO SLEEP
# tcash 
for i in {1,5,10,15,20,25,50,100,150,200,250,300,350,400,450,500,550,600,650,700,750,800,850,900,1000}; do 
    sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
    sleep 3
    echo $i
    sudo ./scripts/tcash_scripts/run-tcash.sh --run=tcash --agents=1 --p=$i
    python3 scripts/plot.py tools/tcash/experiments/crypto_test/tcash tcash_withdrawals_$i
done 

# ecash 
for i in {1,5,10,15,20,25,50,100,150,200,250,300,350,400,450,500,550,600,650,700,750,800,850,900,1000}; do 
    sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
    sleep 3
    echo $i
    sudo ./scripts/tcash_scripts/run-tcash.sh --run=ecash --agents=1 --p=$i
    python3 scripts/plot.py tools/tcash/experiments/crypto_test/ecash ecash_withdrawals_$i
done 

## AGENT
for i in {1..5}; do 
    sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=$i
    sleep 5
    ./scripts/tcash_scripts/run-tcash.sh --run=lua_bench --agents=$i
done

for i in {1..5}; do 
    sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=$i
    sleep 5
    ./scripts/tcash_scripts/run-tcash.sh --run=ecash --agents=$i
done

for i in {1..5}; do 
    sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=$i
    sleep 5
    ./scripts/tcash_scripts/run-tcash.sh --run=tcash --agents=$i
done

for i in {1..5}; do 
    sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=$i
    sleep 5
    ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=16 --file=ToT_16_50k_ordered --update=7 --agents=$i
done


## UPDATE + AGENTS
for j in {0..10}; do
    for i in {1..5}; do 
        sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=$i
        sleep 5
        ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=16 --file=ToT_16_50k_ordered --update=$j --agents=$i
    done
done

## UPDATE
for j in {2,4,8,16,32,64,128,256,512}; do
    for i in {0..10}; do 
        echo trees $j update $i
        sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
        sleep 5
        ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees="$j" --file=ToT_"$j"_deposits --update="$i" --agents=1
    done
done

## WALLETS
for i in {10,15,20,25}; do
    for j in {5,10,15,20,25,30,40,50,60,70,80,90,100}; do 
        sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=$i
        sleep 5
        echo agents $i wallets $j
        ./scripts/tcash_scripts/run-tcash.sh --run=lua_bench --agents=$i --w=$j
    done
done

for i in {1,5,10,15,20,25}; do
    for j in {5,10,15,20,25,30,40,50,60,70,80,90,100}; do 
        sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=$i
        sleep 5
        echo agents $i wallets $j
        ./scripts/tcash_scripts/run-tcash.sh --run=ecash --agents=$i --w=$j
    done
done

sudo ./scripts/tcash_scripts/build-run-parsec.sh --kill
