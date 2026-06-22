function on_block_break_by(x, y, z, pid)
    block.destruct(x, y, z, pid)
    if not player.is_infinite_items(pid) then
        inventory.use(player.get_inventory(pid))
    end
    return true
end
