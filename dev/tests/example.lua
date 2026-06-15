test.set_setting("chunks.load-distance", 3)
test.set_setting("chunks.load-speed", 1)

test.quit()

test.reconfig_packs({"chromaforge"}, {})
test.new_world("demo", "67", "builtin:default")
local pid1 = player.create("Ezhovnik")
assert(player.get_name(pid1) == "Ezhovnik")
local pid2 = player.create("GoshaNotGrisha")
assert(player.get_name(pid2) == "GoshaNotGrisha")

for i=1, 100 do
    if i % 10 == 0 then
        print(tostring(i).." % done")
        print("Сhunks loaded", world.count_chunks())
    end
    player.set_pos(pid1, math.random() * 100 - 50, 100, math.random() * 100 - 50)
    player.set_pos(pid2, math.random() * 200 - 100, 100, math.random() * 200 - 100)
    test.spark()
end

test.close_world(true)
