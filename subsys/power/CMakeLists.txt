# SPDX-License-Identifier: Apache-2.0

zephyr_sources_ifdef(CONFIG_SYS_POWER_MANAGEMENT    power.c)
zephyr_sources_ifdef(CONFIG_DEVICE_POWER_MANAGEMENT device.c)
zephyr_sources_ifdef(CONFIG_SYS_PM_STATE_LOCK       pm_ctrl.c)
zephyr_sources_ifdef(CONFIG_DEVICE_IDLE_PM	    device_pm.c)
zephyr_sources_if_kconfig(reboot.c)
add_subdirectory(policy)

zephyr_sources(
  rt_dpm.c
)

zephyr_include_directories(
  ${ZEPHYR_BASE}/kernel/include
  ${ZEPHYR_BASE}/arch/${ARCH}/include
)
