format_version = "1.0"

local COLOUR_NAMES = {
	"White", "Red", "Orange", "Yellow", "Lime", "Green", "Turquoise",
	"Cyan", "Light Blue", "Blue", "Dark Blue", "Purple", "Fuchsia", "Pink",
}

local OUTPUT_LETTERS = { "a", "b", "c", "d", "e", "f" }
local UNIPOLAR_VALUES = { "0", "16.6", "33.3", "50", "66.6", "83.3", "100" }
local BIPOLAR_VALUES = { "-100", "-66.6", "-33.3", "0", "+33.3", "+66.6", "+100" }
local TRANSITIONS = { "Off", "A", "B", "C", "D" }

-- The settings of a CV output: long name, the letters of the short name and
-- the letter of the shortest.
local OUTPUT_SETTINGS = {
	{ "pattern", "Pattern", "Pt", "P" },
	{ "unipolar", "Unipolar", "Un", "U" },
	{ "bipolar", "Bipolar", "Bi", "B" },
	{ "transition", "Transition", "Tr", "T" },
}

texts = {
	["text_stop"] = "Stop",
	["text_mute"] = "Mute",
	["text_solo"] = "Solo",
	["text_unlit"] = "Unlit",
	["text_dim"] = "Dim",
	["text_vibrant"] = "Vibrant",
	["text_flashing"] = "Flashing",

	["text_setup"] = "Setup",
	["text_setup_short"] = "Setup",
	["text_setup_shortest"] = "Set",

	["text_logo"] = "Logo",
	["text_launch_labels"] = "Launch Button Labels",
	["text_back_button"] = "Back Button",
	["text_back_label"] = "Back Button Label",

	["text_fill_in"] = "Fill-In",
	["text_fill_in_short"] = "Fill-In",
	["text_fill_in_shortest"] = "Fill",
	["text_released"] = "Released",
	["text_pressed"] = "Pressed",
	["text_fill_in_light"] = "Fill-In Light",
	["text_fill_in_light_short"] = "Fill Lgt",
	["text_fill_in_light_shortest"] = "FlLg",
	["text_fill_in_label"] = "Fill-In Caption",
	["text_forward_light"] = "Forward Light",
	["text_forward_light_short"] = "Fwd Lgt",
	["text_forward_light_shortest"] = "FwLg",
	["text_forward_label"] = "Forward Button Label",

	["text_selected_function"] = "Function",
	["text_selected_function_short"] = "Function",
	["text_selected_function_shortest"] = "Func",
	["text_function_select_stop"] = "Stop Function Select Button",
	["text_function_select_mute"] = "Mute Function Select Button",
	["text_function_select_solo"] = "Solo Function Select Button",

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
	["gesture_clip_unipolar"] = "Change clip unipolar value",
	["gesture_clip_bipolar"] = "Change clip bipolar value",
	["gesture_clip_transition"] = "Change clip transition",
	["gesture_clip_colour"] = "Change clip colour",
	["gesture_fill_in"] = "Fill in",
	["gesture_fill_in_present"] = "Switch fill-in on or off",
	["gesture_fill_in_pattern"] = "Change fill-in pattern",
	["gesture_fill_in_unipolar"] = "Change fill-in unipolar value",
	["gesture_fill_in_bipolar"] = "Change fill-in bipolar value",
	["gesture_fill_in_transition"] = "Change fill-in transition",
	["gesture_fill_in_colour"] = "Change fill-in colour",
	["gesture_landing_present"] = "Switch landing on or off",
	["gesture_landing_pattern"] = "Change landing pattern",
	["gesture_landing_unipolar"] = "Change landing unipolar value",
	["gesture_landing_bipolar"] = "Change landing bipolar value",
	["gesture_landing_transition"] = "Change landing transition",
	["gesture_landing_colour"] = "Change landing colour",
	["gesture_function"] = "Change function",
	["gesture_mute"] = "Mute or unmute track",
	["gesture_solo"] = "Solo or unsolo track",

	["text_cv_mode_bipolar"] = "Bipolar",
	["text_cv_mode_unipolar"] = "Unipolar",
	["text_cv_mode_pattern"] = "Pattern",
	["text_transition_diagrams"] = "Transition Diagrams",
}

for index, letter in ipairs(OUTPUT_LETTERS) do
	texts["text_output_" .. letter .. "_labels"] = "Output " .. string.upper(letter) .. " Labels"
end
for value = 1, 7 do
	texts["text_unipolar" .. value] = UNIPOLAR_VALUES[value]
	texts["text_bipolar" .. value] = BIPOLAR_VALUES[value]
end
for transition, name in ipairs(TRANSITIONS) do
	texts["text_transition" .. transition] = name
end

