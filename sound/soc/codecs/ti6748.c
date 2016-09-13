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

#define TI6748_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
			SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE)

static const struct snd_soc_dapm_widget dir_widgets[] = {
	SND_SOC_DAPM_INPUT("RX"),
	SND_SOC_DAPM_OUTPUT("TX"),
};

static const struct snd_soc_dapm_route dir_routes[] = {
	{ "Capture", NULL, "RX" },
	{ "TX", NULL, "Playback" },
};


static struct snd_soc_dai_driver ti6748_dai = {
	.name = "ti6748",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = TI6748_RATES,
		.formats = TI6748_FORMATS,
		},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 2,
		.channels_max = 2,
		.rates = TI6748_RATES,
		.formats = TI6748_FORMATS,
	},
};

//static struct snd_soc_codec_driver soc_codec_dev_ti6748;

static struct snd_soc_codec_driver soc_codec_dev_ti6748 = {
	.dapm_widgets = dir_widgets,
	.num_dapm_widgets = ARRAY_SIZE(dir_widgets),
	.dapm_routes = dir_routes,
	.num_dapm_routes = ARRAY_SIZE(dir_routes),
	.ignore_pmdown_time = true,
};

static  int ti6748_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev,&soc_codec_dev_ti6748,
			&ti6748_dai, 1);
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
	.probe = ti6748_probe,
	.remove = ti6748_remove,
	.driver = {
		.name = "ti6748",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ti6748_of_match),
	},
};

module_platform_driver(ti6748_codec_driver);

MODULE_DESCRIPTION("ASoC ti6748 driver");
MODULE_AUTHOR("Hushan electronic, Inc.");
MODULE_LICENSE("GPL v2");
