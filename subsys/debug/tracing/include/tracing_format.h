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

__syscall void tracing_format_string_from_user(const char* str);

#ifdef __cplusplus
}
#endif

#endif
