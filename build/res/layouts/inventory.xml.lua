function inventory_share_func(invid, slotid)
    inventory.set(invid, slotid, 0, 0)
end

function inventory_reposition()
    return 10, gui.get_viewport()[2] - document.root.size[2] - 100
end
