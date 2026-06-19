local gui_util = {}

function gui_util.parse_query(query)
    local args = {}
    local name

    local index = string.find(query, '?')
    if index then
        local argstr = string.sub(query, index + 1)
        name = string.sub(query, 1, index - 1)

        for key, value in string.gmatch(argstr, "([^=&]*)=([^&]*)") do
            args[key] = value
        end
    else
        name = query
    end
    return name, args
end

function gui_util.load_page(query)
    local name, args = gui_util.parse_query(query)
    local filename = file.find(string.format("layouts/pages/%s.xml", name))
    if filename then
        name = file.prefix(filename)..":pages/"..name
        gui.load_document(filename, name, args)
        return name
    end
end

return gui_util
