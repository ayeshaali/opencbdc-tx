function gen_bytecode()
    tc_deposit_contract = function(param)
        from, commitment, amount = string.unpack("c32 I8 I8", param)

        function insert(commitment)
            updates = MT_insert(commitment)
            return updates
        end 

        function update_pool(updates, deposit_amt)
            pool_data = coroutine.yield("TC_pool")
            pool_amount = 0 
            if string.len(account_data) > 0 then
                pool_amount = string.unpack("I8", pool_data) 
            end
            updates["TC_pool"] = string.pack("I8", pool_amount+deposit_amt)
        end 

        function update_account(updates, from)
            account_data = coroutine.yield("account_" .. from)
            account_balance, account_sequence
            = string.unpack("I8 I8", account_data)
            return account_balance, account_sequence
            updates["TC_pool"] = string.pack("I8", account_balance - amount)
        end 

        updates = insert_MT(commitment)
        updates = update_pool(updates, amount)
        return updates
    end

    tc_withdraw_contract = function(param)
        from, note, amount = string.unpack("c32 c62 I8", param)
        return 1
    end

    tc_state_contract = function()
        function get_MT_elements()
            rt_data = coroutine.yield("rt")
            updates["rt"] = rt_data
            num_leaves_data = coroutine.yield("num_leaves")
            num_leaves = string.unpack("I8", num_leaves_data)
            for i=1,num_leaves do 
                leaf = coroutine.yield("leaf_" .. i)
                updates["leaf_" .. i] = coroutine.yield("leaf_" .. i)
            return updates

        end 

        return get_MT_elements()
    end

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
