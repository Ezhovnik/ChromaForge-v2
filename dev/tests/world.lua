-- Open
test.new_world("demo", "67", "builtin:default")
assert(world.is_open())
assert(world.get_generator() == "builtin:default")
test.sleep(1)
assert(world.get_total_time() > 0.0)
print(world.get_total_time())

-- Close
test.close_world(true)
assert(not world.is_open())

-- Reopen
test.open_world("demo")
assert(world.is_open())
assert(world.get_total_time() > 0.0)
assert(world.get_seed() == 67)
test.spark()

-- Close
test.close_world(true)
