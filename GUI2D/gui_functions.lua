-- Written by tools/make-gui-functions.py; change that instead.
--
-- Whether a name, or the default name that stands in while it is empty, is
-- shown (see docs/adr/0007). On the front panel values[1] is the setup state
-- (0 = launch mode, 1 = clip picking, 2 + i clip editing and 66 + i fill-in
-- editing of launch button i, counted from 0 down track 1 first) and
-- values[2] the name; on the back panel values[1] is the name.

-- Track names on the back panel are always shown.
function is_name_shown(values)
	return values[1] ~= ""
end

function is_default_name_shown(values)
	return values[1] == ""
end

-- Scene names are shown in launch mode only, since in clip picking the scene
-- buttons stand for lengths.
function is_name_shown_in_launch_mode(values)
	return values[1] == 0 and values[2] ~= ""
end

function is_default_name_shown_in_launch_mode(values)
	return values[1] == 0 and values[2] == ""
end

-- Track names: also in the editing of a launch button of the track.
function is_track1_name_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 0 and (values[1] - 2) % 64 <= 7)) and values[2] ~= "" end
function is_track1_name_default_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 0 and (values[1] - 2) % 64 <= 7)) and values[2] == "" end
function is_track2_name_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 8 and (values[1] - 2) % 64 <= 15)) and values[2] ~= "" end
function is_track2_name_default_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 8 and (values[1] - 2) % 64 <= 15)) and values[2] == "" end
function is_track3_name_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 16 and (values[1] - 2) % 64 <= 23)) and values[2] ~= "" end
function is_track3_name_default_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 16 and (values[1] - 2) % 64 <= 23)) and values[2] == "" end
function is_track4_name_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 24 and (values[1] - 2) % 64 <= 31)) and values[2] ~= "" end
function is_track4_name_default_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 24 and (values[1] - 2) % 64 <= 31)) and values[2] == "" end
function is_track5_name_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 32 and (values[1] - 2) % 64 <= 39)) and values[2] ~= "" end
function is_track5_name_default_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 32 and (values[1] - 2) % 64 <= 39)) and values[2] == "" end
function is_track6_name_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 40 and (values[1] - 2) % 64 <= 47)) and values[2] ~= "" end
function is_track6_name_default_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 40 and (values[1] - 2) % 64 <= 47)) and values[2] == "" end
function is_track7_name_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 48 and (values[1] - 2) % 64 <= 55)) and values[2] ~= "" end
function is_track7_name_default_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 48 and (values[1] - 2) % 64 <= 55)) and values[2] == "" end
function is_track8_name_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 56 and (values[1] - 2) % 64 <= 63)) and values[2] ~= "" end
function is_track8_name_default_shown(values) return (values[1] <= 1 or ((values[1] - 2) % 64 >= 56 and (values[1] - 2) % 64 <= 63)) and values[2] == "" end

