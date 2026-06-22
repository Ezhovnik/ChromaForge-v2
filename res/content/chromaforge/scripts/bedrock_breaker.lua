function on_block_break_by(x, y, z, pid)
    block.destruct(x, y, z, pid)
    inventory.use(player.get_inventory(pid))
    return true
end
