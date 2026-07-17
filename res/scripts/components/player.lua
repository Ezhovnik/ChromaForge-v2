local body = entity.rigidbody
local mob = entity:require_component("builtin:mob")

local cheat_speed_mul = 5.0

local function process_player_inputs(pid, delta)
    if not hud or hud.is_inventory_open() or menu.page ~= "" then
        return
    end
    local cam = cameras.get("builtin:first-person")
    local front = cam:get_front()
    local right = cam:get_right()
    front[2] = 0.0
    vec3.normalize(front, front)

    local isjump = input.is_active('movement.jump')
    local issprint = input.is_active('movement.sprint')
    local iscrouch = input.is_active('movement.crouch')
    local isforward = input.is_active('movement.forward')
    local ischeat = input.is_active('movement.cheat')
    local isback = input.is_active('movement.back')
    local isleft = input.is_active('movement.left')
    local isright = input.is_active('movement.right')
    mob.set_flight(player.is_flight(pid))
    body:set_body_type(player.is_noclip(pid) and "kinematic" or "dynamic")
    body:set_crouching(iscrouch)

    local vel = body:get_vel()
    local speed = ischeat and cheat_speed_mul or 1.0

    local dir = {0, 0, 0}

    if isforward then vec3.add(dir, front, dir) end
    if isback then vec3.sub(dir, front, dir) end
    if isright then vec3.add(dir, right, dir) end
    if isleft then vec3.sub(dir, right, dir) end

    if vec3.length(dir) > 0.0 then
        mob.go(dir, speed, issprint, iscrouch, vel)
    end

    if mob.is_flight() then
        if isjump then
            mob.elevate(speed * 24, delta)
        elseif iscrouch then
            mob.lower(speed * 24, delta)
        end
    elseif isjump then
        mob.jump()
    end
end

function on_physics_update(tps)
    local delta = (1.0 / tps)
    local pid = entity:get_player()
    if pid ~= -1 then
        process_player_inputs(pid, delta)
    end
end
