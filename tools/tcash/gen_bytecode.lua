function gen_bytecode()
    tc_deposit_contract = function(param)
        from, commitment = string.unpack("c32 c64", param)
        function insert(commitment)
            num_leaves_data = coroutine.yield("num_leaves")
            num_leaves = 0 
            if string.len(num_leaves_data) > 0 then
                num_leaves = string.unpack("I8", num_leaves_data) 
            end 
            leaves = ""
            if num_leaves > 0 then 
                for i=0,num_leaves-1 do
                    leaf_data = coroutine.yield("leaf_" .. i)
                    leaves = leaves .. string.unpack("c64", leaf_data) 
                end 
            end 
            root = insert_MT(num_leaves, leaves, commitment)
            print(root)
            updates = {}
            updates["root_" .. root] = string.pack("c64", root) 
            updates["leaf_" .. num_leaves] = string.pack("c64", commitment) 
            updates["num_leaves"] = string.pack("I8", num_leaves+1) 
            return updates
        end 

        function update_pool(updates, deposit_amt)
            deposit_amt = deposit_amt or 0.1
            pool_data = coroutine.yield("TC_pool")
            pool_amount = 0 
            if string.len(pool_data) > 0 then
                pool_amount = string.unpack("I8", pool_data) 
            end
            updates["TC_pool"] = string.pack("I8", pool_amount+deposit_amt)
            return updates
        end 

        function update_account(updates, from)
            account_data = coroutine.yield("account_" .. from)
            account_balance = string.unpack("I8", account_data)
            updates["account_" .. from] = string.pack("I8", account_balance - amount)
            return updates
        end 

        updates = insert(commitment)
        -- updates = update_pool(updates)
        -- updates = update_account(updates, from)
        return updates
    end

    tc_withdraw_contract = function(param)
        proof, root, nullifierHash, recipient, fee, refund = string.unpack("c24 c24 c24 c24 c24 c24 c24", param)
        root_data = coroutine.yield("root_" .. root)
        if not (string.len(root_data) > 0) then
            error("root does not exist")
        end

        nullifierHash_data = coroutine.yield("nullifier_" .. nullifierHash)
        if string.len(nullifierHash_data) > 0 then
            error("nullifier hash was seen before")
        end

        verifyProof(proof, root, nullifierHash, recipient, fee, refund)
        
        updates = {}
        updates["nullifier_" .. nullifierHash] = nullifierHash
        return updates
    end

    -- tc_state_contract = function()
    --     function get_MT_elements()
    --         rt_data = coroutine.yield("rt")
    --         updates["rt"] = rt_data
    --         num_leaves_data = coroutine.yield("num_leaves")
    --         num_leaves = string.unpack("I8", num_leaves_data)
    --         for i=1,num_leaves do 
    --             leaf = coroutine.yield("leaf_" .. i)
    --             updates["leaf_" .. i] = coroutine.yield("leaf_" .. i)
    --         return updates

    --     end 

    --     return get_MT_elements()
    -- end

    deposit = string.dump(tc_deposit_contract, true)
    withdraw = string.dump(tc_withdraw_contract, true)
    tot_deposit = ""
    tot_withdraw = ""
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

    return tot_deposit, tot_withdraw
end
