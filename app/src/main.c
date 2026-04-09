/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/autoconf.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#if IS_ENABLED(CONFIG_PARTITION_MANAGER_ENABLED)
#include <pm_config.h>
#endif

#define RETAINED_REGION_NODE DT_NODELABEL(retained_region)

extern char _end[];
extern char __kernel_ram_end[];
extern void *sbrk(intptr_t increment);

int main(void)
{
	char float_buf[16];
	uintptr_t retained = DT_REG_ADDR(RETAINED_REGION_NODE);
	size_t retained_size = DT_REG_SIZE(RETAINED_REGION_NODE);
	uintptr_t heap_base = (uintptr_t)sbrk(0);
	ptrdiff_t delta = (ptrdiff_t)(retained - heap_base);
	void *ret;
	uintptr_t cross;

	snprintf(float_buf, sizeof(float_buf), "%.2f", 3.14);

	printk("retained-ram-newlib example\n");
	printk("PM=%d CONFIG_SRAM_SIZE=%d KB float=%s\n",
	       IS_ENABLED(CONFIG_PARTITION_MANAGER_ENABLED),
	       CONFIG_SRAM_SIZE, float_buf);
	printk("_end=%p __kernel_ram_end=%p retained=%p size=0x%zx delta=0x%tx\n",
	       _end, __kernel_ram_end, (void *)retained, retained_size, delta);

#if IS_ENABLED(CONFIG_PARTITION_MANAGER_ENABLED)
	printk("PM_SRAM_ADDRESS=%p PM_SRAM_SIZE=0x%x\n",
	       (void *)PM_SRAM_ADDRESS, PM_SRAM_SIZE);
#endif

	errno = 0;
	ret = sbrk(delta);
	cross = (uintptr_t)sbrk(16);

	printk("ret=%p cross=%p errno=%d\n", ret, (void *)cross, errno);
	*(volatile uint32_t *)cross = 0xdeadbeef;
	printk("wrote 0xdeadbeef to %p\n", (void *)cross);

	return 0;
}
