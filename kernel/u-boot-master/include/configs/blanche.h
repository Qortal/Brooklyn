/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/blanche.h
 *     This file is blanche board configuration.
 *
 * Copyright (C) 2016 Renesas Electronics Corporation
 */

#ifndef __BLANCHE_H
#define __BLANCHE_H

#include "rcar-gen2-common.h"

/* STACK */
#define CONFIG_SYS_INIT_SP_ADDR		0x4f000000
#define STACK_AREA_SIZE			0x00100000
#define LOW_LEVEL_MERAM_STACK \
		(CONFIG_SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define RCAR_GEN2_SDRAM_BASE		0x40000000
#define RCAR_GEN2_SDRAM_SIZE		(1024u * 1024 * 1024)
#define RCAR_GEN2_UBOOT_SDRAM_SIZE	(512 * 1024 * 1024)

#undef	CONFIG_SYS_MEMTEST_SCRATCH
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE

/* FLASH */
#if !defined(CONFIG_MTD_NOR_FLASH)
#define CONFIG_SH_QSPI_BASE	0xE6B10000
#else
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
#define CONFIG_FLASH_SHOW_PROGRESS	45
#define CONFIG_SYS_FLASH_BASE		0x00000000
#define CONFIG_SYS_FLASH_SIZE		0x04000000	/* 64 MB */
#define CONFIG_SYS_MAX_FLASH_SECT	1024
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_BANKS_LIST	{ (CONFIG_SYS_FLASH_BASE) }
#define CONFIG_SYS_FLASH_BANKS_SIZES	{ (CONFIG_SYS_FLASH_SIZE) }

#define CONFIG_SYS_FLASH_ERASE_TOUT	3000
#define CONFIG_SYS_FLASH_WRITE_TOUT	3000
#define CONFIG_SYS_FLASH_LOCK_TOUT	3000
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	3000
#undef  CONFIG_CMD_SF
#undef  CONFIG_CMD_SPI
#endif

/* Board Clock */
#define RMOBILE_XTAL_CLK	20000000u
#define CONFIG_SYS_CLK_FREQ	RMOBILE_XTAL_CLK

/* ENV setting */
#if !defined(CONFIG_MTD_NOR_FLASH)
#else
#undef  CONFIG_ENV_ADDR
#define CONFIG_ENV_SECT_SIZE	(256 * 1024)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_OFFSET	(CONFIG_ENV_ADDR)
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_SYS_MONITOR_LEN)
#endif

#endif	/* __BLANCHE_H */
