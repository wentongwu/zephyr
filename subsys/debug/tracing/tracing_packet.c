/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define PACKET_SIZE sizeof(struct tracing_packet)
#define NUM_OF_PACKETS (CONFIG_TRACING_BUFFER_SIZE / PACKET_SIZE)

struct k_mem_slab tracing_packet_pool;
static u8_t __noinit __aligned(sizeof(void *))
		tracing_packet_pool_buf[CONFIG_TRACING_BUFFER_SIZE];

void tracing_packet_pool_init(void)
{
	k_mem_slab_init(&tracing_packet_pool,
		tracing_packet_pool_buf, PACKET_SIZE, NUM_OF_PACKETS);
}

/* Return true if interrupts were locked in current context */
static bool is_irq_locked(void)
{
	unsigned int key = z_arch_irq_lock();
	bool ret = z_arch_irq_unlocked(key);

	z_arch_irq_unlock(key);
	return ret;
}

/* Check if context can be blocked and pend on available memory slab.
 * Context can be blocked if in a thread and interrupts are not locked.
 */
static bool block_on_alloc(void)
{
	if (!IS_ENABLED(CONFIG_LOG_BLOCK_IN_THREAD)) {
		return false;
	}

	return (!k_is_in_isr() && !is_irq_locked());
}

struct tracing_packet *tracing_packet_alloc(void)
{
	struct tracing_packet *packet = NULL;

	int err = k_mem_slab_alloc(&tracing_packet_pool, (void **)&packet,
			block_on_alloc() ?
			CONFIG_LOG_BLOCK_IN_THREAD_TIMEOUT_MS : K_NO_WAIT);

	if (err != 0) {
		//TODO
	}

	return packet;
}

struct tracing_packet *tracing_packet_free(struct tracing_packet *packet)
{
	k_mem_slab_free(&tracing_packet_pool, (void **)&packet);
}
