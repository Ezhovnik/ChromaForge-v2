function on_open()
    local worlds = world.get_list()
    for _, info in ipairs(worlds) do
        local major, minor, patch = app.get_version()
        local v = info.version
        if v[1] > major or (v[1] == major and (v[2] > minor or (v[2] == minor and v[3] > patch))) then
            info.versionColor = "#A02010"
        else
            info.versionColor = "#808080"
        end
        info.versionString = string.format("%s.%s.%s", unpack(info.version))

        document.worlds:add(gui.template("world", info))
    end
end
