format_version = "3.0"

-- Vocabulary follows CONTEXT.md. A selection is 0 for stop or 1..8 for the
-- launch button in that row.
--
-- Property tags, as used by Tags.h, for track t and row r, both from 1:
--   setup state:                      50
--   lamp of the setup button:         51
--   length of the switch interval:    52
--   film strip frame of the logo:     53
--   film strip frame of the launch labels: 54
--   film strip frame of the back button:   55
--   film strip frame of its label:         56
--   fill-in button, held down or not:      57
--   light of the fill-in button:           58
--   light of the set fill-in button:       59
--   film strip frames of the fill-in button, its caption, the set fill-in
--   button and its label:                  70, 71, 72, 73
--   function:                              74
--   film strip frame of the function select button of function f (0 = stop,
--   1 = mute, 2 = solo):                   75 + f
--   film strip frame of its caption:       80 + f
--   film strip frame of the label of the scene button of row r: 60 + r
--   selection of track t:            100 + t
--   mute button of track t, held down or not: 110 + t
--   solo button of track t, held down or not: 120 + t
--   light of the function button:   1000 + 10 * t
--   light of a launch button:       1000 + 10 * t + r
--   light of the scene button of row r:      1090 + r
--   film strip frame of that light:          2090 + r
--   film strip frame of that light: 2000 + 10 * t
--   film strip frame of a light:    2000 + 10 * t + r
--   whether a launch button holds a clip: 3000 + 10 * t + r
--   pattern of its clip:            4000 + 10 * t + r
--   colour of its clip:             5000 + 10 * t + r
--   whether the clip holds a fill-in:  6000 + 10 * t + r
--   pattern of the fill-in:            7000 + 10 * t + r
--   colour of the fill-in:             8000 + 10 * t + r

local TRACK_COUNT = 8
local ROW_COUNT = 8
local PATTERN_COUNT = 8
local COLOUR_COUNT = 14

-- Launch mode, clip picking, then clip editing and fill-in editing for each of
-- the launch buttons
local SETUP_COUNT = 2 + 2 * TRACK_COUNT * ROW_COUNT

-- Unlit, then dim, vibrant and flashing for each colour
local LIGHT_COUNT = 1 + 3 * COLOUR_COUNT
-- Unlit, then dim and vibrant for each colour
local LIGHT_FRAME_COUNT = 1 + 2 * COLOUR_COUNT
-- Unlit, then dim and vibrant for red, green, yellow and white
local SCENE_LIGHT_FRAME_COUNT = 9
-- Unlit, then dim and vibrant for red, green, yellow, blue and white
local SIDE_LIGHT_FRAME_COUNT = 11
-- Unlit, then dim and vibrant for red (stop), yellow (mute) and blue (solo)
local FUNCTION_LIGHT_FRAME_COUNT = 7

-- Stop, mute and solo, numbered from 0, with the names used in properties
local FUNCTIONS = { "stop", "mute", "solo" }

-- Immediate, one beat, half a bar, then 1, 2, 4, 8 and 16 bars
local LENGTH_COUNT = 8
local DEFAULT_LENGTH = 5

local selection_texts = { jbox.ui_text("text_stop") }
for row = 1, ROW_COUNT do
	table.insert(selection_texts, jbox.ui_text("text_clip" .. row))
end

local pattern_texts = {}
for pattern = 1, PATTERN_COUNT do
	table.insert(pattern_texts, jbox.ui_text("text_pattern" .. pattern))
end

local length_texts = {}
for length = 1, LENGTH_COUNT do
	table.insert(length_texts, jbox.ui_text("text_length" .. length))
end

local colour_texts = {}
for colour = 1, COLOUR_COUNT do
	table.insert(colour_texts, jbox.ui_text("text_colour" .. colour))
end

local function plain_number(max)
	return jbox.ui_linear({ min = 0, max = max, units = { { decimals = 0 } } })
end

-- Red, orange, yellow, green, cyan, blue, purple and pink: the colour a clip
-- has until another one is chosen, by track. Colours count from 0.
local DEFAULT_COLOURS = { 1, 2, 3, 5, 7, 9, 11, 13 }

local document_properties = {}
local rt_properties = {}
remote_implementation_chart = {}

local function add_remote_item(property, internal_name)
	remote_implementation_chart["/custom_properties/" .. property] = {
		internal_name = internal_name,
		short_ui_name = jbox.ui_text("text_" .. property .. "_short"),
		shortest_ui_name = jbox.ui_text("text_" .. property .. "_shortest"),
	}
