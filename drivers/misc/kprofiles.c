// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Dakkshesh <dakkshesh5@gmail.com>.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kprofiles.h>
#include <linux/fb.h>

static unsigned int mode = CONFIG_KPROFILES_MODE;
static bool auto_kprofiles = false;

module_param(mode, uint, 0664);
module_param(auto_kprofiles, bool, 0664);

static int fb_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	static bool screen_on = true;
	static unsigned int set_mode;

	if (auto_kprofiles) {
		struct fb_event *evdata = data;
		int *blank;

		if (event != FB_EVENT_BLANK)
			return NOTIFY_OK;

		blank = evdata->data;
		switch (*blank) {
			case FB_BLANK_POWERDOWN:
				if (!screen_on)
					break;
				screen_on = false;
				set_mode = mode;
				mode = 1;
				break;
			case FB_BLANK_UNBLANK:
				if (screen_on)
					break;
				screen_on = true;
				mode = set_mode;
		}
	}

	return 0;
}

unsigned int active_mode(void)
{
	if (mode < 4)
		return mode;

	pr_err_once("KPROFILES: Invalid value passed; falling back to level 0.\n");
	return 0;
}

static struct notifier_block fb_notifier_block = {
	.notifier_call = fb_notifier_callback,
};

static int  __init kprofiles_notifier_init(void)
{
	fb_register_client(&fb_notifier_block);
	return 0;
}
late_initcall(kprofiles_notifier_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dakkshesh");
MODULE_DESCRIPTION("KernelSpace Profiles");
MODULE_VERSION("2.0.0");
