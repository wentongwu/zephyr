/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <irq.h>
#include <assert.h>
#include <sys/printk.h>
#include <tracing_buffer.h>
#include <syscall_handler.h>
#include <debug/tracing_core.h>
#include <debug/tracing_format.h>

#ifdef CONFIG_NEWLIB_LIBC
typedef int (*str_put_func_t)(int c, void *ctx);
extern void z_vprintk(str_put_func_t out, void *ctx,
		      const char *fmt, va_list ap);
#else
extern int z_prf(int (*func)(), void *dest, char *format, va_list vargs);
#endif

struct string_format_ctx {
	int status;
	u32_t length;
};

#if defined(CONFIG_TRACING_ASYNC)
static bool tracing_format_data_put(u8_t *data, u32_t size)
{
	u32_t space = tracing_buffer_space_get();

	if (space >= size) {
		tracing_buffer_put(data, size);
		return true;
	} else {
		return false;
	}
}
#endif

static int str_put(int c, void *ctx)
{
	struct string_format_ctx *str_ctx = (struct string_format_ctx *)ctx;

	if (str_ctx->status == 0) {
		u8_t *buf;
		u32_t claimed_size;

		claimed_size = tracing_buffer_put_claim(&buf, 1);
		if (claimed_size) {
			*buf = (u8_t)c;
			str_ctx->length++;
		} else {
			str_ctx->status = -1;
		}
	}

	return 0;
}

static bool tracing_format_str_put(const char *str, va_list args)
{
	struct string_format_ctx str_ctx = {0};

#if !defined(CONFIG_NEWLIB_LIBC) && !defined(CONFIG_ARCH_POSIX)
	(void)z_prf(str_put, (void *)&str_ctx, (char *)str, args);
#else
	z_vprintk(str_put, (void *)&str_ctx, str, args);
#endif

	if (str_ctx.status == 0) {
		tracing_buffer_put_finish(str_ctx.length);
		return true;
	} else {
		tracing_buffer_put_finish(0);
		return false;
	}
}

static void tracing_format_string_handler(const char *str, va_list args)
{
	int key;
#if defined(CONFIG_TRACING_ASYNC)
	bool put_success, before_put_is_empty;

	key = irq_lock();
	before_put_is_empty = tracing_buffer_is_empty();
	put_success = tracing_format_str_put(str, args);
	irq_unlock(key);

	if (put_success) {
		tracing_try_to_trigger_output(before_put_is_empty);
	} else {
		tracing_packet_drop_handle();
	}
#elif defined(CONFIG_TRACING_SYNC)
	u8_t *data;
	bool put_success;
	u32_t length, tracing_buffer_size;

	tracing_buffer_size = tracing_buffer_capacity_get();

	key = irq_lock();
	put_success = tracing_format_str_put(str, args);

	if (put_success) {
		length = tracing_buffer_get_claim(&data, tracing_buffer_size);
		tracing_buffer_handle((u8_t *)data, length);
		tracing_buffer_get_finish(length);
	} else {
		tracing_packet_drop_handle();
	}
	irq_unlock(key);
#endif
}

#ifdef CONFIG_USERSPACE
void z_impl_z_tracing_format_raw_str(const char *data, u32_t length)
{
	ARG_UNUSED(data);
	ARG_UNUSED(length);

	__ASSERT(false, "can only be called from user mode.");
}

void z_vrfy_z_tracing_format_raw_str(const char *data, u32_t length)
{
	int key;
#if defined(CONFIG_TRACING_ASYNC)
	bool put_success, before_put_is_empty;

	Z_OOPS(Z_SYSCALL_VERIFY_MSG(length == 0,
		"Invalid parameter length"));
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(data == NULL,
		"Invalid parameter data"));

	key = irq_lock();
	before_put_is_empty = tracing_buffer_is_empty();
	put_success = tracing_format_data_put(data, length);
	irq_unlock(key);

	if (put_success) {
		tracing_try_to_trigger_output(before_put_is_empty);
	} else {
		tracing_packet_drop_handle();
	}
#elif defined(CONFIG_TRACING_SYNC)
	if (!is_tracing_enabled()) {
		return;
	}

	key = irq_lock();
	tracing_buffer_handle((u8_t *)data, length);
	irq_unlock(key);
#endif
}
#include <syscalls/z_tracing_format_raw_str_mrsh.c>
#else
void z_impl_z_tracing_format_raw_str(const char *data, u32_t length)
{
	ARG_UNUSED(data);
	ARG_UNUSED(length);
}
#endif

void tracing_format_string(const char *str, ...)
{
	va_list args;

	if (!is_tracing_enabled() || is_tracing_thread()) {
		return;
	}

	if (_is_user_context()) {
		int length;
		char data[CONFIG_TRACING_PACKET_MAX_SIZE];

		va_start(args, str);
		length = vsnprintk(data, sizeof(data), str, args);
		length = MIN(length, sizeof(data));
		va_end(args);

		z_tracing_format_raw_str(data, length);
	} else {
		va_start(args, str);
		tracing_format_string_handler(str, args);
		va_end(args);
	}
}

void z_impl_tracing_format_data(const char *data, u32_t length)
{
	int key;
#if defined(CONFIG_TRACING_ASYNC)
	bool put_success, before_put_is_empty;

	if (!is_tracing_enabled() || is_tracing_thread()) {
		return;
	}

	key = irq_lock();
	before_put_is_empty = tracing_buffer_is_empty();
	put_success = tracing_format_data_put((u8_t *)data, length);
	irq_unlock(key);

	if (put_success) {
		tracing_try_to_trigger_output(before_put_is_empty);
	} else {
		tracing_packet_drop_handle();
	}
#elif defined(CONFIG_TRACING_SYNC)
	if (!is_tracing_enabled()) {
		return;
	}

	key = irq_lock();
	tracing_buffer_handle((u8_t *)data, length);
	irq_unlock(key);
#endif
}

#ifdef CONFIG_USERSPACE
void z_vrfy_tracing_format_data(const char *data, u32_t length)
{
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(data == NULL,
		"Invalid parameter data"));
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(length == 0,
		"Invalid parameter length"));

	z_impl_tracing_format_data(data, length);
}
#include <syscalls/tracing_format_data_mrsh.c>
#endif
