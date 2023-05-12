// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2019-2022, Stephan Gerhold <stephan@gerhold.net> */
/* Copyright (c) 2023, Nikita Travkin <nikita@trvn.ru> */

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <debug.h>
#include <fastboot.h>
#include <platform.h>
#include <reg.h>

static bool parse_write_args(const char *arg, uint32_t *addr, uint32_t *value)
{
	char *saveptr;
	char *args = strdup(arg);
	char *addr_str = strtok_r(args, " ", &saveptr);
	char *value_str = strtok_r(NULL, " ", &saveptr);

	if (!addr_str || !value_str) {
		free(args);
		return false;
	}

	*addr = atoi(addr_str);
	*value = atoi(value_str);
	free(args);
	return true;
}

static void cmd_oem_readl(const char *arg, void *data, unsigned sz)
{
	char response[MAX_RSP_SIZE];
	uint32_t addr = 0;

	addr = atoi(arg);
	snprintf(response, sizeof(response), "0x%08x\n", readl(addr));
	fastboot_info(response);

	fastboot_okay("");
}
FASTBOOT_REGISTER("oem readl", cmd_oem_readl);

static void cmd_oem_writel(const char *arg, void *data, unsigned sz)
{
	uint32_t addr = 0, value = 0;

	if (!parse_write_args(arg, &addr, &value)) {
		fastboot_fail("");
		return;
	}

	writel(value, addr);

	fastboot_okay("");
}
FASTBOOT_REGISTER("oem writel", cmd_oem_writel);

static void cmd_oem_readb(const char *arg, void *data, unsigned sz)
{
	char response[MAX_RSP_SIZE];
	uint32_t addr = 0;

	addr = atoi(arg);
	snprintf(response, sizeof(response), "0x%02x\n", readb(addr));
	fastboot_info(response);

	fastboot_okay("");
}
FASTBOOT_REGISTER("oem readb", cmd_oem_readb);

static void cmd_oem_writeb(const char *arg, void *data, unsigned sz)
{
	uint32_t addr = 0, value = 0;

	if (!parse_write_args(arg, &addr, &value) || value > 0xff) {
		fastboot_fail("");
		return;
	}

	writeb(value, addr);

	fastboot_okay("");
}
FASTBOOT_REGISTER("oem writeb", cmd_oem_writeb);

#if WITH_DEBUG_LOG_BUF
static void cmd_oem_log(const char *arg, void *data, unsigned sz)
{
	fastboot_stage(data, log_copy(data));
}
FASTBOOT_REGISTER("oem log", cmd_oem_log);
#endif

static void cmd_oem_reboot_edl(const char *arg, void *data, unsigned sz)
{
	fastboot_okay("");
	reboot_device(EMERGENCY_DLOAD);
}
FASTBOOT_REGISTER("oem reboot-edl", cmd_oem_reboot_edl);
