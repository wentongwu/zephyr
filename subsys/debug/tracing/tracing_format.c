/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <tracing_format.h>

static void tracing_format_string_handler(const char* str, va_list args)
{
	int ret = 0;
	struct tracing_packet *packet = tracing_packet_alloc();

	ret = vsnprintf(packet->buf, sizeof(packet->buf), str, args);
	packet->length = ret < sizeof(packet->buf) ? ret : sizeof(packet->buf);

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
void z_impl_tracing_format_string_from_user(const char* str)
{
	struct tracing_packet *packet = tracing_packet_alloc();

	strcpy(packet->buf, str);

	tracing_list_add_packet(packet);
}

void z_vrfy_tracing_format_string_from_user(const char* str)
{
	Z_OOPS(Z_SYSCALL_VERIFY_MSG(str == NULL,
		"Invalid parameter str"));

	z_impl_tracing_format_string_from_user(str);
}
#include <syscalls/tracing_format_string_from_user_mrsh.c>
#endif

void tracing_format_string(const char* str, ...)
{
	va_list args;

	va_start(args, str);

	if (_is_user_context()) {
		char buf[CONFIG_TRACING_DUP_MAX_STRING + 1];

		vsnprintf(buf, CONFIG_TRACING_DUP_MAX_STRING, str, args);

		buf[CONFIG_TRACING_DUP_MAX_STRING] = '\0';

		tracing_format_string_from_user(buf);
	} else {
		tracing_format_string_handler(str, args);
	}

	va_end(args);
}

#ifdef CONFIG_USERSPACE
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
		"Invalid parameter nargs"));

	z_impl_tracing_format_data_from_user(nargs, buf);
}
#include <syscalls/tracing_format_data_from_user_mrsh.c>
#endif

void tracing_format_data(u32_t nargs, ...)
{
	va_list args;

	va_start(args, nargs);

	if (_is_user_context()) {
		u32_t buf[CONFIG_TRACING_DUP_MAX_STRING / 4];

		if (nargs > CONFIG_TRACING_DUP_MAX_STRING / 4) {
			nargs = CONFIG_TRACING_DUP_MAX_STRING / 4;
		}

		for (u32_t index = 0; index < nargs; index++) {
			buf[index] = va_arg(args, u32_t);
		}

		tracing_format_data_from_user(nargs, buf);
	} else {
		tracing_format_data_handler(nargs, args);
	}

	va_end(args);
}
