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

-- The realtime code hears about every change of a track's selection, whether
-- it comes from a click, automation or a control surface.
rt_input_setup = {
	notify = {
		"/custom_properties/track1_selection",
		"/custom_properties/track2_selection",
		"/custom_properties/track3_selection",
		"/custom_properties/track4_selection",
		"/custom_properties/track5_selection",
		"/custom_properties/track6_selection",
		"/custom_properties/track7_selection",
		"/custom_properties/track8_selection",
	}
}
