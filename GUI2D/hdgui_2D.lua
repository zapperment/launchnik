format_version = "2.0"

local TRACK_COUNT = 8
local ROW_COUNT = 8
local OUTPUT_LETTERS = { "a", "b", "c", "d", "e", "f" }

-- The header is the same on the front and the folded front: the tape, the
-- name of the patch, the buttons for browsing and saving patches, and the
-- logo, which shows the mode colour. A transparent display on top of the logo
-- takes a double-click on it to Panel.cpp, which underscores the default
-- names; each panel has a display_id of its own for it.
local function header_widgets(logo_display_id)
	return {
		jbox.sequence_meter{
			graphics = { node = "Logo" },
			value = "/custom_properties/logo_frame",
		},
		jbox.custom_display{
			graphics = { node = "click_logo" },
			background = "transparent",
			display_width_pixels = 285,
			display_height_pixels = 110,
			display_id = logo_display_id,
			values = { "/custom_properties/setup" },
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

local front_widgets = header_widgets(13)

-- Lamps first, whatever takes the clicks afterwards: later widgets are on top.
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "lamp_setup" },
	value = "/custom_properties/setup_light_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "caption_setup" },
	value = "/custom_properties/setup_caption_frame",
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
	graphics = { node = "lamp_forward" },
	value = "/custom_properties/forward_light_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "label_forward" },
	value = "/custom_properties/forward_label_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "lamp_fill_in" },
	value = "/custom_properties/fill_in_light_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "caption_fill_in" },
	value = "/custom_properties/fill_in_label_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "lamp_clear" },
	value = "/custom_properties/clear_light_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "caption_clear" },
	value = "/custom_properties/clear_caption_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "lamp_duplicate" },
	value = "/custom_properties/duplicate_light_frame",
})
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "caption_duplicate" },
	value = "/custom_properties/duplicate_caption_frame",
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
	graphics = { node = "transition_diagrams" },
	value = "/custom_properties/transition_diagrams_frame",
})
for scene = 1, ROW_COUNT do
	table.insert(front_widgets, jbox.sequence_meter{
		graphics = { node = "label_scene" .. scene },
		value = "/custom_properties/scene" .. scene .. "_label_frame",
	})
end

-- Clicks on the launch buttons, the scene buttons, the setup button, the
-- clear button, the duplicate button, the back button and the forward button
-- mean different things in launch mode, in setup mode, in clear mode and in
-- duplicate mode, and clicks on the function buttons different things in each
-- function, so they go to the click handler in Panel.cpp, which tells the
-- displays apart by display_id. So do the function select buttons, and the
-- fill-in button, which is held down while the mouse button is, as are the
-- function buttons in mute and solo function. The handler reads and
-- writes properties that are not listed here; the setup state is listed only
-- because a display needs at least one value.
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_grid" },
	background = "transparent",
	display_width_pixels = 2190,
	display_height_pixels = 1885,
	display_id = 1,
	values = { "/custom_properties/setup" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_scenes" },
	background = "transparent",
	display_width_pixels = 130,
	display_height_pixels = 1885,
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
	graphics = { node = "click_clear" },
	background = "transparent",
	display_width_pixels = 130,
	display_height_pixels = 135,
	display_id = 11,
	values = { "/custom_properties/setup" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_duplicate" },
	background = "transparent",
	display_width_pixels = 130,
	display_height_pixels = 135,
	display_id = 12,
	values = { "/custom_properties/setup" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_forward" },
	background = "transparent",
	display_width_pixels = 130,
	display_height_pixels = 135,
	display_id = 6,
	values = { "/custom_properties/setup" },
})
table.insert(front_widgets, jbox.custom_display{
	graphics = { node = "click_functions" },
	background = "transparent",
	display_width_pixels = 2055,
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

-- The names (see docs/adr/0007): each is two text widgets on top of each
-- other, bound to the same property, that open the host's text field on a
-- double click. One shows the name while it is not empty, the other the
-- default name while it is. They come after the click displays, so that
-- clicks on a name reach the name. Track names are larger than clip and scene
-- names, all in the light grey of the labels; the functions that show or hide
-- them are in gui_functions.lua.
local NAME_COLOUR = { 208, 208, 208 }
local TRACK_NAME_STYLE = "Arial medium small font"
local NAME_STYLE = "Arial small font"

-- values are the properties the visibility functions read; shown and
-- default_shown name those functions.
local function add_name_widgets(widgets, name, text_style, default_key, values, shown, default_shown)
	local property = "/custom_properties/" .. name
	table.insert(widgets, jbox.value_display{
		graphics = { node = name .. "_value" },
		value = property,
		text_style = text_style,
		text_color = NAME_COLOUR,
		show_remote_box = false,
		show_automation_rect = false,
		visibility = { values = values, func = shown },
	})
	-- A template without ^0 is shown as it is.
	table.insert(widgets, jbox.value_display{
		graphics = { node = name .. "_default" },
		value = property,
		text_style = text_style,
		text_color = NAME_COLOUR,
		value_templates = { jbox.ui_text(default_key) },
		show_remote_box = false,
		show_automation_rect = false,
		visibility = { values = values, func = default_shown },
	})
end

-- On the front panel, every name depends on the setup state as well: track
-- names and clip names have functions of their own, since in clip editing,
-- fill-in editing and landing editing only those of the launch button being
-- edited are shown.
local function add_front_name_widgets(name, text_style, default_key, shown, default_shown)
	add_name_widgets(front_widgets, name, text_style, default_key,
		{ "/custom_properties/setup", "/custom_properties/" .. name }, shown, default_shown)
end

for track = 1, TRACK_COUNT do
	local track_name = "track" .. track .. "_name"
	add_front_name_widgets(track_name, TRACK_NAME_STYLE, "text_default_track_name" .. track,
		"is_" .. track_name .. "_shown", "is_" .. track_name .. "_default_shown")
	for row = 1, ROW_COUNT do
		local clip_name = "track" .. track .. "_clip" .. row .. "_name"
		add_front_name_widgets(clip_name, NAME_STYLE, "text_default_clip_name" .. row,
			"is_" .. clip_name .. "_shown", "is_" .. clip_name .. "_default_shown")
	end
end
for scene = 1, ROW_COUNT do
	add_front_name_widgets("scene" .. scene .. "_name", NAME_STYLE, "text_default_scene_name" .. scene,
		"is_name_shown_in_launch_mode", "is_default_name_shown_in_launch_mode")
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

-- The track names and the CV output names on the back panel are always shown.
local function add_back_name_widgets(name, text_style, default_key)
	add_name_widgets(back_widgets, name, text_style, default_key,
		{ "/custom_properties/" .. name }, "is_name_shown", "is_default_name_shown")
end
for track = 1, TRACK_COUNT do
	add_back_name_widgets("track" .. track .. "_name", TRACK_NAME_STYLE, "text_default_track_name" .. track)
	for _, letter in ipairs(OUTPUT_LETTERS) do
		add_back_name_widgets("track" .. track .. "_output_" .. letter .. "_name", NAME_STYLE,
			"text_default_output_name" .. track .. "_" .. letter)
	end
end

back = jbox.panel{
	graphics = { node = "Bg" },
	widgets = back_widgets,
}

folded_front = jbox.panel{
	graphics = { node = "Bg" },
	widgets = header_widgets(14),
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
