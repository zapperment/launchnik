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
--   light of the forward button:           59
--   film strip frames of the fill-in button, its caption, the forward
--   button and its labels:                 70, 71, 72, 73
--   function:                              74
--   film strip frame of the function select button of function f (0 = stop,
--   1 = mute, 2 = solo):                   75 + f
--   film strip frame of its caption:       80 + f
--   film strip frame of the label of the scene button of row r: 60 + r
--   selection of track t:            100 + t
--   film strip frame of the labels of the row of CV output o: 140 + o
--   film strip frame of the transition diagrams: 150
--   name of track t:                 400 + t
--   name of the scene of row r:      410 + r
--   name of a launch button's clip:  4000 + 10 * t + r
--   name of CV output o of track t:  500 + 10 * t + o
--   CV mode of CV output o of track t:  200 + 10 * t + o
--   the same, for control surfaces:     300 + 10 * t + o
--   mute button of track t, held down or not: 110 + t
--   solo button of track t, held down or not: 120 + t
--   light of the function button:   1000 + 10 * t
--   light of a launch button:       1000 + 10 * t + r
--   light of the scene button of row r:      1090 + r
--   film strip frame of that light:          2090 + r
--   film strip frame of that light: 2000 + 10 * t
--   film strip frame of a light:    2000 + 10 * t + r
--   whether a launch button holds a clip: 3000 + 10 * t + r
--   colour of its clip:             5000 + 10 * t + r
--   whether the clip holds a fill-in:  6000 + 10 * t + r
--   whether the clip holds a landing:  7000 + 10 * t + r
--   colour of the fill-in:             8000 + 10 * t + r
--   colour of the landing:             9000 + 10 * t + r
--   one-bar pattern of the landing:    22000 + 10 * t + r
--   one-bar pattern of the fill-in:    23000 + 10 * t + r
--   and for CV output o, from 1 for A to 6 for F, of the clip, of the
--   fill-in and of the landing, each + 100 * t + 10 * r + o:
--     pattern, unipolar value, bipolar value, transition of the clip:
--                                      10000, 11000, 12000, 13000
--     the same of the fill-in:         14000, 15000, 16000, 17000
--     the same of the landing:         18000, 19000, 20000, 21000

local TRACK_COUNT = 8
local ROW_COUNT = 8
local PATTERN_COUNT = 8
local COLOUR_COUNT = 14

-- Each track has six CV outputs, A to F. A CV output emits a pattern, a
-- unipolar value or a bipolar value, chosen with a switch on the back panel;
-- unipolar and bipolar values are one of seven steps, and each has a
-- transition: off, A, B, C or D.
local OUTPUT_LETTERS = { "a", "b", "c", "d", "e", "f" }
local VALUE_COUNT = 7
local TRANSITION_COUNT = 5
-- Bipolar, unipolar, pattern: the switch's frames from the bottom up.
local CV_MODE_COUNT = 3
local CV_MODE_PATTERN = 2
local CV_MODE_UNIPOLAR = 1
-- A new clip's bipolar value is 0, in the middle.
local BIPOLAR_ZERO = 3
-- The settings of a CV output, in the order of their tag bases.
local OUTPUT_SETTINGS = { "pattern", "unipolar", "bipolar", "transition" }

-- Launch mode, clip picking, then clip editing, fill-in editing and landing
-- editing for each of the launch buttons
local SETUP_COUNT = 2 + 3 * TRACK_COUNT * ROW_COUNT

-- Unlit, then dim, vibrant and flashing for each colour
local LIGHT_COUNT = 1 + 3 * COLOUR_COUNT
-- Unlit, then dim and vibrant for each colour
local LIGHT_FRAME_COUNT = 1 + 2 * COLOUR_COUNT
-- Unlit, then dim and vibrant for red, green, yellow, white and blue
local SCENE_LIGHT_FRAME_COUNT = 11
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

local unipolar_texts = {}
local bipolar_texts = {}
for value = 1, VALUE_COUNT do
	table.insert(unipolar_texts, jbox.ui_text("text_unipolar" .. value))
	table.insert(bipolar_texts, jbox.ui_text("text_bipolar" .. value))
end

local transition_texts = {}
for transition = 1, TRANSITION_COUNT do
	table.insert(transition_texts, jbox.ui_text("text_transition" .. transition))
end

local cv_mode_texts = {
	jbox.ui_text("text_cv_mode_bipolar"),
	jbox.ui_text("text_cv_mode_unipolar"),
	jbox.ui_text("text_cv_mode_pattern"),
}

