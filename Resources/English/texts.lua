format_version = "1.0"

local COLOUR_NAMES = {
	"White", "Red", "Orange", "Yellow", "Lime", "Green", "Turquoise",
	"Cyan", "Light Blue", "Blue", "Dark Blue", "Purple", "Fuchsia", "Pink",
}

texts = {
	["text_stop"] = "Stop",
	["text_unlit"] = "Unlit",
	["text_dim"] = "Dim",
	["text_vibrant"] = "Vibrant",
	["text_flashing"] = "Flashing",

	["text_setup"] = "Setup",
	["text_setup_short"] = "Setup",
	["text_setup_shortest"] = "Set",

	["text_switch_interval"] = "Switch Interval",
	["text_switch_interval_short"] = "Interval",
	["text_switch_interval_shortest"] = "Int",
	["text_length1"] = "Immediate",
	["text_length2"] = "1 Beat",
	["text_length3"] = "1/2 Bar",
	["text_length4"] = "1 Bar",
	["text_length5"] = "2 Bars",
	["text_length6"] = "4 Bars",
	["text_length7"] = "8 Bars",
	["text_length8"] = "16 Bars",

	-- Names of the changes made by clicking the panel, as shown in the undo
	-- history. The click handler refers to them by these keys.
	["gesture_selection"] = "Change track selection",
	["gesture_scene"] = "Launch scene",
	["gesture_length"] = "Change switch interval",
	["gesture_setup"] = "Change setup mode",
	["gesture_clip_present"] = "Switch clip on or off",
	["gesture_clip_pattern"] = "Change clip pattern",
	["gesture_clip_colour"] = "Change clip colour",
}

for track = 1, 8 do
	local selection = "text_track" .. track .. "_selection"
	texts[selection] = "Track " .. track .. " Selection"
	texts[selection .. "_short"] = "T" .. track .. " Sel"
	texts[selection .. "_shortest"] = "T" .. track .. "Sl"

	local stop = "text_track" .. track .. "_stop"
	texts[stop] = "Track " .. track .. " Stop"
	texts[stop .. "_light_short"] = "T" .. track .. " Stop"
	texts[stop .. "_light_shortest"] = "T" .. track .. "St"

	texts["text_cv_out_track" .. track] = "Track " .. track .. " CV Out"

	for row = 1, 8 do
		local clip = "text_track" .. track .. "_clip" .. row
		local long = "Track " .. track .. " Clip " .. row
		local short = "T" .. track .. "C" .. row
		local digits = track .. row

		texts[clip .. "_present"] = long .. " Present"
		texts[clip .. "_present_short"] = short .. " On"
		texts[clip .. "_present_shortest"] = "O" .. digits

		texts[clip .. "_pattern"] = long .. " Pattern"
		texts[clip .. "_pattern_short"] = short .. " Pat"
		texts[clip .. "_pattern_shortest"] = "P" .. digits

		texts[clip .. "_colour"] = long .. " Colour"
		texts[clip .. "_colour_short"] = short .. " Col"
		texts[clip .. "_colour_shortest"] = "C" .. digits

		texts[clip .. "_light"] = long .. " Light"
		texts[clip .. "_light_short"] = short .. " Lgt"
		texts[clip .. "_light_shortest"] = "L" .. digits
	end
end

for scene = 1, 8 do
	local light = "text_scene" .. scene .. "_light"
	texts[light] = "Scene " .. scene .. " Light"
	texts[light .. "_short"] = "Sc" .. scene .. " Lgt"
	texts[light .. "_shortest"] = "SL" .. scene
end

for number = 1, 8 do
	texts["text_clip" .. number] = "Clip " .. number
	texts["text_pattern" .. number] = "Pattern " .. number
end

for colour, name in ipairs(COLOUR_NAMES) do
	texts["text_colour" .. colour] = name
end
