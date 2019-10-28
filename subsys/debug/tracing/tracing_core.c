/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <ctype.h>
#include <kernel.h>
#include <assert.h>

static void tracing_thread_func(void *dummy1, void *dummy2, void *dummy3)
{
	thread_set(k_current_get());

	while (true) {
		;
	}
}

struct k_thread tracing_thread;

K_THREAD_STACK_DEFINE(tracing_thread_stack,
			CONFIG_TRACING_THREAD_STACK_SIZE);

static int enable_trace(void)
{
	k_thread_create(&tracing_thread, tracing_thread_stack,
			K_THREAD_STACK_SIZEOF(tracing_thread_stack),
			tracing_thread_func, NULL, NULL, NULL,
			K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
	k_thread_name_set(&tracing_thread, "trace");
}

SYS_INIT(enable_trace, POST_KERNEL, 0);
