# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load(
    "@bazel_skylib//lib:dicts.bzl",
    "dicts",
)
load(
    "//rules:cross_platform.bzl",
    "dual_cc_device_library_of",
    "dual_cc_library",
    "dual_inputs",
)
load(
    "//rules/opentitan:defs.bzl",
    "EARLGREY_TEST_ENVS",
    "fpga_params",
    "opentitan_test",
    "qemu_params",
    "verilator_params",
)

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "english_breakfast_test_rom_driver_srcs",
    srcs = [
        "flash_ctrl.c",
        "flash_ctrl.h",
        "flash_ctrl_info_pages.c",
        "lifecycle.h",
        "otp.h",
        "rstmgr.h",
        "spi_device.c",
        "spi_device.h",
    ],
)

dual_cc_library(
    name = "alert",
    srcs = dual_inputs(
        device = ["alert.c"],
        host = ["mock_alert.cc"],
    ),
    hdrs = dual_inputs(
        host = ["mock_alert.h"],
        shared = ["alert.h"],
    ),
    deps = dual_inputs(
        device = [
            "//hw/top:alert_handler_c_regs",
            "//hw/top:otp_ctrl_c_regs",
            "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        ],
        host = [
            "//sw/device/lib/base:global_mock",
            "//sw/device/silicon_creator/testing:rom_test",
            "@googletest//:gtest",
        ],
        shared = [
            ":lifecycle",
            ":otp",
            "//sw/device/lib/base:abs_mmio",
            "//sw/device/lib/base:macros",
            "//sw/device/lib/base:memory",
            "//sw/device/lib/base:crc32",
            "//sw/device/silicon_creator/lib:error",
        ],
    ),
)

cc_test(
    name = "alert_unittest",
    srcs = ["alert_unittest.cc"],
    deps = [
        dual_cc_device_library_of(":alert"),
        "//hw/top:alert_handler_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:crc32",
        "@googletest//:gtest_main",
    ],
)

opentitan_test(
    name = "alert_functest",
    srcs = ["alert_functest.c"],
    exec_env = EARLGREY_TEST_ENVS,
    verilator = verilator_params(
        timeout = "long",
    ),
    deps = [
        ":alert",
        ":rstmgr",
        "//hw/top:alert_handler_c_regs",
        "//hw/top:flash_ctrl_c_regs",
        "//hw/top:otp_ctrl_c_regs",
        "//hw/top:rstmgr_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/testing:rstmgr_testutils",
        "//sw/device/lib/testing/test_framework:ottf_main",
    ],
)

cc_library(
    name = "ast",
    srcs = ["ast.c"],
    hdrs = ["ast.h"],
    deps = [
        ":lifecycle",
        ":otp",
        "//hw/top:ast_c_regs",
        "//hw/top:otp_ctrl_c_regs",
        "//hw/top:sensor_ctrl_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/arch:device",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:bitfield",
        "//sw/device/lib/base:csr",
        "//sw/device/lib/base:hardened",
        "//sw/device/lib/base:memory",
        "//sw/device/lib/base:multibits",
        "//sw/device/silicon_creator/lib:error",
    ],
)

cc_test(
    name = "ast_unittest",
    srcs = ["ast_unittest.cc"],
    deps = [
        ":ast",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

dual_cc_library(
    name = "flash_ctrl",
    srcs = dual_inputs(
        device = ["flash_ctrl.c"],
        host = ["mock_flash_ctrl.cc"],
        shared = ["flash_ctrl_info_pages.c"],
    ),
    hdrs = dual_inputs(
        host = ["mock_flash_ctrl.h"],
        shared = ["flash_ctrl.h"],
    ),
    deps = dual_inputs(
        device = [
            "//hw/top:otp_ctrl_c_regs",
        ],
        host = [
            "//sw/device/lib/base:global_mock",
            "//sw/device/silicon_creator/testing:rom_test",
            "@googletest//:gtest",
        ],
        shared = [
            "//hw/top:dt",
            "//hw/top:flash_ctrl_c_regs",
            "//sw/device/lib/base:abs_mmio",
            "//sw/device/lib/base:bitfield",
            "//sw/device/lib/base:hardened",
            "//sw/device/lib/base:memory",
            "//sw/device/lib/base:multibits",
            "//sw/device/silicon_creator/lib:error",
            "//sw/device/silicon_creator/lib/base:sec_mmio",
            "//sw/device/silicon_creator/lib/drivers:otp",
        ],
    ),
)

cc_test(
    name = "flash_ctrl_unittest",
    srcs = ["flash_ctrl_unittest.cc"],
    deps = [
        dual_cc_device_library_of(":flash_ctrl"),
        "@abseil-cpp//absl/strings:str_format",
        "@googletest//:gtest_main",
    ],
)

dual_cc_library(
    name = "hmac",
    srcs = dual_inputs(
        device = ["hmac.c"],
        host = ["mock_hmac.cc"],
    ),
    hdrs = dual_inputs(
        host = ["mock_hmac.h"],
        shared = ["hmac.h"],
    ),
    deps = dual_inputs(
        device = [
            "//hw/top:hmac_c_regs",
            "//hw/top:dt",
        ],
        host = [
            "//sw/device/lib/base:global_mock",
            "//sw/device/silicon_creator/testing:rom_test",
            "@googletest//:gtest",
        ],
        shared = [
            "//sw/device/lib/base:abs_mmio",
            "//sw/device/lib/base:bitfield",
            "//sw/device/lib/base:hardened",
            "//sw/device/lib/base:memory",
            "//sw/device/silicon_creator/lib:error",
        ],
    ),
)

cc_test(
    name = "hmac_unittest",
    srcs = ["hmac_unittest.cc"],
    deps = [
        dual_cc_device_library_of(":hmac"),
        "//hw/top:hmac_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/silicon_creator/lib:error",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

opentitan_test(
    name = "hmac_functest",
    srcs = ["hmac_functest.c"],
    exec_env = EARLGREY_TEST_ENVS,
    verilator = verilator_params(
        timeout = "long",
    ),
    deps = [
        ":hmac",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/testing:hexstr",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:error",
    ],
)

dual_cc_library(
    name = "ibex",
    srcs = dual_inputs(
        host = ["ibex_host.c"],
        shared = ["ibex.c"],
    ),
    hdrs = dual_inputs(
        shared = ["ibex.h"],
    ),
    deps = dual_inputs(
        shared = [
            "//hw/top:rv_core_ibex_c_regs",
            "//hw/top:dt",
            "//sw/device/lib/arch:device",
            "//sw/device/lib/base:abs_mmio",
            "//sw/device/lib/base:csr",
            "//sw/device/lib/base:hardened",
            "//sw/device/lib/base:macros",
            "//sw/device/lib/runtime:hart",
            "//sw/device/silicon_creator/lib/base:sec_mmio",
        ],
    ),
)

cc_test(
    name = "ibex_unittest",
    srcs = ["ibex_unittest.cc"],
    deps = [
        ":ibex",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "keymgr",
    srcs = ["keymgr.c"],
    hdrs = ["keymgr.h"],
    deps = [
        "//hw/top:keymgr_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:macros",
        "//sw/device/lib/runtime:hart",
        "//sw/device/silicon_creator/lib:error",
        "//sw/device/silicon_creator/lib:keymgr_binding",
        "//sw/device/silicon_creator/lib/base:sec_mmio",
    ],
)

cc_test(
    name = "keymgr_unittest",
    srcs = ["keymgr_unittest.cc"],
    deps = [
        ":keymgr",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

opentitan_test(
    name = "keymgr_functest",
    srcs = ["keymgr_functest.c"],
    broken = fpga_params(tags = ["broken"]),
    exec_env = dicts.add(
        EARLGREY_TEST_ENVS,
        {
            "//hw/top_earlgrey:fpga_cw310_test_rom": None,
            "//hw/top_earlgrey:silicon_creator": None,
            # FIXME broken in sival ROM_EXT, remove this line when fixed. See #21706.
            "//hw/top_earlgrey:fpga_cw310_sival_rom_ext": "broken",
        },
    ),
    verilator = verilator_params(
        timeout = "long",
    ),
    deps = [
        ":keymgr",
        ":kmac",
        ":lifecycle",
        ":retention_sram",
        "//hw/top:kmac_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/dif:flash_ctrl",
        "//sw/device/lib/dif:otp_ctrl",
        "//sw/device/lib/dif:rstmgr",
        "//sw/device/lib/testing:flash_ctrl_testutils",
        "//sw/device/lib/testing:keymgr_testutils",
        "//sw/device/lib/testing:otp_ctrl_testutils",
        "//sw/device/lib/testing:pwrmgr_testutils",
        "//sw/device/lib/testing:rstmgr_testutils",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:error",
        "//sw/device/silicon_creator/lib:keymgr_binding",
        "//sw/device/silicon_creator/lib/base:chip",
        "//sw/device/silicon_creator/lib/base:sec_mmio",
    ],
)

cc_library(
    name = "kmac",
    srcs = ["kmac.c"],
    hdrs = ["kmac.h"],
    deps = [
        "//hw/top:kmac_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:macros",
        "//sw/device/silicon_creator/lib:error",
    ],
)

cc_test(
    name = "kmac_unittest",
    srcs = ["kmac_unittest.cc"],
    deps = [
        ":kmac",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

opentitan_test(
    name = "kmac_functest",
    srcs = ["kmac_functest.c"],
    exec_env = EARLGREY_TEST_ENVS,
    verilator = verilator_params(
        timeout = "long",
    ),
    deps = [
        ":kmac",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/testing:entropy_testutils",
        "//sw/device/lib/testing:hexstr",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:error",
    ],
)

dual_cc_library(
    name = "lifecycle",
    srcs = dual_inputs(
        device = ["lifecycle.c"],
        host = ["mock_lifecycle.cc"],
    ),
    hdrs = dual_inputs(
        host = ["mock_lifecycle.h"],
        shared = ["lifecycle.h"],
    ),
    deps = dual_inputs(
        device = [
            "//hw/top:lc_ctrl_c_regs",
            "//hw/top:dt",
        ],
        host = [
            "//sw/device/lib/base:global_mock",
            "//sw/device/silicon_creator/testing:rom_test",
            "@googletest//:gtest",
        ],
        shared = [
            "//sw/device/lib/base:bitfield",
            "//sw/device/lib/base:hardened",
            "//sw/device/lib/base:macros",
            "//sw/device/silicon_creator/lib/base:sec_mmio",
        ],
    ),
)

cc_test(
    name = "lifecycle_unittest",
    srcs = ["lifecycle_unittest.cc"],
    deps = [
        dual_cc_device_library_of(":lifecycle"),
        "//hw/top:lc_ctrl_c_regs",
        "//hw/top:top_lib",
        "//sw/device/lib/base:hardened",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "otbn",
    srcs = ["otbn.c"],
    hdrs = ["otbn.h"],
    deps = [
        "//hw/top:otbn_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:bitfield",
        "//sw/device/silicon_creator/lib:error",
        "//sw/device/silicon_creator/lib/base:sec_mmio",
        "//sw/device/silicon_creator/lib/drivers:rnd",
    ],
)

cc_test(
    name = "otbn_unittest",
    srcs = ["otbn_unittest.cc"],
    deps = [
        ":otbn",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

dual_cc_library(
    name = "otp",
    srcs = dual_inputs(
        device = ["otp.c"],
        host = ["mock_otp.cc"],
    ),
    hdrs = dual_inputs(
        host = ["mock_otp.h"],
        shared = ["otp.h"],
    ),
    deps = dual_inputs(
        device = [
            "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        ],
        host = [
            "//sw/device/lib/base:global_mock",
            "//sw/device/silicon_creator/testing:rom_test",
            "@googletest//:gtest",
        ],
        shared = [
            "//hw/top:otp_ctrl_c_regs",
            "//sw/device/lib/base:hardened",
            "//sw/device/lib/base:macros",
            "//sw/device/silicon_creator/lib/base:sec_mmio",
            "//sw/device/lib/base:abs_mmio",
        ],
    ),
)

cc_test(
    name = "otp_unittest",
    srcs = ["otp_unittest.cc"],
    deps = [
        dual_cc_device_library_of(":otp"),
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "pinmux",
    srcs = ["pinmux.c"],
    hdrs = ["pinmux.h"],
    deps = [
        "//hw/top:gpio_c_regs",
        "//hw/top:otp_ctrl_c_regs",
        "//hw/top:pinmux_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:csr",
        "//sw/device/lib/base:macros",
        "//sw/device/lib/runtime:hart",
        "//sw/device/silicon_creator/lib/base:chip",
        "//sw/device/silicon_creator/lib/drivers:otp",
    ],
)

cc_test(
    name = "pinmux_unittest",
    srcs = ["pinmux_unittest.cc"],
    deps = [
        ":pinmux",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/arch:sim_dv",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:csr",
        "//sw/device/lib/base:macros",
        "//sw/device/silicon_creator/lib/base:chip",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "retention_sram",
    srcs = ["retention_sram.c"],
    hdrs = ["retention_sram.h"],
    deps = [
        "//hw/top:dt",
        "//hw/top:sram_ctrl_c_regs",
        "//hw/top:top_lib",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:macros",
        "//sw/device/lib/base:memory",
        "//sw/device/silicon_creator/lib:boot_log",
        "//sw/device/silicon_creator/lib:error",
        "//sw/device/silicon_creator/lib/boot_svc:boot_svc_msg",
    ],
)

cc_test(
    name = "retention_sram_unittest",
    srcs = ["retention_sram_unittest.cc"],
    deps = [
        ":retention_sram",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

opentitan_test(
    name = "retention_sram_functest",
    srcs = ["retention_sram_functest.c"],
    exec_env = EARLGREY_TEST_ENVS,
    verilator = verilator_params(
        timeout = "long",
    ),
    deps = [
        ":retention_sram",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:macros",
        "//sw/device/lib/base:memory",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:error",
    ],
)

dual_cc_library(
    name = "rnd",
    srcs = dual_inputs(
        device = ["rnd.c"],
        host = ["mock_rnd.cc"],
    ),
    hdrs = dual_inputs(
        host = ["mock_rnd.h"],
        shared = ["rnd.h"],
    ),
    deps = dual_inputs(
        device = [
            ":otp",
            "//sw/device/lib/arch:boot_stage",
            "//sw/device/lib/base:csr",
            "//sw/device/lib/base:abs_mmio",
            "//sw/device/lib/base:hardened",
            "//hw/top:entropy_src_c_regs",
            "//hw/top:otp_ctrl_c_regs",
            "//hw/top:rv_core_ibex_c_regs",
            "//hw/top_earlgrey/sw/autogen:top_earlgrey",
            "//sw/device/lib/base:crc32",
        ],
        host = [
            "//sw/device/lib/base:global_mock",
            "//sw/device/silicon_creator/testing:rom_test",
            "@googletest//:gtest",
        ],
        shared = [
            ":lifecycle",
            "//sw/device/lib/base:macros",
            "//sw/device/silicon_creator/lib:error",
        ],
    ),
)

cc_test(
    name = "rnd_unittest",
    srcs = ["rnd_unittest.cc"],
    deps = [
        dual_cc_device_library_of(":rnd"),
        ":otp",
        "//hw/top:otp_ctrl_c_regs",
        "//hw/top:rv_core_ibex_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/arch:boot_stage_rom_ext",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:csr",
        "//sw/device/silicon_creator/lib/base:sec_mmio",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

opentitan_test(
    name = "rnd_functest",
    srcs = ["rnd_functest.c"],
    broken = fpga_params(tags = ["broken"]),
    exec_env = dicts.add(
        EARLGREY_TEST_ENVS,
        {
            # FIXME broken in sival ROM_EXT, remove this line when fixed. See #21706.
            "//hw/top_earlgrey:fpga_cw310_sival_rom_ext": "broken",
        },
    ),
    deps = [
        ":otp",
        ":rnd",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:crc32",
        "//sw/device/lib/dif:entropy_src",
        "//sw/device/lib/testing:entropy_testutils",
        "//sw/device/lib/testing:rand_testutils",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:error",
    ],
)

dual_cc_library(
    name = "rstmgr",
    srcs = dual_inputs(
        device = ["rstmgr.c"],
        host = ["mock_rstmgr.cc"],
    ),
    hdrs = dual_inputs(
        host = ["mock_rstmgr.h"],
        shared = ["rstmgr.h"],
    ),
    deps = dual_inputs(
        device = [
            "//hw/top:rstmgr_c_regs",
            "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        ],
        host = [
            "//sw/device/lib/base:global_mock",
            "//sw/device/silicon_creator/testing:rom_test",
            "@googletest//:gtest",
        ],
        shared = [
            "//hw/top:otp_ctrl_c_regs",
            "//sw/device/lib/base:bitfield",
            "//sw/device/lib/base:macros",
            "//sw/device/lib/base:multibits",
            "//sw/device/lib/runtime:hart",
            "//sw/device/silicon_creator/lib:error",
            "//sw/device/silicon_creator/lib/base:sec_mmio",
            "//sw/device/silicon_creator/lib/drivers:otp",
        ],
    ),
)

cc_test(
    name = "rstmgr_unittest",
    srcs = ["rstmgr_unittest.cc"],
    deps = [
        dual_cc_device_library_of(":rstmgr"),
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

opentitan_test(
    name = "rstmgr_functest",
    srcs = ["rstmgr_functest.c"],
    exec_env = EARLGREY_TEST_ENVS,
    qemu = qemu_params(
        globals = {
            # Test uses rstmgr, keep running on fatal resets:
            "ot-rstmgr.fatal_reset": 0,
        },
    ),
    verilator = verilator_params(
        timeout = "long",
    ),
    deps = [
        ":rstmgr",
        "//sw/device/lib/testing:rstmgr_testutils",
        "//sw/device/lib/testing/test_framework:ottf_main",
    ],
)

cc_library(
    name = "uart",
    srcs = ["uart.c"],
    hdrs = ["uart.h"],
    deps = [
        ":ibex",
        "//hw/top:dt",
        "//hw/top:uart_c_regs",
        "//sw/device/lib/arch:device",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:bitfield",
        "//sw/device/lib/base:hardened",
        "//sw/device/silicon_creator/lib:error",
    ],
)

cc_test(
    name = "uart_unittest",
    srcs = ["uart_unittest.cc"],
    deps = [
        ":uart",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:hardened",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

opentitan_test(
    name = "uart_functest",
    srcs = ["uart_functest.c"],
    exec_env = EARLGREY_TEST_ENVS,
    deps = [
        ":uart",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:error",
    ],
)

cc_library(
    name = "watchdog",
    srcs = ["watchdog.c"],
    hdrs = ["watchdog.h"],
    deps = [
        ":pwrmgr",
        "//hw/top:aon_timer_c_regs",
        "//hw/top:otp_ctrl_c_regs",
        "//hw/top:pwrmgr_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/silicon_creator/lib/base:sec_mmio",
        "//sw/device/silicon_creator/lib/drivers:lifecycle",
        "//sw/device/silicon_creator/lib/drivers:otp",
    ],
)

cc_test(
    name = "watchdog_unittest",
    srcs = ["watchdog_unittest.cc"],
    deps = [
        ":watchdog",
        "//hw/top:aon_timer_c_regs",
        "//hw/top:otp_ctrl_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/silicon_creator/lib/base:sec_mmio",
        "//sw/device/silicon_creator/lib/drivers:otp",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

opentitan_test(
    name = "watchdog_functest",
    srcs = ["watchdog_functest.c"],
    exec_env = EARLGREY_TEST_ENVS,
    verilator = verilator_params(
        timeout = "long",
    ),
    deps = [
        ":retention_sram",
        ":rstmgr",
        ":watchdog",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/testing:rstmgr_testutils",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:error",
        "//sw/device/silicon_creator/lib/base:sec_mmio",
    ],
)

dual_cc_library(
    name = "spi_device",
    srcs = dual_inputs(
        device = ["spi_device.c"],
        host = ["mock_spi_device.cc"],
    ),
    hdrs = dual_inputs(
        host = ["mock_spi_device.h"],
        shared = ["spi_device.h"],
    ),
    deps = dual_inputs(
        device = [
            "//hw/top:flash_ctrl_c_regs",
            "//hw/top:spi_device_c_regs",
            "//hw/top:dt",
        ],
        host = [
            "//sw/device/lib/base:global_mock",
            "//sw/device/silicon_creator/testing:rom_test",
            "@googletest//:gtest",
        ],
        shared = [
            ":lifecycle",
            "//sw/device/lib/base:abs_mmio",
            "//sw/device/lib/base:memory",
            "//sw/device/silicon_creator/lib:error",
        ],
    ),
)

cc_test(
    name = "spi_device_unittest",
    srcs = [
        "spi_device_unittest.cc",
    ],
    deps = [
        dual_cc_device_library_of(":spi_device"),
        "//hw/top:spi_device_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/silicon_creator/lib:error",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "pwrmgr",
    srcs = ["pwrmgr.c"],
    hdrs = ["pwrmgr.h"],
    deps = [
        ":ibex",
        "//hw/top:pwrmgr_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/silicon_creator/lib/base:sec_mmio",
    ],
)

cc_library(
    name = "sensor_ctrl",
    srcs = ["sensor_ctrl.c"],
    hdrs = ["sensor_ctrl.h"],
    deps = [
        ":lifecycle",
        ":otp",
        "//hw/top:otp_ctrl_c_regs",
        "//hw/top:sensor_ctrl_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:abs_mmio",
        "//sw/device/lib/base:bitfield",
        "//sw/device/lib/base:hardened",
        "//sw/device/silicon_creator/lib/base:sec_mmio",
    ],
)

cc_test(
    name = "sensor_ctrl_unittest",
    srcs = ["sensor_ctrl_unittest.cc"],
    deps = [
        ":sensor_ctrl",
        "//hw/top:otp_ctrl_c_regs",
        "//hw/top:sensor_ctrl_c_regs",
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "epmp",
    srcs = ["epmp.c"],
    hdrs = ["epmp.h"],
    deps = [
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/base:bitfield",
        "//sw/device/lib/base:csr",
        "//sw/device/lib/base:hardened",
        "//sw/device/silicon_creator/lib:epmp_state",
    ],
)

cc_test(
    name = "epmp_unittest",
    srcs = ["epmp_unittest.cc"],
    deps = [
        ":epmp",
        "//sw/device/silicon_creator/testing:rom_test",
        "@googletest//:gtest_main",
    ],
)
