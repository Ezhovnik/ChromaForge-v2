function create_setting(id, name, step, postfix)
    local info = builtin.get_setting_info(id)
    postfix = postfix or ""
    document.root:add(gui.template("track_setting", {
        id=id,
        name=gui.str(name, "settings"),
        value=builtin.get_setting(id),
        min=info.min,
        max=info.max,
        step=step,
        postfix=postfix
    }))
    update_setting(builtin.get_setting(id), id, name, postfix)
end

function update_setting(x, id, name, postfix)
    builtin.set_setting(id, x)
    document[id..".L"].text = string.format(
        "%s: %s%s", 
        gui.str(name, "settings"), 
        builtin.str_setting(id), 
        postfix
    )
end

function create_checkbox(id, name)
    document.root:add(string.format(
        "<checkbox consumer='function(x) builtin.set_setting(\"%s\", x) end' checked='%s'>%s</checkbox>", 
        id, builtin.str_setting(id), gui.str(name, "settings")
    ))
end

function on_open()
    create_setting("chunks.load-distance", "Load Distance", 1)
    create_setting("chunks.load-speed", "Load Speed", 1)
    create_setting("graphics.fog-curve", "Fog Curve", 0.1)
    create_setting("graphics.gamma", "Gamma", 0.05)
    create_setting("camera.fov", "FOV", 1, "°")
    create_checkbox("display.fullscreen", "Fullscreen")
    create_checkbox("display.vsync", "V-Sync")
    create_checkbox("graphics.backlight", "Backlight")
    create_checkbox("camera.shaking", "Camera Shaking")
end
