
local tsf = entity.transform

local props = {}

local function def_prop(name, def_value)
    props[name] = SAVED_DATA[name] or ARGS[name] or def_value
    this["get_"..name] = function() return props[name] end
    this["set_"..name] = function(value)
        props[name] = value
        if math.abs(value - def_value) < 1e-7 then
            SAVED_DATA[name] = nil
        else
            SAVED_DATA[name] = value
        end
    end
end

def_prop("jump_force", 0.0)
def_prop("air_damping", 1.0)
def_prop("ground_damping", 1.0)
def_prop("movement_speed", 3.0)
def_prop("run_speed_mul", 1.5)
def_prop("crouch_speed_mul", 0.35)
def_prop("flight_speed_mul", 4.0)
def_prop("cheat_speed_mul", 5.0)
def_prop("gravity_scale", 1.0)

function jump(multiplier)
    if body:is_grounded() then
        local vel = body:get_vel()
        body:set_vel(
            vec3.add(vel, {0, props.jump_force * (multiplier or 1.0), 0}, vel))
    end
end

function elevate(speed, delta, vel)
    vel = vel or body:get_vel()
    body:set_vel(
        vec3.add(vel, {0, speed * delta, 0}, vel))
end

function lower(speed, delta, vel)
    vel = vel or body:get_vel()
    body:set_vel(
        vec3.add(vel, {0, -speed * delta, 0}, vel))
end

function move_horizontal(speed, dir, vel)
    vel = vel or body:get_vel()
    if vec3.length(dir) > 0.0 then
        vec3.normalize(dir, dir)

        vel[1] = dir[1] * speed
        vel[3] = dir[3] * speed
    end
    body:set_vel(vel)
end

local function process_player_inputs(pid, delta)
    if not hud or hud.is_inventory_open() or menu.page ~= "" then
        return
    end
    -- TODO: replace with entity direction
    local cam = cameras.get("builtin:first-person")
    local front = cam:get_front()
    local right = cam:get_right()
    front[2] = 0.0
    vec3.normalize(front, front)

    local grounded = body:is_grounded()

    local isjump = input.is_active('movement.jump')
    local issprint = input.is_active('movement.sprint')
    local iscrouch = input.is_active('movement.crouch')
    local isforward = input.is_active('movement.forward')
    local ischeat = input.is_active('movement.cheat')
    local isback = input.is_active('movement.back')
    local isleft = input.is_active('movement.left')
    local isright = input.is_active('movement.right')
    local flight = player.is_flight(pid)
    local noclip = player.is_noclip(pid)

    local vel = body:get_vel()

    local speed = props.movement_speed

    if flight then
        speed = speed * props.flight_speed_mul
    elseif issprint then
        speed = speed * props.run_speed_mul
    elseif iscrouch and grounded then
        speed = speed * props.crouch_speed_mul
    end
    body:set_crouching(iscrouch)

    if ischeat then
        speed = speed * props.cheat_speed_mul
    end

    local dir = {0, 0, 0}
    if isforward then
        vec3.add(dir, front, dir)
    end
    if isback then
        vec3.sub(dir, front, dir)
    end
    if isright then
        vec3.add(dir, right, dir)
    end
    if isleft then
        vec3.sub(dir, right, dir)
    end

    if vec3.length(dir) > 0.0 then
        move_horizontal(speed, dir, vel)
    end

    if flight then
        if isjump then
            elevate(speed * 8.0, delta)
        elseif iscrouch then
            lower(speed * 8.0, delta)
        end
    elseif isjump then
        jump()
    end
    body:set_vdamping(flight)
    body:set_gravity_scale({0, flight and 0.0 or props.gravity_scale, 0})
    body:set_linear_damping(
        (flight or not grounded) and props.air_damping or props.ground_damping
    )
    body:set_body_type(noclip and "kinematic" or "dynamic")
end

local prev_angle = 0.0

local function follow_waypoints(pathfinding, delta)
    local pos = tsf:get_pos()
    pathfinding.set_target(vec3.add(pos, {math.random(-15, 15), 1, math.random(-15, 15)}))
    local waypoint = pathfinding.next_waypoint()
    if not waypoint then
        return
    end
    local speed = props.movement_speed
    local vel = body:get_vel()
    local dir = vec3.sub(
        vec3.add(waypoint, {0.5, 0, 0.5}),
        {pos[1], math.floor(pos[2]), pos[3]}
    )
    local upper = dir[2] > 0
    dir[2] = 0.0
    local t = delta * 16.0
    local angle = (vec2.angle({dir[3], dir[1]}) + 180) * t + prev_angle * (1 - t)
    tsf:set_rot(mat4.rotate({0, 1, 0}, angle))
    prev_angle = angle
    vec3.normalize(dir, dir)
    move_horizontal(speed, dir, vel)
    if body:is_grounded() and upper then
        jump(1.0)
    end
end

function on_physics_update(tps)
    local delta = (1.0 / tps)
    local pid = entity:get_player()
    if pid ~= -1 then
        process_player_inputs(pid, delta)
    else
        local pathfinding = entity:get_component("builtin:pathfinding")
        if pathfinding then
            follow_waypoints(pathfinding, delta)
        end
    end
end
