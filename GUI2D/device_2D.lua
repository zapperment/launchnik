format_version = "2.0"

local TRACK_COUNT = 8
local PATTERN_COUNT = 8

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
		offset = { 2876, 109 },
		{ path = "Logo" },
	},
	DeviceName = {
		offset = { 180, 160 },
		{ path = "TapeHorz" },
	},
}

-- Every button is a lamp with an invisible click area of the same size on top.
for track = 1, TRACK_COUNT do
	local stop_offset = { STOP_LEFT + TRACK_PITCH * (track - 1), STOP_TOP }
	front["lamp_track" .. track .. "_stop"] = {
		offset = stop_offset,
		{ path = "Button_21_2frames", frames = 2 },
	}
	front["click_track" .. track .. "_stop"] = {
		offset = stop_offset,
		{ path = "ClickArea_Stop_2frames", frames = 2 },
	}
	for pattern = 1, PATTERN_COUNT do
		local launch_offset = { LAUNCH_LEFT + TRACK_PITCH * (track - 1), LAUNCH_TOP + ROW_PITCH * (pattern - 1) }
		front["lamp_track" .. track .. "_pattern" .. pattern] = {
			offset = launch_offset,
			{ path = "Button_50_2frames", frames = 2 },
		}
		front["click_track" .. track .. "_pattern" .. pattern] = {
			offset = launch_offset,
			{ path = "ClickArea_Launch_2frames", frames = 2 },
		}
	end
end

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
