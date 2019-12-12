/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/ring_buffer.h>

static u8_t tracing_cmd_buffer[CONFIG_TRACING_CMD_BUFFER_SIZE];

#ifdef CONFIG_NEWLIB_LIBC
typedef int (*str_put_func_t)(int c, void *ctx);
extern void z_vprintk(str_put_func_t out, void *ctx,
		      const char *fmt, va_list ap);
#else
extern int z_prf(int (*func)(), void *dest, char *format, va_list vargs);
#endif

u8_t *tracing_cmd_buffer_alloc(void)
{
	return tracing_cmd_buffer;
}

#ifdef CONFIG_TRACING_ASYNC
static u32_t total_size;
static struct ring_buf tracing_ring_buf;
static u8_t tracing_buffer[CONFIG_TRACING_BUFFER_SIZE];

static int str_put(int c, void *ctx)
{
	u8_t *dst;
	u32_t granted_size;
	int *result = (int *)ctx;

	if (*result) {
		granted_size = ring_buf_put_claim(&tracing_ring_buf, &dst, 1);
		if (granted_size) {
			*dst = (u8_t)c;
			total_size++;
		} else {
			*result = 0;
		}
	}

	return 0;
}

bool tracing_buffer_str_put(const char *str, va_list args)
{
	int result = 1;

	total_size = 0;
#if !defined(CONFIG_NEWLIB_LIBC) && !defined(CONFIG_ARCH_POSIX)
	(void)z_prf(str_put, (void *)&result, (char *)str, args);
#else
	z_vprintk(str_put, (void *)&result, str, args);
#endif

	if (result) {
		ring_buf_put_finish(&tracing_ring_buf, total_size);
		return true;
	} else {
		ring_buf_put_finish(&tracing_ring_buf, 0);
		return false;
	}
}

bool tracing_buffer_put(u8_t *data, u32_t size)
{
	u32_t ring_buf_space = ring_buf_space_get(&tracing_ring_buf);

	if (ring_buf_space >= size) {
		/*
		 * has enough space
		 * no need check return value
		 */
		ring_buf_put(&tracing_ring_buf, data, size);

		return true;
	} else {
		return false;
	}
}

u32_t tracing_buffer_get(u8_t **data, u32_t size)
{
	return ring_buf_get_claim(&tracing_ring_buf, data, size);
}

int tracing_buffer_get_finish(u32_t size)
{
	return ring_buf_get_finish(&tracing_ring_buf, size);
}

void tracing_buffer_init(void)
{
	ring_buf_init(&tracing_ring_buf,
		      sizeof(tracing_buffer), tracing_buffer);
}

bool tracing_buffer_is_empty(void)
{
	return ring_buf_is_empty(&tracing_ring_buf);
}

u32_t tracing_buffer_capacity_get(void)
{
	return ring_buf_capacity_get(&tracing_ring_buf);
}
#endif

#ifdef CONFIG_TRACING_SYNC
static u32_t total_size;
static u8_t tracing_buffer[CONFIG_TRACING_PACKET_BUF_SIZE];

static int str_put(int c, void *ctx)
{
	tracing_buffer[total_size] = (u8_t)c;
	total_size++;

	return 0;
}

bool tracing_buffer_str_put(const char *str, va_list args)
{
	total_size = 0;
#if !defined(CONFIG_NEWLIB_LIBC) && !defined(CONFIG_ARCH_POSIX)
	(void)z_prf(str_put, NULL, (char *)str, args);
#else
	z_vprintk(str_put, NULL, str, args);
#endif

	tracing_buffer_handle(tracing_buffer, total_size);
	/*
	 * TODO put tracing_buffer directly to backend. Need to think about API.
	 * Known that this function is protected by irq_lock and irq_unlock.
	 */
}

bool tracing_buffer_put(u8_t *data, u32_t size)
{
	/*
	 * TODO put data directly to backend. Need to think about API.
	 * Known that this function is protected by irq_lock and irq_unlock.
	 */
	tracing_buffer_handle(data, size);
}
#endif
