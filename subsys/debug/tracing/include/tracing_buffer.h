/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TRACE_BUFFER_H
#define _TRACE_BUFFER_H

#include <sys/slist.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tracing_buffer_str_put(const char *str, va_list args);
bool tracing_buffer_put(u8_t *data, u32_t size);
u32_t tracing_buffer_get(u8_t *data, u32_t size);
int tracing_buffer_get_finish(u32_t size);
void tracing_buffer_init(void);

bool tracing_buffer_is_empty(void);
u32_t tracing_buffer_capacity_get(void);
#ifdef __cplusplus
}
#endif

#endif
