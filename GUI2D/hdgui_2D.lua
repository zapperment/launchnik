format_version = "2.0"

local TRACK_COUNT = 8
local ROW_COUNT = 8
local OUTPUT_LETTERS = { "a", "b", "c", "d", "e", "f" }

-- The header is the same on the front and the folded front: the tape, the
-- name of the patch, the buttons for browsing and saving patches, and the
-- logo, which shows the mode colour.
local function header_widgets()
	return {
		jbox.sequence_meter{
			graphics = { node = "Logo" },
			value = "/custom_properties/logo_frame",
		},
		jbox.device_name{
			graphics = { node = "DeviceName" },
		},
		jbox.patch_name{
			graphics = { node = "PatchName" },
			fg_color = { 0, 179, 179 },
			loader_alt_color = { 0, 90, 90 },
			center = false,
			text_style = "Bold LCD font",
		},
		jbox.patch_browse_group{
			graphics = { node = "PatchBrowseGroup" },
		},
	}
end

local front_widgets = header_widgets()

-- Lamps first, whatever takes the clicks afterwards: later widgets are on top.
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "lamp_setup" },
	value = "/custom_properties/setup_lamp",
})
for track = 1, TRACK_COUNT do
	table.insert(front_widgets, jbox.sequence_meter{
		graphics = { node = "lamp_track" .. track .. "_function" },
		value = "/custom_properties/track" .. track .. "_function_light_frame",
	})
	for row = 1, ROW_COUNT do
		table.insert(front_widgets, jbox.sequence_meter{
			graphics = { node = "lamp_track" .. track .. "_clip" .. row },
			value = "/custom_properties/track" .. track .. "_clip" .. row .. "_light_frame",
		})
	end
end

for scene = 1, ROW_COUNT do
	table.insert(front_widgets, jbox.sequence_meter{
		graphics = { node = "lamp_scene" .. scene },
		value = "/custom_properties/scene" .. scene .. "_light_frame",
	})
end

table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "lamp_back" },
	value = "/custom_properties/back_button_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "label_back" },
	value = "/custom_properties/back_label_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "lamp_set_fill_in" },
	value = "/custom_properties/set_fill_in_light_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "label_set_fill_in" },
	value = "/custom_properties/set_fill_in_label_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "lamp_fill_in" },
	value = "/custom_properties/fill_in_light_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "caption_fill_in" },
	value = "/custom_properties/fill_in_label_frame",
})

-- Stop, mute and solo, in the order of their display ids from 8 (see Panel.h).
local FUNCTIONS = { "stop", "mute", "solo" }
for _, name in ipairs(FUNCTIONS) do
	table.insert(front_widgets, jbox.sequence_meter{
		graphics = { node = "lamp_function_select_" .. name },
		value = "/custom_properties/function_select_" .. name .. "_light_frame",
	})
	table.insert(front_widgets, jbox.sequence_meter{
		graphics = { node = "caption_function_select_" .. name },
		value = "/custom_properties/function_select_" .. name .. "_caption_frame",
	})
end

table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "labels_launch" },
	value = "/custom_properties/launch_labels_frame",
})
for _, letter in ipairs(OUTPUT_LETTERS) do
	table.insert(front_widgets, jbox.sequence_meter{
		graphics = { node = "labels_output_" .. letter },
		value = "/custom_properties/output_" .. letter .. "_labels_frame",
	})
end
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "transition_diagram" },
	value = "/custom_properties/transition_diagram_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "label_transitions" },
	value = "/custom_properties/transition_labels_frame",
})
for scene = 1, ROW_COUNT do
	table.insert(front_widgets, jbox.sequence_meter{
		graphics = { node = "label_scene" .. scene },
		value = "/custom_properties/scene" .. scene .. "_label_frame",
	})
end

-- Clicks on the launch buttons, the scene buttons, the setup button, the back
-- button and the set fill-in button mean different things in launch mode and
-- in setup mode, and clicks on the function buttons different things in each
-- function, so they go to the click handler in Panel.cpp, which tells the
-- displays apart by display_id. So do the function select buttons, and the
-- fill-in button, which is held down while the mouse button is, as are the
-- function buttons in mute and solo function. The handler reads and
-- writes properties that are not listed here; the setup state is listed only
-- because a display needs at least one value.
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_grid" },
	background = "transparent",
	display_width_pixels = 1970,
	display_height_pixels = 1780,
	display_id = 1,
	values = { "/custom_properties/setup" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_scenes" },
	background = "transparent",
	display_width_pixels = 130,
	display_height_pixels = 1780,
	display_id = 3,
	values = { "/custom_properties/setup" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_setup" },
	background = "transparent",
	display_width_pixels = 70,
	display_height_pixels = 75,
	display_id = 2,
	values = { "/custom_properties/setup" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_back" },
	background = "transparent",
	display_width_pixels = 130,
	display_height_pixels = 135,
	display_id = 4,
	values = { "/custom_properties/setup" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_fill_in" },
	background = "transparent",
	display_width_pixels = 130,
	display_height_pixels = 135,
	display_id = 5,
	values = { "/custom_properties/fill_in" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_set_fill_in" },
	background = "transparent",
	display_width_pixels = 130,
	display_height_pixels = 135,
	display_id = 6,
	values = { "/custom_properties/setup" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_functions" },
	background = "transparent",
	display_width_pixels = 1835,
	display_height_pixels = 75,
	display_id = 7,
	values = { "/custom_properties/selected_function" },
})
for index, name in ipairs(FUNCTIONS) do
	table.insert(front_widgets, jbox.custom_display{
		graphics = { node = "click_function_select_" .. name },
		background = "transparent",
		display_width_pixels = 130,
		display_height_pixels = 135,
		display_id = 7 + index,
		values = { "/custom_properties/selected_function" },
	})
end

front = jbox.panel{
	graphics = { node = "Bg" },
	widgets = front_widgets,
}

local back_widgets = {
	jbox.device_name{
		graphics = { node = "DeviceName" },
	},
}

-- Each CV output with the switch for its CV mode, a vertical fader with
-- three positions that is dragged or clicked.
for track = 1, TRACK_COUNT do
	for _, letter in ipairs(OUTPUT_LETTERS) do
		table.insert(back_widgets, jbox.cv_output_socket{
			graphics = { node = "cv_out_track" .. track .. "_" .. letter },
			socket = "/cv_outputs/track" .. track .. "_" .. letter,
		})
		table.insert(back_widgets, jbox.sequence_fader{
			graphics = { node = "cv_mode_track" .. track .. "_" .. letter },
			handle_size = 0,
			value = "/custom_properties/track" .. track .. "_cv_mode_" .. letter,
		})
	end
end

back = jbox.panel{
	graphics = { node = "Bg" },
	widgets = back_widgets,
}

folded_front = jbox.panel{
	graphics = { node = "Bg" },
	widgets = header_widgets(),
}

folded_back = jbox.panel{
	graphics = { node = "Bg" },
	cable_origin = { node = "CableOrigin" },
	widgets = {
		jbox.device_name{
			graphics = { node = "DeviceName" },
		},
	},
}
