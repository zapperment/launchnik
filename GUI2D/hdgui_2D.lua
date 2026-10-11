format_version = "2.0"

local TRACK_COUNT = 8
local ROW_COUNT = 8

local front_widgets = {
	jbox.device_name{
		graphics = { node = "DeviceName" },
	},
}

-- Lamps first, whatever takes the clicks afterwards: later widgets are on top.
table.insert(front_widgets, jbox.sequence_meter{
	graphics = { node = "lamp_setup" },
	value = "/custom_properties/setup_lamp",
})
for track = 1, TRACK_COUNT do
	table.insert(front_widgets, jbox.sequence_meter{
		graphics = { node = "lamp_track" .. track .. "_stop" },
		value = "/custom_properties/track" .. track .. "_stop_lamp",
	})
	for row = 1, ROW_COUNT do
		table.insert(front_widgets, jbox.sequence_meter{
			graphics = { node = "lamp_track" .. track .. "_clip" .. row },
			value = "/custom_properties/track" .. track .. "_clip" .. row .. "_light_frame",
		})
	end
end

for track = 1, TRACK_COUNT do
	table.insert(front_widgets, jbox.radio_button{
		graphics = { node = "click_track" .. track .. "_stop" },
		value = "/custom_properties/track" .. track .. "_selection",
		index = 0,
	})
end

-- Clicks on the launch buttons and the setup button mean different things in
-- launch mode and in setup mode, so they go to the click handler in Panel.cpp,
-- which tells the two displays apart by display_id. The handler reads and
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
	graphics = { node = "click_setup" },
	background = "transparent",
	display_width_pixels = 70,
	display_height_pixels = 75,
	display_id = 2,
	values = { "/custom_properties/setup" },
})

front = jbox.panel{
	graphics = { node = "Bg" },
	widgets = front_widgets,
}

local back_widgets = {
	jbox.device_name{
		graphics = { node = "DeviceName" },
	},
}

for track = 1, TRACK_COUNT do
	table.insert(back_widgets, jbox.cv_output_socket{
		graphics = { node = "cv_out_track" .. track },
		socket = "/cv_outputs/track" .. track,
	})
end

back = jbox.panel{
	graphics = { node = "Bg" },
	widgets = back_widgets,
}

folded_front = jbox.panel{
	graphics = { node = "Bg" },
	widgets = {
		jbox.device_name{
			graphics = { node = "DeviceName" },
		},
	},
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
