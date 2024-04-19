def build10k(filename):
    deposits = []
    withdrawals = []
    total = []
    f = open(filename+".txt", "r")
    count = 0
    deposit_num = 0
    for tx in f:
        print(count)
        total.append(tx)
        if tx[0] == "1":
            withdrawals.append(tx)
            count+=1
        else: 
            if tx[0] == "0":
                if deposit_num < 6500:
                    deposits.append(tx)
                    count+=1
                    deposit_num+=1
            else:
                deposits.append(tx)
        if count >= 10000:
            break
    f.close()

    with open(filename[:-3]+"10k_ordered.txt", 'w') as f:
        for line in deposits:
            f.write(f"{line}")
        
        for line in withdrawals:
            f.write(f"{line}")
        
        f.close()

    with open(filename[:-3]+"10k.txt", 'w') as f:
        for line in total:
            f.write(f"{line}")
        f.close()

def order(filename):
    deposits = []
    withdrawals = []
    f = open(filename+".txt", "r")
    count = 0
    for tx in f:
        if tx[0] == "1":
            withdrawals.append(tx)
        else: 
            deposits.append(tx)
        count+=1
    f.close()

    with open(filename+"_ordered.txt", 'w') as f:
        for line in deposits:
            f.write(f"{line}")
        
        for line in withdrawals:
            f.write(f"{line}")
        
        f.close()

def findupdate(filename):
    f = open(filename+".txt", "r")
    count = 0
    for tx in f:
        if tx[0] == "0":
            count+=1
        elif tx[0] == "2":
            print(count)
    f.close()

# files = ["ToT_2_50k", "ToT_4_50k", "ToT_8_50k", "ToT_16_50k", "ToT_32_50k", "ToT_64_50k"]
# files = ["ToT_2_10k", "ToT_4_10k", "ToT_8_10k", "ToT_16_10k", "ToT_32_10k", "ToT_64_10k"]

# for f in files:
#     print(f)
#     findupdate(f)

files = ["ecash_100k"]
for f in files:
    order(f)