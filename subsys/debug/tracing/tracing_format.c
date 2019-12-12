/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <sys/printk.h>
#include <debug/tracing_core.h>
#include <syscall_handler.h>
#include <debug/tracing_format.h>
#include <tracing_buffer.h>
#include <irq.h>

static void tracing_format_string_handler(const char *str, va_list args)
{
	int key;
	bool put_success, before_put_is_empty;

	key = irq_lock();
	before_put_is_empty = tracing_buffer_is_empty();
	put_success = tracing_buffer_str_put(str, args);
	irq_unlock(key);

	if (put_success) {
		tracing_try_to_trigger_output(before_put_is_empty);
	} else {
		tracing_packet_drop_handle();
	}
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
	bool put_success, before_put_is_empty;

	Z_OOPS(Z_SYSCALL_VERIFY_MSG(length == 0,
		"Invalid parameter length"));
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(data == NULL,
		"Invalid parameter data"));

	key = irq_lock();
	before_put_is_empty = tracing_buffer_is_empty();
	put_success = tracing_buffer_put(data, length);
	irq_unlock(key);

	if (put_success) {
		tracing_try_to_trigger_output(before_put_is_empty);
	} else {
		tracing_packet_drop_handle();
	}
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
		char data[CONFIG_TRACING_PACKET_BUF_SIZE];

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
	bool put_success, before_put_is_empty;

	if (!is_tracing_enabled() || is_tracing_thread()) {
		return;
	}

	key = irq_lock();
	before_put_is_empty = tracing_buffer_is_empty();
	put_success = tracing_buffer_put((u8_t *)data, length);
	irq_unlock(key);

	if (put_success) {
		tracing_try_to_trigger_output(before_put_is_empty);
	} else {
		tracing_packet_drop_handle();
	}
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
