/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx51.h>
#include <asm/arch/sys_proto.h>
#include <malloc.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/fbpanel.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/spi.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <asm/arch/crm_regs.h>
#include <i2c.h>
#include <input.h>
#include <netdev.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mc13892.h>
#include <usb/ehci-fsl.h>

/* Special MXCFB sync flags are here. */
#include "../drivers/video/mxcfb.h"


DECLARE_GLOBAL_DATA_PTR;

#define AUD_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_MED |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define BUTTON_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_DSE_MED |			\
	PAD_CTL_HYS)

#define CEC_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_22K_UP | PAD_CTL_ODE | \
	PAD_CTL_DSE_MED | PAD_CTL_SRE_FAST)

#define CSI_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_DSE_MED |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define ESDHC_PAD_CTRL	(PAD_CTL_DSE_HIGH | PAD_CTL_DVS | \
			PAD_CTL_PUS_47K_UP | PAD_CTL_SRE_FAST)

#define ENET_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_DSE_MED | PAD_CTL_HYS)

#define HIGH_Z_SLOW	(PAD_CTL_HYS|PAD_CTL_SPEED_LOW | PAD_CTL_DSE_DISABLE)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_DSE_HIGH | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define I2C2_PAD_CTRL	(PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_DSE_HIGH | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define OUTPUT_40OHM	(PAD_CTL_DSE_MED)

#define RGB_PAD_CTRL	(PAD_CTL_DSE_MED | PAD_CTL_SRE_FAST)

#define SPI_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_DSE_MED | PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_DSE_MED |			\
	PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define WEAK_PULLDN	(PAD_CTL_PUS_100K_DOWN |		\
	PAD_CTL_DSE_MED |			\
	PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

#define WEAK_PULLDN_OUTPUT (PAD_CTL_PUS_100K_DOWN |		\
	PAD_CTL_DSE_MED |			\
	PAD_CTL_SRE_SLOW)

#define WEAK_PULLUP	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_DSE_MED |			\
	PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

#define WEAK_PULLUP_OUTPUT (PAD_CTL_PUS_100K_UP |		\
	PAD_CTL_DSE_MED |			\
	PAD_CTL_SRE_SLOW)

/*
 *
 */
