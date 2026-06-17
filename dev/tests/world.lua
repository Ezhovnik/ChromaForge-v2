-- Open
app.reconfig_packs({"chromaforge"}, {})
app.new_world("demo", "67", "builtin:default")
assert(world.is_open())
assert(world.get_generator() == "builtin:default")
app.sleep(1)
assert(world.get_total_time() > 0.0)
print(world.get_total_time())

-- Close
app.close_world(true)
assert(not world.is_open())

-- Reopen
app.open_world("demo")
assert(world.is_open())
assert(world.get_total_time() > 0.0)
assert(world.get_seed() == 67)
app.spark()
app.reconfig_packs({}, {"chromaforge"})
app.spark()

-- Close
app.close_world(true)
app.delete_world("demo")
