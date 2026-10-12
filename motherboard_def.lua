format_version = "3.0"

-- Vocabulary follows CONTEXT.md. A selection is 0 for the stop button or 1..8
-- for the launch button in that row.
--
-- Property tags, as used by Tags.h, for track t and row r, both from 1:
--   setup state:                      50
--   lamp of the setup button:         51
--   selection of track t:            100 + t
--   light of the stop button:       1000 + 10 * t
--   light of a launch button:       1000 + 10 * t + r
--   light of the scene button of row r:      1090 + r
--   film strip frame of that light:          2090 + r
--   film strip frame of that light: 2000 + 10 * t
--   film strip frame of a light:    2000 + 10 * t + r
--   whether a launch button holds a clip: 3000 + 10 * t + r
--   pattern of its clip:            4000 + 10 * t + r
--   colour of its clip:             5000 + 10 * t + r

local TRACK_COUNT = 8
local ROW_COUNT = 8
local PATTERN_COUNT = 8
local COLOUR_COUNT = 14

-- Launch mode, clip picking, and clip editing for each of the launch buttons
local SETUP_COUNT = 2 + TRACK_COUNT * ROW_COUNT

-- Unlit, then dim, vibrant and flashing for each colour
local LIGHT_COUNT = 1 + 3 * COLOUR_COUNT
-- Unlit, then dim and vibrant for each colour
local LIGHT_FRAME_COUNT = 1 + 2 * COLOUR_COUNT
-- Unlit, then dim and vibrant for red, green and yellow
local SCENE_LIGHT_FRAME_COUNT = 7

local selection_texts = { jbox.ui_text("text_stop") }
for row = 1, ROW_COUNT do
	table.insert(selection_texts, jbox.ui_text("text_clip" .. row))
end

local pattern_texts = {}
for pattern = 1, PATTERN_COUNT do
	table.insert(pattern_texts, jbox.ui_text("text_pattern" .. pattern))
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
-- Only the selections are automatable; clips and the setup state are not.
local FIRST_SELECTION_MIDI_CC = 256
local midi_cc_chart = {}

-- 0 = launch mode, 1 = clip picking, 2..65 = clip editing for one launch
-- button. Written by the panel and by control surfaces, read by the realtime
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

-- Whether the setup button is lit and the logo is green.
rt_properties.setup_lamp = jbox.boolean{
	property_tag = 51,
	default = false,
	ui_name = jbox.ui_text("text_setup"),
	ui_type = plain_number(1),
}

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

	-- What the stop button shows, for control surfaces: 0 = unlit (the track
	-- has no clips), 1 = dim (stopped), 2 = vibrant (a clip is active), 3 =
	-- flashing (about to stop). Computed by the realtime code.
	local stop = "track" .. track .. "_stop"
	rt_properties[stop .. "_light"] = jbox.number{
		property_tag = 1000 + 10 * track,
		default = 0,
		steps = 4,
		ui_name = jbox.ui_text("text_" .. stop),
		ui_type = jbox.ui_selector{ jbox.ui_text("text_unlit"), jbox.ui_text("text_dim"), jbox.ui_text("text_vibrant"), jbox.ui_text("text_flashing") },
	}
	add_remote_item(stop .. "_light", "Track " .. track .. " Stop Light")

	-- The frame of the stop buttons' film strip the front panel shows right
	-- now: unlit, dim or vibrant.
	rt_properties[stop .. "_light_frame"] = jbox.number{
		property_tag = 2000 + 10 * track,
		default = 0,
		steps = 3,
		ui_name = jbox.ui_text("text_" .. stop),
		ui_type = plain_number(2),
	}

	for row = 1, ROW_COUNT do
		local clip = "track" .. track .. "_clip" .. row
		local remote_name = "Track " .. track .. " Clip " .. row

		-- What the launch button holds. Pattern and colour are kept while it is
		-- empty, so that a clip comes back as it was. These three make up a patch.
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
