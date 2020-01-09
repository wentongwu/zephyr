/*
 * Copyright (c) 2018 Oticon A/S
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <tracing_test.h>
#include <kernel_structs.h>
#include <kernel_internal.h>
#include <tracing_data_format_test.h>

#define THREAD_CONTENT_PARAM                                \
	(u32_t)(uintptr_t)thread, thread->stack_info.start, \
	thread->stack_info.size, thread->base.prio, name


static inline void sys_trace_thread_name_process(struct k_thread *pthread,
						 ctf_bounded_string_t *name)
{
	const char *tname = k_thread_name_get(pthread);

	if (tname != NULL) {
		strncpy(name->buf, tname, sizeof(name->buf));
		/* strncpy may not always null-terminate */
		name->buf[sizeof(name->buf) - 1] = 0;
	}
}

void sys_trace_thread_switched_out(void)
{
	struct k_thread *thread = k_current_get();
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_switched_out(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_switched_in(void)
{
	struct k_thread *thread = k_current_get();
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_switched_in(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_priority_set(struct k_thread *thread)
{
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_priority_set(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_create(struct k_thread *thread)
{
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_create(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_abort(struct k_thread *thread)
{
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_abort(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_suspend(struct k_thread *thread)
{
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_suspend(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_resume(struct k_thread *thread)
{
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_resume(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_ready(struct k_thread *thread)
{
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_ready(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_pend(struct k_thread *thread)
{
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_pend(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_info(struct k_thread *thread)
{
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_info(THREAD_CONTENT_PARAM);
}

void sys_trace_thread_name_set(struct k_thread *thread)
{
	ctf_bounded_string_t name = { "Unnamed thread" };

	sys_trace_thread_name_process(thread, &name);

	ctf_top_thread_name_set(THREAD_CONTENT_PARAM);
}

void sys_trace_isr_enter(void)
{
	ctf_top_isr_enter();
}

void sys_trace_isr_exit(void)
{
	ctf_top_isr_exit();
}

void sys_trace_isr_exit_to_scheduler(void)
{
	ctf_top_isr_exit_to_scheduler();
}

void sys_trace_idle(void)
{
	ctf_top_idle();
}

void sys_trace_void(unsigned int id)
{
	ctf_top_void(id);
}

void sys_trace_end_call(unsigned int id)
{
	ctf_top_end_call(id);
}
