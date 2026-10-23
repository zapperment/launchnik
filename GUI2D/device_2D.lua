format_version = "2.0"

local TRACK_COUNT = 8

-- Layout of the button grid, in pixels. Each scene has a row of clip names
-- above its launch buttons and a row of labels below them, and the first one
-- the track names above as well; the back button and the forward button sit
-- right below the header, the function select buttons near the bottom.
-- The grid is centred on the panel: the fill-in button on the left and the
-- scene buttons on the right are 100 pixels from it, and 562 pixels from the
-- edges of the panel (see SIDE_GAP below).
local LAUNCH_LEFT = 792
local LAUNCH_TOP = 460
local TRACK_PITCH = 283
local ROW_PITCH = 250
local FUNCTION_LEFT = LAUNCH_LEFT + 67
local FUNCTION_TOP = 2440
local FUNCTION_WIDTH = 70
local FUNCTION_HEIGHT = 75

-- The header is the strip above the divider on the front panel. It is as high
-- as the folded panel, so the logo sits at the same place on both. It is a
-- lamp with one frame for each mode colour: light grey in launch mode, green
-- in clip picking, yellow in clip editing, red in fill-in editing, blue in
-- landing editing.
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
		{ path = "Logo_5frames", frames = 5 },
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
local GRID_WIDTH = TRACK_PITCH * (TRACK_COUNT - 1) + BUTTON_WIDTH + 4
local GRID_HEIGHT = ROW_PITCH * (ROW_COUNT - 1) + BUTTON_HEIGHT
front.click_grid = {
	offset = { LAUNCH_LEFT, LAUNCH_TOP },
	{ size = { GRID_WIDTH, GRID_HEIGHT } },
}

-- The scene buttons stand 100 pixels to the right of the grid, each level with
-- its scene. They are lamps as well, showing one of eleven frames (unlit,
-- then dim and vibrant for red, green, yellow, white and blue), with one
-- transparent display on top of all eight.
local SCENE_WIDTH = 130
local SIDE_GAP = 100
local SCENE_LEFT = LAUNCH_LEFT + TRACK_PITCH * (TRACK_COUNT - 1) + BUTTON_WIDTH + SIDE_GAP
for scene = 1, ROW_COUNT do
	front["lamp_scene" .. scene] = {
		offset = { SCENE_LEFT, LAUNCH_TOP + ROW_PITCH * (scene - 1) },
		{ path = "Scene_Button_11frames", frames = 11 },
	}
end
front.click_scenes = {
	offset = { SCENE_LEFT, LAUNCH_TOP },
	{ size = { SCENE_WIDTH, GRID_HEIGHT } },
}

-- The labels sit centred below their buttons, their tops 8 pixels above the
-- bottom of the buttons' images, right below the buttons themselves. Those of
-- the launch buttons in scenes 1 and 2 are one lamp whose film strip starts
-- 25 pixels left of the grid: all unlit, or SET/CLEAR or all in yellow, red
-- or blue (see tools/make-label-strips.sh). Each of scenes 3
-- to 8, which stand for the CV outputs A to F, has a lamp of its own, with
-- two lines below every launch button (see tools/make-cv-labels.py). Each
-- scene button has a lamp of its own, 250 pixels wide and unlit, white or
-- green.
local LABEL_GAP = -8
local SCENE_LABEL_WIDTH = 250
front.labels_launch = {
	offset = { LAUNCH_LEFT - 25, LAUNCH_TOP + BUTTON_HEIGHT + LABEL_GAP },
	{ path = "Labels_Launch_7frames", frames = 7 },
}
local OUTPUT_LETTERS = { "a", "b", "c", "d", "e", "f" }
for output, letter in ipairs(OUTPUT_LETTERS) do
	front["labels_output_" .. letter] = {
		offset = { LAUNCH_LEFT - 25, LAUNCH_TOP + ROW_PITCH * (output + 1) + BUTTON_HEIGHT + LABEL_GAP },
		{ path = "Labels_Output_34frames", frames = 34 },
	}
end

