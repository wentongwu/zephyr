/*
 * Copyright (c) 2020 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_POWER_RT_DPM_H
#define ZEPHYR_INCLUDE_POWER_RT_DPM_H

#include <device.h>
#include <sys/atomic.h>
#include <kernel_structs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief States defined for device runtime power management.
 */
enum rt_dpm_state {
	RT_DPM_ACTIVE,
	RT_DPM_RESUMING,
	RT_DPM_SUSPENDED,
	RT_DPM_SUSPENDING
};

/**
 * @brief Structure used to do device runtime power management.
 */
struct rt_dpm {
	_wait_q_t wait_q;
	atomic_t usage_count;
	atomic_t disable_count;
	struct k_spinlock lock;
	enum rt_dpm_state state;
};

/**
 * @brief Loop over parent for the given device.
 */
#define DEVICE_PARENT_FOREACH(dev, iterator) \
	for (struct device *iterator = dev->parent; \
	     iterator != NULL; iterator = iterator->next)

/**
 * @brief Initialize device runtime power management.
 */
void rt_dpm_init(struct device *dev);

/**
 * @brief Enable device runtime power management.
 */
void rt_dpm_enable(struct device *dev);

/**
 * @brief Disable device runtime power management.
 */
void rt_dpm_disable(struct device *dev);

/**
 * @brief Claim a device to mark the device as being used.
 */
int rt_dpm_claim(struct device *dev);

/**
 * @brief Release a device to mark the device as not being used.
 */
int rt_dpm_release(struct device *dev);

#ifdef __cplusplus
}
#endif

#endif
