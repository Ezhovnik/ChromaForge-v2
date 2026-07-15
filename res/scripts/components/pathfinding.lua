local target
local route
local started

local tsf = entity.transform

agent = pathfinding.create_agent()
pathfinding.set_max_visited(agent, 100000)

function set_target(new_target)
    target = new_target
end

function get_target()
    return target
end

function get_route()
    return route
end

function on_update()
    if not started then
        if target then
            pathfinding.make_route_async(agent, tsf:get_pos(), target)
            started = true
        end
    else
        local new_route = pathfinding.pull_route(agent)
        if new_route then
            route = new_route
            started = false
        end
    end
end

function on_despawn()
    pathfinding.remove_agent(agent)
end
