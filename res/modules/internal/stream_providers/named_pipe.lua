local FFI = ffi

if FFI.os == "Windows" then
    return require "builtin:internal/stream_providers/named_pipe_windows"
else
    return require "builtin:internal/stream_providers/named_pipe_unix"
end