for track = 1, 8 do
	local selection = "text_track" .. track .. "_selection"
	texts[selection] = "Track " .. track .. " Selection"
	texts[selection .. "_short"] = "T" .. track .. " Sel"
	texts[selection .. "_shortest"] = "T" .. track .. "Sl"

	local mute = "text_track" .. track .. "_mute"
	texts[mute] = "Track " .. track .. " Mute"
	texts[mute .. "_short"] = "T" .. track .. " Mute"
	texts[mute .. "_shortest"] = "T" .. track .. "Mu"

	local solo = "text_track" .. track .. "_solo"
	texts[solo] = "Track " .. track .. " Solo"
	texts[solo .. "_short"] = "T" .. track .. " Solo"
	texts[solo .. "_shortest"] = "T" .. track .. "So"

	texts["text_track" .. track .. "_name"] = "Track " .. track .. " Name"
	-- What the panel shows while the name is empty (see docs/adr/0007).
	texts["text_default_track_name" .. track] = "Track " .. track

	local function_button = "text_track" .. track .. "_function"
	texts[function_button] = "Track " .. track .. " Function Light"
	texts[function_button .. "_light_short"] = "T" .. track .. " Func"
	texts[function_button .. "_light_shortest"] = "T" .. track .. "Fn"

	for _, letter in ipairs(OUTPUT_LETTERS) do
		local upper = string.upper(letter)
		texts["text_cv_out_track" .. track .. "_" .. letter] = "Track " .. track .. " CV Out " .. upper
		texts["text_track" .. track .. "_cv_mode_" .. letter] = "Track " .. track .. " Output " .. upper .. " CV Mode"
		local copy = "text_track" .. track .. "_output_" .. letter .. "_cv_mode"
		texts[copy] = "Track " .. track .. " Output " .. upper .. " CV Mode"
		texts[copy .. "_short"] = "T" .. track .. " " .. upper .. "Mode"
		texts[copy .. "_shortest"] = "M" .. track .. upper
		texts["text_track" .. track .. "_output_" .. letter .. "_name"] = "Track " .. track .. " Output " .. upper .. " Name"
		-- What the panel shows while the name is empty.
		texts["text_default_output_name" .. track .. "_" .. letter] = "CV Out " .. track .. upper
	end

	for row = 1, 8 do
		local clip = "text_track" .. track .. "_clip" .. row
		local long = "Track " .. track .. " Clip " .. row
		local short = "T" .. track .. "C" .. row
		local digits = track .. row

		texts[clip .. "_present"] = long .. " Present"
		texts[clip .. "_present_short"] = short .. " On"
		texts[clip .. "_present_shortest"] = "O" .. digits

		texts[clip .. "_name"] = long .. " Name"

		texts[clip .. "_colour"] = long .. " Colour"
		texts[clip .. "_colour_short"] = short .. " Col"
		texts[clip .. "_colour_shortest"] = "C" .. digits

		texts[clip .. "_fill_in_present"] = long .. " Fill-In Present"
		texts[clip .. "_fill_in_present_short"] = short .. " FOn"
		texts[clip .. "_fill_in_present_shortest"] = "FO" .. digits

		texts[clip .. "_fill_in_colour"] = long .. " Fill-In Colour"
		texts[clip .. "_fill_in_colour_short"] = short .. " FCl"
		texts[clip .. "_fill_in_colour_shortest"] = "FC" .. digits

		texts[clip .. "_landing_present"] = long .. " Landing Present"
		texts[clip .. "_landing_present_short"] = short .. " LOn"
		texts[clip .. "_landing_present_shortest"] = "LO" .. digits

		texts[clip .. "_landing_colour"] = long .. " Landing Colour"
		texts[clip .. "_landing_colour_short"] = short .. " LCl"
		texts[clip .. "_landing_colour_shortest"] = "LC" .. digits

		-- The settings of the CV outputs, for example "T1C1 APt" and "11AP"
		-- for the clip, "T1C1FAPt" and "11ap" for the fill-in, "T1C1LAPt" and
		-- "11Ap" for the landing.
		for _, letter in ipairs(OUTPUT_LETTERS) do
			local upper = string.upper(letter)
			for _, setting in ipairs(OUTPUT_SETTINGS) do
				local name, short_letters, shortest_letter = setting[2], setting[3], setting[4]
				local property = clip .. "_" .. letter .. "_" .. setting[1]
				texts[property] = long .. " Output " .. upper .. " " .. name
				texts[property .. "_short"] = short .. " " .. upper .. short_letters
				texts[property .. "_shortest"] = digits .. upper .. shortest_letter
				local fill_in = clip .. "_fill_in_" .. letter .. "_" .. setting[1]
				texts[fill_in] = long .. " Fill-In Output " .. upper .. " " .. name
				texts[fill_in .. "_short"] = short .. "F" .. upper .. short_letters
				texts[fill_in .. "_shortest"] = digits .. letter .. string.lower(shortest_letter)
				local landing = clip .. "_landing_" .. letter .. "_" .. setting[1]
				texts[landing] = long .. " Landing Output " .. upper .. " " .. name
				texts[landing .. "_short"] = short .. "L" .. upper .. short_letters
				texts[landing .. "_shortest"] = digits .. upper .. string.lower(shortest_letter)
			end
		end

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
	texts["text_scene" .. scene .. "_label"] = "Scene " .. scene .. " Label"
	texts["text_scene" .. scene .. "_name"] = "Scene " .. scene .. " Name"
	-- What the panel shows while the name is empty: a clip name's default
	-- goes by its scene, whatever its track.
	texts["text_default_scene_name" .. scene] = "Scene " .. scene
	texts["text_default_clip_name" .. scene] = "Clip " .. scene
end

for number = 1, 8 do
	texts["text_clip" .. number] = "Clip " .. number
	texts["text_pattern" .. number] = "Pattern " .. number
end

for colour, name in ipairs(COLOUR_NAMES) do
	texts["text_colour" .. colour] = name
end
