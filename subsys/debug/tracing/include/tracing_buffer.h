/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TRACE_BUFFER_H
#define _TRACE_BUFFER_H

#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize tracing buffer.
 */
void tracing_buffer_init(void);

/**
 * @brief Tracing buffer is empty or not.
 *
 * @return true if the ring buffer is empty, or false if not.
 */
bool tracing_buffer_is_empty(void);

/**
 * @brief Get free space in the tracing buffer.
 *
 * @return Tracing buffer free space (in bytes).
 */
u32_t tracing_buffer_space_get(void);

/**
 * @brief Get tracing buffer capacity (max size).
 *
 * @return Tracing buffer capacity (in bytes).
 */
u32_t tracing_buffer_capacity_get(void);

/**
 * @brief Try to allocate buffer in the tracing buffer.
 *
 * @param data Pointer to the address. It's set to a location
 *             within the tracing buffer.
 * @param size Requested buffer size (in bytes).
 *
 * @return Size of allocated buffer which can be smaller than
 *         requested if there isn't enough free space or buffer wraps.
 */
u32_t tracing_buffer_put_claim(u8_t **data, u32_t size);

/**
 * @brief Indicate number of bytes written to the allocated buffer.
 *
 * @param size Number of bytes written to the allocated buffer.
 *
 * @retval 0 Successful operation.
 * @retval -EINVAL Given @a size exceeds free space of the tracing buffer.
 */
int tracing_buffer_put_finish(u32_t size);

/**
 * @brief Write data to tracing buffer.
 *
 * This routine writes data to the tracing buffer.
 *
 * @param data Address of data.
 * @param size Data size (in bytes).
 *
 * @retval Number of bytes written.
 */
u32_t tracing_buffer_put(u8_t *data, u32_t size);

u32_t tracing_buffer_get(u8_t *data, u32_t size);
u32_t tracing_buffer_get_claim(u8_t **data, u32_t size);
int tracing_buffer_get_finish(u32_t size);
u8_t *tracing_cmd_buffer_alloc(void);

#ifdef __cplusplus
}
#endif

#endif
