format_version = "2.0"

local TRACK_COUNT = 8

-- Layout of the button grid, in pixels
local LAUNCH_LEFT = 907
local LAUNCH_TOP = 237
local TRACK_PITCH = 252
local ROW_PITCH = 235
local STOP_LEFT = 974
local STOP_TOP = 2113

front = {
	Bg = {
		{ path = "Panel_Front_7U_dark" },
	},
	Labels = {
		offset = { 0, 0 },
		{ path = "Labels_Front" },
	},
	ScrewTopLeft = { offset = { 38, 38 }, { path = "Screw_17_1frames" } },
	ScrewTopRight = { offset = { 3637, 38 }, { path = "Screw_17_1frames" } },
	ScrewBottomLeft = { offset = { 38, 2282 }, { path = "Screw_17_1frames" } },
	ScrewBottomRight = { offset = { 3637, 2282 }, { path = "Screw_17_1frames" } },
	Logo = {
		offset = { 2874, 107 },
		{ path = "Logo" },
	},
	DeviceName = {
		offset = { 180, 160 },
		{ path = "TapeHorz" },
	},
}

local ROW_COUNT = 8
local BUTTON_WIDTH = 205
local BUTTON_HEIGHT = 135
local SETUP_LEFT = 580

-- The launch buttons are lamps showing one frame of a film strip: unlit, then
-- dim and vibrant for each of the fourteen colours. One transparent display
-- lies on top of all of them and takes the clicks; its size is rounded up to
-- a multiple of 5 pixels.
for track = 1, TRACK_COUNT do
	for row = 1, ROW_COUNT do
		front["lamp_track" .. track .. "_clip" .. row] = {
			offset = { LAUNCH_LEFT + TRACK_PITCH * (track - 1), LAUNCH_TOP + ROW_PITCH * (row - 1) },
			{ path = "Launch_Button_29frames", frames = 29 },
		}
	end
end
front.click_grid = {
	offset = { LAUNCH_LEFT, LAUNCH_TOP },
	{ size = { 1970, 1780 } },
}

-- A stop button is a lamp showing one of three frames (unlit, dim, vibrant)
-- with an invisible click area of the same size on top.
for track = 1, TRACK_COUNT do
	local stop_offset = { STOP_LEFT + TRACK_PITCH * (track - 1), STOP_TOP }
	front["lamp_track" .. track .. "_stop"] = {
		offset = stop_offset,
		{ path = "Stop_Button_3frames", frames = 3 },
	}
	front["click_track" .. track .. "_stop"] = {
		offset = stop_offset,
		{ path = "ClickArea_Stop_2frames", frames = 2 },
	}
end

-- The setup button is a lamp with a transparent display on top, like the grid.
front.lamp_setup = {
	offset = { SETUP_LEFT, STOP_TOP },
	{ path = "Button_21_green_2frames", frames = 2 },
}
front.click_setup = {
	offset = { SETUP_LEFT, STOP_TOP },
	{ size = { 70, 75 } },
}

back = {
	Bg = {
		{ path = "Panel_Back_7U" },
	},
	Labels = {
		offset = { 0, 0 },
		{ path = "Labels_Back" },
	},
	DeviceName = {
		offset = { 200, 150 },
		{ path = "TapeHorz" },
	},
}

for track = 1, TRACK_COUNT do
	back["cv_out_track" .. track] = {
		offset = { 1100 + 220 * (track - 1), 1160 },
		{ path = "SharedCVJack", frames = 3 },
	}
end

folded_front = {
	Bg = {
		{ path = "Panel_Folded_Front" },
	},
	DeviceName = {
		offset = { 400, 42 },
		{ path = "TapeHorz" },
	},
}

folded_back = {
	Bg = {
		{ path = "Panel_Folded_Back" },
	},
	DeviceName = {
		offset = { 400, 42 },
		{ path = "TapeHorz" },
	},
	CableOrigin = {
		offset = { 1885, 75 },
	},
}