end

-- Reason only offers automation for properties that have a number in the MIDI
-- CC chart. Numbers from 256 upwards enable automation without responding to
-- any MIDI CC, so nothing a Launchpad sends can reach a selection by accident.
-- Only the selections, the fill-in button and the mute and solo buttons are
-- automatable; clips, the setup state and the function are not.
local FIRST_SELECTION_MIDI_CC = 256
local FILL_IN_MIDI_CC = FIRST_SELECTION_MIDI_CC + TRACK_COUNT
local FIRST_MUTE_MIDI_CC = FILL_IN_MIDI_CC + 1
local FIRST_SOLO_MIDI_CC = FIRST_MUTE_MIDI_CC + TRACK_COUNT
local midi_cc_chart = {}

-- 0 = launch mode, 1 = clip picking, 2..65 = clip editing and 66..129 =
-- fill-in editing for one launch button. Written by the panel and by control surfaces, read by the realtime
-- code (see docs/adr/0002). Not saved: a song opens in launch mode.
document_properties.setup = jbox.number{
	property_tag = 50,
	default = 0,
	steps = SETUP_COUNT,
	persistence = "none",
	ui_name = jbox.ui_text("text_setup"),
	ui_type = plain_number(SETUP_COUNT - 1),
}
add_remote_item("setup", "Setup")

-- The length of the switch interval: 0 = immediate, 1 = one beat, 2 = half a
-- bar, 3..7 = 1, 2, 4, 8 and 16 bars. Saved with the song and in patches (see
-- docs/adr/0003), but not automatable.
document_properties.switch_interval = jbox.number{
	property_tag = 52,
	default = DEFAULT_LENGTH,
	steps = LENGTH_COUNT,
	ui_name = jbox.ui_text("text_switch_interval"),
	ui_type = jbox.ui_selector(length_texts),
}
add_remote_item("switch_interval", "Switch Interval")

-- 1 while the fill-in button is held down. The realtime code triggers a fill-in
-- when it goes down; that is also what automation records and plays back. Not
-- saved, so that a song never opens with the button held.
document_properties.fill_in = jbox.boolean{
	property_tag = 57,
	default = false,
	persistence = "none",
	ui_name = jbox.ui_text("text_fill_in"),
	ui_type = jbox.ui_selector{ jbox.ui_text("text_released"), jbox.ui_text("text_pressed") },
}
midi_cc_chart[FILL_IN_MIDI_CC] = "/custom_properties/fill_in"
add_remote_item("fill_in", "Fill-In")

-- What the function buttons do: 0 = stop, 1 = mute, 2 = solo. Written by the
-- panel and by control surfaces, which look it up to tell what a press of a
-- function button means. Not saved, so that a song opens with stop, and not
-- automatable: automation records the mute and solo presses themselves (see
-- docs/adr/0005).
document_properties.selected_function = jbox.number{
	property_tag = 74,
	default = 0,
	steps = #FUNCTIONS,
	persistence = "none",
	ui_name = jbox.ui_text("text_selected_function"),
	ui_type = jbox.ui_selector{ jbox.ui_text("text_stop"), jbox.ui_text("text_mute"), jbox.ui_text("text_solo") },
}
add_remote_item("selected_function", "Function")

-- The frames the front panel shows of the function select buttons, in the
-- side buttons' film strip, and of their captions: 0 = white, 1 = in the
-- colour of the function while it is chosen. Computed by the realtime code.
for index, name in ipairs(FUNCTIONS) do
	rt_properties["function_select_" .. name .. "_light_frame"] = jbox.number{
		property_tag = 75 + index - 1,
		default = 0,
		steps = SIDE_LIGHT_FRAME_COUNT,
		ui_name = jbox.ui_text("text_function_select_" .. name),
		ui_type = plain_number(SIDE_LIGHT_FRAME_COUNT - 1),
	}
	rt_properties["function_select_" .. name .. "_caption_frame"] = jbox.number{
		property_tag = 80 + index - 1,
		default = 0,
		steps = 2,
		ui_name = jbox.ui_text("text_function_select_" .. name),
		ui_type = plain_number(1),
	}
end

