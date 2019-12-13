/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _TRACE_BACKEND_H
#define _TRACE_BACKEND_H

#include <sys/util.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Tracing backend
 * @defgroup Tracing_backend Tracing backend
 * @{
 * @}
 */

struct tracing_backend;

/**
 * @brief Tracing backend API.
 */
struct tracing_backend_api {
	void (*init)(void);
	void (*output)(const struct tracing_backend *backend,
		       u8_t *data, u32_t length);
};

/**
 * @brief Tracing backend structure.
 */
struct tracing_backend {
	const char *name;
	const struct tracing_backend_api *api;
};

/**
 * @brief Create tracing_backend instance.
 *
 * @param _name Instance name.
 * @param _api  Tracing backend API.
 */
#define TRACING_BACKEND_DEFINE(_name, _api)                              \
	static const Z_STRUCT_SECTION_ITERABLE(tracing_backend, _name) = \
	{                                                                \
		.name = STRINGIFY(_name),                                \
		.api = &_api,                                            \
	}

/**
 * @brief Initialize tracing backend.
 *
 * @param backend Pointer to tracing_backend instance.
 */
static inline void tracing_backend_init(
		const struct tracing_backend *backend)
{
	if (backend) {
		backend->api->init();
	}
}

/**
 * @brief Output tracing packet with tracing backend.
 *
 * @param backend Pointer to tracing_backend instance.
 * @param packet  Pointer to tracing_packet instance.
 */
static inline void tracing_backend_output(
		const struct tracing_backend *backend,
		u8_t *data, u32_t length)
{
	if (backend) {
		backend->api->output(backend, data, length);
	}
}

extern char __tracing_backends_start[];
extern char __tracing_backends_end[];

/**
 * @brief Get the number of enabled backends.
 *
 * @return Number of enabled backends.
 */
static inline u32_t tracing_backend_num_get(void)
{
	return (__tracing_backends_end -
			__tracing_backends_start) /
				sizeof(struct tracing_backend);
}

/**
 * @brief Get the backend pointer based on the
 *        index in tracing backend section.
 *
 * @return Pointer of the backend.
 */
static inline const struct tracing_backend *tracing_backend_get(u32_t index)
{
	const struct tracing_backend *tracing_backend_start_addr =
		(const struct tracing_backend *)&__tracing_backends_start;

	return tracing_backend_start_addr + index;
}

#ifdef __cplusplus
}
#endif

#endif
