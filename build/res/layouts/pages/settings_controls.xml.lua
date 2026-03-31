function refresh_sensitivity()
    document.sensitivity_label.text = string.format(
        "%s: %s", 
        gui.str("Mouse Sensitivity", "settings"),
        builtin.str_setting("camera.sensitivity")
    )
end

function change_sensitivity(val)
    builtin.set_setting("camera.sensitivity", val)
    refresh_sensitivity()
end

function on_open()
    document.sensitivity_track.value = builtin.get_setting("camera.sensitivity")
    refresh_sensitivity()

    local panel = document.bindings_panel
    local bindings = builtin.get_bindings()
    table.sort(bindings, function(a, b) return a > b end)
    for i,name in ipairs(bindings) do
        panel:add(gui.template("binding", {
            id=name, name=gui.str(name)
        }))
    end
end
