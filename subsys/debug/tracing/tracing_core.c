/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <string.h>
#include <kernel.h>
#include <spinlock.h>
#include <sys/util.h>
#include <sys/atomic.h>
#include <tracing_backend.h>
#include <debug/tracing_core.h>
#include <syscall_handler.h>
#include <tracing_buffer.h>

#define TRACING_CMD_ENABLE  "enable"
#define TRACING_CMD_DISABLE "disable"
#define TRACING_THREAD_NAME "tracing_thread"

#define TRACING_BACKEND_UART_NAME "tracing_backend_uart"
#define TRACING_BACKEND_USB_NAME  "tracing_backend_usb"

enum tracing_state {
	TRACING_DISABLE = 0,
	TRACING_ENABLE
};

static atomic_t tracing_state;
static const struct tracing_backend *working_backend;

static k_tid_t tracing_thread_tid;
static struct k_thread tracing_thread;
static K_THREAD_STACK_DEFINE(tracing_thread_stack,
			CONFIG_TRACING_THREAD_STACK_SIZE);

static void tracing_set_state(enum tracing_state state)
{
	atomic_set(&tracing_state, state);
}

static const struct tracing_backend *tracing_get_working_backend(
		const char *name)
{
	u32_t num = tracing_backend_num_get();
	const struct tracing_backend *backend = NULL;

	for (u32_t index = 0; index < num; index++) {
		backend = tracing_backend_get(index);
		if (strcmp(backend->name, name) == 0) {
			return backend;
		}
	}

	return NULL;
}

static void tracing_thread_func(void *dummy1, void *dummy2, void *dummy3)
{
	u8_t *transfering_buf;
	u32_t transfering_length;
	u32_t tracing_buffer_max_length = tracing_buffer_capacity_get();

	tracing_thread_tid = k_current_get();

	if (IS_ENABLED(CONFIG_TRACING_HANDLE_HOST_CMD)) {
		tracing_set_state(TRACING_DISABLE);
	} else {
		tracing_set_state(TRACING_ENABLE);
	}

	while (true) {
		if (tracing_buffer_is_empty()) {
			k_sleep(100);
		} else {
			transfering_length =
				tracing_buffer_get(&transfering_buf,
						   tracing_buffer_max_length);

			tracing_buffer_handle(transfering_buf,
					      transfering_length);
			tracing_buffer_get_finish(transfering_length);
		}
	}
}

static int tracing_init(struct device *arg)
{
	ARG_UNUSED(arg);

	tracing_buffer_init();

	if (working_backend == NULL) {
		if (IS_ENABLED(CONFIG_TRACING_BACKEND_USB)) {
			working_backend =
			tracing_get_working_backend(TRACING_BACKEND_USB_NAME);

			tracing_backend_init(working_backend);
		} else if (IS_ENABLED(CONFIG_TRACING_BACKEND_UART)) {
			working_backend =
			tracing_get_working_backend(TRACING_BACKEND_UART_NAME);

			tracing_backend_init(working_backend);
		}
	}

	k_thread_create(&tracing_thread, tracing_thread_stack,
			K_THREAD_STACK_SIZEOF(tracing_thread_stack),
			tracing_thread_func, NULL, NULL, NULL,
			K_LOWEST_APPLICATION_THREAD_PRIO, 0, K_NO_WAIT);
	k_thread_name_set(&tracing_thread, TRACING_THREAD_NAME);

	return 0;
}

SYS_INIT(tracing_init, POST_KERNEL, 0);

bool z_impl_is_tracing_enabled(void)
{
	return atomic_get(&tracing_state) == TRACING_ENABLE;
}

#ifdef CONFIG_USERSPACE
bool z_vrfy_is_tracing_enabled(void)
{
	return z_impl_is_tracing_enabled();
}
#include <syscalls/is_tracing_enabled_mrsh.c>
#endif

bool is_tracing_thread(void)
{
	return (!k_is_in_isr() && (k_current_get() == tracing_thread_tid));
}

void tracing_cmd_handle(u8_t *buf, u32_t length)
{
	if (strncmp(buf,
	    TRACING_CMD_ENABLE, length) == 0) {
		tracing_set_state(TRACING_ENABLE);
	} else if (strncmp(buf,
	    TRACING_CMD_DISABLE, length) == 0) {
		tracing_set_state(TRACING_DISABLE);
	}
}

void tracing_buffer_handle(u8_t *data, u32_t length)
{
	tracing_backend_output(working_backend, data, length);
}
