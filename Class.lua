--Based on http://www.rtsoft.com/forums/showthread.php?1466-A-cleaner-easier-class-idiom-in-Lua

local function RegisterSetter(self, fieldName, setter)
	local setters = self.__setters__
	if not setters then
		setters = {}
		self.__setters__ = setters
	end
	setters[fieldName] = setter
end

local function RegisterGetter(self, fieldName, getter)
	local getters = self.__getters__
	if not getters then
		getters = {}
		self.__getters__ = getters
	end
	getters[fieldName] = getter
end

local function UndefinedNewIndex(instance, name, value)
	local __undefinedNewIndex__ = instance.__undefinedNewIndex__
	if __undefinedNewIndex__ then
		__undefinedNewIndex__(instance, name, value)
	else
		local setters = instance.__setters__
		if setters then
			local setter = setters[name]
			if setter then setter(instance, value) end
		else
			error("With object: " .. tostring(instance) .. ", caught write of undefined field: \"" .. name .. "\"", 2)
		end
	end
end

local function UndefinedIndex(self, name)
	if name == "__getters__" then return end
	local getters = self.__getters__
	if getters then
		local getter = getters[name]
		if getter then return getter(self) end
	end
end

--[[local function UndefinedIndex(instance, name)
	error("With object: " .. tostring(instance) ", caught read of undefined field: \"" .. name .. "\"", 2)
end]]

local RootClassMetatable = {__index = UndefinedIndex}

return function(superclass)
	local class = superclass and superclass() or {}
	rawset(class, "__instanceMetatable__", {__index = class, __newindex = UndefinedNewIndex})
	rawset(class, "__registerSetter__", RegisterSetter)
	rawset(class, "__registerGetter__", RegisterGetter)
	rawset(class, "__super__", superclass)
	rawset(class, "__class__", class)
	rawset(class, "__callSuperMethod__", function(self, name, ...)
		local super = self.__super__
		if not super then return end
		local method = super[name]
		if not method then return end
		return method(self, ...)
	end)
	rawset(class, "__define__", rawset)
	local classMetatable = superclass and {__index = superclass} or RootClassMetatable
	function classMetatable.__call(class, ...)
		local instance = setmetatable({}, class.__instanceMetatable__)
		local init = class.__init__
		if init then
			init(instance, ...)
		end
		return instance
	end
	return setmetatable(class, classMetatable)
end
