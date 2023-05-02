// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2023 Nikita Travkin <nikita@trvn.ru> */

#include <debug.h>
#include <scm.h>
#include <mmc.h>
#include <stdlib.h>
#include <string.h>
#include <partition_parser.h>

#include <lk2nd/init.h>

#include "takeover.h"

#define QHYPSTUB_SIZE           4096
#define QHYPSTUB_MAGIC          0x6275747370796871 // "qhypstub"
#define QHYPSTUB_VBAR_OFFT      0x800

#define QHYPSTUB_STATE_CALL     0x86004242
#define QHYPSTUB_STATE_AARCH32  1
#define QHYPSTUB_STATE_AARCH64  2

/* Hypervisor Call */
int hvc(unsigned long x0, unsigned long x1)
{
	register unsigned long r0 __asm__("r0") = x0;
	register unsigned long r1 __asm__("r1") = x1;

	__asm__ volatile(
		".arch_extension virt\n"
		"hvc	#0\n"
		: "+r" (r0), "+r" (r1) : : "r2", "r3"
	);
	return r0;
}

static void hyp_replace(void *payload, int payload_size)
{
	int ret;

	if (!is_scm_armv8_support()) {
		dprintf(INFO, "qhypstub: Not loading on 32-bit firmware\n");
		return;
	}

	ret = hvc(MAKE_SIP_SCM_CMD(SCM_SVC_MILESTONE_32_64_ID, SCM_SVC_MILESTONE_CMD_ID), 0);
	if (ret != -2) {
		dprintf(INFO, "qhypstub: Already loaded (hvc ret=%d)\n", ret);
		return;
	}

	dprintf(INFO, "qhypstub: Trying to load...\n");

	/* Call platform specific takeover code. */
	ret = lk2nd_replace_hyp(payload, payload_size, QHYPSTUB_VBAR_OFFT);
	if (ret) {
		dprintf(CRITICAL, "qhypstub: Failed to replace hyp! (ret=%d)\n", ret);
		return;
	}

	/* Configure state to aarch32 by default, as if qhypstub was booted directly */
	ret = hvc(QHYPSTUB_STATE_CALL, QHYPSTUB_STATE_AARCH32);
	if (ret) {
		dprintf(CRITICAL, "qhypstub: Failed to change state! (ret=%d)\n", ret);
		return;
	}

	dprintf(INFO, "qhypstub: Loaded!\n");
}

static void lk2nd_load_qhypstub(void)
{
	uint64_t *qhypstub_payload = malloc(QHYPSTUB_SIZE);
	unsigned long long ptn = 0;
	int index = INVALID_PTN;
	bool magic_found = false;

	index = partition_get_index("qhypstub");
	ptn = partition_get_offset(index);
	if (ptn == 0) {
		dprintf(CRITICAL, "qhypstub: No qhypstub partition found!\n");
		return;
	}

	if (mmc_read(ptn, (unsigned int *)qhypstub_payload, QHYPSTUB_SIZE)) {
		dprintf(CRITICAL, "qhypstub: Cannot read the image!\n");
		return;
	}

	/* Scan image for the magic. */
	for (unsigned int i = 0; i < QHYPSTUB_SIZE / sizeof(uint64_t); ++i) {
		if (qhypstub_payload[i] == QHYPSTUB_MAGIC) {
			magic_found = true;
			break;
		}
	}

	if (!magic_found) {
		dprintf(INFO, "qhypstub: Image lacks magic, not loading\n");
		return;
	}

	hyp_replace(qhypstub_payload, QHYPSTUB_SIZE);
}
LK2ND_INIT(lk2nd_load_qhypstub);