static const iomux_v3_cfg_t init_pads[] = {
	/* AUDMUX */
//	IOMUX_PAD_CTRL(AUD3_BB_TXD__AUD3_TXD, AUD_PAD_CTRL),
//	IOMUX_PAD_CTRL(AUD3_BB_RXD__AUD3_RXD, AUD_PAD_CTRL),
//	IOMUX_PAD_CTRL(AUD3_BB_CK__AUD3_TXC, AUD_PAD_CTRL),
//	IOMUX_PAD_CTRL(AUD3_BB_FS__AUD3_TXFS, AUD_PAD_CTRL),

	/* ECSPI1 */
	IOMUX_PAD_CTRL(CSPI1_MISO__ECSPI1_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(CSPI1_MOSI__ECSPI1_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(CSPI1_SCLK__ECSPI1_SCLK, SPI_PAD_CTRL),
#define GP_ECSPI1_PMIC	IMX_GPIO_NR(4, 24)
	IOMUX_PAD_CTRL(CSPI1_SS0__GPIO4_24, WEAK_PULLDN), /* SS0 - active high */
#define GP_ECSPI1_FLASH	IMX_GPIO_NR(4, 25)
	IOMUX_PAD_CTRL(CSPI1_SS1__GPIO4_25, WEAK_PULLUP), /* SS1 - active low */

	/* ESDHC1 - FULL sd */
	IOMUX_PAD_CTRL(SD1_CMD__SD1_CMD, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_CLK__SD1_CLK, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DATA0__SD1_DATA0, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DATA1__SD1_DATA1, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DATA2__SD1_DATA2, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DATA3__SD1_DATA3, ESDHC_PAD_CTRL),
#define GP_ESDHC1_CD	IMX_GPIO_NR(1, 0)
	IOMUX_PAD_CTRL(GPIO1_0__GPIO1_0, WEAK_PULLUP),
#define GP_ESDHC1_WP	IMX_GPIO_NR(1, 1)
	IOMUX_PAD_CTRL(GPIO1_1__GPIO1_1, WEAK_PULLUP),

	/* ESDHC2  */
	IOMUX_PAD_CTRL(SD2_CMD__SD2_CMD, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_CLK__SD2_CLK, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DATA0__SD2_DATA0, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DATA1__SD2_DATA1, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DATA2__SD2_DATA2, ESDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DATA3__SD2_DATA3, ESDHC_PAD_CTRL),
#define GP_ESDHC2_SPARE	IMX_GPIO_NR(4, 30)
	IOMUX_PAD_CTRL(UART1_RTS__GPIO4_30, 0x1e5),	/* spare */
#define GPIRQ_ESDHC2	IMX_GPIO_NR(4, 31)
	IOMUX_PAD_CTRL(UART1_CTS__GPIO4_31, 0x1e5),	/* sdio_int */

	/* FEC pads */
	IOMUX_PAD_CTRL(EIM_EB2__FEC_MDIO, 0x01f5),
	IOMUX_PAD_CTRL(NANDF_CS3__FEC_MDC, 0x2004),
	IOMUX_PAD_CTRL(NANDF_RDY_INT__FEC_TX_CLK, 0x2180),
	IOMUX_PAD_CTRL(NANDF_CS2__FEC_TX_ER, 0x2004),
	IOMUX_PAD_CTRL(NANDF_CS7__FEC_TX_EN, 0x2004),
	IOMUX_PAD_CTRL(NANDF_D8__FEC_TDATA0, 0x2004),
	IOMUX_PAD_CTRL(NANDF_CS4__FEC_TDATA1, 0x2004),
	IOMUX_PAD_CTRL(NANDF_CS5__FEC_TDATA2, 0x2004),
	IOMUX_PAD_CTRL(NANDF_CS6__FEC_TDATA3, 0x2004),
	IOMUX_PAD_CTRL(NANDF_RB3__FEC_RX_CLK, 0x0180),
	IOMUX_PAD_CTRL(EIM_CS4__FEC_RX_ER, 0x0180),
	IOMUX_PAD_CTRL(NANDF_D11__FEC_RX_DV, 0x20a4),
	IOMUX_PAD_CTRL(EIM_CS5__FEC_CRS, 0x0180),
	IOMUX_PAD_CTRL(NANDF_RB2__FEC_COL, 0x0180),
	IOMUX_PAD_CTRL(NANDF_D9__FEC_RDATA0, 0x2180),
	IOMUX_PAD_CTRL(EIM_EB3__FEC_RDATA1, 0x0085),
	IOMUX_PAD_CTRL(EIM_CS2__FEC_RDATA2, 0x0085),
	IOMUX_PAD_CTRL(EIM_CS3__FEC_RDATA3, 0x0085),

	/* GPIO_KEYS */
#define GP_GPIOKEYS_1		IMX_GPIO_NR(3, 3)
	IOMUX_PAD_CTRL(NANDF_WE_B__GPIO3_3, WEAK_PULLUP),
#define GP_GPIOKEYS_2		IMX_GPIO_NR(3, 4)
	IOMUX_PAD_CTRL(NANDF_RE_B__GPIO3_4, WEAK_PULLUP),
#define GP_GPIOKEYS_3		IMX_GPIO_NR(1, 4)
	IOMUX_PAD_CTRL(GPIO1_4__GPIO1_4, WEAK_PULLUP),
#define GP_GPIOKEYS_4		IMX_GPIO_NR(3, 6)
	IOMUX_PAD_CTRL(NANDF_CLE__GPIO3_6, WEAK_PULLUP),

	/* GPIO_LEDS */
#define GP_GPIOLEDS_1		IMX_GPIO_NR(3, 7)
	IOMUX_PAD_CTRL(NANDF_WP_B__GPIO3_7, WEAK_PULLUP_OUTPUT),
#define GP_GPIOLEDS_2		IMX_GPIO_NR(3, 8)
	IOMUX_PAD_CTRL(NANDF_RB0__GPIO3_8, WEAK_PULLUP_OUTPUT),
#define GP_GPIOLEDS_3		IMX_GPIO_NR(3, 9)
	IOMUX_PAD_CTRL(NANDF_RB1__GPIO3_9, WEAK_PULLUP_OUTPUT),
#define GP_GPIOLEDS_4		IMX_GPIO_NR(3, 16)
	IOMUX_PAD_CTRL(NANDF_CS0__GPIO3_16, WEAK_PULLUP_OUTPUT),

	/* Hog */
#define GP_HOG_TP27		IMX_GPIO_NR(3, 17)
	IOMUX_PAD_CTRL(NANDF_CS1__GPIO3_17, WEAK_PULLUP),

	/* hs_i2c1 */
	IOMUX_PAD_CTRL(I2C1_CLK__I2C1_CLK, I2C_PAD_CTRL),
	IOMUX_PAD_CTRL(I2C1_DAT__I2C1_DAT, I2C_PAD_CTRL),

	IOMUX_PAD_CTRL(EIM_D27__I2C2_SCL, I2C2_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D24__I2C2_SDA, I2C2_PAD_CTRL),

	/* hs_i2c1_pic16f616 */
#define GPIRQ_PIC16F616		IMX_GPIO_NR(2, 1)
	IOMUX_PAD_CTRL(EIM_D17__GPIO2_1, WEAK_PULLUP),

	/* hs_i2c1_tfp410 */
#define GPIRQ_DVI		IMX_GPIO_NR(3, 28)
	IOMUX_PAD_CTRL(NANDF_D12__GPIO3_28, WEAK_PULLUP),
#define GP_TFP410_I2C_SEL	IMX_GPIO_NR(3, 5)
	IOMUX_PAD_CTRL(DISPB2_SER_DIN__GPIO3_5, WEAK_PULLDN),

	/* i2c1 SGTL5000 */
#define GP_SGTL5000_HP_MUTE	IMX_GPIO_NR(2, 17)
	IOMUX_PAD_CTRL(EIM_A23__GPIO2_17, WEAK_PULLDN_OUTPUT),

	/* ipu_disp1 */
	IOMUX_PAD_CTRL(DI1_DISP_CLK__DI1_DISP_CLK, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DI1_PIN15__DI1_PIN15, RGB_PAD_CTRL),	/* DRDY */
	IOMUX_PAD_CTRL(DI1_PIN2__DI1_PIN2, RGB_PAD_CTRL),	/* HSYNC */
	IOMUX_PAD_CTRL(DI1_PIN3__DI1_PIN3, RGB_PAD_CTRL),	/* VSYNC */
	IOMUX_PAD_CTRL(DISP1_DAT0__DISP1_DAT0, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT1__DISP1_DAT1, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT2__DISP1_DAT2, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT3__DISP1_DAT3, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT4__DISP1_DAT4, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT5__DISP1_DAT5, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT6__DISP1_DAT6, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT7__DISP1_DAT7, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT8__DISP1_DAT8, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT9__DISP1_DAT9, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT10__DISP1_DAT10, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT11__DISP1_DAT11, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT12__DISP1_DAT12, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT13__DISP1_DAT13, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT14__DISP1_DAT14, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT15__DISP1_DAT15, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT16__DISP1_DAT16, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT17__DISP1_DAT17, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT18__DISP1_DAT18, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT19__DISP1_DAT19, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT20__DISP1_DAT20, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT21__DISP1_DAT21, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT22__DISP1_DAT22, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP1_DAT23__DISP1_DAT23, RGB_PAD_CTRL),

	/* ipu_disp2 */
	IOMUX_PAD_CTRL(DI2_DISP_CLK__DI2_DISP_CLK, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DI_GP4__DI2_PIN15, RGB_PAD_CTRL),	/* DRDY */
	IOMUX_PAD_CTRL(DISP2_DAT0__DISP2_DAT0, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT1__DISP2_DAT1, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT2__DISP2_DAT2, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT3__DISP2_DAT3, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT4__DISP2_DAT4, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT5__DISP2_DAT5, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT6__DISP2_DAT6, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT7__DISP2_DAT7, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT8__DISP2_DAT8, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT9__DISP2_DAT9, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT10__DISP2_DAT10, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT11__DISP2_DAT11, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT12__DISP2_DAT12, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT13__DISP2_DAT13, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT14__DISP2_DAT14, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP2_DAT15__DISP2_DAT15, RGB_PAD_CTRL),

	/* PMIC */
#define GPIRQ_PMIC	IMX_GPIO_NR(1, 8)
	IOMUX_PAD_CTRL(GPIO1_8__GPIO1_8, WEAK_PULLUP),

	/* PWM1 - lcd backlight */
#define GP_LCD_BACKLIGHT	IMX_GPIO_NR(1, 2)
	IOMUX_PAD_CTRL(GPIO1_2__GPIO1_2, WEAK_PULLDN_OUTPUT),

	/* reg3p3v */
#define GP_REG3P3V_EN		IMX_GPIO_NR(2, 6)
	IOMUX_PAD_CTRL(EIM_D22__GPIO2_6, WEAK_PULLUP_OUTPUT),

	/* UART1  */
	IOMUX_PAD_CTRL(UART1_TXD__UART1_TXD, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(UART1_RXD__UART1_RXD, UART_PAD_CTRL),

	/* UART2 */
	IOMUX_PAD_CTRL(UART2_TXD__UART2_TXD, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(UART2_RXD__UART2_RXD, UART_PAD_CTRL),

	/* UART3 */
	IOMUX_PAD_CTRL(UART3_TXD__UART3_TXD, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(UART3_RXD__UART3_RXD, UART_PAD_CTRL),

	/* USBH1 */
	IOMUX_PAD_CTRL(USBH1_CLK__USBH1_CLK, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_DIR__USBH1_DIR, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_NXT__USBH1_NXT, 0x1e5),
#define GP_USBH1_STP		IMX_GPIO_NR(1, 27)
	IOMUX_PAD_CTRL(USBH1_STP__GPIO1_27, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_DATA0__USBH1_DATA0, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_DATA1__USBH1_DATA1, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_DATA2__USBH1_DATA2, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_DATA3__USBH1_DATA3, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_DATA4__USBH1_DATA4, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_DATA5__USBH1_DATA5, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_DATA6__USBH1_DATA6, 0x1e5),
	IOMUX_PAD_CTRL(USBH1_DATA7__USBH1_DATA7, 0x1e5),
#define GP_USBH1_RESET		IMX_GPIO_NR(2, 5)
	IOMUX_PAD_CTRL(EIM_D21__GPIO2_5, WEAK_PULLDN_OUTPUT),

};

static const iomux_v3_cfg_t usbh1_stp_gpio[] = {
	IOMUX_PAD_CTRL(USBH1_STP__GPIO1_27, 0x1e5),
};

static const iomux_v3_cfg_t usbh1_stp_usb[] = {
	IOMUX_PAD_CTRL(USBH1_STP__USBH1_STP, 0x1e5),
};

static const iomux_v3_cfg_t usbotg_power_on_pads[] = {
	IOMUX_PAD_CTRL(EIM_D26__KEY_COL7, WEAK_PULLDN_OUTPUT),		/* low is on */
};

static const iomux_v3_cfg_t usbotg_power_off_pads[] = {
	IOMUX_PAD_CTRL(EIM_D26__KEY_COL7, WEAK_PULLUP_OUTPUT),		/* high is off */
};

/*
 *
 */
static struct i2c_pads_info i2c_pads[] = {
	/* I2C1, SGTL5000 */
	I2C_PADS_INFO_ENTRY(I2C1, EIM_D19, 2, 3, EIM_D16, 2, 0, I2C_PAD_CTRL),
	/* I2C2 */
	I2C_PADS_INFO_ENTRY(I2C2, EIM_D27, 2, 9, EIM_D24, 2, 8, I2C2_PAD_CTRL),
};

int dram_init(void)
{
	gd->ram_size = ((ulong)CONFIG_DDR_MB * 1024 * 1024);
	return 0;
}

#ifdef CONFIG_USB_EHCI_MX5
int board_ehci_hcd_init(int port)
{
	if (port) {
		/* Set USBH1_STP to GPIO and toggle it */
		gpio_set_value(GP_USBH1_RESET, 0);
		gpio_set_value(GP_USBH1_STP, 0);
		SETUP_IOMUX_PADS(usbh1_stp_gpio);
		mdelay(10);
		gpio_set_value(GP_USBH1_STP, 1);
		SETUP_IOMUX_PADS(usbh1_stp_usb);
		mdelay(2);

		gpio_set_value(GP_USBH1_RESET, 1);
		return 0;
	}
	SETUP_IOMUX_PADS(usbotg_power_on_pads);
	return 0;
}

#endif

#define REV_ATLAS_LITE_1_0         0x8
#define REV_ATLAS_LITE_1_1         0x9
#define REV_ATLAS_LITE_2_0         0x10
#define REV_ATLAS_LITE_2_1         0x11

#define SREV3_0 0x10

static unsigned get_srev(void)
{
	struct iim_regs *piim = (struct iim_regs *)IIM_BASE_ADDR;
	return readl(&piim->srev);
}

static void power_init(void)
{
	unsigned val, sw1, sw2, sw3;
	unsigned sw1_volt, sw2_volt, sw3_volt;
	unsigned mode;
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)MXC_CCM_BASE;
	struct pmic *p;
	int ret;

	ret = pmic_init(CONFIG_FSL_PMIC_BUS);
	if (ret)
		return;

	p = pmic_get("FSL_PMIC");
	if (!p)
		return;

	/* Write needed to Power Gate 2 register */
	pmic_reg_read(p, REG_POWER_MISC, &val);
	val &= ~PWGT2SPIEN;
	pmic_reg_write(p, REG_POWER_MISC, val);

	/* Externally powered */
	pmic_reg_read(p, REG_CHARGE, &val);
	val |= ICHRG0 | ICHRG1 | ICHRG2 | ICHRG3 | CHGAUTOB;
	pmic_reg_write(p, REG_CHARGE, val);

	/* power up the system first */
	pmic_reg_write(p, REG_POWER_MISC, PWUP);

	/* Set core voltage to 1.1V */
	sw1_volt = SWx_1_100V;
	/* Setup VCC (SW2) to 1.25 */
	sw2_volt = SWx_1_250V;
	/* Setup 1V2_DIG1 (SW3) to 1.275 */
	sw3_volt = SWx_1_275V;

	pmic_reg_read(p, REG_SW_0, &sw1);
	sw1 = (sw1 & ~SWx_VOLT_MASK) | sw1_volt;
	pmic_reg_write(p, REG_SW_0, sw1);

	pmic_reg_read(p, REG_SW_1, &sw2);
	sw2 = (sw2 & ~SWx_VOLT_MASK) | sw2_volt;
	pmic_reg_write(p, REG_SW_1, sw2);

	pmic_reg_read(p, REG_SW_2, &sw3);
	sw3 = (sw3 & ~SWx_VOLT_MASK) | sw3_volt;
	pmic_reg_write(p, REG_SW_2, sw3);

	if (get_srev() < SREV3_0) {
		udelay(50);

		/* Raise the core frequency to 800MHz */
		writel(CONFIG_SYS_ARM_PODF, &mxc_ccm->cacrr);
	}

	pmic_reg_read(p, REG_IDENTIFICATION, &val);
	if (((val & 0x1f) < REV_ATLAS_LITE_2_0) || (((val >> 9) & 0x3) == 0)) {
		mode = SWMODE_PWM_PWM;
	} else {
		mode = SWMODE_AUTO_AUTO;
	}
	/* Set switchers in Auto in NORMAL mode & STANDBY mode */
	/* Setup the switcher mode for SW1 & SW2*/
	pmic_reg_read(p, REG_SW_4, &val);
	val &= ~((SWMODE_MASK << SWMODE1_SHIFT) |
		(SWMODE_MASK << SWMODE2_SHIFT));
	val |= (mode << SWMODE1_SHIFT) |
		(mode << SWMODE2_SHIFT);
	pmic_reg_write(p, REG_SW_4, val);

	/* Setup the switcher mode for SW3 & SW4 */
	pmic_reg_read(p, REG_SW_5, &val);
	val &= ~((SWMODE_MASK << SWMODE3_SHIFT) |
		(SWMODE_MASK << SWMODE4_SHIFT));
	val |= (mode << SWMODE3_SHIFT) |
		(mode << SWMODE4_SHIFT);
	pmic_reg_write(p, REG_SW_5, val);

	/* Set VDIG to 1.65V, VGEN3 to 1.8V, VCAM to 2.5V */
	pmic_reg_read(p, REG_SETTING_0, &val);
	val &= ~(VCAM_MASK | VGEN3_MASK | VDIG_MASK);
	val |= VDIG_1_65 | VGEN3_1_8 | VCAM_2_5;
	pmic_reg_write(p, REG_SETTING_0, val);

	/* Set VVIDEO to 2.775V, VAUDIO to 3V, VSD to 3.15V */
	pmic_reg_read(p, REG_SETTING_1, &val);
	val &= ~(VVIDEO_MASK | VSD_MASK | VAUDIO_MASK);
	val |= VSD_3_15 | VAUDIO_3_0 | VVIDEO_2_775;
	pmic_reg_write(p, REG_SETTING_1, val);

	/* Configure VGEN3 and VCAM regulators to use external PNP */
	val = VGEN3CONFIG | VCAMCONFIG;
	pmic_reg_write(p, REG_MODE_1, val);
	udelay(200);

	/* Enable VGEN3, VCAM, VAUDIO, VVIDEO, VSD regulators */
	val = VGEN3EN | VGEN3CONFIG | VCAMEN | VCAMCONFIG |
		VVIDEOEN | VAUDIOEN  | VSDEN;
	pmic_reg_write(p, REG_MODE_1, val);

	udelay(500);
}

#ifdef CONFIG_FSL_ESDHC

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;

	if (cfg->esdhc_base == MMC_SDHC2_BASE_ADDR)
		return 1;	/* always present */
	return !gpio_get_value(GP_ESDHC1_CD);
}

static struct fsl_esdhc_cfg esdhc_cfg[] = {
	{.esdhc_base = MMC_SDHC1_BASE_ADDR, .max_bus_width = 4},
	{.esdhc_base = MMC_SDHC2_BASE_ADDR, .max_bus_width = 4},
};

int board_mmc_init(bd_t *bis)
{
	int ret;
	u32 index = 0;

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM; ++index) {
		switch (index) {
		case 0:
			break;
		case 1:
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) then supported by the board (%d)\n",
				index + 1, CONFIG_SYS_FSL_ESDHC_NUM);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
		if (ret)
			return ret;
	}
	return 0;
}
#endif

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if (bus == 0 && cs == 0)
		return GP_ECSPI1_PMIC;
	if (bus == 0 && cs == 1)
		return GP_ECSPI1_FLASH;
	return -1;
}
#endif


