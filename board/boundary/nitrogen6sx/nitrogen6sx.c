/*
 * Copyright (C) 2015 Boundary Devices, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/fbpanel.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/io.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <linux/sizes.h>
#include <malloc.h>
#include <mmc.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include "../common/bd_common.h"
#include "../common/padctrl.h"

DECLARE_GLOBAL_DATA_PTR;

#define AUD_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define CSI_PAD_CTL	PAD_CTL_DSE_120ohm


#define I2C_PAD_CTRL    (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_ODE)

#define LCDIF_PAD_CTL	PAD_CTL_DSE_120ohm

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define USDHC2_PAD_CTRL (PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define USDHC2_CLK_PAD_CTRL (PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define USDHC_CLK_PAD_CTRL (PAD_CTL_SPEED_LOW | \
	PAD_CTL_DSE_80ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)

static const iomux_v3_cfg_t init_pads[] = {
	/* Audmux */
	IOMUX_PAD_CTRL(SD1_DATA0__AUDMUX_AUD5_RXD, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DATA1__AUDMUX_AUD5_TXC, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DATA2__AUDMUX_AUD5_TXFS, AUD_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DATA3__AUDMUX_AUD5_TXD, AUD_PAD_CTRL),

#define GP_BT_RFKILL_RESET	IMX_GPIO_NR(2, 17)
	IOMUX_PAD_CTRL(KEY_ROW2__GPIO2_IO_17, WEAK_PULLDN_OUTPUT),

	/* CSI */
	IOMUX_PAD_CTRL(CSI_MCLK__CSI1_MCLK, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_PIXCLK__CSI1_PIXCLK, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_HSYNC__CSI1_HSYNC, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_VSYNC__CSI1_VSYNC, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_DATA00__CSI1_DATA_2, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_DATA01__CSI1_DATA_3, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_DATA02__CSI1_DATA_4, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_DATA03__CSI1_DATA_5, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_DATA04__CSI1_DATA_6, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_DATA05__CSI1_DATA_7, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_DATA06__CSI1_DATA_8, CSI_PAD_CTL),
	IOMUX_PAD_CTRL(CSI_DATA07__CSI1_DATA_9, CSI_PAD_CTL),
#define GP_OV5642_RESET		IMX_GPIO_NR(4, 2)
	IOMUX_PAD_CTRL(NAND_CE1_B__GPIO4_IO_2, WEAK_PULLDN_OUTPUT),
