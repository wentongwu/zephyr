/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <kernel.h>
#include <spinlock.h>
#include <sys/util.h>
#include <sys/atomic.h>
#include <tracing_packet.h>

static sys_slist_t tracing_list;
static atomic_t tracing_list_packet_num;
static struct k_spinlock tracing_list_lock;

static k_tid_t tracing_thread_tid;
static struct k_thread tracing_thread;
static struct k_timer tracing_thread_timer;
static K_SEM_DEFINE(tracing_thread_sem, 0, 1);
static K_THREAD_STACK_DEFINE(tracing_thread_stack,
			CONFIG_TRACING_THREAD_STACK_SIZE);

struct tracing_packet *tracing_list_get_packet(void);

static void tracing_packet_handle(struct tracing_packet * packet)
{
	ARG_UNUSED(packet);
}

static void tracing_thread_func(void *dummy1, void *dummy2, void *dummy3)
{
	struct tracing_packet *packet = NULL;

	tracing_thread_tid = k_current_get();

	while (true) {
		packet = tracing_list_get_packet();
		if (packet == NULL) {
			k_sem_take(&tracing_thread_sem, K_FOREVER);
		} else {
			tracing_packet_handle(packet);
		}
	}
}

static void tracing_thread_timer_expiry_fn(struct k_timer *timer)
{
	k_sem_give(&tracing_thread_sem);
}

static int tracing_enable(struct device *arg)
{
	ARG_UNUSED(arg);

	k_timer_init(&tracing_thread_timer,
		tracing_thread_timer_expiry_fn, NULL);

	tracing_packet_pool_init();

	k_thread_create(&tracing_thread, tracing_thread_stack,
			K_THREAD_STACK_SIZEOF(tracing_thread_stack),
			tracing_thread_func, NULL, NULL, NULL,
			K_HIGHEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
	k_thread_name_set(&tracing_thread, "tracing");

	return 0;
}

void tracing_list_add_packet(struct tracing_packet *packet)
{
	k_spinlock_key_t key;

	key = k_spin_lock(&tracing_list_lock);
	sys_slist_append(&tracing_list, &packet->list_node);
	k_spin_unlock(&tracing_list_lock, key);

	atomic_inc(&tracing_list_packet_num);

	if (tracing_thread_tid != NULL && tracing_list_packet_num == 1) {
		k_timer_start(&tracing_thread_timer,
			CONFIG_TRACING_THREAD_SLEEP_MS, K_NO_WAIT);
	} else if (IS_ENABLED(CONFIG_TRACING_THREAD_TRIGGER_THRESHOLD) &&
	    tracing_thread_tid && (tracing_list_packet_num ==
	    CONFIG_TRACING_THREAD_TRIGGER_THRESHOLD)) {
		k_timer_stop(&tracing_thread_timer);
		k_sem_give(&tracing_thread_sem);
	}
}

struct tracing_packet *tracing_list_get_packet(void)
{
	k_spinlock_key_t key;
	struct tracing_packet *packet = NULL;

	key = k_spin_lock(&tracing_list_lock);
	packet = CONTAINER_OF(sys_slist_get(&tracing_list),
			struct tracing_packet, list_node);
	k_spin_unlock(&tracing_list_lock, key);

	return packet;
}

SYS_INIT(tracing_enable, POST_KERNEL, 0);
