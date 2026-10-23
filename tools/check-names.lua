-- Checks the host's limits on names, which the build does not: a property
-- name may have at most 34 characters, a Remote item's internal name fewer
-- than 64, its short name at most 8 and its shortest name at most 4. Recon
-- refuses a device that breaks them, with an assertion or a "damaged format"
-- error, when it is put into the rack.
--
-- Run from the root of the Rack Extension: luajit tools/check-names.lua

jbox = setmetatable({ ui_text = function(key) return key end }, {
	__index = function() return function(value) return value end end,
})
dofile("motherboard_def.lua")
dofile("Resources/English/texts.lua")

local problems = 0
local function problem(text)
	problems = problems + 1
	print(text)
end

for _, owner in ipairs({ "document_owner", "rt_owner" }) do
	for name in pairs(custom_properties[owner].properties) do
		if #name > 34 then
			problem("property name longer than 34 characters: " .. name)
		end
	end
end
for _, item in pairs(remote_implementation_chart) do
	local short, shortest = texts[item.short_ui_name], texts[item.shortest_ui_name]
	if #item.internal_name >= 64 then
		problem("Remote internal name of 64 characters or more: " .. item.internal_name)
	end
	if not short or #short > 8 then
		problem("Remote short name missing or longer than 8 characters: " .. item.internal_name)
	end
	if not shortest or #shortest > 4 then
		problem("Remote shortest name missing or longer than 4 characters: " .. item.internal_name)
	end
end

print("Names: " .. problems .. " problems")
os.exit(problems == 0 and 0 or 1)
