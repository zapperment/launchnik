format_version = "2.0"

local TRACK_COUNT = 8

-- Layout of the button grid, in pixels. Everything below the header sits 50
-- pixels higher than the grid alone would, centred, to make room for the row
-- of function select buttons at the bottom.
local LAUNCH_LEFT = 907
local LAUNCH_TOP = 462
local TRACK_PITCH = 252
local ROW_PITCH = 235
local FUNCTION_LEFT = 974
local FUNCTION_TOP = 2338
local FUNCTION_WIDTH = 70
local FUNCTION_HEIGHT = 75

-- The header is the strip above the divider on the front panel. It is as high
-- as the folded panel, so the logo sits at the same place on both. It is a
-- lamp with one frame for each mode colour: light grey in launch mode, green
-- in clip picking, yellow in clip editing, red in fill-in editing.
local LOGO_OFFSET = { 3385, 20 }

-- The tape, the patch name display and the patch browse group also sit at the
-- same place on both. The name itself is drawn by the host inside the area.
local TAPE_OFFSET = { 180, 42 }
local PATCH_DISPLAY_OFFSET = { 620, 30 }
local PATCH_NAME_OFFSET = { 635, 50 }
local PATCH_BROWSE_OFFSET = { 1345, 20 }

-- The upper screws sit below the divider, which ends at 160 pixels. The left
-- screws are 40 pixels from the edge, and the right ones as far from theirs.
-- Each shows a different frame of the screw strip (see tools/make-screws.sh).
local PANEL_WIDTH = 3770
local SCREW_SIZE = 95
local SCREW_LEFT = 40
local SCREW_RIGHT = PANEL_WIDTH - SCREW_LEFT - SCREW_SIZE
local SCREW_TOP = 198
local SCREW_BOTTOM = 2617

