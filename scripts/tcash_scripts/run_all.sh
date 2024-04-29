#!/bin/bash

## MAIN 
sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=lua_bench --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ecash --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=tcash --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=2 --file=ToT_2_10k_ordered --update=7 --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=4 --file=ToT_4_10k_ordered --update=7 --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=8 --file=ToT_8_10k_ordered --update=7 --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=16 --file=ToT_16_10k_ordered --update=7 --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=32 --file=ToT_32_10k_ordered --update=6 --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=64 --file=ToT_64_10k_ordered --update=5 --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=128 --file=ToT_128_50k_ordered --update=4 --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=256 --file=ToT_256_50k_ordered --update=3 --agents=1

sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
sleep 3
sudo ./scripts/tcash_scripts/run-tcash.sh --run=ToT --trees=512 --file=ToT_512_50k_ordered --update=2 --agents=1

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
# parameters: # agents, # wallets, design
wallet_wrapper() {
    sudo ./scripts/tcash_scripts/build-run-parsec.sh --all --agents=$1
    sleep 5
    echo agents $i wallets $j
    ./scripts/tcash_scripts/run-tcash.sh --run=$3 --agents=$1 --w=$2
}

design=tcash
for i in {1,5,10,15,20,25}; do
    for j in {5,10,15,20,25,30,40,50,60,70,80,90,100}; do 
        IN_PROGRESS=1;
        echo $i $j
        while [[ $IN_PROGRESS -eq 1 ]]; do
            wallet_wrapper "$i" "$j" "$design" > log 2>&1
            result=$?
            if [[ $result -ne 0 ]]; then 
                echo failure
                IN_PROGRESS=1
            else 
                echo success
                IN_PROGRESS=0
            fi 
            sleep 5
        done
    done
done

## ADVERSARIAL WORKLOADS
for j in {10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100}; do 
    sudo ./scripts/tcash_scripts/build-run-parsec.sh --agents=1
    sleep 5
    echo probability $j/100
    ./scripts/tcash_scripts/run-tcash.sh --run=ToT  --trees=16 --file=ToT_16_10k_ordered --update=7 --agents=1 --prob=$j
done

sudo ./scripts/tcash_scripts/build-run-parsec.sh --kill
