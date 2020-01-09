/*
 * Copyright (c) 2018 Oticon A/S
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <stdio.h>
#include <kernel.h>
#include <assert.h>
#include <cmdline.h>
#include <tracing_backend.h>

static void *out_stream;
static const char *file_name;

static void tracing_backend_posix_init(void)
{
	if (file_name == NULL) {
		file_name = "log_posix";
	}

	out_stream = (void *)fopen(file_name, "wb");

	assert(out_stream != NULL);
}

static void tracing_backend_posix_output(
		const struct tracing_backend *backend,
		u8_t *data, u32_t length)
{
	fwrite(data, 1, length, (FILE *)out_stream);

	if (!k_is_in_isr()) {
		fflush((FILE *)out_stream);
	}
}

const struct tracing_backend_api tracing_backend_posix_api = {
	.init = tracing_backend_posix_init,
	.output  = tracing_backend_posix_output
};

TRACING_BACKEND_DEFINE(tracing_backend_posix, tracing_backend_posix_api);

void tracing_backend_posix_option(void)
{
	static struct args_struct_t tracing_backend_option[] = {
		{
			.manual = false,
			.is_mandatory = false,
			.is_switch = false,
			.option = "posix-file",
			.name = "file_name",
			.type = 's',
			.dest = (void *)&file_name,
			.call_when_found = NULL,
			.descript = "File name for posix tracing backend.",
		},
		ARG_TABLE_ENDMARKER
	};

	native_add_command_line_opts(tracing_backend_option);
}

NATIVE_TASK(tracing_backend_posix_option, PRE_BOOT_1, 1);
