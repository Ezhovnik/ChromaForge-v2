local util = {}

function util.create_demo_world(generator)
    app.reconfig_packs({"chromaforge"}, {})
    app.new_world("demo", "67", generator or "builtin:default")
end

return util
