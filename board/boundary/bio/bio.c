/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/fbpanel.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/video.h>
#include <video_fb.h>
#include <spl.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <dm.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <linux/usb/dwc3.h>
#include "../common/padctrl.h"
#include "../common/bd_common.h"

DECLARE_GLOBAL_DATA_PTR;

static iomux_v3_cfg_t const init_pads[] = {
	IMX8MQ_PAD_GPIO1_IO02__WDOG1_WDOG_B | MUX_PAD_CTRL(WDOG_PAD_CTRL),
	IMX8MQ_PAD_UART1_RXD__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MQ_PAD_UART1_TXD__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),

#define GPIRQ_GT911 			IMX_GPIO_NR(3, 12)
	IMX8MQ_PAD_NAND_DATA06__GPIO3_IO12 | MUX_PAD_CTRL(0xd6),
#define GP_GT911_RESET			IMX_GPIO_NR(3, 13)
#define GP_ST1633_RESET			IMX_GPIO_NR(3, 13)
	IMX8MQ_PAD_NAND_DATA07__GPIO3_IO13 | MUX_PAD_CTRL(0x49),

#define GP_GPIOKEY_POWER			IMX_GPIO_NR(1, 7)
	IMX8MQ_PAD_GPIO1_IO07__GPIO1_IO7 | MUX_PAD_CTRL(WEAK_PULLUP),

#define GP_TC358762_EN			IMX_GPIO_NR(3, 14)
#define GP_I2C2_SN65DSI83_EN		IMX_GPIO_NR(3, 14)
#define GP_MIPI_RESET			IMX_GPIO_NR(3, 14)
	IMX8MQ_PAD_NAND_DQS__GPIO3_IO14 | MUX_PAD_CTRL(0x6),


#define GP_EMMC_RESET			IMX_GPIO_NR(2, 10)
	IMX8MQ_PAD_SD1_RESET_B__GPIO2_IO10 | MUX_PAD_CTRL(0x41),

#define GP_CSI1_MIPI_PWDN		IMX_GPIO_NR(4, 25)
	IMX8MQ_PAD_SAI2_TXC__GPIO4_IO25 | MUX_PAD_CTRL(0x61),
#define GP_CSI1_MIPI_RESET		IMX_GPIO_NR(4, 24)
	IMX8MQ_PAD_SAI2_TXFS__GPIO4_IO24 | MUX_PAD_CTRL(0x61),
};

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(init_pads, ARRAY_SIZE(init_pads));
	set_wdog_reset(wdog);

	gpio_direction_output(GP_EMMC_RESET, 1);
	gpio_direction_output(GP_I2C2_SN65DSI83_EN, 0);
	gpio_direction_output(GP_CSI1_MIPI_PWDN, 1);
	gpio_direction_output(GP_CSI1_MIPI_RESET, 0);

	return 0;
}

#ifdef CONFIG_BOARD_POSTCLK_INIT
int board_postclk_init(void)
{
	/* TODO */
	return 0;
}
#endif

#define MAX_LOW_SIZE	(0x100000000ULL - CONFIG_SYS_SDRAM_BASE)
#define SDRAM_SIZE	((1ULL * CONFIG_DDR_MB) << 20)

#if SDRAM_SIZE > MAX_LOW_SIZE
#define MEM_SIZE	MAX_LOW_SIZE
#else
#define MEM_SIZE	SDRAM_SIZE
#endif

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = SDRAM_SIZE;
	return 0;
}

