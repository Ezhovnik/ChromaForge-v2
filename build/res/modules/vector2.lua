local vector2 = 
{
    __type = "vec2",
    __call = function(vector2, x, y) 
            return vector2:new(x, y) end
}

vector2.__index = vector2



function vector2:new(x, y)
    if type(x) ~= "number" or type(y) ~= "number" then
        error("Invalid input argument. Expected two numbers. [ vec2(number, number) ]")
    end

    local obj = {
        x = x or 0,
        y = y or 0
    }

    setmetatable(obj, self)
    return obj
end

-- string vec
function vector2:strvec2()
    return "(" .. self.x .. ", " .. self.y ..  ")"
end

function vector2:round(decimals)
    decimals = decimals or 0
    self.x = math.floor(self.x * 10^decimals + 0.5) / 10^decimals
    self.y = math.floor(self.y * 10^decimals + 0.5) / 10^decimals
    return self
end

function vector2:len() 
    return math.sqrt(self.x * self.x + self.y * self.y)
end

function vector2:abtw(vector)
    local dot_product = self:dot(vector)
    local len_product = self:len() * vector:len()
    if len_product == 0 then
        return 0
    end
    local radians = math.acos(dot_product / len_product)
    return radians
end

function vector2:norm()
    local length_vectors = self:len()
    return vector2:new(self.x / length_vectors, self.y / length_vectors)
end

function vector2:proj(vector)
    if type(vector) == "number" then
        print("\n(( TypeError : proj(vec2) ))\nType arg proj(vec2)")
        error("Invalid input argument. Expected a vector2 object")
    end
    local dot_product = self:dot(vector)
    return vector:new(dot_product * (vector.x / vector:len()^2), dot_product * (vector.y / vector:len()^2))
end

function vector2:vxld(vector)
    if type(vector) == "number" then
        print("\n(( TypeError : vxld(vec2) ))\nType arg vxld(vec2, vec2)")
        error("Invalid input argument. Expected a vector2 object\n")
    end
    return vector2:new(self.x - vector.x, self.y - vector.y)
end

function vector2:dot(vector)
    if type(vector) == "number" then
        print("\n(( TypeError : dot(vec2) ))\nType arg dot(vec2)")
        error("Invalid input argument. Expected a vector2 object")
    end
    return self.x * vector.x + self.y * vector.y
end

function vector2:lerp(b, t)
    if type(b) ~= "table" then
        print("\n(( TypeError : lerp(vec2) ))\nType arg lerp(vec2, number)")
        error("Invalid input argument. Expected a vector2 object\n")
    end
    if type(t) ~= "number" or t < 0 or t > 1 then
        error("Invalid input argument. Expected a number between 0 and 1 .. (0 <= t <= 1)")
    end
    return vector2:new(self.x + t * (b.x - self.x), self.y + t * (b.y - self.y));
end

function vector2:dist(vector)
    if type(vector) ~= "table" then
        print("\n(( TypeError : dist(vec2) ))\nType arg dist(vec2)")
        error("Invalid input argument. Expected a vec2 object or a table with two numbers.")
    end
    local dx = self.x - vector.x
    local dy = self.y - vector.y
    local result = vec2(dx, dy)
    return result:len()
end

function vector2:cross(v)
    if type(v) == "number" then
        print("\n(( TypeError : cross ))\nType arg cross(vec2)")
        error("Invalid input argument. Expected a vec2 object.\n")
    end
    return self.x * v.y - self.y * v.x
end

function vector2:rot(angle, axis, convert2deg)
    if convert2deg == true then 
        angle = math.rad(angle) 
    end

    if type(axis) == "string" then
        if axis == "x" then 
            local x_new = self.x
            local y_new = self.y * math.cos(angle) - self.y * math.sin(angle)
            self.y = y_new
        elseif axis == "y" then
            local x_new = self.x * math.cos(angle) + self.x * math.sin(angle)
            local y_new = self.y
            self.x = x_new
        elseif axis == "z" then
            local x_new = self.x * math.cos(angle) - self.y * math.sin(angle)
            local y_new = self.x * math.sin(angle) + self.y * math.cos(angle)
            self.x, self.y = x_new, y_new
        end
    elseif axis == nil then
        local x_new = self.x * math.cos(angle) - self.y * math.sin(angle)
        local y_new = self.x * math.sin(angle) + self.y * math.cos(angle)
        self.x, self.y = x_new, y_new
    end
    return self:round(15)
end

function vector2.__add(value_1, value_2)
    if type(value_1) == "number" then 
        if value_1 == 0 then 
            return vector2:new(value_2.x, value_2.y)
        else 
            return vector2:new(value_2.x + value_1, value_2.y + value_1)
        end
    else
        if type(value_2) == "number" then
            if value_2 == 0 then
                return vector2:new(value_1.x, value_1.y)
            else
                return vector2:new(value_1.x + value_2, value_1.y + value_2)
            end
        else
            return vector2:new(value_1.x + value_2.x, value_1.y + value_2.y)
        end
    end
end

function vector2.__sub(value_1, value_2)
    if type(value_1) == "number" then 
        if value_1 == 0 then 
            return vector2:new(value_2.x, value_2.y)
        else 
            return vector2:new(value_2.x - value_1, value_2.y - value_1)
        end
    else
        if type(value_2) == "number" then
            if value_2 == 0 then
                return vector2:new(value_1.x, value_1.y)
            else
                return vector2:new(value_1.x - value_2, value_1.y - value_2)
            end
        else
            return vector2:new(value_1.x - value_2.x, value_1.y - value_2.y)
        end
    end
end

function vector2.__mul(value_1, value_2)
    if type(value_1) == "number" then
        return vector2:new(value_2.x * value_1, value_2.y * value_1)
    else
        if type(value_2) == "number" then
            return vector2:new(value_1.x * value_2, value_1.y * value_2)
        else
            return vector2:new(value_1.x * value_2.x, value_1.y * value_2.y)
        end
    end
end

function vector2.__pow(value_1, value_2)
    if type(value_1) == "number" then
        return vector2:new(value_1.x ^ value_2, value_2.y ^ value_1)
    else
        if type(value_2) == "number" then
            return vector2:new(value_1.x ^ value_2, value_1.y ^ value_2)
        else
            return vector2:new(value_1.x ^ value_2.x, value_1.y ^ value_2.y)
        end
    end
end

function vector2.__div(value_1, value_2)
    if type(value_1) == "number" then
        return vector2:new(value_2.x / value_1, value_2.y / value_1)
    else
        if type(value_2) == "number" then
            return vector2:new(value_1.x / value_2, value_1.y / value_2)
        else
            return vector2:new(value_1.x / value_2.x, value_1.y / value_2.y)
        end
    end
end

function vector2.__eq(a, b)
    return a.x == b.x and a.y == b.y
end

function vector2.__tostring(vcrt)
    return vcrt:strvec2()
end

return setmetatable(vector2, vector2)