front = {
	Bg = {
		{ path = "Panel_Front_8U_dark" },
	},
	Labels = {
		offset = { 0, 0 },
		{ path = "Labels_Front" },
	},
	ScrewTopLeft = { offset = { SCREW_LEFT, SCREW_TOP }, { path = "Screw_17_1" } },
	ScrewTopRight = { offset = { SCREW_RIGHT, SCREW_TOP }, { path = "Screw_17_2" } },
	ScrewBottomLeft = { offset = { SCREW_LEFT, SCREW_BOTTOM }, { path = "Screw_17_3" } },
	ScrewBottomRight = { offset = { SCREW_RIGHT, SCREW_BOTTOM }, { path = "Screw_17_4" } },
	Logo = {
		offset = LOGO_OFFSET,
		{ path = "Logo_4frames", frames = 4 },
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
local SETUP_LEFT = 720

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
-- 25 pixels left of the grid: all unlit, SET/CLEAR or all in yellow, or
-- SET/CLEAR or all in red (see tools/make-label-strips.sh). Each scene button has a lamp of its own, 250
-- pixels wide and unlit, white or green.
local LABEL_GAP = 20
local SCENE_LABEL_WIDTH = 250
front.labels_launch = {
	offset = { LAUNCH_LEFT - 25, LAUNCH_TOP + BUTTON_HEIGHT + LABEL_GAP },
	{ path = "Labels_Launch_5frames", frames = 5 },
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

-- A function button is a lamp showing one of seven frames (unlit, then dim and
-- vibrant for red, yellow and blue). One transparent display lies on top of
-- all eight; its width is rounded up to a multiple of 5 pixels.
for track = 1, TRACK_COUNT do
	front["lamp_track" .. track .. "_function"] = {
		offset = { FUNCTION_LEFT + TRACK_PITCH * (track - 1), FUNCTION_TOP },
		{ path = "Function_Button_7frames", frames = 7 },
	}
end
front.click_functions = {
	offset = { FUNCTION_LEFT, FUNCTION_TOP },
	{ size = { 1835, FUNCTION_HEIGHT } },
}

-- The back button stands above the first track, centred in the width of a
-- launch button and as far above it as the launch buttons are apart, all as
-- drawn: 122 pixels. It is a lamp showing one of nine frames, with a
-- transparent display of the same size on top.
local BACK_LEFT = LAUNCH_LEFT + 38
local BACK_TOP = LAUNCH_TOP - 232
front.lamp_back = {
	offset = { BACK_LEFT, BACK_TOP },
	{ path = "Back_Button_9frames", frames = 9 },
}
front.click_back = {
	offset = { BACK_LEFT, BACK_TOP },
	{ size = { 130, 135 } },
}

-- Its label sits below it like those of the scene buttons: centred, its top
-- 20 pixels below the button, unlit, white, green or yellow.
front.label_back = {
	offset = { BACK_LEFT + (130 - SCENE_LABEL_WIDTH) / 2, BACK_TOP + 135 + LABEL_GAP },
	{ path = "Label_Back_4frames", frames = 4 },
}

-- The set fill-in button stands beside it, above the second track, and looks
-- like a scene button; its label is unlit or red.
local SET_FILL_IN_LEFT = BACK_LEFT + TRACK_PITCH
front.lamp_set_fill_in = {
	offset = { SET_FILL_IN_LEFT, BACK_TOP },
	{ path = "Scene_Button_9frames", frames = 9 },
}
front.click_set_fill_in = {
	offset = { SET_FILL_IN_LEFT, BACK_TOP },
	{ size = { 130, 135 } },
}
front.label_set_fill_in = {
	offset = { SET_FILL_IN_LEFT + (130 - SCENE_LABEL_WIDTH) / 2, BACK_TOP + 135 + LABEL_GAP },
	{ path = "Label_Set_Fill_In_2frames", frames = 2 },
}

-- The fill-in button stands to the left of scene 6, as far from the grid as
-- the scene buttons are on the right. It is a side button: like a scene button
-- but without an arrow, with one of eleven frames (unlit, then dim and vibrant
-- for red, green, yellow, blue and white). Its caption is white or red,
-- centred below it like a label.
local FILL_IN_LEFT = LAUNCH_LEFT - (SCENE_LEFT - (LAUNCH_LEFT + TRACK_PITCH * (TRACK_COUNT - 1) + BUTTON_WIDTH)) - SCENE_WIDTH
local FILL_IN_TOP = LAUNCH_TOP + ROW_PITCH * 5
local CAPTION_WIDTH = 250
front.lamp_fill_in = {
	offset = { FILL_IN_LEFT, FILL_IN_TOP },
	{ path = "Side_Button_11frames", frames = 11 },
}
front.click_fill_in = {
	offset = { FILL_IN_LEFT, FILL_IN_TOP },
	{ size = { 130, 135 } },
}
front.caption_fill_in = {
	offset = { FILL_IN_LEFT + (SCENE_WIDTH - CAPTION_WIDTH) / 2, FILL_IN_TOP + BUTTON_HEIGHT + LABEL_GAP },
	{ path = "Caption_Fill_In_2frames", frames = 2 },
}

-- The function select buttons are side buttons in a row below the function
-- buttons: MUTE below track 2, SOLO below track 3, STOP below track 8, each
-- centred in the width of a launch button like the back button. Each has a
-- transparent display on top and its caption centred below it like a label,
-- white or in the colour of its function.
local FUNCTION_SELECT_TOP = FUNCTION_TOP + FUNCTION_HEIGHT + 35
local FUNCTION_SELECT_TRACKS = { stop = 8, mute = 2, solo = 3 }
local FUNCTION_SELECT_CAPTIONS = { stop = "Caption_Stop_2frames", mute = "Caption_Mute_2frames", solo = "Caption_Solo_2frames" }
for name, track in pairs(FUNCTION_SELECT_TRACKS) do
	local left = LAUNCH_LEFT + TRACK_PITCH * (track - 1) + 38
	front["lamp_function_select_" .. name] = {
		offset = { left, FUNCTION_SELECT_TOP },
		{ path = "Side_Button_11frames", frames = 11 },
	}
	front["click_function_select_" .. name] = {
		offset = { left, FUNCTION_SELECT_TOP },
		{ size = { SCENE_WIDTH, BUTTON_HEIGHT } },
	}
	front["caption_function_select_" .. name] = {
		offset = { left + (SCENE_WIDTH - CAPTION_WIDTH) / 2, FUNCTION_SELECT_TOP + BUTTON_HEIGHT + LABEL_GAP },
		{ path = FUNCTION_SELECT_CAPTIONS[name], frames = 2 },
	}
end

-- The setup button is a lamp with a transparent display on top, like the grid.
front.lamp_setup = {
	offset = { SETUP_LEFT, FUNCTION_TOP },
	{ path = "Button_21_green_2frames", frames = 2 },
}
front.click_setup = {
	offset = { SETUP_LEFT, FUNCTION_TOP },
	{ size = { 70, 75 } },
}

back = {
	Bg = {
		{ path = "Panel_Back_8U" },
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
		offset = { 3170, 2405 },
		{ path = "Powercord_11_1frames" },
	},
}

-- Together with their labels the CV outputs are vertically centred on the
-- back panel.
for track = 1, TRACK_COUNT do
	back["cv_out_track" .. track] = {
		offset = { 1100 + 220 * (track - 1), 1440 },
		{ path = "SharedCVJack", frames = 3 },
	}
end

folded_front = {
	Bg = {
		{ path = "Panel_Folded_Front_dark" },
	},
	Logo = {
		offset = LOGO_OFFSET,
		{ path = "Logo_4frames", frames = 4 },
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
