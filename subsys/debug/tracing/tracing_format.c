/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

//TODO system call
static void tracing_format_string_from_user(const char* str, va_list args)
{
	char buffer[CONFIG_TRACING_DUP_MAX_STRING];

	vsnprintf(buffer, sizeof(buffer), str, args);
}

static void tracing_format_string_handler(const char* str, va_list args)
{
	struct tracing_packet *packet = tracing_packet_alloc();

	TRACING_PREFIX(packet);

	vsnprintf(packet->buf, sizeof(packet->buf), str, args);

	TRACING_POSTFIX(packet);

	//TODO add packet to list
}

void tracing_format_string(const char* str, ...)
{
	va_list args;

	va_start(args, str);

	if (_is_user_context()) {
		tracing_format_string_from_user(str, args);
	} else {
		tracing_format_string_handler(str, args);
	}

	va_end(args);
}
