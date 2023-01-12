/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef LK2ND_DEVICE_DEVICE_H
#define LK2ND_DEVICE_DEVICE_H

#include <lk2nd/device.h>

#if WITH_LK2ND_DEVICE_2ND
const void *lk2nd_device2nd_init(void);
int lk2nd_device2nd_match_device_node(const void *dtb, int lk2nd_node);
#else
static inline const void *lk2nd_device2nd_init(void) { return NULL; }
static inline int lk2nd_device2nd_match_device_node(const void *dtb, int lk2nd_node) { return -1; }
#endif

struct lk2nd_device_init {
	const char *compatible;
	int (*init)(const void *dtb, int node);
};
#define LK2ND_DEVICE_INIT(compatible, handler) \
	static const struct lk2nd_device_init _lk2nd_device_init_##handler \
		__SECTION(".lk2nd_device_init") __USED = { (compatible), (handler) }

#endif /* LK2ND_DEVICE_DEVICE_H */