int board_eth_init(bd_t *bis)
{
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

#ifdef CONFIG_FEC_MXC
	bus = fec_get_miibus(base, -1);
	if (!bus)
		return 0;
	/* scan phy 4,5,6,7 */
	phydev = phy_find_by_mask(bus, (0xf << 4), PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		free(bus);
		return 0;
	}
	printf("using phy at %d\n", phydev->addr);
	ret  = fec_probe(bis, -1, base, bus, phydev);
	if (ret) {
		printf("FEC MXC: %s:failed\n", __func__);
		free(phydev);
		free(bus);
	}
#endif
#ifdef CONFIG_CI_UDC
	/* For otg ethernet*/
	if (!getenv("eth1addr"))
		setenv("eth1addr", getenv("usbnet_devaddr"));
	usb_eth_initialize(bis);
#endif
	return 0;
}

#ifdef CONFIG_CMD_FBPANEL
void board_enable_lcd(const struct display_info_t *di, int enable)
{
	gpio_direction_output(GP_LCD_BACKLIGHT, enable);
}

static const struct display_info_t displays[] = {
	/* PIC16F616 */
	VD_HITACHI_HVGA565(LCD2, NULL, 0, 0x38),
	/* PMIC touch controller */
	VD_800X300_565(LCD2, NULL, 0, 0x38),
	VD_OKAYA_480_272(LCD, NULL, 0, 0x48),
};
#endif

