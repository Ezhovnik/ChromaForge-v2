local tostring_overrides = {}
tostring_overrides["display.framerate"] = function(x)
    if x == -1 then
        return gui.str("V-Sync")
    elseif x == 0 then
        return gui.str("Unlimited")
    else
        return tostring(x)
    end
end

function create_setting(id, name, step, postfix, tooltip, changeonrelease)
    local info = builtin.get_setting_info(id)
    postfix = postfix or ""
    tooltip = tooltip or ""
    changeonrelease = changeonrelease or ""
    document.root:add(gui.template("track_setting", {
        id=id,
        name=gui.str(name, "settings"),
        value=builtin.get_setting(id),
        min=info.min,
        max=info.max,
        step=step,
        postfix=postfix,
        tooltip=tooltip,
        changeonrelease=changeonrelease
    }))
    update_setting(builtin.get_setting(id), id, name, postfix)
end

function update_setting(x, id, name, postfix)
    local str
    local func = tostring_overrides[id]
    if func then
        str = func(x)
    else
        str = builtin.str_setting(id)
    end
    document[id..".L"].text = string.format(
        "%s: %s%s", 
        gui.str(name, "settings"), 
        str, 
        postfix
    )
end

function create_checkbox(id, name, tooltip)
    tooltip = tooltip or ''
    document.root:add(string.format(
        "<checkbox consumer='function(x) builtin.set_setting(\"%s\", x) end' checked='%s' tooltip='%s'>%s</checkbox>", 
        id, builtin.str_setting(id), gui.str(tooltip, "settings"), gui.str(name, "settings")
    ))
end

function on_open()
    create_setting("camera.fov", "FOV", 1, "°")
    create_setting("display.framerate", "Framerate", 1, "", "", true)
    create_checkbox("display.fullscreen", "Fullscreen")
    create_checkbox("camera.shaking", "Camera Shaking")
    create_checkbox("camera.inertia", "Camera Inertia")
end
