/*
 * wm8524.c
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>

/*
 * Note this is a simple chip with no configuration interface, sample rate is
 * determined automatically by examining the Master clock and Bit clock ratios
 */
#if 0 //debug
#define WM8524_RATES  (SNDRV_PCM_RATE_48000)

#else
#define WM8524_RATES  (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |\
			SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |\
			SNDRV_PCM_RATE_192000)

#endif

static struct snd_soc_dai_driver wm8524_dai = {
	.name = "wm8524",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = WM8524_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
		},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = WM8524_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
};

static struct snd_soc_codec_driver soc_codec_dev_wm8524;

static  int wm8524_probe(struct platform_device *pdev)
{
	printk(KERN_CRIT "+++++++++++++++++++++++ {CKS} wm8524_probe ++++++++++++++++++++\n");
	return snd_soc_register_codec(&pdev->dev,
			&soc_codec_dev_wm8524, &wm8524_dai, 1);
}

static int  wm8524_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static const struct of_device_id wm8524_of_match[] = {
	{ .compatible = "wlf,wm8524", },
	{ }
};
MODULE_DEVICE_TABLE(of, wm8962_of_match);

static struct platform_driver wm8524_codec_driver = {
	.driver = {
			.name = "wm8524-codec",
			.owner = THIS_MODULE,
                .of_match_table = wm8524_of_match,
	},
	.probe = wm8524_probe,
	.remove = wm8524_remove,
        //.pm=wm8524_pm, //johnli add
     
};

module_platform_driver(wm8524_codec_driver);

MODULE_DESCRIPTION("ASoC wm8524 driver");
MODULE_LICENSE("GPL");
