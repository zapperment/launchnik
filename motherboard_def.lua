format_version = "3.0"

-- Vocabulary follows CONTEXT.md. A selection is 0 for the stop button or 1..8
-- for the launch button of that pattern.
--
-- Property tags, as used by Launchnik.cpp:
--   selection of track t:            100 + t
--   state of button s on track t:   1000 + 10 * t + s
--   lamp of button s on track t:    2000 + 10 * t + s

local TRACK_COUNT = 8
local PATTERN_COUNT = 8

-- "stop" for selection 0, otherwise "pattern1" .. "pattern8"
local function button_name(selection)
	if selection == 0 then
		return "stop"
	end
	return "pattern" .. selection
end

local selection_texts = { jbox.ui_text("text_stop") }
for pattern = 1, PATTERN_COUNT do
	table.insert(selection_texts, jbox.ui_text("text_pattern" .. pattern))
end

local document_properties = {}
local rt_properties = {}
remote_implementation_chart = {}

-- Reason only offers automation for properties that have a MIDI CC number.
local FIRST_SELECTION_MIDI_CC = 12
local midi_cc_chart = {}

for track = 1, TRACK_COUNT do
	-- What the user last chose on the track. Saved with the song, automatable,
	-- and the only thing a control surface can write.
	local selection = "track" .. track .. "_selection"
	document_properties[selection] = jbox.number{
		property_tag = 100 + track,
		default = 0,
		steps = PATTERN_COUNT + 1,
		ui_name = jbox.ui_text("text_" .. selection),
		ui_type = jbox.ui_selector(selection_texts),
	}
	midi_cc_chart[FIRST_SELECTION_MIDI_CC + track - 1] = "/custom_properties/" .. selection
	remote_implementation_chart["/custom_properties/" .. selection] = {
		internal_name = "Track " .. track .. " Selection",
		short_ui_name = jbox.ui_text("text_" .. selection .. "_short"),
		shortest_ui_name = jbox.ui_text("text_" .. selection .. "_shortest"),
	}

	for button = 0, PATTERN_COUNT do
		local name = "track" .. track .. "_" .. button_name(button)
		local is_stop = button == 0

		-- 0 = off, 1 = queued, 2 = active. Computed by the realtime code and sent
		-- to control surfaces; see docs/adr/0001.
		local state = name .. "_state"
		rt_properties[state] = jbox.number{
			property_tag = 1000 + 10 * track + button,
			default = is_stop and 2 or 0,
			steps = 3,
			ui_name = jbox.ui_text("text_" .. name),
			ui_type = jbox.ui_selector{ jbox.ui_text("text_off"), jbox.ui_text("text_queued"), jbox.ui_text("text_active") },
		}
		remote_implementation_chart["/custom_properties/" .. state] = {
			internal_name = "Track " .. track .. (is_stop and " Stop" or (" Pattern " .. button)),
			short_ui_name = jbox.ui_text("text_" .. name .. "_short"),
			shortest_ui_name = jbox.ui_text("text_" .. name .. "_shortest"),
		}

		-- Whether the button's light is shining; flashes while queued.
		rt_properties[name .. "_lamp"] = jbox.boolean{
			property_tag = 2000 + 10 * track + button,
			default = is_stop,
			ui_name = jbox.ui_text("text_" .. name),
			ui_type = jbox.ui_linear({ min = 0, max = 1, units = { { decimals = 0 } } }),
		}
	end
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
