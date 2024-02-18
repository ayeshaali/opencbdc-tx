function gen_bytecode()
    tc_deposit_contract = function(param)
        from, commitment = string.unpack("c32 c64", param)

        function initial_subtrees()
            return "2fe54c60d3acabf3343a35b6eba15db4821b340f76e741e2249685ed4899af6c256a6135777eee2fd26f54b8b7037a25439d5235caee224154186d2b8a52e31d1151949895e82ab19924de92c40a3d6f7bcb60d92b00504b8199613683f0c20020121ee811489ff8d61f09fb89e313f14959a0f28bb428a20dba6b0b068b3bdb0a89ca6ffa14cc462cfedb842c30ed221a50a3d6bf022a6a57dc82ab24c157c924ca05c2b5cd42e890d6be94c68d0689f4f21c9cec9c0f13fe41d566dfb549591ccb97c932565a92c60156bdba2d08f3bf1377464e025cee765679e604a7315c19156fbd7d1a8bf5cba8909367de1b624534ebab4f0f79e003bccdd1b182bdb4261af8c1f0912e465744641409f622d466c3920ac6e5ff37e36604cb11dfff800058459724ff6ca5a1652fcbc3e82b93895cf08e975b19beab3f54c217d1c0071f04ef20dee48d39984d8eabe768a70eafa6310ad20849d4573c3c40c2ad1e301bea3dec5dab51567ce7e200a30f7ba6d4276aeaa53e2686f962a46c66d511e50ee0f941e2da4b9e31c3ca97a40d8fa9ce68d97c084177071b3cb46cd3372f0f1ca9503e8935884501bbaf20be14eb4c46b89772c97b96e3b2ebf3a36a948bbd133a80e30697cd55d8f7d4b0965b7be24057ba5dc3da898ee2187232446cb10813e6d8fc88839ed76e182c2a779af5b2c0da9dd18c90427a644f7e148a6253b61eb16b057a477f4bc8f572ea6bee39561098f78f15bfb3699dcbb7bd8db618540da2cb16a1ceaabf1c16b838f7a9e3f2a3a3088d9e0a6debaa748114620696ea24a3b3d822420b14b5d8cb6c28a574f01e98ea9e940551d2ebd75cee12649f9d198622acbd783d1b0d9064105b1fc8e4d8889de95c4c519b3f635809fe6afc0529d7ed391256ccc3ea596c86e933b89ff339d25ea8ddced975ae2fe30b5296d419be59f2f0413ce78c0c3703a3a5451b1d7f39629fa33abd11548a76065b29671ff3f61797e538b70e619310d33f2a063e7eb59104e112e95738da1254dc345310c16ae9959cf8358980d9dd9616e48228737310a10e2b6b731c1a548f036c480ba433a63174a90ac20992e75e3095496812b652685b5e1a2eae0b1bf4e8fcd1019ddb9df2bc98d987d0dfeca9d2b643deafab8f7036562e627c3667266a044c2d3c88b23175c5a5565db928414c66d1912b11acf974b2e644caaac04739ce992eab55f6ae4e66e32c5189eed5c470840863445760f5ed7e7b69b2a62600f354002df37a2642621802383cf952bf4dd1f32e05433beeb1fd41031fb7eace979d104aeb41435db66c3e62feccc1d6f5d98d0a0ed75d1374db457cf462e3a1f4271f3c6fd858e9a7d4b0d1f38e256a09d81d5a5e3c963987e2d4b814cfab7c6ebb2c7a07d20dff79d01fecedc1134284a8d08436606c93693b67e333f671bf69cc"
        end 

        function insert(commitment)
            num_leaves_data = coroutine.yield("num_leaves")
            num_leaves = 0 
            if string.len(num_leaves_data) > 0 then
                num_leaves = string.unpack("I8", num_leaves_data) 
            end
            print(num_leaves)

            subtrees = ""
            subtrees_data = coroutine.yield("subtrees")
            if string.len(subtrees_data) > 0 then
                subtrees = string.unpack("c1280", subtrees_data)
            else 
                subtrees = initial_subtrees()
            end 

            MT_updates = insert_MT(num_leaves, subtrees, commitment, 20)
            root = string.sub(MT_updates, 1, 64)
            root_update = "root_" .. root
            leaf_update = "leaf_" .. commitment
            print("commitment")
            print(commitment)
            print("root")
            print(root)
            leaf_data = coroutine.yield(leaf_update)
            root_data = coroutine.yield(root_update)
            num_leaves = num_leaves + 1
            
            updates = {}
            updates[root_update] = string.pack("c64", root) 
            updates[leaf_update] = string.pack("c64", commitment) 
            updates["subtrees"] = string.pack("c1280", string.sub(MT_updates, 65, 1280))            
            updates["num_leaves"] = string.pack("I8", num_leaves) 
            return updates
        end 

        function update_balances(updates, from)
            deposit_amt = 1
          
            pool_data = coroutine.yield("TC_pool")
            pool_amount = 0 
            if string.len(pool_data) > 0 then
                pool_amount = string.unpack("I8", pool_data) 
            end
            pool_amount=pool_amount+deposit_amt
            updates["TC_pool"] = string.pack("I8",  pool_amount)
            
            account_data = coroutine.yield("account_" .. from)
            account_balance = string.unpack("I8", account_data)
            account_balance = account_balance - deposit_amt
            updates["account_" .. from] = string.pack("I8", account_balance)
            return updates
        end 

        function dump(o)
            if type(o) == 'table' then
               local s = '{ '
               for k,v in pairs(o) do
                  if type(k) ~= 'number' then k = '"'..k..'"' end
                  s = s .. '['..k..'] = ' .. dump(v) .. ','
               end
               return s .. '} '
            else
               return tostring(o)
            end
        end

        updates = insert(commitment)
        print("deposited")
        updates = update_balances(updates, from)
        print("balanced")
        -- print(dump(updates))
        return updates
    end

    tc_withdraw_contract = function(param)
        from, proof, root, nullifierHash, recipient, relayer, fee, refund = string.unpack("c32 c512 c64 c64 c40 c40 c64 c64", param)
        print("withdrawing")
        nullifierHash_update = "nullifier_" .. nullifierHash
        nullifierHash_data = coroutine.yield(nullifierHash_update)
        if string.len(nullifierHash_data) > 0 then
            print("nullifier hash was seen before")
            error("nullifier hash was seen before")
        end
        root_data = coroutine.yield("root_" .. root)

        function root_check()
            if not (string.len(root_data) > 0) then
                print("root does not exist")
                error("root does not exist")
            end
        end

        print("verifying")
        verify_proof(proof, root, nullifierHash, recipient, relayer, fee, refund)
        print("proof verified")

        function update_balances(updates, from)
            deposit_amt = 1
          
            pool_data = coroutine.yield("TC_pool")
            pool_amount = 0 
            if string.len(pool_data) > 0 then
                pool_amount = string.unpack("I8", pool_data) 
            end
            pool_amount=pool_amount-deposit_amt
            updates["TC_pool"] = string.pack("I8",  pool_amount)
            
            account_data = coroutine.yield("account_" .. from)
            account_balance = string.unpack("I8", account_data)
            account_balance = account_balance + deposit_amt
            updates["account_" .. from] = string.pack("I8", account_balance)
            return updates
        end 

        updates = {}
        updates[nullifierHash_update] = nullifierHash
        updates = update_balances(updates, from)
        print("finished")
        return updates
    end

    ToT_deposit_contract = function(param)
        from, index, commitment = string.unpack("c32 I8 c64", param)

        function initial_subtrees()
            return "2fe54c60d3acabf3343a35b6eba15db4821b340f76e741e2249685ed4899af6c256a6135777eee2fd26f54b8b7037a25439d5235caee224154186d2b8a52e31d1151949895e82ab19924de92c40a3d6f7bcb60d92b00504b8199613683f0c20020121ee811489ff8d61f09fb89e313f14959a0f28bb428a20dba6b0b068b3bdb0a89ca6ffa14cc462cfedb842c30ed221a50a3d6bf022a6a57dc82ab24c157c924ca05c2b5cd42e890d6be94c68d0689f4f21c9cec9c0f13fe41d566dfb549591ccb97c932565a92c60156bdba2d08f3bf1377464e025cee765679e604a7315c19156fbd7d1a8bf5cba8909367de1b624534ebab4f0f79e003bccdd1b182bdb4261af8c1f0912e465744641409f622d466c3920ac6e5ff37e36604cb11dfff800058459724ff6ca5a1652fcbc3e82b93895cf08e975b19beab3f54c217d1c0071f04ef20dee48d39984d8eabe768a70eafa6310ad20849d4573c3c40c2ad1e301bea3dec5dab51567ce7e200a30f7ba6d4276aeaa53e2686f962a46c66d511e50ee0f941e2da4b9e31c3ca97a40d8fa9ce68d97c084177071b3cb46cd3372f0f1ca9503e8935884501bbaf20be14eb4c46b89772c97b96e3b2ebf3a36a948bbd133a80e30697cd55d8f7d4b0965b7be24057ba5dc3da898ee2187232446cb10813e6d8fc88839ed76e182c2a779af5b2c0da9dd18c90427a644f7e148a6253b61eb16b057a477f4bc8f572ea6bee39561098f78f15bfb3699dcbb7bd8db618540da2cb16a1ceaabf1c16b838f7a9e3f2a3a3088d9e0a6debaa748114620696ea24a3b3d822420b14b5d8cb6c28a574f01e98ea9e940551d2ebd75cee12649f9d198622acbd783d1b0d9064105b1fc8e4d8889de95c4c519b3f635809fe6afc0529d7ed391256ccc3ea596c86e933b89ff339d25ea8ddced975ae2fe30b5296d419be59f2f0413ce78c0c3703a3a5451b1d7f39629fa33abd11548a76065b29671ff3f61797e538b70e619310d33f2a063e7eb59104e112e95738da1254dc345310c16ae9959cf8358980d9dd9616e48228737310a10e2b6b731c1a548f036c480ba433a63174a90ac20992e75e3095496812b652685b5e1a2eae0b1bf4e8fcd1019ddb9df2bc98d987d0dfeca9d2b643deafab8f7036562e627c3667266a044c2d3c88b23175c5a5565db928414c66d1912b11acf974b2e644caaac04739ce992eab55f6ae4e66e32c5189eed5c470840863445760f5ed7e7b69b2a62600f354002df37a2642621802383cf952bf4dd1f32e05433beeb1fd41031fb7eace979d104aeb41435db66c3e62feccc1d6f5d98d0a0ed75d1374db457cf462e3a1f4271f3c6fd858e9a7d4b0d1f38e256a09d81d5a5e3c963987e2d4b814cfab7c6ebb2c7a07d20dff79d01fecedc1134284a8d08436606c93693b67e333f671bf69cc"
        end 

        function insert(index, commitment)
            tree_prefix = "tree_" .. index .. "_"
            num_leaves_data = coroutine.yield(tree_prefix .. "num_leaves")
            num_leaves = 0 
            if string.len(num_leaves_data) > 0 then
                num_leaves = string.unpack("I8", num_leaves_data) 
            end
            print("deposit to tree " .. index)
            print("num_leaves:  " .. num_leaves)

            subtrees = ""
            subtrees_data = coroutine.yield(tree_prefix .. "subtrees")
            if string.len(subtrees_data) > 0 then
                subtrees = string.unpack("c1280", subtrees_data)
            else 
                subtrees = initial_subtrees()
            end 

            MT_updates = insert_MT(num_leaves, subtrees, commitment, 16)
            root = string.sub(MT_updates, 1, 64)
            root_update = tree_prefix .. "root"
            leaf_update = tree_prefix .. "leaf_" .. commitment
            print("commitment")
            print(commitment)
            print("root")
            print(root)
            leaf_data = coroutine.yield(leaf_update)
            root_data = coroutine.yield(root_update)
            num_leaves = num_leaves + 1
            
            updates = {}
            updates[root_update] = string.pack("c64", root) 
            updates[leaf_update] = string.pack("c64", commitment) 
            updates[tree_prefix .. "subtrees"] = string.pack("c1280", string.sub(MT_updates, 65, 1280))            
            updates[tree_prefix .. "num_leaves"] = string.pack("I8", num_leaves) 
            return updates
        end 

        function update_balances(updates, index, from)
            deposit_amt = 1
            tree_prefix = "tree_" .. index .. "_"

            pool_data = coroutine.yield(tree_prefix .. "pool")
            pool_amount = 0 
            if string.len(pool_data) > 0 then
                pool_amount = string.unpack("I8", pool_data) 
            end
            pool_amount=pool_amount+deposit_amt
            updates[tree_prefix .. "pool"] = string.pack("I8",  pool_amount)
            
            account_data = coroutine.yield("account_" .. from)
            account_balance = string.unpack("I8", account_data)
            account_balance = account_balance - deposit_amt
            updates["account_" .. from] = string.pack("I8", account_balance)
            return updates
        end 

        updates = insert(index, commitment)
        print("deposited")
        updates = update_balances(updates, index, from)
        print("balanced")
        return updates
    end

    ToT_withdraw_contract = function(param)
        from, proof, root, nullifierHash, recipient, relayer, fee, refund = string.unpack("c32 c512 c64 c64 c40 c40 c64 c64", param)
        print("withdrawing ToT")
        nullifierHash_update = "nullifier_" .. nullifierHash
        nullifierHash_data = coroutine.yield(nullifierHash_update)
        if string.len(nullifierHash_data) > 0 then
            print("nullifier hash was seen before")
            error("nullifier hash was seen before")
        end

        print("ToT withdraw root: " .. root)
        root_data = coroutine.yield("ToT_root_" .. root)
        
        function root_check()
            if not (string.len(root_data) > 0) then
                print("root does not exist")
                error("root does not exist")
            end
        end

        print("verifying")
        verify_proof(proof, root, nullifierHash, recipient, relayer, fee, refund)
        print("proof verified")

        function update_balances(updates, from)
            deposit_amt = 1
          
            pool_data = coroutine.yield("TC_pool")
            pool_amount = 0 
            if string.len(pool_data) > 0 then
                pool_amount = string.unpack("I8", pool_data) 
            end
            pool_amount=pool_amount-deposit_amt
            updates["TC_pool"] = string.pack("I8",  pool_amount)
            
            account_data = coroutine.yield("account_" .. from)
            account_balance = string.unpack("I8", account_data)
            account_balance = account_balance + deposit_amt
            updates["account_" .. from] = string.pack("I8", account_balance)
            return updates
        end 

        updates = {}
        updates[nullifierHash_update] = nullifierHash
        updates = update_balances(updates, from)
        print("finished")
        return updates
    end

    ToT_update_contract = function(param)
        print("trying to update")
        subtrees = "2fe54c60d3acabf3343a35b6eba15db4821b340f76e741e2249685ed4899af6c256a6135777eee2fd26f54b8b7037a25439d5235caee224154186d2b8a52e31d1151949895e82ab19924de92c40a3d6f7bcb60d92b00504b8199613683f0c20020121ee811489ff8d61f09fb89e313f14959a0f28bb428a20dba6b0b068b3bdb0a89ca6ffa14cc462cfedb842c30ed221a50a3d6bf022a6a57dc82ab24c157c924ca05c2b5cd42e890d6be94c68d0689f4f21c9cec9c0f13fe41d566dfb549591ccb97c932565a92c60156bdba2d08f3bf1377464e025cee765679e604a7315c19156fbd7d1a8bf5cba8909367de1b624534ebab4f0f79e003bccdd1b182bdb4261af8c1f0912e465744641409f622d466c3920ac6e5ff37e36604cb11dfff800058459724ff6ca5a1652fcbc3e82b93895cf08e975b19beab3f54c217d1c0071f04ef20dee48d39984d8eabe768a70eafa6310ad20849d4573c3c40c2ad1e301bea3dec5dab51567ce7e200a30f7ba6d4276aeaa53e2686f962a46c66d511e50ee0f941e2da4b9e31c3ca97a40d8fa9ce68d97c084177071b3cb46cd3372f0f1ca9503e8935884501bbaf20be14eb4c46b89772c97b96e3b2ebf3a36a948bbd133a80e30697cd55d8f7d4b0965b7be24057ba5dc3da898ee2187232446cb10813e6d8fc88839ed76e182c2a779af5b2c0da9dd18c90427a644f7e148a6253b61eb16b057a477f4bc8f572ea6bee39561098f78f15bfb3699dcbb7bd8db618540da2cb16a1ceaabf1c16b838f7a9e3f2a3a3088d9e0a6debaa748114620696ea24a3b3d822420b14b5d8cb6c28a574f01e98ea9e940551d2ebd75cee12649f9d198622acbd783d1b0d9064105b1fc8e4d8889de95c4c519b3f635809fe6afc0529d7ed391256ccc3ea596c86e933b89ff339d25ea8ddced975ae2fe30b5296d419be59f2f0413ce78c0c3703a3a5451b1d7f39629fa33abd11548a76065b29671ff3f61797e538b70e619310d33f2a063e7eb59104e112e95738da1254dc345310c16ae9959cf8358980d9dd9616e48228737310a10e2b6b731c1a548f036c480ba433a63174a90ac20992e75e3095496812b652685b5e1a2eae0b1bf4e8fcd1019ddb9df2bc98d987d0dfeca9d2b643deafab8f7036562e627c3667266a044c2d3c88b23175c5a5565db928414c66d1912b11acf974b2e644caaac04739ce992eab55f6ae4e66e32c5189eed5c470840863445760f5ed7e7b69b2a62600f354002df37a2642621802383cf952bf4dd1f32e05433beeb1fd41031fb7eace979d104aeb41435db66c3e62feccc1d6f5d98d0a0ed75d1374db457cf462e3a1f4271f3c6fd858e9a7d4b0d1f38e256a09d81d5a5e3c963987e2d4b814cfab7c6ebb2c7a07d20dff79d01fecedc1134284a8d08436606c93693b67e333f671bf69cc"
        num_trees = string.unpack("I8", param)
        print("number of trees " .. num_trees)
        roots = ""
        for i=0,num_trees-1,1 do
        tree_prefix = "tree_" .. i .. "_"
            root_data = coroutine.yield(tree_prefix .. "root")
            if string.len(root_data) > 0 then
                root = string.unpack("c64", root_data) 
                print("found tree ".. i)
            else
                root = "1eb16b057a477f4bc8f572ea6bee39561098f78f15bfb3699dcbb7bd8db61854"
            end 
            roots = roots .. root
        end

        print("calculating root")
        print("roots: " .. roots)
        root = ToT_MT(subtrees, roots, num_trees)
        print("ToT update root: " .. root)
        root_update = "ToT_root_" .. root
        root_data = coroutine.yield(root_update)
            
        updates = {}
        updates[root_update] = string.pack("c64", root) 
        return updates
    end

    deposit = string.dump(tc_deposit_contract, true)
    withdraw = string.dump(tc_withdraw_contract, true)
    tc_deposit = ""
    tc_withdraw = ""
    for i = 1, string.len(deposit) do
        hex = string.format("%x", string.byte(deposit, i))
        if string.len(hex) < 2 then
            hex = "0" .. hex
        end
        tc_deposit = tc_deposit .. hex
    end

    for i = 1, string.len(withdraw) do
        hex = string.format("%x", string.byte(withdraw, i))
        if string.len(hex) < 2 then
            hex = "0" .. hex
        end
        tc_withdraw = tc_withdraw .. hex
    end

    deposit = string.dump(ToT_deposit_contract, true)
    withdraw = string.dump(ToT_withdraw_contract, true)
    update = string.dump(ToT_update_contract, true)
    tot_deposit = ""
    tot_withdraw = ""
    tot_update = ""

    for i = 1, string.len(deposit) do
        hex = string.format("%x", string.byte(deposit, i))
        if string.len(hex) < 2 then
            hex = "0" .. hex
        end
        tot_deposit = tot_deposit .. hex
    end

    for i = 1, string.len(withdraw) do
        hex = string.format("%x", string.byte(withdraw, i))
        if string.len(hex) < 2 then
            hex = "0" .. hex
        end
        tot_withdraw = tot_withdraw .. hex
    end

    for i = 1, string.len(update) do
        hex = string.format("%x", string.byte(update, i))
        if string.len(hex) < 2 then
            hex = "0" .. hex
        end
        tot_update = tot_update .. hex
    end

    return tc_deposit, tc_withdraw, tot_deposit, tot_withdraw, tot_update
end