-- How each setting of a CV output is shown, and what a new clip has.
local OUTPUT_SETTING_TYPES = {
	pattern = { steps = PATTERN_COUNT, texts = pattern_texts },
	unipolar = { steps = VALUE_COUNT, texts = unipolar_texts, default = 0 },
	bipolar = { steps = VALUE_COUNT, texts = bipolar_texts, default = BIPOLAR_ZERO },
	transition = { steps = TRANSITION_COUNT, texts = transition_texts, default = 0 },
}

-- A track, clip, scene or CV output name: text the user types on the panel. Saved with
-- the song and in patches (see docs/adr/0003), not automatable and not a
-- Remote item. Empty stands for the default name, which the panel shows in
-- its place and which is never stored (see docs/adr/0007).
local function name_property(tag, text_key)
	return jbox.string{
		property_tag = tag,
		default = "",
		ui_name = jbox.ui_text(text_key),
	}
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
-- automatable; clips, CV modes, the setup state and the function are not.
local FIRST_SELECTION_MIDI_CC = 256
local FILL_IN_MIDI_CC = FIRST_SELECTION_MIDI_CC + TRACK_COUNT
local FIRST_MUTE_MIDI_CC = FILL_IN_MIDI_CC + 1
local FIRST_SOLO_MIDI_CC = FIRST_MUTE_MIDI_CC + TRACK_COUNT
local midi_cc_chart = {}

-- 0 = launch mode, 1 = clip picking, 2..65 = clip editing, 66..129 = fill-in
-- editing and 130..193 = landing editing for one launch button. Written by
-- the panel and by control surfaces, read by the realtime code (see
-- docs/adr/0002). Not saved: a song opens in launch mode.
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

-- What the fill-in button and the forward button show, for control surfaces,
-- encoded like the light of a launch button: dim white (1) or vibrant red (5)
-- for the fill-in button, unlit (0), vibrant red (5) or vibrant blue (29) for
-- the forward button. Computed by the realtime code.
rt_properties.fill_in_light = jbox.number{
	property_tag = 58,
	default = 0,
	steps = LIGHT_COUNT,
	ui_name = jbox.ui_text("text_fill_in_light"),
	ui_type = plain_number(LIGHT_COUNT - 1),
}
add_remote_item("fill_in_light", "Fill-In Light")

rt_properties.forward_light = jbox.number{
	property_tag = 59,
	default = 0,
	steps = LIGHT_COUNT,
	ui_name = jbox.ui_text("text_forward_light"),
	ui_type = plain_number(LIGHT_COUNT - 1),
}
add_remote_item("forward_light", "Forward Light")

-- The frames the front panel shows of the two buttons, and of their
-- captions: the fill-in button's film strip is that of the side buttons, the
-- forward button's that of the scene buttons; FILL-IN is 0 = white, 1 = red;
-- the forward button's labels 0 = unlit, 1 = SET FILL-IN in red, 2 = SET
-- LANDING in blue.
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

rt_properties.forward_light_frame = jbox.number{
	property_tag = 72,
	default = 0,
	steps = SCENE_LIGHT_FRAME_COUNT,
	ui_name = jbox.ui_text("text_forward_light"),
	ui_type = plain_number(SCENE_LIGHT_FRAME_COUNT - 1),
}

rt_properties.forward_label_frame = jbox.number{
	property_tag = 73,
	default = 0,
	steps = 3,
	ui_name = jbox.ui_text("text_forward_label"),
	ui_type = plain_number(2),
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
-- (clip editing), 3 = red (fill-in editing), 4 = blue (landing editing). The
-- labels of the launch buttons are lit only in clip editing, in yellow, in
-- fill-in editing, in red, and in landing editing, in blue: 0 = unlit, then
-- SET/CLEAR and all in yellow (1, 2), red (3, 4) and blue (5, 6); the colours
-- and patterns only while the picked launch button holds a clip, or in
-- fill-in editing a fill-in, or in landing editing a landing. A scene
-- button's label is 0 = unlit, 1 = white, 2 = green.
rt_properties.logo_frame = jbox.number{
	property_tag = 53,
	default = 0,
	steps = 5,
	ui_name = jbox.ui_text("text_logo"),
	ui_type = plain_number(4),
}

rt_properties.launch_labels_frame = jbox.number{
	property_tag = 54,
	default = 0,
	steps = 7,
	ui_name = jbox.ui_text("text_launch_labels"),
	ui_type = plain_number(6),
}

-- The back button's frame, computed by the realtime code: 0 = dark (launch
-- mode), 7 = vibrant white (clip picking), 3 = vibrant green (clip editing), 5
-- = vibrant yellow (fill-in editing), 1 = vibrant red (landing editing). Not a
-- Remote item: a codec can tell it from the setup state.
rt_properties.back_button_frame = jbox.number{
	property_tag = 55,
	default = 0,
	steps = 9,
	ui_name = jbox.ui_text("text_back_button"),
	ui_type = plain_number(8),
}

