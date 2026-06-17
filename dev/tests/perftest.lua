app.set_setting("chunks.load-distance", 3)
app.set_setting("chunks.load-speed", 16)

app.reconfig_packs({"chromaforge"}, {})
app.new_world("demo", "67", "builtin:default")

local pid = player.create("aboba")
assert(player.get_name(pid) == "aboba")
app.spark()

timeit(1e7, block.get, 0, 0, 0)
timeit(1e7, block.set, 0, 0, 0, 6)
