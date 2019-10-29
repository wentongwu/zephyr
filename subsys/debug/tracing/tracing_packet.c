/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <sys/atomic.h>
#include <tracing_packet.h>

#define PACKET_SIZE sizeof(struct tracing_packet)
#define NUM_OF_PACKETS (CONFIG_TRACING_BUFFER_SIZE / PACKET_SIZE)

static atomic_t dropped_num;

static struct k_mem_slab tracing_packet_pool;
static u8_t __noinit __aligned(sizeof(void *))
		tracing_packet_pool_buf[CONFIG_TRACING_BUFFER_SIZE];

/* Return true if interrupts were locked in current context */
static bool is_irq_locked(void)
{
	unsigned int key = z_arch_irq_lock();
	bool ret = z_arch_irq_unlocked(key);

	z_arch_irq_unlock(key);
	return ret;
}

/*
 * Context can be blocked in a thread if interrupts are not locked.
 */
static bool block_on_alloc(void)
{
	return (!k_is_in_isr() && !is_irq_locked());
}

static void tracing_packet_drop(void)
{
	atomic_inc(&dropped_num);
}

void tracing_packet_pool_init(void)
{
	k_mem_slab_init(&tracing_packet_pool,
		tracing_packet_pool_buf, PACKET_SIZE, NUM_OF_PACKETS);
}

struct tracing_packet *tracing_packet_alloc(void)
{
	int ret = 0;
	struct tracing_packet *packet = NULL;

	ret = k_mem_slab_alloc(&tracing_packet_pool, (void **)&packet,
			block_on_alloc() ?
			CONFIG_TRACING_BLOCK_IN_THREAD_TIMEOUT_MS : K_NO_WAIT);

	if (ret != 0) {
		tracing_packet_drop();
	}

	return packet;
}

void tracing_packet_free(struct tracing_packet *packet)
{
	k_mem_slab_free(&tracing_packet_pool, (void **)&packet);
}
