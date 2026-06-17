local util = require("builtin:tests_util")

util.create_demo_world("builtin:default")
app.set_setting("chunks.load-distance", 3)
app.set_setting("chunks.load-speed", 1)

local pid = player.create("Ezhovnik")
player.set_spawnpoint(pid, 0, 100, 0)
player.set_pos(pid, 0, 100, 0)

app.sleep_until(function () return block.get(0, 0, 0) ~= -1 end)

block.place(0, 2, 0, block.index("chromaforge:sand"), 0, pid)
app.tick()

assert(block.get(0, 2, 0) == 0)

app.sleep_until(function () return block.get(0, 1, 0) == block.index("chromaforge:sand") end, 100)