#define GP_OV5642_POWER_DOWN	IMX_GPIO_NR(4, 0)
	IOMUX_PAD_CTRL(NAND_ALE__GPIO4_IO_0, WEAK_PULLUP_OUTPUT),

	/* ECSPI1 (serial nor eeprom) */
	IOMUX_PAD_CTRL(KEY_COL1__ECSPI1_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(KEY_ROW0__ECSPI1_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(KEY_COL0__ECSPI1_SCLK, SPI_PAD_CTRL),
#define GP_ECSPI1_NOR_CS	IMX_GPIO_NR(2, 16)
	IOMUX_PAD_CTRL(KEY_ROW1__GPIO2_IO_16, WEAK_PULLUP),

	/* enet phy */
	IOMUX_PAD_CTRL(ENET1_MDC__ENET1_MDC, WEAK_PULLUP),
	IOMUX_PAD_CTRL(ENET1_MDIO__ENET1_MDIO, WEAK_PULLUP),

	/* fec1 */
	IOMUX_PAD_CTRL(RGMII1_TD0__ENET1_TX_DATA_0, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII1_TD1__ENET1_TX_DATA_1, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII1_TD2__ENET1_TX_DATA_2, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII1_TD3__ENET1_TX_DATA_3, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII1_TXC__ENET1_RGMII_TXC, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII1_TX_CTL__ENET1_TX_EN, PAD_CTRL_ENET_TX),
	/* AR8035 PHY Reset */
#define GP_RGMII_PHY_RESET	IMX_GPIO_NR(2, 7)
	IOMUX_PAD_CTRL(ENET2_CRS__GPIO2_IO_7, WEAK_PULLUP),
#define GP_RGMII_PHY_INT	IMX_GPIO_NR(2, 4)
	IOMUX_PAD_CTRL(ENET1_RX_CLK__GPIO2_IO_4, WEAK_PULLUP),
	IOMUX_PAD_CTRL(ENET1_TX_CLK__GPIO2_IO_5, WEAK_PULLUP),

	/* fec2 */
	IOMUX_PAD_CTRL(RGMII2_TD0__ENET2_TX_DATA_0, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII2_TD1__ENET2_TX_DATA_1, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII2_TD2__ENET2_TX_DATA_2, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII2_TD3__ENET2_TX_DATA_3, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII2_TXC__ENET2_RGMII_TXC, PAD_CTRL_ENET_TX),
	IOMUX_PAD_CTRL(RGMII2_TX_CTL__ENET2_TX_EN, PAD_CTRL_ENET_TX),
	/* AR8035 PHY Reset */
#define GP_RGMII2_PHY_RESET	IMX_GPIO_NR(2, 6)
	IOMUX_PAD_CTRL(ENET2_COL__GPIO2_IO_6, WEAK_PULLUP),
#define GP_RGMII2_PHY_INT	IMX_GPIO_NR(2, 8)
	IOMUX_PAD_CTRL(ENET2_RX_CLK__GPIO2_IO_8, WEAK_PULLUP),
	IOMUX_PAD_CTRL(ENET2_TX_CLK__GPIO2_IO_9, WEAK_PULLUP),

	/* flexcan1 */
	IOMUX_PAD_CTRL(QSPI1B_DQS__CAN1_TX, WEAK_PULLUP),
	IOMUX_PAD_CTRL(QSPI1A_SS1_B__CAN1_RX, WEAK_PULLUP),
#define GP_CAN1_STANDBY		IMX_GPIO_NR(4, 27)
	IOMUX_PAD_CTRL(QSPI1B_DATA3__GPIO4_IO_27, WEAK_PULLUP),

	/* flexcan2 */
	IOMUX_PAD_CTRL(QSPI1A_DQS__CAN2_TX, WEAK_PULLUP),
	IOMUX_PAD_CTRL(QSPI1B_SS1_B__CAN2_RX, WEAK_PULLUP),
#define GP_CAN2_STANDBY		IMX_GPIO_NR(4, 24)
	IOMUX_PAD_CTRL(QSPI1B_DATA0__GPIO4_IO_24, WEAK_PULLUP),

	/* hogs - expanders */
	IOMUX_PAD_CTRL(NAND_CE0_B__GPIO4_IO_1, WEAK_PULLUP),
	IOMUX_PAD_CTRL(NAND_WE_B__GPIO4_IO_14, WEAK_PULLUP),
	IOMUX_PAD_CTRL(NAND_WP_B__GPIO4_IO_15, WEAK_PULLUP),
	IOMUX_PAD_CTRL(NAND_READY_B__GPIO4_IO_13, WEAK_PULLUP),
	IOMUX_PAD_CTRL(QSPI1A_DATA0__GPIO4_IO_16, WEAK_PULLUP),
	IOMUX_PAD_CTRL(QSPI1A_DATA1__GPIO4_IO_17, WEAK_PULLUP),
	IOMUX_PAD_CTRL(QSPI1A_DATA2__GPIO4_IO_18, WEAK_PULLUP),
	IOMUX_PAD_CTRL(QSPI1A_DATA3__GPIO4_IO_19, WEAK_PULLUP),
#define GP_WIFI_PASS	IMX_GPIO_NR(4, 6)
	IOMUX_PAD_CTRL(NAND_DATA02__GPIO4_IO_6, WEAK_PULLDN_OUTPUT),
#define GP_WIFI_FAIL	IMX_GPIO_NR(4, 4)
	IOMUX_PAD_CTRL(NAND_DATA00__GPIO4_IO_4, WEAK_PULLDN_OUTPUT),
#define GP_WIFI_TEST	IMX_GPIO_NR(4, 7)
	IOMUX_PAD_CTRL(NAND_DATA03__GPIO4_IO_7, WEAK_PULLUP),

	/* hogs - Test points */
	IOMUX_PAD_CTRL(NAND_DATA04__GPIO4_IO_8, WEAK_PULLUP),

	/* I2C2 */
#define GPIRQ_I2C2_TFP410	IMX_GPIO_NR(4, 3)
	IOMUX_PAD_CTRL(NAND_CLE__GPIO4_IO_3, WEAK_PULLUP),
#define GP_I2C2_TFP410_I2C_SEL	IMX_GPIO_NR(4, 12)
	IOMUX_PAD_CTRL(NAND_RE_B__GPIO4_IO_12, WEAK_PULLUP),

	/* I2C3 */
#define GPIRQ_I2C3_J18	IMX_GPIO_NR(4, 25)
	IOMUX_PAD_CTRL(QSPI1B_DATA1__GPIO4_IO_25, WEAK_PULLUP),

	/* LVDS */
#define GP_LVDS_BKL_EN	IMX_GPIO_NR(4, 21)
	IOMUX_PAD_CTRL(QSPI1A_SCLK__GPIO4_IO_21, WEAK_PULLDN),

	/* PCIe */
#define GP_PCIE_WAKE	IMX_GPIO_NR(4, 9)
	IOMUX_PAD_CTRL(NAND_DATA05__GPIO4_IO_9, WEAK_PULLUP),
#define GP_PCIE_RESET	IMX_GPIO_NR(4, 10)
	IOMUX_PAD_CTRL(NAND_DATA06__GPIO4_IO_10, WEAK_PULLUP),
#define GP_PCIE_DISABLE	IMX_GPIO_NR(4, 11)
	IOMUX_PAD_CTRL(NAND_DATA07__GPIO4_IO_11, WEAK_PULLUP),

	/* PWM4 - for LVDS panel */
#define GP_BACKLIGHT_LVDS	IMX_GPIO_NR(1, 13)
	IOMUX_PAD_CTRL(GPIO1_IO13__GPIO1_IO_13, WEAK_PULLDN_OUTPUT),

	/* reg_wlan */
#define GP_REG_WLAN_EN		IMX_GPIO_NR(7, 6)
	IOMUX_PAD_CTRL(SD3_DATA4__GPIO7_IO_6, WEAK_PULLDN_OUTPUT),
	IOMUX_PAD_CTRL(GPIO1_IO11__CCM_CLKO1, OUTPUT_40OHM),

#define GP_REG_WIFI_1P8V_EN	IMX_GPIO_NR(4, 5)
	IOMUX_PAD_CTRL(NAND_DATA01__GPIO4_IO_5, WEAK_PULLDN_OUTPUT),

#define GP_REG_WIFI_3P3V_EN	IMX_GPIO_NR(6, 1)
	IOMUX_PAD_CTRL(SD1_CMD__GPIO6_IO_1, WEAK_PULLDN_OUTPUT),

	/* sgtl5000 */
	IOMUX_PAD_CTRL(GPIO1_IO12__CCM_CLKO2, OUTPUT_40OHM),
#define GP_SGTL5000_HP_DETECT	IMX_GPIO_NR(2, 0)
	IOMUX_PAD_CTRL(ENET1_COL__GPIO2_IO_0, WEAK_PULLUP),
#define GP_SGTL5000_MIC_DETECT	IMX_GPIO_NR(2, 1)
	IOMUX_PAD_CTRL(ENET1_CRS__GPIO2_IO_1, WEAK_PULLUP),
#define GP_SGTL5000_MUTE		IMX_GPIO_NR(4, 22)
	IOMUX_PAD_CTRL(QSPI1A_SS0_B__GPIO4_IO_22, WEAK_PULLDN_OUTPUT),

	/* uart1 */
	IOMUX_PAD_CTRL(GPIO1_IO04__UART1_TX, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(GPIO1_IO05__UART1_RX, UART_PAD_CTRL),

	/* uart2 */
	IOMUX_PAD_CTRL(GPIO1_IO06__UART2_TX, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(GPIO1_IO07__UART2_RX, UART_PAD_CTRL),

	/* uart3 */
	IOMUX_PAD_CTRL(QSPI1B_SS0_B__UART3_TX, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(QSPI1B_SCLK__UART3_RX, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DATA7__UART3_CTS_B, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DATA6__UART3_RTS_B, UART_PAD_CTRL),

	/* uart5 */
	IOMUX_PAD_CTRL(KEY_COL3__UART5_TX, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(KEY_ROW3__UART5_RX, UART_PAD_CTRL),

	/* USB OTG1 */
	IOMUX_PAD_CTRL(GPIO1_IO08__USB_OTG1_OC, WEAK_PULLUP),
	IOMUX_PAD_CTRL(GPIO1_IO10__ANATOP_OTG1_ID, WEAK_PULLUP),

	/* USB OTG1 vbus */
#define GP_USB_OTG1_PWR		IMX_GPIO_NR(1, 9)
	IOMUX_PAD_CTRL(GPIO1_IO09__GPIO1_IO_9, WEAK_PULLDN_OUTPUT),

	/* USB OTG2 */
	/* USB Hub Reset for USB2513 3 port hub */
#define GP_USB_HUB_RESET	IMX_GPIO_NR(4, 26)
	IOMUX_PAD_CTRL(QSPI1B_DATA2__GPIO4_IO_26, OUTPUT_40OHM),

	/* usdhc2 - micro SD */
	IOMUX_PAD_CTRL(SD2_CLK__USDHC2_CLK, USDHC2_CLK_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_CMD__USDHC2_CMD, USDHC2_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DATA0__USDHC2_DATA0, USDHC2_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DATA1__USDHC2_DATA1, USDHC2_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DATA2__USDHC2_DATA2, USDHC2_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DATA3__USDHC2_DATA3, USDHC2_PAD_CTRL),
#define GP_USDHC2_CD	IMX_GPIO_NR(2, 12)
	IOMUX_PAD_CTRL(KEY_COL2__GPIO2_IO_12, WEAK_PULLUP),

	/* usdhc3 - wifi */
	IOMUX_PAD_CTRL(SD3_CLK__USDHC3_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_CMD__USDHC3_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DATA0__USDHC3_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DATA1__USDHC3_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DATA2__USDHC3_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DATA3__USDHC3_DATA3, USDHC_PAD_CTRL),

	/* usdhc4 - eMMC */
	IOMUX_PAD_CTRL(SD4_CLK__USDHC4_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_CMD__USDHC4_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_RESET_B__USDHC4_RESET_B, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DATA0__USDHC4_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DATA1__USDHC4_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DATA2__USDHC4_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DATA3__USDHC4_DATA3, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DATA4__USDHC4_DATA4, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DATA5__USDHC4_DATA5, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DATA6__USDHC4_DATA6, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DATA7__USDHC4_DATA7, USDHC_PAD_CTRL),

#define GPIRQ_WLAN	IMX_GPIO_NR(7, 7)
	IOMUX_PAD_CTRL(SD3_DATA5__GPIO7_IO_7, WEAK_PULLUP),
};

#ifdef CONFIG_CMD_FBPANEL
static const iomux_v3_cfg_t rgb_pads[] = {
	/* LCDIF1 */
	IOMUX_PAD_CTRL(LCD1_CLK__LCDIF1_CLK, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_ENABLE__LCDIF1_ENABLE, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_HSYNC__LCDIF1_HSYNC, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_VSYNC__LCDIF1_VSYNC, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_RESET__LCDIF1_RESET, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA00__LCDIF1_DATA_0, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA01__LCDIF1_DATA_1, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA02__LCDIF1_DATA_2, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA03__LCDIF1_DATA_3, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA04__LCDIF1_DATA_4, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA05__LCDIF1_DATA_5, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA06__LCDIF1_DATA_6, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA07__LCDIF1_DATA_7, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA08__LCDIF1_DATA_8, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA09__LCDIF1_DATA_9, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA10__LCDIF1_DATA_10, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA11__LCDIF1_DATA_11, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA12__LCDIF1_DATA_12, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA13__LCDIF1_DATA_13, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA14__LCDIF1_DATA_14, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA15__LCDIF1_DATA_15, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA16__LCDIF1_DATA_16, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA17__LCDIF1_DATA_17, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA18__LCDIF1_DATA_18, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA19__LCDIF1_DATA_19, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA20__LCDIF1_DATA_20, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA21__LCDIF1_DATA_21, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA22__LCDIF1_DATA_22, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA23__LCDIF1_DATA_23, LCDIF_PAD_CTL),
};
#endif

static const iomux_v3_cfg_t rgb_gpio_pads[] = {
	/* LCDIF1 */
	IOMUX_PAD_CTRL(LCD1_CLK__GPIO3_IO_0, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_ENABLE__GPIO3_IO_25, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_HSYNC__GPIO3_IO_26, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_VSYNC__GPIO3_IO_28, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_RESET__GPIO3_IO_27, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA00__GPIO3_IO_1, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA01__GPIO3_IO_2, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA02__GPIO3_IO_3, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA03__GPIO3_IO_4, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA04__GPIO3_IO_5, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA05__GPIO3_IO_6, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA06__GPIO3_IO_7, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA07__GPIO3_IO_8, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA08__GPIO3_IO_9, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA09__GPIO3_IO_10, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA10__GPIO3_IO_11, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA11__GPIO3_IO_12, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA12__GPIO3_IO_13, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA13__GPIO3_IO_14, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA14__GPIO3_IO_15, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA15__GPIO3_IO_16, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA16__GPIO3_IO_17, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA17__GPIO3_IO_18, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA18__GPIO3_IO_19, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA19__GPIO3_IO_20, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA20__GPIO3_IO_21, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA21__GPIO3_IO_22, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA22__GPIO3_IO_23, LCDIF_PAD_CTL),
	IOMUX_PAD_CTRL(LCD1_DATA23__GPIO3_IO_24, LCDIF_PAD_CTL),
};

static const struct i2c_pads_info i2c_pads[] = {
	/* I2C1, rv4162 */
	I2C_PADS_INFO_ENTRY(I2C1, GPIO1_IO00, 1, 0, GPIO1_IO01, 1, 1, I2C_PAD_CTRL),
	I2C_PADS_INFO_ENTRY(I2C2, GPIO1_IO02, 1, 2, GPIO1_IO03, 1, 3, I2C_PAD_CTRL),
	I2C_PADS_INFO_ENTRY(I2C3, KEY_COL4, 2, 14, KEY_ROW4, 2, 19, I2C_PAD_CTRL),
};
#define I2C_BUS_CNT	3

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? GP_ECSPI1_NOR_CS : (cs >> 8) ? (cs >> 8) : -1;
}
#endif

#ifdef CONFIG_USB_EHCI_MX6
#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		(1 << 9)

int board_usb_phy_mode(int port)
{
	if (port == 1)
		return USB_INIT_HOST;
	else
		return usb_phy_mode(port);
}

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1)
		return -EINVAL;
	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
			port * 4);
	setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

	/* Reset USB hub */
	gpio_direction_output(GP_USB_HUB_RESET, 0);
	mdelay(2);
	gpio_set_value(GP_USB_HUB_RESET, 1);
	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		return 0;
	gpio_set_value(GP_USB_OTG1_PWR, on);
	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg board_usdhc_cfg[] = {
	{.esdhc_base = USDHC2_BASE_ADDR, .bus_width = 4,
			.gp_cd = GP_USDHC2_CD},
	{.esdhc_base = USDHC4_BASE_ADDR, .bus_width = 8,},
};
#endif

#ifdef CONFIG_CMD_FBPANEL
void board_enable_lvds(const struct display_info_t *di, int enable)
{
	gpio_direction_output(GP_BACKLIGHT_LVDS, enable);
}

void board_enable_lcd(const struct display_info_t *di, int enable)
{
	unsigned tfp410_i2c_addr = 0x38;
	unsigned gp = GP_I2C2_TFP410_I2C_SEL;
	int ret;
	u8 orig_i2c_bus;
	u8 val8;

	orig_i2c_bus = i2c_get_bus_num();
	i2c_set_bus_num(1);
	if (enable) {
		//tfp410 low to high of sel is reset, then i2c_mode
		gpio_set_value(gp, 0);
		udelay(5);
		gpio_set_value(gp, 1);
		SETUP_IOMUX_PADS(rgb_pads);

		val8 = 0xbd;	/* ON */
		ret = i2c_write(tfp410_i2c_addr, 0x8, 1, &val8, 1);
		if (ret) {
			/* On i2c failure, put back into non-i2c mode */
			gpio_set_value(gp, 0);
		}
	} else {
		val8 = 0xbc;	/* OFF */
		i2c_write(tfp410_i2c_addr, 0x8, 1, &val8, 1);
		SETUP_IOMUX_PADS(rgb_gpio_pads);
	}
	i2c_set_bus_num(orig_i2c_bus);
}

static const struct display_info_t displays[] = {
	/* hdmi/lcd via tfp410 */
	VDF_1280_720M_60(LCD, "1280x720M@60", RGB24, 0, fbp_detect_i2c, 2, 0x50),
	VDF_1920_1080M_60(LCD, "1920x1080M@60", RGB24, 0, NULL, 2, 0x50),
	VDF_1024_768M_60(LCD, "1024x768M@60", RGB24, 0, NULL, 2, 0x50),

	/* ft5x06 */
	VD_HANNSTAR7(LVDS, fbp_detect_i2c, fbp_bus_gp(2, 0, GP_LVDS_BKL_EN, 0), 0x38),
	VD_AUO_B101EW05(LVDS, NULL, fbp_bus_gp(2, 0, GP_LVDS_BKL_EN, 0), 0x38),
	VD_LG1280_800(LVDS, NULL, fbp_bus_gp(2, 0, GP_LVDS_BKL_EN, 0), 0x38),
	VD_M101NWWB(LVDS, NULL, fbp_bus_gp(2, 0, GP_LVDS_BKL_EN, 0), 0x38),
	VD_DT070BTFT(LVDS, NULL, fbp_bus_gp(2, 0, GP_LVDS_BKL_EN, 0), 0x38),
	VD_WSVGA(LVDS, NULL, fbp_bus_gp(2, 0, GP_LVDS_BKL_EN, 0), 0x38),

	/* ili210x */
	VD_AMP1024_600(LVDS, fbp_detect_i2c, fbp_bus_gp(2, 0, GP_LVDS_BKL_EN, 0), 0x41),

	/* egalax_ts */
	VD_HANNSTAR(LVDS, fbp_detect_i2c, fbp_bus_gp(2, 0, GP_LVDS_BKL_EN, 0), 0x04),
	VD_LG9_7(LVDS, NULL, fbp_bus_gp(2, 0, GP_LVDS_BKL_EN, 0), 0x04),

	/* fusion7 specific touchscreen */
	VDF_FUSION7(LCD, "fusion7", RGB666, 0, fbp_detect_i2c, 2, 0x10),

	VD_SHARP_LQ101K1LY04(LVDS, NULL, fbp_bus_gp(0, 0, GP_LVDS_BKL_EN, 0), 0x00),
	VD_WXGA_J(LVDS, NULL, fbp_bus_gp(0, 0, GP_LVDS_BKL_EN, 0), 0x00),
	VD_WXGA(LVDS, NULL, fbp_bus_gp(0, 0, GP_LVDS_BKL_EN, 0), 0x00),
	VD_WVGA(LVDS, NULL, fbp_bus_gp(0, 0, GP_LVDS_BKL_EN, 0), 0x00),
	VD_AA065VE11(LVDS, NULL, fbp_bus_gp(0, 0, GP_LVDS_BKL_EN, 0), 0x00),
	VD_VGA(LVDS, NULL, fbp_bus_gp(0, 0, GP_LVDS_BKL_EN, 0), 0x00),

	/* tsc2004 */
	VDF_CLAA_WVGA(LCD, "CLAA-WVGA", RGB666, 0, fbp_detect_i2c, 2, 0x48),
	VDF_SHARP_WVGA(LCD, "sharp-wvga", RGB24, 0, NULL, 2, 0x48),
	VDF_DC050WX(LCD, "DC050WX", RGB24, 0, NULL, 2, 0x48),
	VDF_QVGA(LCD, "qvga", RGB24, 0, NULL, 2, 0x48),
	VDF_AT035GT_07ET3(LCD, "AT035GT-07ET3", RGB24, 0, NULL, 2, 0x48),

	VDF_LSA40AT9001(LCD, "LSA40AT9001", RGB24, 0, NULL, 0, 0x00),
};
#define display_cnt	ARRAY_SIZE(displays)
#else
#define displays	NULL
#define display_cnt	0
#endif

static const unsigned short gpios_out_low[] = {
	GP_REG_WLAN_EN,		/* disable wireless */
	GP_REG_WIFI_1P8V_EN,
	GP_REG_WIFI_3P3V_EN,
	GP_WIFI_PASS,
	GP_WIFI_FAIL,
	GP_RGMII_PHY_RESET,
	GP_RGMII2_PHY_RESET,
	GP_BACKLIGHT_LVDS,
	GP_USB_HUB_RESET,
	GP_USB_OTG1_PWR,
	GP_LVDS_BKL_EN,
	GP_PCIE_RESET,
	GP_SGTL5000_MUTE,
};

static const unsigned short gpios_out_high[] = {
	GP_ECSPI1_NOR_CS,
	GP_I2C2_TFP410_I2C_SEL,
	GP_CAN1_STANDBY,
	GP_CAN2_STANDBY,
};

static const unsigned short gpios_in[] = {
	GP_WIFI_TEST,
	GP_RGMII_PHY_INT,
	GP_RGMII2_PHY_INT,
	GP_USDHC2_CD,
	GP_PCIE_WAKE,
	GP_PCIE_DISABLE,
	GP_SGTL5000_HP_DETECT,
	GP_SGTL5000_MIC_DETECT,
	GPIRQ_WLAN,
};

int board_early_init_f(void)
{
	set_gpios_in(gpios_in, ARRAY_SIZE(gpios_in));
	set_gpios(gpios_out_high, ARRAY_SIZE(gpios_out_high), 1);
	set_gpios(gpios_out_low, ARRAY_SIZE(gpios_out_low), 0);
	SETUP_IOMUX_PADS(init_pads);
	SETUP_IOMUX_PADS(rgb_gpio_pads);
	return 0;
}

int board_init(void)
{
	common_board_init(i2c_pads, I2C_BUS_CNT, 0, displays, display_cnt, 0);
	return 0;
}

const struct button_key board_buttons[] = {
	{NULL, 0, 0, 0},
};

#ifdef CONFIG_CMD_BMODE
const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",        MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"mmc1",        MAKE_CFGVAL(0x60, 0x58, 0x00, 0x00)},
	{NULL,          0},
};
#endif