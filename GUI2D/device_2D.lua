format_version = "2.0"

local TRACK_COUNT = 8

-- Layout of the button grid, in pixels. Together with its labels the grid is
-- vertically centred in the main section below the header.
local LAUNCH_LEFT = 907
local LAUNCH_TOP = 342
local TRACK_PITCH = 252
local ROW_PITCH = 235
local STOP_LEFT = 974
local STOP_TOP = 2218

-- The header is the strip above the divider on the front panel. It is as high
-- as the folded panel, so the logo sits at the same place on both. It is a
-- lamp with one frame for each mode colour: light grey in launch mode, green
-- in clip picking, yellow in clip editing.
local LOGO_OFFSET = { 3385, 20 }

-- The tape, the patch name display and the patch browse group also sit at the
-- same place on both. The name itself is drawn by the host inside the area.
local TAPE_OFFSET = { 180, 42 }
local PATCH_DISPLAY_OFFSET = { 620, 30 }
local PATCH_NAME_OFFSET = { 635, 50 }
local PATCH_BROWSE_OFFSET = { 1345, 20 }

-- The upper screws sit below the divider, which ends at 160 pixels. The left
-- screws line up with the fold button that Reason draws in the header, and
-- the right ones are as far from their edge.
local PANEL_WIDTH = 3770
local SCREW_SIZE = 95
local SCREW_LEFT = 65
local SCREW_RIGHT = PANEL_WIDTH - SCREW_LEFT - SCREW_SIZE
local SCREW_TOP = 198
local SCREW_BOTTOM = 2272

front = {
	Bg = {
		{ path = "Panel_Front_7U_dark" },
	},
	Labels = {
		offset = { 0, 0 },
		{ path = "Labels_Front" },
	},
	ScrewTopLeft = { offset = { SCREW_LEFT, SCREW_TOP }, { path = "Screw_17_1frames" } },
	ScrewTopRight = { offset = { SCREW_RIGHT, SCREW_TOP }, { path = "Screw_17_1frames" } },
	ScrewBottomLeft = { offset = { SCREW_LEFT, SCREW_BOTTOM }, { path = "Screw_17_1frames" } },
	ScrewBottomRight = { offset = { SCREW_RIGHT, SCREW_BOTTOM }, { path = "Screw_17_1frames" } },
	Logo = {
		offset = LOGO_OFFSET,
		{ path = "Logo_3frames", frames = 3 },
	},
	DeviceName = {
		offset = TAPE_OFFSET,
		{ path = "TapeHorz" },
	},
	PatchDisplay = {
		offset = PATCH_DISPLAY_OFFSET,
		{ path = "Display_Patch_Name" },
	},
	PatchName = {
		offset = PATCH_NAME_OFFSET,
		{ path = "Patch_Name_Area" },
	},
	PatchBrowseGroup = {
		offset = PATCH_BROWSE_OFFSET,
		{ path = "PatchBrowseGroup" },
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

-- The scene buttons stand one track further to the right, each centred in the
-- width of a launch button and level with its scene. They are lamps as well,
-- showing one of nine frames (unlit, then dim and vibrant for red, green,
-- yellow and white), with one transparent display on top of all eight.
local SCENE_WIDTH = 130
local SCENE_LEFT = LAUNCH_LEFT + TRACK_PITCH * TRACK_COUNT + 35
for scene = 1, ROW_COUNT do
	front["lamp_scene" .. scene] = {
		offset = { SCENE_LEFT, LAUNCH_TOP + ROW_PITCH * (scene - 1) },
		{ path = "Scene_Button_9frames", frames = 9 },
	}
end
front.click_scenes = {
	offset = { SCENE_LEFT, LAUNCH_TOP },
	{ size = { SCENE_WIDTH, 1780 } },
}

-- The labels sit centred below their buttons, their tops 20 pixels below the
-- buttons. Those of the launch buttons are one lamp whose film strip starts
-- 25 pixels left of the grid: all unlit, SET/CLEAR and BACK in yellow, or all
-- in yellow (see tools/make-label-strips.sh). Each scene button has a lamp of its own, 250
-- pixels wide and unlit, white or green.
local LABEL_GAP = 20
local SCENE_LABEL_WIDTH = 250
front.labels_launch = {
	offset = { LAUNCH_LEFT - 25, LAUNCH_TOP + BUTTON_HEIGHT + LABEL_GAP },
	{ path = "Labels_Launch_3frames", frames = 3 },
}
for scene = 1, ROW_COUNT do
	front["label_scene" .. scene] = {
		offset = {
			SCENE_LEFT + (SCENE_WIDTH - SCENE_LABEL_WIDTH) / 2,
			LAUNCH_TOP + ROW_PITCH * (scene - 1) + BUTTON_HEIGHT + LABEL_GAP,
		},
		{ path = "Label_Scene" .. scene .. "_3frames", frames = 3 },
	}
end

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
		offset = { 240, 42 },
		{ path = "TapeHorz" },
	},
	PowerCord = {
		offset = { 3170, 2060 },
		{ path = "Powercord_11_1frames" },
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
		{ path = "Panel_Folded_Front_dark" },
	},
	Logo = {
		offset = LOGO_OFFSET,
		{ path = "Logo_3frames", frames = 3 },
	},
	DeviceName = {
		offset = TAPE_OFFSET,
		{ path = "TapeHorz" },
	},
	PatchDisplay = {
		offset = PATCH_DISPLAY_OFFSET,
		{ path = "Display_Patch_Name" },
	},
	PatchName = {
		offset = PATCH_NAME_OFFSET,
		{ path = "Patch_Name_Area" },
	},
	PatchBrowseGroup = {
		offset = PATCH_BROWSE_OFFSET,
		{ path = "PatchBrowseGroup" },
	},
}

folded_back = {
	Bg = {
		{ path = "Panel_Folded_Back_dark" },
	},
	DeviceName = {
		offset = { 240, 42 },
		{ path = "TapeHorz" },
	},
	CableOrigin = {
		offset = { 1885, 75 },
	},
}