-- What the fill-in button and the set fill-in button show, for control
-- surfaces, encoded like the light of a launch button: dim white (1) or
-- vibrant red (5) for the fill-in button, unlit (0) or vibrant red (5) for the
-- set fill-in button. Computed by the realtime code.
rt_properties.fill_in_light = jbox.number{
	property_tag = 58,
	default = 0,
	steps = LIGHT_COUNT,
	ui_name = jbox.ui_text("text_fill_in_light"),
	ui_type = plain_number(LIGHT_COUNT - 1),
}
add_remote_item("fill_in_light", "Fill-In Light")

rt_properties.set_fill_in_light = jbox.number{
	property_tag = 59,
	default = 0,
	steps = LIGHT_COUNT,
	ui_name = jbox.ui_text("text_set_fill_in_light"),
	ui_type = plain_number(LIGHT_COUNT - 1),
}
add_remote_item("set_fill_in_light", "Set Fill-In Light")

-- The frames the front panel shows of the two buttons, and of their
-- captions: the fill-in button's film strip is that of the side buttons, the
-- set fill-in button's that of the scene buttons; FILL-IN is 0 = white, 1 =
-- red, SET FILL-IN 0 = unlit, 1 = red.
rt_properties.fill_in_light_frame = jbox.number{
	property_tag = 70,
	default = 0,
	steps = SIDE_LIGHT_FRAME_COUNT,
	ui_name = jbox.ui_text("text_fill_in_light"),
	ui_type = plain_number(SIDE_LIGHT_FRAME_COUNT - 1),
}

rt_properties.fill_in_label_frame = jbox.number{
	property_tag = 71,
	default = 0,
	steps = 2,
	ui_name = jbox.ui_text("text_fill_in_label"),
	ui_type = plain_number(1),
}

rt_properties.set_fill_in_light_frame = jbox.number{
	property_tag = 72,
	default = 0,
	steps = SCENE_LIGHT_FRAME_COUNT,
	ui_name = jbox.ui_text("text_set_fill_in_light"),
	ui_type = plain_number(SCENE_LIGHT_FRAME_COUNT - 1),
}

rt_properties.set_fill_in_label_frame = jbox.number{
	property_tag = 73,
	default = 0,
	steps = 2,
	ui_name = jbox.ui_text("text_set_fill_in_label"),
	ui_type = plain_number(1),
}

-- Whether the setup button is lit: in every step of setup mode.
rt_properties.setup_lamp = jbox.boolean{
	property_tag = 51,
	default = false,
	ui_name = jbox.ui_text("text_setup"),
	ui_type = plain_number(1),
}

-- The labels and the logo, computed by the realtime code. The logo shows the
-- mode colour: 0 = white (launch mode), 1 = green (clip picking), 2 = yellow
-- (clip editing), 3 = red (fill-in editing). The labels of the launch buttons
-- are lit only in clip editing, in yellow, and in fill-in editing, in red: 0 =
-- unlit, 1 = SET/CLEAR in yellow, 2 = all in yellow, 3 = SET/CLEAR in red, 4 =
-- all in red; the colours and patterns only while the picked launch button
-- holds a clip, or in fill-in editing a fill-in. A scene button's label is 0 =
-- unlit, 1 = white, 2 = green.
rt_properties.logo_frame = jbox.number{
	property_tag = 53,
	default = 0,
	steps = 4,
	ui_name = jbox.ui_text("text_logo"),
	ui_type = plain_number(3),
}

rt_properties.launch_labels_frame = jbox.number{
	property_tag = 54,
	default = 0,
	steps = 5,
	ui_name = jbox.ui_text("text_launch_labels"),
	ui_type = plain_number(4),
}

-- The back button's frame, computed by the realtime code: 0 = dark (launch
-- mode), 7 = vibrant white (clip picking), 3 = vibrant green (clip editing), 5
-- = vibrant yellow (fill-in editing). Not a Remote item: a codec can tell it
-- from the setup state.
rt_properties.back_button_frame = jbox.number{
	property_tag = 55,
	default = 0,
	steps = 9,
	ui_name = jbox.ui_text("text_back_button"),
	ui_type = plain_number(8),
}

-- Its label, in the colour of the button: 0 = unlit, 1 = white, 2 = green, 3
-- = yellow.
rt_properties.back_label_frame = jbox.number{
	property_tag = 56,
	default = 0,
	steps = 4,
	ui_name = jbox.ui_text("text_back_label"),
	ui_type = plain_number(3),
}