-- Clip names: also in the editing of their launch button.
function is_track1_clip1_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 0) and values[2] ~= "" end
function is_track1_clip1_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 0) and values[2] == "" end
function is_track1_clip2_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 1) and values[2] ~= "" end
function is_track1_clip2_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 1) and values[2] == "" end
function is_track1_clip3_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 2) and values[2] ~= "" end
function is_track1_clip3_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 2) and values[2] == "" end
function is_track1_clip4_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 3) and values[2] ~= "" end
function is_track1_clip4_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 3) and values[2] == "" end
function is_track1_clip5_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 4) and values[2] ~= "" end
function is_track1_clip5_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 4) and values[2] == "" end
function is_track1_clip6_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 5) and values[2] ~= "" end
function is_track1_clip6_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 5) and values[2] == "" end
function is_track1_clip7_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 6) and values[2] ~= "" end
function is_track1_clip7_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 6) and values[2] == "" end
function is_track1_clip8_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 7) and values[2] ~= "" end
function is_track1_clip8_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 7) and values[2] == "" end
function is_track2_clip1_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 8) and values[2] ~= "" end
function is_track2_clip1_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 8) and values[2] == "" end
function is_track2_clip2_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 9) and values[2] ~= "" end
function is_track2_clip2_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 9) and values[2] == "" end
function is_track2_clip3_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 10) and values[2] ~= "" end
function is_track2_clip3_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 10) and values[2] == "" end
function is_track2_clip4_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 11) and values[2] ~= "" end
function is_track2_clip4_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 11) and values[2] == "" end
function is_track2_clip5_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 12) and values[2] ~= "" end
function is_track2_clip5_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 12) and values[2] == "" end
function is_track2_clip6_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 13) and values[2] ~= "" end
function is_track2_clip6_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 13) and values[2] == "" end
function is_track2_clip7_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 14) and values[2] ~= "" end
function is_track2_clip7_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 14) and values[2] == "" end
function is_track2_clip8_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 15) and values[2] ~= "" end
function is_track2_clip8_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 15) and values[2] == "" end
function is_track3_clip1_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 16) and values[2] ~= "" end
function is_track3_clip1_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 16) and values[2] == "" end
function is_track3_clip2_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 17) and values[2] ~= "" end
function is_track3_clip2_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 17) and values[2] == "" end
function is_track3_clip3_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 18) and values[2] ~= "" end
function is_track3_clip3_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 18) and values[2] == "" end
function is_track3_clip4_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 19) and values[2] ~= "" end
function is_track3_clip4_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 19) and values[2] == "" end
function is_track3_clip5_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 20) and values[2] ~= "" end
function is_track3_clip5_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 20) and values[2] == "" end
function is_track3_clip6_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 21) and values[2] ~= "" end
function is_track3_clip6_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 21) and values[2] == "" end
function is_track3_clip7_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 22) and values[2] ~= "" end
function is_track3_clip7_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 22) and values[2] == "" end
function is_track3_clip8_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 23) and values[2] ~= "" end
function is_track3_clip8_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 23) and values[2] == "" end
function is_track4_clip1_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 24) and values[2] ~= "" end
function is_track4_clip1_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 24) and values[2] == "" end
function is_track4_clip2_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 25) and values[2] ~= "" end
function is_track4_clip2_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 25) and values[2] == "" end
function is_track4_clip3_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 26) and values[2] ~= "" end
function is_track4_clip3_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 26) and values[2] == "" end
function is_track4_clip4_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 27) and values[2] ~= "" end
function is_track4_clip4_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 27) and values[2] == "" end
function is_track4_clip5_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 28) and values[2] ~= "" end
function is_track4_clip5_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 28) and values[2] == "" end
function is_track4_clip6_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 29) and values[2] ~= "" end
function is_track4_clip6_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 29) and values[2] == "" end
function is_track4_clip7_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 30) and values[2] ~= "" end
function is_track4_clip7_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 30) and values[2] == "" end
function is_track4_clip8_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 31) and values[2] ~= "" end
function is_track4_clip8_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 31) and values[2] == "" end
function is_track5_clip1_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 32) and values[2] ~= "" end
function is_track5_clip1_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 32) and values[2] == "" end
function is_track5_clip2_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 33) and values[2] ~= "" end
function is_track5_clip2_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 33) and values[2] == "" end
function is_track5_clip3_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 34) and values[2] ~= "" end
function is_track5_clip3_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 34) and values[2] == "" end
function is_track5_clip4_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 35) and values[2] ~= "" end
function is_track5_clip4_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 35) and values[2] == "" end
function is_track5_clip5_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 36) and values[2] ~= "" end
function is_track5_clip5_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 36) and values[2] == "" end
function is_track5_clip6_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 37) and values[2] ~= "" end
function is_track5_clip6_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 37) and values[2] == "" end
function is_track5_clip7_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 38) and values[2] ~= "" end
function is_track5_clip7_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 38) and values[2] == "" end
function is_track5_clip8_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 39) and values[2] ~= "" end
function is_track5_clip8_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 39) and values[2] == "" end
function is_track6_clip1_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 40) and values[2] ~= "" end
function is_track6_clip1_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 40) and values[2] == "" end
function is_track6_clip2_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 41) and values[2] ~= "" end
function is_track6_clip2_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 41) and values[2] == "" end
function is_track6_clip3_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 42) and values[2] ~= "" end
function is_track6_clip3_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 42) and values[2] == "" end
function is_track6_clip4_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 43) and values[2] ~= "" end
function is_track6_clip4_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 43) and values[2] == "" end
function is_track6_clip5_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 44) and values[2] ~= "" end
function is_track6_clip5_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 44) and values[2] == "" end
function is_track6_clip6_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 45) and values[2] ~= "" end
function is_track6_clip6_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 45) and values[2] == "" end
function is_track6_clip7_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 46) and values[2] ~= "" end
function is_track6_clip7_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 46) and values[2] == "" end
function is_track6_clip8_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 47) and values[2] ~= "" end
function is_track6_clip8_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 47) and values[2] == "" end
function is_track7_clip1_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 48) and values[2] ~= "" end
function is_track7_clip1_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 48) and values[2] == "" end
function is_track7_clip2_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 49) and values[2] ~= "" end
function is_track7_clip2_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 49) and values[2] == "" end
function is_track7_clip3_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 50) and values[2] ~= "" end
function is_track7_clip3_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 50) and values[2] == "" end
function is_track7_clip4_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 51) and values[2] ~= "" end
function is_track7_clip4_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 51) and values[2] == "" end
function is_track7_clip5_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 52) and values[2] ~= "" end
function is_track7_clip5_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 52) and values[2] == "" end
function is_track7_clip6_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 53) and values[2] ~= "" end
function is_track7_clip6_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 53) and values[2] == "" end
function is_track7_clip7_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 54) and values[2] ~= "" end
function is_track7_clip7_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 54) and values[2] == "" end
function is_track7_clip8_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 55) and values[2] ~= "" end
function is_track7_clip8_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 55) and values[2] == "" end
function is_track8_clip1_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 56) and values[2] ~= "" end
function is_track8_clip1_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 56) and values[2] == "" end
function is_track8_clip2_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 57) and values[2] ~= "" end
function is_track8_clip2_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 57) and values[2] == "" end
function is_track8_clip3_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 58) and values[2] ~= "" end
function is_track8_clip3_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 58) and values[2] == "" end
function is_track8_clip4_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 59) and values[2] ~= "" end
function is_track8_clip4_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 59) and values[2] == "" end
function is_track8_clip5_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 60) and values[2] ~= "" end
function is_track8_clip5_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 60) and values[2] == "" end
function is_track8_clip6_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 61) and values[2] ~= "" end
function is_track8_clip6_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 61) and values[2] == "" end
function is_track8_clip7_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 62) and values[2] ~= "" end
function is_track8_clip7_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 62) and values[2] == "" end
function is_track8_clip8_name_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 63) and values[2] ~= "" end
function is_track8_clip8_name_default_shown(values) return (values[1] <= 1 or (values[1] - 2) % 64 == 63) and values[2] == "" end
