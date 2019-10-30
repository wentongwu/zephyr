/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TRACE_PACKET_H
#define _TRACE_PACKET_H

#include <sys/slist.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tracing_packet {
	sys_snode_t list_node;
	u32_t length;
	u8_t bytes[CONFIG_TRACING_DUP_MAX_STRING];
};

void tracing_packet_pool_init(void);
struct tracing_packet *tracing_packet_alloc(void);
void tracing_packet_free(struct tracing_packet *packet);

#ifdef __cplusplus
}
#endif

#endif
