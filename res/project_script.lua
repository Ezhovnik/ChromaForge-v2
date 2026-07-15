local menubg

function on_screen_changed(screen)
    if screen ~= "menu" then
        if menubg then
            menubg:destruct()
            menubg = nil
        end
        return
    end
    local controller = {}
    function controller.resize_menu_bg()
        local w, h = unpack(gui.get_viewport())
        if menubg then
            menubg.region = {0, math.floor(h / 48), math.floor(w / 48), 0}
            menubg.pos = {0, 0}
        end
        return w, h
    end
    _GUI_ROOT.root:add(
        "<image id='menubg' src='gui/menubg' size-func='DATA.resize_menu_bg' "..
        "z-index='-1000' interactive='true'/>", controller)
    menubg = _GUI_ROOT.menubg
    controller.resize_menu_bg()
    menu.page = "main"
end
