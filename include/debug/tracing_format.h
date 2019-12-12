/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_TRACE_FORMAT_H
#define ZEPHYR_INCLUDE_TRACE_FORMAT_H

#include <syscall.h>
#include <toolchain/common.h>
#include <debug/tracing_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Macro to trace a message in string format.
 */
#define TRACING_STRING(fmt, ...)                                          \
	do {                                                              \
		tracing_format_string(fmt, ##__VA_ARGS__);                \
	} while (false)

/**
 * @brief Tracing a message in string format.
 *
 * @param str String to format.
 * @param ... Variable length arguments.
 */
void tracing_format_string(const char *str, ...);

/* Internal function used by tracing_format_string */
__syscall void z_tracing_format_raw_str(const char *data, u32_t length);

/**
 * @brief Tracing a message in data format.
 *
 * @param data   Data to be traced.
 * @param length Data length.
 */
__syscall void tracing_format_data(const char *data, u32_t length);

#include <syscalls/tracing_format.h>

#ifdef __cplusplus
}
#endif

#endif
