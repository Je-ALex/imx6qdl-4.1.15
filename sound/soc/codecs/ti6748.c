/*
 * ti6748.c
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
#define TI6748_RATES  (SNDRV_PCM_RATE_48000)

#else
#define TI6748_RATES  (SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 |\
			SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 |\
			SNDRV_PCM_RATE_192000)

#endif

static struct snd_soc_dai_driver ti6748_dai = {
	.name = "ti6748",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = TI6748_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
		},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = TI6748_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
};

static struct snd_soc_codec_driver soc_codec_dev_ti6748;

static  int ti6748_probe(struct platform_device *pdev)
{
	printk(KERN_CRIT "+++++++++++++++++++++++ {CKS} ti6748_probe ++++++++++++++++++++\n");
	return snd_soc_register_codec(&pdev->dev,
			&soc_codec_dev_ti6748, &ti6748_dai, 1);
}

static int  ti6748_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static const struct of_device_id ti6748_of_match[] = {
	{ .compatible = "ti,ti6748", },
	{ }
};
MODULE_DEVICE_TABLE(of, ti6748_of_match);

static struct platform_driver ti6748_codec_driver = {
	.driver = {
			.name = "ti6748-audio",
			.owner = THIS_MODULE,
            .of_match_table = ti6748_of_match,
	},
	.probe = ti6748_probe,
	.remove = ti6748_remove,
     
};

module_platform_driver(ti6748_codec_driver);

MODULE_DESCRIPTION("ASoC wm8524 driver");
MODULE_AUTHOR("Hushan electronic, Inc.");
MODULE_LICENSE("GPL");