int dram_init(void)
{
	/* rom_pointer[1] contains the size of TEE occupies */
	gd->ram_size = MEM_SIZE - rom_pointer[1];
	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif


#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)

#define USB_PHY_CTRL0			0xF0040
#define USB_PHY_CTRL0_REF_SSP_EN	BIT(2)

#define USB_PHY_CTRL1			0xF0044
#define USB_PHY_CTRL1_RESET		BIT(0)
#define USB_PHY_CTRL1_COMMONONN		BIT(1)
#define USB_PHY_CTRL1_ATERESET		BIT(3)
#define USB_PHY_CTRL1_VDATSRCENB0	BIT(19)
#define USB_PHY_CTRL1_VDATDETENB0	BIT(20)

#define USB_PHY_CTRL2			0xF0048
#define USB_PHY_CTRL2_TXENABLEN0	BIT(8)

static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_SUPER,
	.base = USB1_BASE_ADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
//	.power_down_scale = 2,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

static void dwc3_nxp_usb_phy_init(struct dwc3_device *dwc3)
{
	struct dwc3 *dwc3_reg = (struct dwc3 *)(dwc3->base + DWC3_REG_OFFSET);
	u32 val;

	val = readl(dwc3->base + USB_PHY_CTRL1);
	val &= ~(USB_PHY_CTRL1_VDATSRCENB0 | USB_PHY_CTRL1_VDATDETENB0 |
			USB_PHY_CTRL1_COMMONONN);
	val |= USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET;
	writel(val, dwc3->base + USB_PHY_CTRL1);

	val = readl(dwc3->base + USB_PHY_CTRL0);
	val |= USB_PHY_CTRL0_REF_SSP_EN;
	writel(val, dwc3->base + USB_PHY_CTRL0);

	val = readl(dwc3->base + USB_PHY_CTRL2);
	val |= USB_PHY_CTRL2_TXENABLEN0;
	writel(val, dwc3->base + USB_PHY_CTRL2);

	val = readl(dwc3->base + USB_PHY_CTRL1);
	val &= ~(USB_PHY_CTRL1_RESET | USB_PHY_CTRL1_ATERESET);
	writel(val, dwc3->base + USB_PHY_CTRL1);

	val = readl(&dwc3_reg->g_ctl);
	val &= ~(DWC3_GCTL_PWRDNSCALE_MASK);
	val |= DWC3_GCTL_PWRDNSCALE(2);

	writel(val, &dwc3_reg->g_ctl);
}
#endif

#if defined(CONFIG_USB_DWC3) || defined(CONFIG_USB_XHCI_IMX8M)
int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;
	imx8m_usb_power(index, true);

	if (index == 0 && init == USB_INIT_DEVICE) {
		dwc3_nxp_usb_phy_init(&dwc3_device_data);
		return dwc3_uboot_init(&dwc3_device_data);
	} else if (index == 0 && init == USB_INIT_HOST) {
		return ret;
	}

	if (index == 1) {
		/* Release HUB reset */
#define GP_USB1_HUB_RESET	IMX_GPIO_NR(1, 14)
		imx_iomux_v3_setup_pad(IMX8MQ_PAD_GPIO1_IO14__GPIO1_IO14 |
				       MUX_PAD_CTRL(WEAK_PULLUP));
		gpio_request(GP_USB1_HUB_RESET, "usb1_rst");
		gpio_direction_output(GP_USB1_HUB_RESET, 1);
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	int ret = 0;
	if (index == 0 && init == USB_INIT_DEVICE)
		dwc3_uboot_exit(index);

	imx8m_usb_power(index, false);

	return ret;
}
#endif

#ifdef CONFIG_CMD_FBPANEL

int board_detect_hdmi(struct display_info_t const *di)
{
	return hdmi_hpd_status() ? 1 : 0;
}

int board_detect_gt911(struct display_info_t const *di)
{
	int ret;
	struct udevice *dev, *chip;

	if (di->bus_gp)
		gpio_direction_output(di->bus_gp, 1);
	gpio_set_value(GP_GT911_RESET, 0);
	mdelay(20);
	gpio_direction_output(GPIRQ_GT911, di->addr_num == 0x14 ? 1 : 0);
	udelay(100);
	gpio_set_value(GP_GT911_RESET, 1);
	mdelay(6);
	gpio_set_value(GPIRQ_GT911, 0);
	mdelay(50);
	gpio_direction_input(GPIRQ_GT911);
	ret = uclass_get_device(UCLASS_I2C, di->bus_num, &dev);
	if (ret)
		return 0;

	ret = dm_i2c_probe(dev, di->addr_num, 0x0, &chip);
	if (ret && di->bus_gp)
		gpio_direction_input(di->bus_gp);
	return (ret == 0);
}

static const struct display_info_t displays[] = {
	/* hdmi */
	VD_1920_1080M_60(HDMI, board_detect_hdmi, 0, 0x50),
	VD_1280_720M_60(HDMI, NULL, 0, 0x50),
	VD_MIPI_M101NWWB(MIPI, fbp_detect_i2c, fbp_bus_gp(3, GP_I2C2_SN65DSI83_EN, 0, 0), 0x2c),
	VD_LTK0680YTMDB(MIPI, NULL, fbp_bus_gp(3, GP_MIPI_RESET, GP_MIPI_RESET, 0), 0x0),
	VD_MIPI_COM50H5N03ULC(MIPI, NULL, fbp_bus_gp(3, GP_MIPI_RESET, GP_MIPI_RESET, 0), 0x00),
};
#define display_cnt	ARRAY_SIZE(displays)
#else
#define displays	NULL
#define display_cnt	0
#endif

int board_init(void)
{
	gpio_request(GP_I2C2_SN65DSI83_EN, "sn65dsi83_enable");
	gpio_request(GP_GT911_RESET, "gt911_reset");
	gpio_request(GPIRQ_GT911, "gt911_irq");
	gpio_direction_output(GP_GT911_RESET, 0);
#ifdef CONFIG_DM_ETH
	board_eth_init(gd->bd);
#endif
#ifdef CONFIG_CMD_FBPANEL
	fbp_setup_display(displays, display_cnt);
#endif

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return 0;
}

#if defined(CONFIG_CMD_FASTBOOT) || defined(CONFIG_CMD_DFU)
extern void imx_get_mac_from_fuse(int dev_id, unsigned char *mac);
static void addserial_env(const char* env_var)
{
	unsigned char mac_address[8];
	char serialbuf[20];

	if (!env_get(env_var)) {
		imx_get_mac_from_fuse(0, mac_address);
		snprintf(serialbuf, sizeof(serialbuf), "%02x%02x%02x%02x%02x%02x",
			 mac_address[0], mac_address[1], mac_address[2],
			 mac_address[3], mac_address[4], mac_address[5]);
		env_set(env_var, serialbuf);
	}
}
#endif

#ifdef CONFIG_CMD_BMODE
const struct boot_mode board_boot_modes[] = {
        /* 4 bit bus width */
	{"emmc0",	MAKE_CFGVAL(0x22, 0x20, 0x00, 0x10)},
        {NULL,          0},
};
#endif

static int fastboot_key_pressed(void)
{
	gpio_request(GP_GPIOKEY_POWER, "fastboot_key");
	gpio_direction_input(GP_GPIOKEY_POWER);
	return !gpio_get_value(GP_GPIOKEY_POWER);
}

void board_late_mmc_env_init(void);
void init_usb_clk(int usbno);

static void set_env_vars(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (!env_get("board"))
		env_set("board", "bio");
	env_set("soc", "imx8mq");
	env_set("imx_cpu", get_imx_type((get_cpu_rev() & 0xFF000) >> 12));
	env_set("uboot_defconfig", CONFIG_DEFCONFIG);
#endif
}

void board_set_default_env(void)
{
	set_env_vars();
#ifdef CONFIG_CMD_FBPANEL
	fbp_setup_env_cmds();
#endif
	board_eth_addresses();
}

int board_late_init(void)
{
	set_env_vars();
#if defined(CONFIG_USB_FUNCTION_FASTBOOT) || defined(CONFIG_CMD_DFU)
	addserial_env("serial#");
	if (fastboot_key_pressed()) {
		printf("Starting fastboot...\n");
		env_set("preboot", "fastboot 0");
	}
#endif
#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	init_usb_clk(0);
	init_usb_clk(1);
	return 0;
}