-- The three sets of transition diagrams, TRANSITION, FILL-IN and LANDING,
-- stand one above the other to the right of the scene buttons, centred on the
-- grid. One lamp shows them all grey or one set lit (see
-- tools/make-cv-labels.py).
front.transition_diagrams = {
	offset = { 3327, 900 },
	{ path = "Transition_Diagrams_4frames", frames = 4 },
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

-- The names the user types (see "Track name", "Clip name" and "Scene name" in
-- CONTEXT.md) are drawn by the host into transparent areas as wide as a track,
-- centred over their buttons. Each name has two areas on top of each other:
-- one for the name, one for the default that stands in while it is empty.
-- A clip name sits right above its launch button, in the space between the
-- labels of the scene above and the button; a scene name likewise above its
-- scene button, in a narrower area that keeps clear of the clip names of
-- track 8. The track names sit above the clip names of scene 1, in a taller
-- area for their larger font.
local NAME_WIDTH = 280
local SCENE_NAME_WIDTH = 250
local NAME_HEIGHT = 50
local TRACK_NAME_HEIGHT = 60
local NAME_INSET = math.floor((BUTTON_WIDTH - NAME_WIDTH) / 2)
local function add_name_areas(panel, name, left, top, path)
	panel[name .. "_value"] = { offset = { left, top }, { path = path } }
	panel[name .. "_default"] = { offset = { left, top }, { path = path } }
end
for track = 1, TRACK_COUNT do
	local left = LAUNCH_LEFT + TRACK_PITCH * (track - 1) + NAME_INSET
	add_name_areas(front, "track" .. track .. "_name", left, LAUNCH_TOP - NAME_HEIGHT - TRACK_NAME_HEIGHT, "Name_Area_Track")
	for row = 1, ROW_COUNT do
		local top = LAUNCH_TOP + ROW_PITCH * (row - 1) - NAME_HEIGHT
		add_name_areas(front, "track" .. track .. "_clip" .. row .. "_name", left, top, "Name_Area")
	end
end
for scene = 1, ROW_COUNT do
	local top = LAUNCH_TOP + ROW_PITCH * (scene - 1) - NAME_HEIGHT
	add_name_areas(front, "scene" .. scene .. "_name", SCENE_LEFT + (SCENE_WIDTH - SCENE_NAME_WIDTH) / 2, top, "Name_Area_Scene")
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
	{ size = { TRACK_PITCH * (TRACK_COUNT - 1) + FUNCTION_WIDTH + 4, FUNCTION_HEIGHT } },
}

-- The back button stands above the first track, centred in the width of a
-- launch button, 20 pixels below the header, which leaves room for the track
-- names and the clip names of scene 1 between it and the grid. It is a lamp
-- showing one of nine frames, with a transparent display of the same size on
-- top.
local BACK_LEFT = LAUNCH_LEFT + 38
local BACK_TOP = 180
front.lamp_back = {
	offset = { BACK_LEFT, BACK_TOP },
	{ path = "Back_Button_9frames", frames = 9 },
}
front.click_back = {
	offset = { BACK_LEFT, BACK_TOP },
	{ size = { 130, 135 } },
}

-- Its labels, one line for each step it goes back to, stand to its left,
-- aligned on the right 10 pixels from the button and as high as it; one line
-- is lit at a time (see tools/make-label-strips.sh).
local STEP_LABEL_WIDTH = 300
local STEP_LABEL_GAP = 10
front.label_back = {
	offset = { BACK_LEFT - STEP_LABEL_GAP - STEP_LABEL_WIDTH, BACK_TOP },
	{ path = "Label_Back_5frames", frames = 5 },
}

-- The forward button stands beside it, above the second track, and looks
-- like a scene button; its labels stand to its right, aligned on the left.
local FORWARD_LEFT = BACK_LEFT + TRACK_PITCH
front.lamp_forward = {
	offset = { FORWARD_LEFT, BACK_TOP },
	{ path = "Scene_Button_11frames", frames = 11 },
}
front.click_forward = {
	offset = { FORWARD_LEFT, BACK_TOP },
	{ size = { 130, 135 } },
}
front.label_forward = {
	offset = { FORWARD_LEFT + 130 + STEP_LABEL_GAP, BACK_TOP },
	{ path = "Label_Forward_3frames", frames = 3 },
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

-- The setup button is a lamp with a transparent display on top, like the grid,
-- centred below the fill-in button.
local SETUP_LEFT = FILL_IN_LEFT + (SCENE_WIDTH - 70) / 2
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

-- Six CV outputs, A to F from the top, for each track, 300 pixels apart
-- across, which leaves room for the track names, and 240 down, which leaves
-- room for the CV output names, each with the switch for its CV mode to its
-- left: the stock bypass switch, whose frames from the bottom position up
-- stand for bipolar, unipolar and pattern. Their captions are in Labels_Back
-- (see tools/make-cv-labels.py).
local JACK_LEFT = 840
local JACK_TOP = 1065
local JACK_PITCH = 300
local OUTPUT_PITCH = 240
for track = 1, TRACK_COUNT do
	-- Names are centred over the switches and the jacks.
	local column_centre = JACK_LEFT + JACK_PITCH * (track - 1) - 5
	for output, letter in ipairs(OUTPUT_LETTERS) do
		local left = JACK_LEFT + JACK_PITCH * (track - 1)
		local top = JACK_TOP + OUTPUT_PITCH * (output - 1)
		back["cv_out_track" .. track .. "_" .. letter] = {
			offset = { left, top },
			{ path = "SharedCVJack", frames = 3 },
		}
		back["cv_mode_track" .. track .. "_" .. letter] = {
			offset = { left - 90, top + 45 - 60 },
			{ path = "CV_Mode_Switch_3frames", frames = 3 },
		}
		-- The CV output's name right above its switch, like a clip name above
		-- its launch button.
		add_name_areas(back, "track" .. track .. "_output_" .. letter .. "_name", column_centre - NAME_WIDTH / 2, top - 70, "Name_Area")
	end
	-- The track name above the name of output A, as wide as the columns are
	-- apart and as tall as on the front.
	add_name_areas(back, "track" .. track .. "_name", column_centre - JACK_PITCH / 2, JACK_TOP - 135, "Name_Area_Back")
end

folded_front = {
	Bg = {
		{ path = "Panel_Folded_Front_dark" },
	},
	Logo = {
		offset = LOGO_OFFSET,
		{ path = "Logo_5frames", frames = 5 },
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
