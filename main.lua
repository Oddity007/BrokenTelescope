--local ImageGeneratorInit = package.loadlib("generated_dependencies/libImageGenerator.dylib", "_libinit")
--ImageGeneratorInit()
--Hello()
--Process()

local Tweet = require "Tweet"
local Checkin = require "Checkin"

--Tweet("Hello again")
Checkin()

--Ripped from http://stackoverflow.com/questions/1426954/split-string-in-lua
local function split(inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        local t={} ; i=1
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                t[i] = str
                i = i + 1
        end
        return t
end

local function LoadFileAsString(name)
	local file = io.open(name, "r")
	local s = file:read("*all")
	file:close()
	return s
end

--[[local words = split(LoadFileAsString("data/google-10000-english.txt"))

math.randomseed(os.time())

local names = {}
local namefile = io.open("data/names/yob1969.txt")
while true do
	local line = namefile:read("*l")
	if not line then break end
	names[#names + 1] = split(line, ",")[1]
end

namefile:close()

local name = names[math.random(1, #names)]

local output = {name}
for i = 0, math.random(0, 3) do
	output[#output + 1] = words[math.random(1, #words)]
end

local message = table.concat(output, " ") .. "."
print(message)]]
local message = ""
os.execute("./generated_dependencies/gen")
Tweet(message, "temp/output.png")