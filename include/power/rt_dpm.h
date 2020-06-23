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
 * @brief Callbacks defined for device runtime power management.
 */
struct rt_dpm_ops{
	int (*resume)(struct device *dev);
	int (*suspend)(struct device *dev);
	int (*resume_prepare)(struct device *dev);
	int (*suspend_prepare)(struct device *dev);
};

/**
 * @brief Structure used to do device runtime power management.
 */
struct rt_dpm {
	_wait_q_t wait_q;
	struct k_work work;
	atomic_t usage_count;
	struct k_spinlock lock;
	struct rt_dpm_ops *ops;
	enum rt_dpm_state state;
	unsigned int disable_count;
};

/**
 * @brief Loop over parent for the given device.
 */
#define DEVICE_PARENT_FOREACH(dev, iterator) \
	for (struct device *iterator = dev; iterator != NULL; )

/**
 * @brief Initialize device runtime power management.
 *
 * Initialize device runtime power management for the given device.
 *
 * @param dev Pointer to the given device.
 */
void rt_dpm_init(struct device *dev);

/**
 * @brief Enable device runtime power management.
 *
 * Enable device runtime power management for the given device.
 *
 * @param dev Pointer to the given device.
 */
void rt_dpm_enable(struct device *dev);

/**
 * @brief Disable device runtime power management.
 *
 * Disable device runtime power management for the given device.
 *
 * @param dev Pointer to the given device.
 */
void rt_dpm_disable(struct device *dev);

/**
 * @brief Claim a device to mark the device as being used.
 *
 * Claim the given device to make sure the coming device operations
 * after this call are safe. Can't be used in ISRs.
 *
 * @param dev Pointer to the given device.
 *
 * @retval 0 if successfully claimed the given device.
 * @retval -EIO if error happens during device resume prepare.
 * @retval -EACCES if disabled device runtime power management.
 */
int rt_dpm_claim(struct device *dev);

/**
 * @brief Release the given device.
 *
 * Synchronously decrease a usage count of the given device and suspend
 * the given device if all the conditions satisfied, this function must
 * be called after any claim happens, forbid asymmetric release.
 *
 * @param dev Pointer to the given device.
 *
 * @retval 0 if successfully release the given device.
 * @retval -EIO if error happens during device suspend prepare.
 * @retval -EACCES if disabled device runtime power management.
 */
int rt_dpm_release(struct device *dev);

/**
 * @brief Release the given device asynchronously.
 *
 * @param dev Pointer to the given device.
 */
void rt_dpm_release_async(struct device *dev);

#ifdef __cplusplus
}
#endif

#endif
