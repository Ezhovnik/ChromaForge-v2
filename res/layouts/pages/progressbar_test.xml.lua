local anim_value = 0
local anim_dir = 1

function animated_supplier()
    anim_value = anim_value + anim_dir * 2
    if anim_value >= 100 then
        anim_value = 100
        anim_dir = -1
    elseif anim_value <= 0 then
        anim_value = 0
        anim_dir = 1
    end
    return anim_value
end


