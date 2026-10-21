-- Whether a name, or the default name that stands in while it is empty, is
-- shown (see docs/adr/0007). On the front panel values[1] is the setup state
-- (0 = launch mode, 1 = clip picking, from 2 clip editing and fill-in editing)
-- and values[2] the name; on the back panel values[1] is the name.

function is_name_shown(values)
	return values[1] ~= ""
end

function is_default_name_shown(values)
	return values[1] == ""
end

-- Track names and clip names.
function is_name_shown_in_launch_mode_and_clip_picking(values)
	return values[1] <= 1 and values[2] ~= ""
end

function is_default_name_shown_in_launch_mode_and_clip_picking(values)
	return values[1] <= 1 and values[2] == ""
end

-- Scene names, since in clip picking the scene buttons stand for lengths.
function is_name_shown_in_launch_mode(values)
	return values[1] == 0 and values[2] ~= ""
end

function is_default_name_shown_in_launch_mode(values)
	return values[1] == 0 and values[2] == ""
end
