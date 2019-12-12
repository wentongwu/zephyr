/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SUBSYS_DEBUG_TRACING_BOTTOMS_GENERIC_CTF_BOTTOM_H
#define SUBSYS_DEBUG_TRACING_BOTTOMS_GENERIC_CTF_BOTTOM_H

#include <stdio.h>
#include <string.h>
#include <sys/util.h>
#include <assert.h>
#include <debug/tracing_format.h>

/**
 * @brief Macro to do buffer copy for all the given parameter.
 *
 * Macro used internally. This Macro do the buffer copy for the
 * given parameter. TRACING_DATA will map this Macro to all of the
 * parameters with the help of MACRO_MAP.
 */
#define _TRACING_APPEND_ERR "please configure larger value for"                \
	STRINGIFY(CONFIG_TRACING_PACKET_BUF_SIZE) "to cover all the parameters"
#define _TRACING_PARAM_APPEND(x)                                               \
	{                                                                      \
		if (length + sizeof(x) <= CONFIG_TRACING_PACKET_BUF_SIZE) {    \
			memcpy(packet_index, &(x), sizeof(x));                 \
			length += sizeof(x);                                   \
			packet_index += sizeof(x);                             \
		} else {                                                       \
			__ASSERT(false, _TRACING_APPEND_ERR);                  \
		}                                                              \
	}

/**
 * @brief Macro to trace a message in data format.
 *
 * This Macro will copy all the parameters to the calling thread stack
 * buffer if context in user space or to the tracing_packet buffer and
 * at the same time get the length of the copied buffer, finally the
 * buffer and length will be given to tracing_format_data, or the
 * tracing_packet will be directly added to tracing list after the copy.
 */
#define TRACING_DATA(...)                                                      \
	do {                                                                   \
		u32_t length = 0;                                              \
		u8_t *packet_index = NULL;                                     \
									       \
		u8_t packet[CONFIG_TRACING_PACKET_BUF_SIZE];           \
									       \
		packet_index = &packet[0];                             \
		MACRO_MAP(_TRACING_PARAM_APPEND, ##__VA_ARGS__);       \
		tracing_format_data(packet, length);                   \
	} while (false)

/*
 * Gather fields to a contiguous event-packet, then atomically emit.
 * Used by middle-layer.
 */
#define CTF_BOTTOM_FIELDS(...)						    \
{									    \
	TRACING_DATA(__VA_ARGS__);                                          \
}

#define CTF_BOTTOM_LOCK()         { /* empty */ }
#define CTF_BOTTOM_UNLOCK()       { /* empty */ }

#define CTF_BOTTOM_TIMESTAMPED_INTERNALLY

void ctf_bottom_configure(void);

void ctf_bottom_start(void);

#endif
