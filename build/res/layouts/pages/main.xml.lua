function on_open()
    local worlds = builtin.get_worlds_list()
    for _, name in ipairs(worlds) do
        document.worlds:add(
            "<container ".. 
                "size='380,46' "..
                "color='#0F1E2DB2' "..
                "hover-color='#162B3399' "..
                "onclick='builtin.open_world(\""..name.."\")'"..
            ">"..
                "<label pos='8,8'>"..name.."</label>"..
            "</container>"
        )
    end
end
