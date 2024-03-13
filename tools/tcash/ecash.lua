function ecash()
    ecash_deposit_contract = function(param)
        from, cm_x, cm_y = string.unpack("c32 c64 c64", param)

        function update_balances(updates, from)
            deposit_amt = 1
            account_data = coroutine.yield("account_" .. from)
            account_balance = string.unpack("I8", account_data)
            account_balance = account_balance - deposit_amt
            updates["account_" .. from] = string.pack("I8", account_balance)
            return updates
        end 

        signature = ecash_sign(cm_x, cm_y)
        updates = {}
        updates = update_balances(updates, from)
        return updates
    end

    ecash_withdraw_contract = function(param)
        from, nullifier, sig_x, sig_y = string.unpack("c32 c62 c64 c64", param)
        nullifier_update = "nullifier_" .. nullifier
        nullifier_data = coroutine.yield(nullifier_update)
        if string.len(nullifier_data) > 0 then
            error("nullifier was seen before")
        end

        ecash_verify(nullifier, sig_x, sig_y)

        function update_balances(updates, from)
            deposit_amt = 1
            account_data = coroutine.yield("account_" .. from)
            account_balance = string.unpack("I8", account_data)
            account_balance = account_balance + deposit_amt
            updates["account_" .. from] = string.pack("I8", account_balance)
            return updates
        end 

        updates = {}
        updates[nullifier_update] = nullifier
        updates = update_balances(updates, from)
        return updates
    end

    deposit = string.dump(ecash_deposit_contract, true)
    withdraw = string.dump(ecash_withdraw_contract, true)
    ecash_deposit = ""
    ecash_withdraw = ""
    for i = 1, string.len(deposit) do
        hex = string.format("%x", string.byte(deposit, i))
        if string.len(hex) < 2 then
            hex = "0" .. hex
        end
        ecash_deposit = ecash_deposit .. hex
    end

    for i = 1, string.len(withdraw) do
        hex = string.format("%x", string.byte(withdraw, i))
        if string.len(hex) < 2 then
            hex = "0" .. hex
        end
        ecash_withdraw = ecash_withdraw .. hex
    end

    return ecash_deposit, ecash_withdraw
end