for scene = 1, ROW_COUNT do
	rt_properties["scene" .. scene .. "_label_frame"] = jbox.number{
		property_tag = 60 + scene,
		default = 0,
		steps = 3,
		ui_name = jbox.ui_text("text_scene" .. scene .. "_label"),
		ui_type = plain_number(2),
	}
end

for track = 1, TRACK_COUNT do
	-- What the user last chose on the track. Saved with the song, automatable,
	-- but not part of a patch (see docs/adr/0003).
	local selection = "track" .. track .. "_selection"
	document_properties[selection] = jbox.number{
		property_tag = 100 + track,
		default = 0,
		steps = ROW_COUNT + 1,
		persistence = "song",
		ui_name = jbox.ui_text("text_" .. selection),
		ui_type = jbox.ui_selector(selection_texts),
	}
	midi_cc_chart[FIRST_SELECTION_MIDI_CC + track - 1] = "/custom_properties/" .. selection
	add_remote_item(selection, "Track " .. track .. " Selection")

	-- 1 while the track's mute or solo button is held down. The realtime code
	-- switches the track's mute or solo when it goes down; that is also what
	-- automation records and plays back. Not saved, so that a song never opens
	-- with a button held (see docs/adr/0005).
	for index, name in ipairs({ "mute", "solo" }) do
		local press = "track" .. track .. "_" .. name
		document_properties[press] = jbox.boolean{
			property_tag = 100 + 10 * index + track,
			default = false,
			persistence = "none",
			ui_name = jbox.ui_text("text_" .. press),
			ui_type = jbox.ui_selector{ jbox.ui_text("text_released"), jbox.ui_text("text_pressed") },
		}
		local first_midi_cc = name == "mute" and FIRST_MUTE_MIDI_CC or FIRST_SOLO_MIDI_CC
		midi_cc_chart[first_midi_cc + track - 1] = "/custom_properties/" .. press
		add_remote_item(press, "Track " .. track .. " " .. (name == "mute" and "Mute" or "Solo"))
	end

	-- What the function button shows, for control surfaces, in the colour of
	-- the function: 0 = unlit, 1 = dim, 2 = vibrant, 3 = flashing. In stop
	-- function: unlit (the track has no clips), dim (stopped), vibrant (a clip
	-- is active), flashing (about to stop). In mute function vibrant, dim while
	-- muted; in solo function dim, vibrant while soloed; unlit without clips.
	-- Computed by the realtime code.
	local function_button = "track" .. track .. "_function"
	rt_properties[function_button .. "_light"] = jbox.number{
		property_tag = 1000 + 10 * track,
		default = 0,
		steps = 4,
		ui_name = jbox.ui_text("text_" .. function_button),
		ui_type = jbox.ui_selector{ jbox.ui_text("text_unlit"), jbox.ui_text("text_dim"), jbox.ui_text("text_vibrant"), jbox.ui_text("text_flashing") },
	}
	add_remote_item(function_button .. "_light", "Track " .. track .. " Function Light")

	-- The frame of the function buttons' film strip the front panel shows
	-- right now: unlit, then dim and vibrant for red, yellow and blue.
	rt_properties[function_button .. "_light_frame"] = jbox.number{
		property_tag = 2000 + 10 * track,
		default = 0,
		steps = FUNCTION_LIGHT_FRAME_COUNT,
		ui_name = jbox.ui_text("text_" .. function_button),
		ui_type = plain_number(FUNCTION_LIGHT_FRAME_COUNT - 1),
	}

	for row = 1, ROW_COUNT do
		local clip = "track" .. track .. "_clip" .. row
		local remote_name = "Track " .. track .. " Clip " .. row

		-- What the launch button holds. Pattern, colour and fill-in are kept while
		-- it is empty, so that a clip comes back as it was. These six make up a
		-- patch.
		document_properties[clip .. "_present"] = jbox.boolean{
			property_tag = 3000 + 10 * track + row,
			default = false,
			ui_name = jbox.ui_text("text_" .. clip .. "_present"),
			ui_type = plain_number(1),
		}
		add_remote_item(clip .. "_present", remote_name .. " Present")

		-- A new clip plays the pattern of its row.
		document_properties[clip .. "_pattern"] = jbox.number{
			property_tag = 4000 + 10 * track + row,
			default = row - 1,
			steps = PATTERN_COUNT,
			ui_name = jbox.ui_text("text_" .. clip .. "_pattern"),
			ui_type = jbox.ui_selector(pattern_texts),
		}
		add_remote_item(clip .. "_pattern", remote_name .. " Pattern")

		document_properties[clip .. "_colour"] = jbox.number{
			property_tag = 5000 + 10 * track + row,
			default = DEFAULT_COLOURS[track],
			steps = COLOUR_COUNT,
			ui_name = jbox.ui_text("text_" .. clip .. "_colour"),
			ui_type = jbox.ui_selector(colour_texts),
		}
		add_remote_item(clip .. "_colour", remote_name .. " Colour")

		-- The clip's fill-in. A new one has the pattern of its clip, in white:
		-- whoever sets one up writes all three (see Press.h), so these defaults
		-- only matter for a fill-in set up some other way.
		document_properties[clip .. "_fill_in_present"] = jbox.boolean{
			property_tag = 6000 + 10 * track + row,
			default = false,
			ui_name = jbox.ui_text("text_" .. clip .. "_fill_in_present"),
			ui_type = plain_number(1),
		}
		add_remote_item(clip .. "_fill_in_present", remote_name .. " Fill-In Present")

		document_properties[clip .. "_fill_in_pattern"] = jbox.number{
			property_tag = 7000 + 10 * track + row,
			default = row - 1,
			steps = PATTERN_COUNT,
			ui_name = jbox.ui_text("text_" .. clip .. "_fill_in_pattern"),
			ui_type = jbox.ui_selector(pattern_texts),
		}
		add_remote_item(clip .. "_fill_in_pattern", remote_name .. " Fill-In Pattern")

		document_properties[clip .. "_fill_in_colour"] = jbox.number{
			property_tag = 8000 + 10 * track + row,
			default = 0,
			steps = COLOUR_COUNT,
			ui_name = jbox.ui_text("text_" .. clip .. "_fill_in_colour"),
			ui_type = jbox.ui_selector(colour_texts),
		}
		add_remote_item(clip .. "_fill_in_colour", remote_name .. " Fill-In Colour")

		-- What the launch button shows, for control surfaces: 0 = unlit, then
		-- dim, vibrant and flashing for each colour. Flashing is one steady value.
		rt_properties[clip .. "_light"] = jbox.number{
			property_tag = 1000 + 10 * track + row,
			default = 0,
			steps = LIGHT_COUNT,
			ui_name = jbox.ui_text("text_" .. clip .. "_light"),
			ui_type = plain_number(LIGHT_COUNT - 1),
		}
		add_remote_item(clip .. "_light", remote_name .. " Light")

		-- The frame of the film strip the front panel shows right now.
		rt_properties[clip .. "_light_frame"] = jbox.number{
			property_tag = 2000 + 10 * track + row,
			default = 0,
			steps = LIGHT_FRAME_COUNT,
			ui_name = jbox.ui_text("text_" .. clip .. "_light"),
			ui_type = plain_number(LIGHT_FRAME_COUNT - 1),
		}
	end
