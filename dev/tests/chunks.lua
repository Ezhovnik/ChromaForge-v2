app.set_setting("chunks.load-distance", 3)
app.set_setting("chunks.load-speed", 1)

app.reconfig_packs({"chromaforge"}, {})
app.new_world("demo", "67", "builtin:default")

local pid1 = player.create("Ezhovnik")
assert(player.get_name(pid1) == "Ezhovnik")

local pid2 = player.create("GoshaNotGrisha")
assert(player.get_name(pid2) == "GoshaNotGrisha")

local seed = math.floor(math.random() * 1e6)
print("random seed", seed)
math.randomseed(seed)

for i=1, 25 do
    if i % 5 == 0 then
        print(tostring(i*4).." % done")
        print("chunks loaded", world.count_chunks())
    end
    player.set_pos(pid1, math.random() * 100 - 50, 100, math.random() * 100 - 50)
    player.set_pos(pid2, math.random() * 200 - 100, 100, math.random() * 200 - 100)
    app.spark()
end

player.delete(pid2)

app.close_world(true)
