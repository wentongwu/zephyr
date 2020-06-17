/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <spinlock.h>
#include <power/rt_dpm.h>

int rt_dpm_release(struct device *dev)
{
	int ret;
	k_spinlock_key_t key;
	struct k_thread *thread;
	atomic_val_t pre_usage_count;
	struct rt_dpm *rt_pm = &dev->rt_pm;

	pre_usage_count = atomic_dec(&rt_pm->usage_count);
	if (pre_usage_count > 1) {
		return 0;
	}
	__ASSERT_NO_MSG(pre_usage_count == 1);

	key = k_spin_lock(&rt_pm->lock);

	if (rt_pm->disable_count > 0) {
		k_spin_unlock(&rt_pm->lock, key);
		return -EACCES;
	} else if (rt_pm->usage_count > 0) {
		k_spin_unlock(&rt_pm->lock, key);
		return -EAGAIN;
	} else if (rt_pm->state == RT_DPM_SUSPENDED) {
		k_spin_unlock(&rt_pm->lock, key);
		return 0;
	}

	rt_pm->state = RT_DPM_SUSPENDING;

	if (rt_pm->suspend_prepare) {
		k_spin_unlock(&rt_pm->lock, key);
		ret = (rt_pm->suspend_prepare)(dev);
		key = k_spin_lock(&rt_pm->lock);
	}

	if (ret) {
		rt_pm->state = RT_DPM_ACTIVE;
	} else {
		if (rt_pm->suspend) {
			(rt_pm->suspend)(dev);
		}
		rt_pm->state = RT_DPM_SUSPENDED;
		DEVICE_PARENT_FOREACH(dev, parent) {
			atomic_dec(&parent->child_count);
		}
	}

	thread = z_unpend_first_thread(&rt_pm->wait_q);
	if (thread) {
		arch_thread_return_value_set(thread, 0);
		z_ready_thread(thread);
		z_reschedule(&rt_pm->lock, key);
	} else {
		k_spin_unlock(&rt_pm->lock, key);
	}
	return ret;
}

int rt_dpm_claim(struct device *dev)
{
	int ret;
	k_spinlock_key_t key;
	struct k_thread *thread;
	struct rt_dpm *rt_pm = &dev->rt_pm;

	atomic_inc(&rt_pm->usage_count);

	key = k_spin_lock(&rt_pm->lock);
again:
	if (rt_pm->disable_count > 0) {
		ret = -EACCES;
		goto out;
	} else if (rt_pm->state == RT_DPM_ACTIVE) {
		ret = 0;
		goto out;
	}

	if (rt_pm->state == RT_DPM_SUSPENDING
	    || rt_pm->state == RT_DPM_RESUMING) {
		do {
			z_pend_curr(&rt_pm->lock,
				    key, &rt_pm->wait_q, K_FOREVER);
			key = k_spin_lock(&rt_pm->lock);
		} while (rt_pm->state == RT_DPM_SUSPENDING
			 || rt_pm->state == RT_DPM_RESUMING);
		goto again;
	}

	rt_pm->state = RT_DPM_RESUMING;

	if (rt_pm->resume_prepare) {
		k_spin_unlock(&rt_pm->lock, key);
		ret = (rt_pm->resume_prepare)(dev);
		key = k_spin_lock(&rt_pm->lock);
	}

	if (ret) {
		rt_pm->state = RT_DPM_SUSPENDED;
	} else {
		if (rt_pm->resume) {
			(rt_pm->resume)(dev);
		}
		rt_pm->state = RT_DPM_ACTIVE;
	}

	thread = z_unpend_first_thread(&rt_pm->wait_q);
	if (thread) {
		arch_thread_return_value_set(thread, 0);
		z_ready_thread(thread);
		z_reschedule(&rt_pm->lock, key);
	} else {
out:
		k_spin_unlock(&rt_pm->lock, key);
	}
}

void rt_dpm_enable(struct device *dev)
{
	k_spinlock_key_t key;
	struct rt_dpm *rt_pm = &dev->rt_pm;

	key = k_spin_lock(&rt_pm->lock);
	if (rt_pm->disable_count > 0) {
		atomic_dec(&rt_pm->disable_count);
	}
	k_spin_unlock(&rt_pm->lock, key);
}

void rt_dpm_disable(struct device *dev)
{
	k_spinlock_key_t key;
	struct rt_dpm *rt_pm = &dev->rt_pm;

	key = k_spin_lock(&rt_pm->lock);
	atomic_inc(&rt_pm->disable_count);
	if (rt_pm->state == RT_DPM_SUSPENDING
	    || rt_pm->state == RT_DPM_RESUMING) {
		do {
			z_pend_curr(&rt_pm->lock,
				    key, &rt_pm->wait_q, K_FOREVER);
			key = k_spin_lock(&rt_pm->lock);
		} while (rt_pm->state == RT_DPM_SUSPENDING
			 || rt_pm->state == RT_DPM_RESUMING);
	}

	if (rt_pm->state == RT_DPM_SUSPENDED) {
		rt_pm->state = RT_DPM_RESUMING;

		if (rt_pm->resume_prepare) {
			k_spin_unlock(&rt_pm->lock, key);
			ret = (rt_pm->resume_prepare)(dev);
			key = k_spin_lock(&rt_pm->lock);
		}

		if (ret) {
			rt_pm->state = RT_DPM_SUSPENDED;
		} else {
			if (rt_pm->resume) {
				(rt_pm->resume)(dev);
			}
			rt_pm->state = RT_DPM_ACTIVE;
		}
	}
	k_spin_unlock(&rt_pm->lock, key);
}

void rt_dpm_init(struct device *dev)
{
	k_spinlock_key_t key;
	struct rt_dpm *rt_pm = &dev->rt_pm;

	key = k_spin_lock(&rt_pm->lock);
	rt_pm->usage_count = 0;
	rt_pm->disable_count = 0;
	rt_pm->state = RT_DPM_SUSPENDED;
	rt_pm->wait_q = Z_WAIT_Q_INIT(&rt_pm->wait_q);
	k_spin_unlock(&rt_pm->lock, key);
}