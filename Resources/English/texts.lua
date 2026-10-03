format_version = "1.0"

texts = {
	["text_stop"] = "Stop",
	["text_off"] = "Off",
	["text_queued"] = "Queued",
	["text_active"] = "Active",
}

for track = 1, 8 do
	local selection = "text_track" .. track .. "_selection"
	texts[selection] = "Track " .. track .. " Selection"
	texts[selection .. "_short"] = "T" .. track .. " Sel"
	texts[selection .. "_shortest"] = "T" .. track .. "Sl"

	local stop = "text_track" .. track .. "_stop"
	texts[stop] = "Track " .. track .. " Stop"
	texts[stop .. "_short"] = "T" .. track .. " Stop"
	texts[stop .. "_shortest"] = "T" .. track .. "St"

	texts["text_cv_out_track" .. track] = "Track " .. track .. " CV Out"

	for pattern = 1, 8 do
		local button = "text_track" .. track .. "_pattern" .. pattern
		texts[button] = "Track " .. track .. " Pattern " .. pattern
		texts[button .. "_short"] = "T" .. track .. " P" .. pattern
		texts[button .. "_shortest"] = "T" .. track .. "P" .. pattern
	end
end

for pattern = 1, 8 do
	texts["text_pattern" .. pattern] = "Pattern " .. pattern
end
