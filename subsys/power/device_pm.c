/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <kernel.h>
#include <device.h>
#include <sys/__assert.h>

#define LOG_LEVEL CONFIG_SYS_PM_LOG_LEVEL /* From power module Kconfig */
#include <logging/log.h>
LOG_MODULE_DECLARE(power);

static const struct onoff_transitions transitions = {
		.start = start,
		.stop = stop,
		/* reset not supported */
};

static void start(struct onoff_manager *mgr,
		  onoff_notify_fn notify)
{
	int ret = 0;
	struct device *dev = CONTAINER_OF(mgr, struct device, mgr);

	device_set_power_state(dev,
			       DEVICE_PM_ACTIVE_STATE,
			       notify, (void *)mgr);
}

static int device_pm_get_helper(struct device *dev, struct onoff_client *cli)
{
	//TODO alloc parent_cli for every parent
	//and call device_pm_get_helper to queue request
	DEVICE_PARENT_FOREACH(dev, parent) {
		device_pm_get_helper(parent, parent_cli);
	}

	//TODO someone may fail for queuing or transition

	//TODO after all the queue, wait every parent done.

	//then queue request for dev
	onoff_request(&dev->mgr, cli);
}

int device_pm_get(struct device *dev)
{
	struct onoff_client cli;

	//TODO initial cli for async

	//queue request
	device_pm_get_helper(dev, &cli);
}

int device_pm_get_sync(struct device *dev)
{
	struct onoff_client cli;

	//TODO initial cli for sync api

	//queue request
	device_pm_get_helper(dev, &cli);

	//TODO wait for request done
}

int device_pm_put(struct device *dev)
{
}

int device_pm_put_sync(struct device *dev)
{
}

void device_pm_enable(struct device *dev)
{
	k_sem_take(&dev->pm->lock, K_FOREVER);
	dev->pm->enable = true;

	/* During the driver init, device can set the
	 * PM state accordingly. For later cases we need
	 * to check the usage and set the device PM state.
	 */
	if (!dev->pm->dev) {
		dev->pm->dev = dev;
		atomic_set(&dev->pm->fsm_state,
			   DEVICE_PM_FSM_STATE_SUSPENDED);
		k_work_init(&dev->pm->work, pm_work_handler);
	} else {
		k_work_submit(&dev->pm->work);
	}
	k_sem_give(&dev->pm->lock);
}

void device_pm_disable(struct device *dev)
{
	k_sem_take(&dev->pm->lock, K_FOREVER);
	dev->pm->enable = false;
	/* Bring up the device before disabling the Idle PM */
	k_work_submit(&dev->pm->work);
	k_sem_give(&dev->pm->lock);
}