static const unsigned short gpios_out_low[] = {
	GP_ECSPI1_PMIC,
	GP_SGTL5000_HP_MUTE,
	GP_LCD_BACKLIGHT,
	GP_USBH1_RESET,
	GP_USBH1_STP,
	GP_TFP410_I2C_SEL,
};

static const unsigned short gpios_out_high[] = {
	GP_ECSPI1_FLASH,	/* SS1 of spi nor */
	GP_GPIOLEDS_1,
	GP_GPIOLEDS_2,
	GP_GPIOLEDS_3,
	GP_GPIOLEDS_4,
	GP_REG3P3V_EN,
};

static const unsigned short gpios_in[] = {
	GP_ESDHC1_CD,
	GP_ESDHC1_WP,
	GP_ESDHC2_SPARE,
	GPIRQ_ESDHC2,
	GP_GPIOKEYS_1,
	GP_GPIOKEYS_2,
	GP_GPIOKEYS_3,
	GP_GPIOKEYS_4,
	GP_HOG_TP27,
	GPIRQ_PIC16F616,
	GPIRQ_DVI,
	GPIRQ_PMIC,

};

static void set_gpios_in(const unsigned short *p, int cnt)
{
	int i;

	for (i = 0; i < cnt; i++)
		gpio_direction_input(*p++);
}

