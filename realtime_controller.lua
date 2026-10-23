format_version = "1.0"

rtc_bindings = {
	{ source = "/environment/system_sample_rate", dest = "/global_rtc/init_instance" },
	{ source = "/environment/instance_id", dest = "/global_rtc/init_instance" },
}

global_rtc = {
	init_instance = function(source_property_path, instance_id)
		local sample_rate = jbox.load_property("/environment/system_sample_rate")
		local instance_id = jbox.load_property("/environment/instance_id")

		local new_no = jbox.make_native_object_rw("Instance", {sample_rate, instance_id})
		jbox.store_property("/custom_properties/instance", new_no);
	end,
}

sample_rate_setup = {
	native = {
		22050,
		44100,
		48000,
		88200,
		96000,
		192000
	},
}

-- The realtime code hears about every change of the setup state, of the length
-- of the switch interval, of the fill-in button, of the function, of a track's
-- selection, mute and solo buttons and CV modes, and of what a launch button
-- holds, whether it comes from a click, automation or a control surface.
local OUTPUT_LETTERS = { "a", "b", "c", "d", "e", "f" }
local OUTPUT_SETTINGS = { "pattern", "unipolar", "bipolar", "transition" }
local notify = { "/custom_properties/setup", "/custom_properties/switch_interval", "/custom_properties/fill_in", "/custom_properties/selected_function" }
for track = 1, 8 do
	table.insert(notify, "/custom_properties/track" .. track .. "_selection")
	table.insert(notify, "/custom_properties/track" .. track .. "_mute")
	table.insert(notify, "/custom_properties/track" .. track .. "_solo")
	for _, letter in ipairs(OUTPUT_LETTERS) do
		table.insert(notify, "/custom_properties/track" .. track .. "_cv_mode_" .. letter)
	end
	for row = 1, 8 do
		local clip = "/custom_properties/track" .. track .. "_clip" .. row
		table.insert(notify, clip .. "_present")
		table.insert(notify, clip .. "_colour")
		table.insert(notify, clip .. "_fill_in_present")
		table.insert(notify, clip .. "_fill_in_colour")
		table.insert(notify, clip .. "_landing_present")
		table.insert(notify, clip .. "_landing_colour")
		for _, kind in ipairs({ "", "_fill_in", "_landing" }) do
			for _, letter in ipairs(OUTPUT_LETTERS) do
				for _, setting in ipairs(OUTPUT_SETTINGS) do
					table.insert(notify, clip .. kind .. "_" .. letter .. "_" .. setting)
				end
			end
		end
	end
end

rt_input_setup = {
	notify = notify
}
