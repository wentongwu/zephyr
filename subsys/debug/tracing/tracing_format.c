/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <tracing_format.h>

static void tracing_format_string_handler(const char* str, va_list args)
{
	struct tracing_packet *packet = tracing_packet_alloc();

	TRACING_PREFIX(packet);

	vsnprintf(packet->buf, sizeof(packet->buf), str, args);

	TRACING_POSTFIX(packet);

	tracing_list_add_packet(packet);
}

#ifdef CONFIG_USERSPACE
void z_impl_tracing_format_string_from_user(const char* str)
{

}

void z_vrfy_tracing_format_string_from_user(const char* str)
{

}
#include <syscalls/tracing_format_string_from_user_mrsh.c>
#endif

void tracing_format_string(const char* str, ...)
{
	va_list args;

	va_start(args, str);

	if (_is_user_context()) {
		char buffer[CONFIG_TRACING_DUP_MAX_STRING];

		vsnprintf(buffer, sizeof(buffer), str, args);

		tracing_format_string_from_user(buffer);
	} else {
		tracing_format_string_handler(str, args);
	}

	va_end(args);
}