static void set_gpios(const unsigned short *p, int cnt, int val)
{
	int i;

	for (i = 0; i < cnt; i++)
		gpio_direction_output(*p++, val);
}

int board_early_init_f(void)
{
	set_gpios_in(gpios_in, ARRAY_SIZE(gpios_in));
	set_gpios(gpios_out_high, ARRAY_SIZE(gpios_out_high), 1);
	set_gpios(gpios_out_low, ARRAY_SIZE(gpios_out_low), 0);
	SETUP_IOMUX_PADS(init_pads);
	SETUP_IOMUX_PADS(usbotg_power_off_pads);
	return 0;
}

#ifdef CONFIG_VIDEO
/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}
#endif

int board_init(void)
{
	int i;
	struct i2c_pads_info *p = i2c_pads + i2c_get_info_entry_offset();

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
#if 1
	for (i = 0; i < 2; i++) {
		setup_i2c(i, CONFIG_SYS_I2C_SPEED, 0x7f, p);
		p += I2C_PADS_INFO_ENTRY_SPACING;
	}
#endif
#ifdef CONFIG_CMD_FBPANEL
	fbp_setup_display(displays, ARRAY_SIZE(displays));
#endif
	return 0;
}

int checkboard(void)
{
	puts("Board: Boundary nitrogen51_vm\n");
	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{"mmc1",	MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
	{NULL,		0},
};
#endif

int misc_init_r(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	return 0;
}

int board_late_init(void)
{
	int cpurev = get_cpu_rev();

#ifdef CONFIG_MXC_SPI
	power_init();
#endif
	setenv("cpu", get_imx_type((cpurev & 0xFF000) >> 12));
	if (!getenv("board"))
		setenv("board", "nitrogen51_vm");
	setenv("uboot_defconfig", CONFIG_DEFCONFIG);
	return 0;
}