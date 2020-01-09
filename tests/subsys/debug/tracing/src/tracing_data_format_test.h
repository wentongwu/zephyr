/*
 * Copyright (c) 2018 Oticon A/S
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TRACE_DATA_FORMAT_TEST_H
#define _TRACE_DATA_FORMAT_TEST_H

#include <stddef.h>
#include <string.h>
#include <debug/tracing_format.h>

/* Limit strings to 20 bytes to optimize bandwidth */
#define CTF_MAX_STRING_LEN 20

/* Anonymous compound literal with 1 member. Legal since C99.
 * This permits us to take the address of literals, like so:
 *  &CTF_LITERAL(int, 1234)
 *
 * This may be required if a ctf_bottom layer uses memcpy.
 *
 * NOTE string literals already support address-of and sizeof,
 * so string literals should not be wrapped with CTF_LITERAL.
 */
#define CTF_LITERAL(type, value)  ((type) { (type)(value) })

#define CTF_EVENT(...)					                       \
	{							               \
		const u32_t tstamp = k_cycle_get_32();                         \
									       \
		TRACING_DATA(TRACING_FORMAT_DATA(tstamp), __VA_ARGS__);        \
	}

#define CTF_EVENT_ID(event_id)                                                 \
	TRACING_FORMAT_DATA(CTF_LITERAL(u8_t, event_id))

#define CTF_EVENT_PAYLOAD_THREAD_CONTENT(tid, stack_base,                      \
					 stack_size, priority, thread_name)    \
	TRACING_FORMAT_DATA(tid),                                              \
	TRACING_FORMAT_DATA(stack_base),                                       \
	TRACING_FORMAT_DATA(stack_size),                                       \
	TRACING_FORMAT_DATA(priority),                                         \
	TRACING_FORMAT_DATA(thread_name)

#define CTF_EVENT_PAYLOAD_CALL_ID(call_id)                                     \
	TRACING_FORMAT_DATA(call_id)

enum ctf_event_id {
	CTF_EVENT_ID_THREAD_SWITCHED_OUT   =  0x10,
	CTF_EVENT_ID_THREAD_SWITCHED_IN    =  0x11,
	CTF_EVENT_ID_THREAD_PRIORITY_SET   =  0x12,
	CTF_EVENT_ID_THREAD_CREATE         =  0x13,
	CTF_EVENT_ID_THREAD_ABORT          =  0x14,
	CTF_EVENT_ID_THREAD_SUSPEND        =  0x15,
	CTF_EVENT_ID_THREAD_RESUME         =  0x16,
	CTF_EVENT_ID_THREAD_READY          =  0x17,
	CTF_EVENT_ID_THREAD_PENDING        =  0x18,
	CTF_EVENT_ID_THREAD_INFO           =  0x19,
	CTF_EVENT_ID_THREAD_NAME_SET       =  0x1A,
	CTF_EVENT_ID_ISR_ENTER             =  0x20,
	CTF_EVENT_ID_ISR_EXIT              =  0x21,
	CTF_EVENT_ID_ISR_EXIT_TO_SCHEDULER =  0x22,
	CTF_EVENT_ID_IDLE                  =  0x30,
	CTF_EVENT_ID_START_CALL            =  0x41,
	CTF_EVENT_ID_END_CALL              =  0x42
};

typedef struct {
	char buf[CTF_MAX_STRING_LEN];
} ctf_bounded_string_t;

static inline void ctf_top_thread_switched_out(u32_t thread_id,
					       u32_t stack_base,
					       u32_t stack_size,
					       s8_t prio,
					       ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_SWITCHED_OUT),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_switched_in(u32_t thread_id,
					      u32_t stack_base,
					      u32_t stack_size,
					      s8_t prio,
					      ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_SWITCHED_IN),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_priority_set(u32_t thread_id,
					       u32_t stack_base,
					       u32_t stack_size,
					       s8_t prio,
					       ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_PRIORITY_SET),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_create(u32_t thread_id,
					 u32_t stack_base,
					 u32_t stack_size,
					 s8_t prio,
					 ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_CREATE),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_abort(u32_t thread_id,
					u32_t stack_base,
					u32_t stack_size,
					s8_t prio,
					ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_ABORT),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_suspend(u32_t thread_id,
					  u32_t stack_base,
					  u32_t stack_size,
					  s8_t prio,
					  ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_SUSPEND),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_resume(u32_t thread_id,
					 u32_t stack_base,
					 u32_t stack_size,
					 s8_t prio,
					 ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_RESUME),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_ready(u32_t thread_id,
					u32_t stack_base,
					u32_t stack_size,
					s8_t prio,
					ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_READY),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_pend(u32_t thread_id,
				       u32_t stack_base,
				       u32_t stack_size,
				       s8_t prio,
				       ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_PENDING),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_info(u32_t thread_id,
				       u32_t stack_base,
				       u32_t stack_size,
				       s8_t prio,
				       ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_INFO),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_thread_name_set(u32_t thread_id,
					   u32_t stack_base,
					   u32_t stack_size,
					   s8_t prio,
					   ctf_bounded_string_t name)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_THREAD_NAME_SET),
		CTF_EVENT_PAYLOAD_THREAD_CONTENT(thread_id, stack_base,
						 stack_size, prio, name)
		);
}

static inline void ctf_top_isr_enter(void)
{
}

static inline void ctf_top_isr_exit(void)
{
}

static inline void ctf_top_isr_exit_to_scheduler(void)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_ISR_EXIT_TO_SCHEDULER)
		);
}

static inline void ctf_top_idle(void)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_IDLE)
		);
}

static inline void ctf_top_void(u32_t id)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_START_CALL),
		CTF_EVENT_PAYLOAD_CALL_ID(id)
		);
}

static inline void ctf_top_end_call(u32_t id)
{
	CTF_EVENT(
		CTF_EVENT_ID(CTF_EVENT_ID_END_CALL),
		CTF_EVENT_PAYLOAD_CALL_ID(id)
		);
}

#endif
