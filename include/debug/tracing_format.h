/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TRACE_FORMAT_H
#define _TRACE_FORMAT_H

#include <syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

__syscall void tracing_format_string_from_user(const char* str, u32_t length);

__syscall void tracing_format_data_from_user(u32_t nargs, u32_t *buf);

#ifdef __cplusplus
}
#endif

#endif
