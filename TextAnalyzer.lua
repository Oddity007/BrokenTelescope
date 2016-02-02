local Class = require "Class"

local TextAnalyzer = Class()

function TextAnalyzer:__init__()
	self.words = {}
end

function TextAnalyzer:insertEdge(from, to)
	local word = self.words[from]
	if not word then
		word = {}
		self.words[from] = word
	end
	word[to] = (word[to] or 0) + 1
end

return TextAnalyzer