end

for scene = 1, ROW_COUNT do
	-- What the scene button shows as its part of the interval meter, for
	-- control surfaces; encoded like the light of a launch button. A scene
	-- button has no property for presses: a press writes the selection of
	-- every track (see docs/adr/0004).
	local light = "scene" .. scene .. "_light"
	rt_properties[light] = jbox.number{
		property_tag = 1090 + scene,
		default = 0,
		steps = LIGHT_COUNT,
		ui_name = jbox.ui_text("text_" .. light),
		ui_type = plain_number(LIGHT_COUNT - 1),
	}
	add_remote_item(light, "Scene " .. scene .. " Light")

	-- The frame of the scene buttons' film strip the front panel shows right
	-- now.
	rt_properties[light .. "_frame"] = jbox.number{
		property_tag = 2090 + scene,
		default = 0,
		steps = SCENE_LIGHT_FRAME_COUNT,
		ui_name = jbox.ui_text("text_" .. light),
		ui_type = plain_number(SCENE_LIGHT_FRAME_COUNT - 1),
	}
end

custom_properties = jbox.property_set{
	document_owner = {
		properties = document_properties
	},
	rt_owner = {
		properties = rt_properties
	},
	rtc_owner = {
		properties = {
			instance = jbox.native_object{},
		}
	},
}

midi_implementation_chart = {
	midi_cc_chart = midi_cc_chart
}

cv_outputs = {}
for track = 1, TRACK_COUNT do
	cv_outputs["track" .. track] = jbox.cv_output{
		ui_name = jbox.ui_text("text_cv_out_track" .. track)
	}
end
