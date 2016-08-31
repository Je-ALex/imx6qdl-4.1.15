/***********************************************************************
 * imx-wm8524.c
 *
 * Copyright (C) 2013 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 **********************************************************************/
//johnli
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/soc-dapm.h>
#include <linux/pinctrl/consumer.h>

#include "imx-audmux.h"
#include "imx-ssi.h"

#define DAI_NAME_SIZE	32

struct imx_priv {
	int sysclk;         /*mclk from the outside*/
	int codec_sysclk;
	int dai_hifi;
	int hp_irq;
	int hp_status;
	int amic_irq;
	int amic_status;
	struct platform_device *pdev;
};

struct imx_wm8524_data {
	struct snd_soc_dai_link dai;
	struct snd_soc_card card;
	char codec_dai_name[DAI_NAME_SIZE];
	char platform_name[DAI_NAME_SIZE];
	unsigned int clk_frequency;
};

unsigned int sample_format = SNDRV_PCM_FMTBIT_S16_LE;
static struct imx_priv card_priv;
static struct snd_soc_card snd_soc_card_imx;

static int imx_hifi_startup(struct snd_pcm_substream *substream)
{


	return 0;
}

static void imx_hifi_shutdown(struct snd_pcm_substream *substream)
{
	return;
}

static int imx_hifi_hw_params(struct snd_pcm_substream *substream,
				     struct snd_pcm_hw_params *params)
{
	//6q is master
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	unsigned int channels = params_channels(params);
	int ret = 0;
	u32 dai_format;

	/* set i.MX active slot mask */
	snd_soc_dai_set_tdm_slot(cpu_dai,
				 channels == 1 ? 0xfffffffe : 0xfffffffc,
				 channels == 1 ? 0xfffffffe : 0xfffffffc,
				 2, 32);

//	dai_format = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_IF |
//		SND_SOC_DAIFMT_CBS_CFS;

	dai_format = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
			SND_SOC_DAIFMT_CBM_CFM;
	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, dai_format);
	if (ret < 0)
		return ret;
#if 0 //johnli debug, bitclk set already
	/* set the SSI system clock as input (unused) */
       snd_soc_dai_set_sysclk(cpu_dai,  IMX_SSP_SYS_CLK, 0, SND_SOC_CLOCK_IN);

	snd_soc_dai_set_clkdiv(cpu_dai, IMX_SSI_TX_DIV_PM, 3);
	snd_soc_dai_set_clkdiv(cpu_dai, IMX_SSI_TX_DIV_2, 0);
	snd_soc_dai_set_clkdiv(cpu_dai, IMX_SSI_TX_DIV_PSR, 0);
#endif
	return 0;

}
/*
static int imx_wm8524_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}
*/

static struct snd_soc_ops imx_hifi_ops = {
	.startup   = imx_hifi_startup,
	.shutdown  = imx_hifi_shutdown,
	.hw_params = imx_hifi_hw_params,
};

static struct platform_device *imx_snd_device;

static int imx_wm8524_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *cpu_np, *codec_np;
	struct platform_device *cpu_pdev;
	struct imx_priv *priv = &card_priv;
     struct imx_wm8524_data *data;
	struct clk *codec_clk = NULL;
	int int_port, ext_port;
	int ret;

	priv->pdev = pdev;

	cpu_np = of_parse_phandle(pdev->dev.of_node, "cpu-dai", 0);
	if (!cpu_np) {
		dev_err(&pdev->dev, "cpu dai phandle missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	if (!strstr(cpu_np->name, "ssi"))
		goto audmux_bypass;

	ret = of_property_read_u32(np, "mux-int-port", &ext_port);
	if (ret) {
		dev_err(&pdev->dev, "mux-int-port missing or invalid\n");
		return ret;
	}
	ret = of_property_read_u32(np, "mux-ext-port", &int_port);
	if (ret) {
		dev_err(&pdev->dev, "mux-ext-port missing or invalid\n");
		return ret;
	}

	/*
	 * The port numbering in the hardware manual starts at 1, while
	 * the audmux API expects it starts at 0.
	 */
	int_port--;
	ext_port--;
	ret = imx_audmux_v2_configure_port(ext_port,
			IMX_AUDMUX_V2_PTCR_SYN |
			IMX_AUDMUX_V2_PTCR_TFSEL(int_port) |
			IMX_AUDMUX_V2_PTCR_TCSEL(int_port) |
			IMX_AUDMUX_V2_PTCR_TFSDIR |
			IMX_AUDMUX_V2_PTCR_TCLKDIR,
			IMX_AUDMUX_V2_PDCR_RXDSEL(int_port));
	if (ret) {
		dev_err(&pdev->dev, "audmux internal port setup failed\n");
		return ret;
	}
	imx_audmux_v2_configure_port(int_port,
			IMX_AUDMUX_V2_PTCR_SYN,
			IMX_AUDMUX_V2_PDCR_RXDSEL(ext_port));
	if (ret) {
		dev_err(&pdev->dev, "audmux external port setup failed\n");
		return ret;
	}

audmux_bypass:
	codec_np = of_parse_phandle(pdev->dev.of_node, "audio-codec", 0);
	if (!codec_np) {
		dev_err(&pdev->dev, "phandle missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	cpu_pdev = of_find_device_by_node(cpu_np);
	if (!cpu_pdev) {
		dev_err(&pdev->dev, "failed to find SSI platform device\n");
		ret = -EINVAL;
		goto fail;
	}




	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto fail;
	}

	
	data->dai.name = "HiFi";
	data->dai.stream_name = "HiFi";
	data->dai.codec_dai_name = "wm8524";
	data->dai.codec_of_node = codec_np;
	data->dai.cpu_dai_name = dev_name(&cpu_pdev->dev);
	data->dai.platform_of_node = cpu_np;
	data->dai.ops = &imx_hifi_ops;
//	data->dai.dai_fmt = SND_SOC_DAIFMT_I2S |  SND_SOC_DAIFMT_NB_IF |
//		SND_SOC_DAIFMT_CBS_CFS;
	data->dai.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
			    SND_SOC_DAIFMT_CBM_CFM;

	data->card.dev = &pdev->dev;
	ret = snd_soc_of_parse_card_name(&data->card, "model");
	if (ret)
		goto fail;

	data->card.num_links = 1;
	data->card.dai_link = &data->dai;


	platform_set_drvdata(pdev, &data->card);
	snd_soc_card_set_drvdata(&data->card, data);

	ret = devm_snd_soc_register_card(&pdev->dev, &data->card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n", ret);
		goto fail;
	}




	goto fail;
fail:
	if (cpu_np)
		of_node_put(cpu_np);
	if (codec_np)
		of_node_put(codec_np);

	return ret;
}

static int imx_wm8524_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id imx_wm8524_dt_ids[] = {
	{ .compatible = "fsl,imx-audio-wm8524", },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, imx_wm8524_dt_ids);

static struct platform_driver imx_wm8524_driver = {
	.driver = {
		.name = "imx-wm8524",
		.owner = THIS_MODULE,
		.of_match_table = imx_wm8524_dt_ids,
	},
	.probe = imx_wm8524_probe,
	.remove = imx_wm8524_remove,
};
module_platform_driver(imx_wm8524_driver);

/* Module information */
MODULE_DESCRIPTION("ALSA SoC imx wm8524");
MODULE_LICENSE("GPL");
