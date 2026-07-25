local menubg

function on_menu_clear()
    if menubg then
        menubg:destruct()
        menubg = nil
    end
end

function on_menu_setup()
    local screen_info = gui.get_screen_info()
    if not (screen_info and screen_info.panorama) then
        local controller = {}
        function controller.resize_menu_bg()
            local w, h = unpack(gui.get_viewport())
            if menubg then
                menubg.region = {0, math.floor(h / 48), math.floor(w / 48), 0}
                menubg.pos = {0, 0}
            end
            return w, h
        end
        gui.root.root:add(
            "<image id='menubg' src='gui/menubg' size-func='DATA.resize_menu_bg' "..
            "z-index='-1' interactive='true'/>", controller)
        menubg = gui.root.menubg
        controller.resize_menu_bg()
    end
    menu.page = "main"
    menu.visible = true
end
