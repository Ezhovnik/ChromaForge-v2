function is_array(x)
    if #t > 0 then
        return true
    end
    for k, v in pairs(x) do
        return false
    end
    return true
end

function parse_path(path)
    local index = string.find(path, ':')
    if index == nil then
        error("invalid path syntax (':' missing)")
    end
    return string.sub(path, 1, index - 1), string.sub(path, index + 1, -1)
end

local __cached_scripts = {}
local __cached_results = {}

function load_script(path, nocache)
    local packname, filename = parse_path(path)
    local fullpath = file.resolve(path);

    if not nocache and __cached_scripts[fullpath] ~= nil then
        return __cached_results[fullpath]
    end
    local script = loadfile(fullpath)
    if script == nil then
        error("script '"..filename.."' not found in '"..packname.."'")
    end
    local result = script()
    if not nocache then
        __cached_scripts[fullpath] = script
        __cached_results[fullpath] = result
    end
    return result
end

function sleep(timesec)
    local start = time.uptime()
    while time.uptime() - start < timesec do
        coroutine.yield()
    end
end

_dofile = dofile
function dofile(path)
    local index = string.find(path, "/content/")
    if index then
        local newpath = string.sub(path, index + 9)
        index = string.find(newpath, "/")
        if index then
            local label = string.sub(newpath, 1, index - 1)
            newpath = label..':'..string.sub(newpath, index + 1)
            if file.isfile(newpath) then
                return load_script(newpath, true)
            end
        end
    end
    return _dofile(path)
end
