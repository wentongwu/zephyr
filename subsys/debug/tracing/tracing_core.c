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
#if 0
static struct k_timer tracing_thread_timer;
static K_SEM_DEFINE(tracing_thread_sem, 0, 1);
#endif
static K_THREAD_STACK_DEFINE(tracing_thread_stack,
			CONFIG_TRACING_THREAD_STACK_SIZE);

static void tracing_set_state(enum tracing_state state)
{
	atomic_set(&tracing_state, state);
}

void tracing_cmd_handle(u8_t *buf)
{
	if (strncmp(buf,
	    TRACING_CMD_ENABLE, 6) == 0) {
		tracing_set_state(TRACING_ENABLE);
	} else if (strncmp(buf,
	    TRACING_CMD_DISABLE, 6) == 0) {
		tracing_set_state(TRACING_DISABLE);
	}
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

static void tracing_buffer_handle(u8_t *data, u32_t size)
{
	tracing_backend_output(working_backend, data, size);
}

static void tracing_thread_func(void *dummy1, void *dummy2, void *dummy3)
{
	u8_t *transfering_buf = NULL;
	u32_t transfering_size = 0;
	u32_t working_backend_max_size =
		tracing_backend_get_max_buffer_size(working_backend);

	tracing_thread_tid = k_current_get();

	if (IS_ENABLED(CONFIG_TRACING_HANDLE_HOST_CMD)) {
		tracing_set_state(TRACING_DISABLE);
	} else {
		tracing_set_state(TRACING_ENABLE);
	}

	printk("working_backend_max_size = %d\n", working_backend_max_size);
	while (true) {
		if (tracing_buffer_empty()) {
			//k_sem_take(&tracing_thread_sem, K_FOREVER);
			k_sleep(100);
		} else {
			transfering_size = tracing_buffer_get(transfering_buf, 1000);
						//working_backend_max_size - 1);
			printk("transfering_size = %d\n", transfering_size);
			tracing_buffer_handle(transfering_buf,
					      transfering_size);

			tracing_buffer_get_finish(transfering_size);
		}
	}
}

#if 0
static void tracing_thread_timer_expiry_fn(struct k_timer *timer)
{
	k_sem_give(&tracing_thread_sem);
}
#endif

static int tracing_init(struct device *arg)
{
	ARG_UNUSED(arg);

#if 0
	k_timer_init(&tracing_thread_timer,
		tracing_thread_timer_expiry_fn, NULL);
#endif

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

#if 0
void tracing_core_process(void)
{
	if (tracing_thread_tid != NULL && tracing_list_packet_num == 1) {
		k_timer_start(&tracing_thread_timer,
			CONFIG_TRACING_THREAD_WAIT_THRESHOLD_MS, K_NO_WAIT);
	} else if (CONFIG_TRACING_THREAD_TRIGGER_THRESHOLD &&
	    tracing_thread_tid != NULL && (tracing_list_packet_num ==
	    CONFIG_TRACING_THREAD_TRIGGER_THRESHOLD)) {
		k_timer_stop(&tracing_thread_timer);
		k_sem_give(&tracing_thread_sem);
	}
}

bool tracing_packet_try_free(void)
{
	struct tracing_packet *packet = NULL;

	packet = tracing_list_get_packet();
	if (packet) {
		tracing_packet_handle(packet);
	}

	return tracing_list_peek_head();
}
#endif

bool is_tracing_thread(void)
{
	return (!k_is_in_isr() && (k_current_get() == tracing_thread_tid));
}