-- Its labels, one line for each step it goes back to, lit in the colour of
-- the button: 0 = all unlit, 1 = LAUNCH in white, 2 = PICK CLIP in green, 3 =
-- EDIT CLIP in yellow, 4 = SET FILL-IN in red.
rt_properties.back_label_frame = jbox.number{
	property_tag = 56,
	default = 0,
	steps = 5,
	ui_name = jbox.ui_text("text_back_label"),
	ui_type = plain_number(4),
}

-- The labels of the rows of the CV outputs, lit only in clip editing,
-- fill-in editing and landing editing of a clip, fill-in or landing: 0 =
-- unlit, then in yellow, in red and in blue each 1 = the patterns, 2..6 = the
-- unipolar values with transition off, A, B, C or D, 7..11 = the bipolar
-- values with transition off, A, B, C or D. Computed by the realtime code
-- from the CV mode of the picked track's output.
local OUTPUT_LABELS_FRAME_COUNT = 1 + 3 * (1 + 2 * TRANSITION_COUNT)
for output, letter in ipairs(OUTPUT_LETTERS) do
	rt_properties["output_" .. letter .. "_labels_frame"] = jbox.number{
		property_tag = 140 + output,
		default = 0,
		steps = OUTPUT_LABELS_FRAME_COUNT,
		ui_name = jbox.ui_text("text_output_" .. letter .. "_labels"),
		ui_type = plain_number(OUTPUT_LABELS_FRAME_COUNT - 1),
	}
end

