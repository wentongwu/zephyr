# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(tracing_tests)

target_sources(app PRIVATE src/main.c)
target_sources_ifdef(
  CONFIG_TRACING_STRING_FORMAT_TEST
  app PRIVATE
  src/tracing_string_format_test.c
)

target_sources_ifdef(
  CONFIG_TRACING_DATA_FORMAT_TEST
  app PRIVATE
  src/tracing_data_format_test.c
)

zephyr_include_directories_ifdef(
  CONFIG_TRACING_TEST
  ${APPLICATION_SOURCE_DIR}/src/
)
