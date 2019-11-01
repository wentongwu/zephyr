/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/printk.h>
#include <tracing_packet.h>
#include <syscall_handler.h>
#include <debug/tracing_format.h>

extern void tracing_list_add_packet(struct tracing_packet *packet);

static void tracing_format_string_handler(const char* str, va_list args)
{
	int length;
	struct tracing_packet *packet = tracing_packet_alloc();

	length = vsnprintk(packet->buf, sizeof(packet->buf), str, args);
	packet->length = MIN(length, sizeof(packet->buf));

	tracing_list_add_packet(packet);
}

static void tracing_format_data_handler(u32_t nargs, va_list args)
{
	u32_t index = 0;
	struct tracing_packet *packet = tracing_packet_alloc();

	if (nargs > CONFIG_TRACING_DUP_MAX_STRING / 4) {
		nargs = CONFIG_TRACING_DUP_MAX_STRING / 4;
	}

	while (index < nargs) {
		u32_t tmp = va_arg(args, u32_t);

		for (int i = 0; i < sizeof(u32_t); i++) {
			packet->buf[index * 4 + i] = ((u8_t *)(&tmp))[i];
		}

		index++;
	}

	packet->length = 4 * nargs;

	tracing_list_add_packet(packet);
}

#ifdef CONFIG_USERSPACE
void z_impl_tracing_format_string_from_user(const char* str, u32_t length)
{
	struct tracing_packet *packet = tracing_packet_alloc();

	length = MIN(length, sizeof(packet->buf));
	for(u32_t i = 0; i < length; i++) {
		*(packet->buf + i) = str[i];
	}
	packet->length = length;

	tracing_list_add_packet(packet);
}

void z_vrfy_tracing_format_string_from_user(const char* str, u32_t length)
{
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(str == NULL,
		"Invalid parameter str"));
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(length == 0,
		"Invalid parameter length"));

	z_impl_tracing_format_string_from_user(str, length);
}
#include <syscalls/tracing_format_string_from_user_mrsh.c>

void z_impl_tracing_format_data_from_user(u32_t nargs, u32_t *buf)
{
	struct tracing_packet *packet = tracing_packet_alloc();

	for (u32_t index = 0; index < nargs; index++) {
		u32_t tmp = *(buf + index);

		for (int i = 0; i < sizeof(u32_t); i++) {
			packet->buf[index * 4 + i] = ((u8_t *)(&tmp))[i];
		}
	}

	packet->length = 4 * nargs;

	tracing_list_add_packet(packet);
}

void z_vrfy_tracing_format_data_from_user(u32_t nargs, u32_t *buf)
{
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(nargs == 0,
		"Invalid parameter nargs"));
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(buf == NULL,
		"Invalid parameter buf"));

	z_impl_tracing_format_data_from_user(nargs, buf);
}
#include <syscalls/tracing_format_data_from_user_mrsh.c>

#else

void tracing_format_string_from_user(const char* str, u32_t length)
{
	ARG_UNUSED(str);
	ARG_UNUSED(length);
}

void tracing_format_data_from_user(u32_t nargs, u32_t *buf)
{
	ARG_UNUSED(buf);
	ARG_UNUSED(nargs);
}
#endif

void tracing_format_string(const char* str, ...)
{
	va_list args;

	if (_is_user_context()) {
		int length;
		char buf[CONFIG_TRACING_DUP_MAX_STRING];

		va_start(args, str);
		length = vsnprintk(buf, sizeof(buf), str, args);
		length = MIN(length, sizeof(buf));
		va_end(args);

		tracing_format_string_from_user(buf, length);
	} else {
		va_start(args, str);
		tracing_format_string_handler(str, args);
		va_end(args);
	}
}

void tracing_format_data(u32_t nargs, ...)
{
	va_list args;

	if (_is_user_context()) {
		u32_t buf[CONFIG_TRACING_DUP_MAX_STRING / 4];

		if (nargs > CONFIG_TRACING_DUP_MAX_STRING / 4) {
			nargs = CONFIG_TRACING_DUP_MAX_STRING / 4;
		}

		va_start(args, nargs);
		for (u32_t index = 0; index < nargs; index++) {
			buf[index] = va_arg(args, u32_t);
		}
		va_end(args);

		tracing_format_data_from_user(nargs, buf);
	} else {
		va_start(args, nargs);
		tracing_format_data_handler(nargs, args);
		va_end(args);
	}

}
