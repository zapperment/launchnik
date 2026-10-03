format_version = "2.0"

local TRACK_COUNT = 8
local PATTERN_COUNT = 8

local front_widgets = {
	jbox.device_name{
		graphics = { node = "DeviceName" },
	},
}

-- Lamps first, click areas afterwards: later widgets are on top.
for track = 1, TRACK_COUNT do
	table.insert(front_widgets, jbox.sequence_meter{
		graphics = { node = "lamp_track" .. track .. "_stop" },
		value = "/custom_properties/track" .. track .. "_stop_lamp",
	})
	for pattern = 1, PATTERN_COUNT do
		table.insert(front_widgets, jbox.sequence_meter{
			graphics = { node = "lamp_track" .. track .. "_pattern" .. pattern },
			value = "/custom_properties/track" .. track .. "_pattern" .. pattern .. "_lamp",
		})
	end
end

for track = 1, TRACK_COUNT do
	local selection = "/custom_properties/track" .. track .. "_selection"
	table.insert(front_widgets, jbox.radio_button{
		graphics = { node = "click_track" .. track .. "_stop" },
		value = selection,
		index = 0,
	})
	for pattern = 1, PATTERN_COUNT do
		table.insert(front_widgets, jbox.radio_button{
			graphics = { node = "click_track" .. track .. "_pattern" .. pattern },
			value = selection,
			index = pattern,
		})
	end
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