-- The three sets of transition diagrams, lit like the labels of the CV
-- outputs: 0 = all grey, 1 = TRANSITION in yellow, 2 = FILL-IN in red, 3 =
-- LANDING in blue.
rt_properties.transition_diagrams_frame = jbox.number{
	property_tag = 150,
	default = 0,
	steps = 4,
	ui_name = jbox.ui_text("text_transition_diagrams"),
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

	-- The CV mode of each CV output, set with the switch beside it on the back
	-- panel. Saved with the song and in patches, but not automatable, and not
	-- changed from control surfaces: the host requires a Remote item to have a
	-- widget on the front panel, and the switch is on the back. A copy written
	-- by the realtime code is an output-only Remote item instead, so that a
	-- codec can tell what a press in clip editing means.
	for output, letter in ipairs(OUTPUT_LETTERS) do
		local cv_mode = "track" .. track .. "_cv_mode_" .. letter
		local default = output == 1 and CV_MODE_PATTERN or CV_MODE_UNIPOLAR
		document_properties[cv_mode] = jbox.number{
			property_tag = 200 + 10 * track + output,
			default = default,
			steps = CV_MODE_COUNT,
			ui_name = jbox.ui_text("text_" .. cv_mode),
			ui_type = jbox.ui_selector(cv_mode_texts),
		}
		local copy = "track" .. track .. "_output_" .. letter .. "_cv_mode"
		rt_properties[copy] = jbox.number{
			property_tag = 300 + 10 * track + output,
			default = default,
			steps = CV_MODE_COUNT,
			ui_name = jbox.ui_text("text_" .. copy),
			ui_type = jbox.ui_selector(cv_mode_texts),
		}
		add_remote_item(copy, "Track " .. track .. " Output " .. string.upper(letter) .. " CV Mode")

		local output_name = "track" .. track .. "_output_" .. letter .. "_name"
		document_properties[output_name] = name_property(500 + 10 * track + output, "text_" .. output_name)
	end

	local track_name = "track" .. track .. "_name"
	document_properties[track_name] = name_property(400 + track, "text_" .. track_name)

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

		-- What the launch button holds. Its settings, fill-in and landing are
		-- kept while it is empty, so that a clip comes back as it was. All of
		-- them make up a patch.
		document_properties[clip .. "_present"] = jbox.boolean{
			property_tag = 3000 + 10 * track + row,
			default = false,
			ui_name = jbox.ui_text("text_" .. clip .. "_present"),
			ui_type = plain_number(1),
		}
		add_remote_item(clip .. "_present", remote_name .. " Present")

		document_properties[clip .. "_colour"] = jbox.number{
			property_tag = 5000 + 10 * track + row,
			default = DEFAULT_COLOURS[track],
			steps = COLOUR_COUNT,
			ui_name = jbox.ui_text("text_" .. clip .. "_colour"),
			ui_type = jbox.ui_selector(colour_texts),
		}
		add_remote_item(clip .. "_colour", remote_name .. " Colour")

		-- The clip name belongs to the launch button and is kept while it is
		-- empty, like the clip's settings.
		document_properties[clip .. "_name"] = name_property(4000 + 10 * track + row, "text_" .. clip .. "_name")

		-- The clip's fill-in. A new one is a copy of its clip's values, with
		-- every transition off, in white: whoever sets one up writes all of
		-- them (see Press.h), so these defaults only matter for a fill-in set
		-- up some other way.
		document_properties[clip .. "_fill_in_present"] = jbox.boolean{
			property_tag = 6000 + 10 * track + row,
			default = false,
			ui_name = jbox.ui_text("text_" .. clip .. "_fill_in_present"),
			ui_type = plain_number(1),
		}
		add_remote_item(clip .. "_fill_in_present", remote_name .. " Fill-In Present")

		document_properties[clip .. "_fill_in_colour"] = jbox.number{
			property_tag = 8000 + 10 * track + row,
			default = 0,
			steps = COLOUR_COUNT,
			ui_name = jbox.ui_text("text_" .. clip .. "_fill_in_colour"),
			ui_type = jbox.ui_selector(colour_texts),
		}
		add_remote_item(clip .. "_fill_in_colour", remote_name .. " Fill-In Colour")

		-- While on, the fill-in's patterns play in the last bar before the
		-- switch point only.
		document_properties[clip .. "_fill_in_one_bar_pattern"] = jbox.boolean{
			property_tag = 23000 + 10 * track + row,
			default = false,
			ui_name = jbox.ui_text("text_" .. clip .. "_fill_in_one_bar_pattern"),
			ui_type = plain_number(1),
		}
		add_remote_item(clip .. "_fill_in_one_bar_pattern", remote_name .. " Fill-In One-Bar Pattern")

		-- The clip's landing, set up like its fill-in.
		document_properties[clip .. "_landing_present"] = jbox.boolean{
			property_tag = 7000 + 10 * track + row,
			default = false,
			ui_name = jbox.ui_text("text_" .. clip .. "_landing_present"),
			ui_type = plain_number(1),
		}
		add_remote_item(clip .. "_landing_present", remote_name .. " Landing Present")

		document_properties[clip .. "_landing_colour"] = jbox.number{
			property_tag = 9000 + 10 * track + row,
			default = 0,
			steps = COLOUR_COUNT,
			ui_name = jbox.ui_text("text_" .. clip .. "_landing_colour"),
			ui_type = jbox.ui_selector(colour_texts),
		}
		add_remote_item(clip .. "_landing_colour", remote_name .. " Landing Colour")

		-- While on, the landing's patterns play for its first bar only.
		document_properties[clip .. "_landing_one_bar_pattern"] = jbox.boolean{
			property_tag = 22000 + 10 * track + row,
			default = false,
			ui_name = jbox.ui_text("text_" .. clip .. "_landing_one_bar_pattern"),
			ui_type = plain_number(1),
		}
		add_remote_item(clip .. "_landing_one_bar_pattern", remote_name .. " Landing One-Bar Pattern")

		-- For each CV output, a pattern, a unipolar value, a bipolar value and
		-- a transition, of the clip, of its fill-in and of its landing (see
		-- docs/adr/0006). A new clip plays the pattern of its row, at unipolar
		-- and bipolar 0, with every transition off.
		local remote_kinds = { [""] = "", ["_fill_in"] = " Fill-In", ["_landing"] = " Landing" }
		for kind_index, kind in ipairs({ "", "_fill_in", "_landing" }) do
			for setting_index, setting in ipairs(OUTPUT_SETTINGS) do
				local setting_type = OUTPUT_SETTING_TYPES[setting]
				local base = 10000 + 1000 * (4 * (kind_index - 1) + setting_index - 1)
				for output, letter in ipairs(OUTPUT_LETTERS) do
					local property = clip .. kind .. "_" .. letter .. "_" .. setting
					document_properties[property] = jbox.number{
						property_tag = base + 100 * track + 10 * row + output,
						default = setting_type.default or row - 1,
						steps = setting_type.steps,
						ui_name = jbox.ui_text("text_" .. property),
						ui_type = jbox.ui_selector(setting_type.texts),
					}
					add_remote_item(property, remote_name .. remote_kinds[kind] .. " Output " .. string.upper(letter) .. " " .. setting:gsub("^%l", string.upper))
				end
			end
		end

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
	local scene_name = "scene" .. scene .. "_name"
	document_properties[scene_name] = name_property(410 + scene, "text_" .. scene_name)

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
	for _, letter in ipairs(OUTPUT_LETTERS) do
		cv_outputs["track" .. track .. "_" .. letter] = jbox.cv_output{
			ui_name = jbox.ui_text("text_cv_out_track" .. track .. "_" .. letter)
		}
	end
end
