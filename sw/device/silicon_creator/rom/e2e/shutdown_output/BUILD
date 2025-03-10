# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load(
    "//rules/opentitan:defs.bzl",
    "cw310_params",
    "dv_params",
    "ecdsa_key_for_lc_state",
    "fpga_params",
    "opentitan_test",
)
load("//rules/opentitan:keyutils.bzl", "ECDSA_ONLY_KEY_STRUCTS")
load(
    "//rules:const.bzl",
    "CONST",
    "get_lc_items",
    "hex",
    "hex_digits",
)
load(
    "//rules:manifest.bzl",
    "manifest",
)
load(
    "//rules:otp.bzl",
    "STD_OTP_OVERLAYS",
    "otp_image",
)
load(
    "//rules:rom_e2e.bzl",
    "maybe_skip_in_ci",
)
load(
    "//sw/device/silicon_creator/rom/e2e:defs.bzl",
    "MSG_PASS",
    "MSG_TEMPLATE_BFV_LCV",
)

package(default_visibility = ["//visibility:public"])

[
    otp_image(
        name = "otp_img_shutdown_output_{}".format(lc_state),
        src = "//hw/top_earlgrey/data/otp:otp_json_{}".format(lc_state),
        overlays = STD_OTP_OVERLAYS,
    )
    for lc_state, _ in get_lc_items()
]

manifest(
    d = {
        "name": "manifest_bad_identifier",
        "address_translation": hex(CONST.HARDENED_FALSE),
        "identifier": "0",
    },
)

[
    opentitan_test(
        name = "shutdown_output_{}".format(lc_state),
        dv = dv_params(
            binaries = {"//sw/device/silicon_creator/rom/e2e:new_empty_test_slot_a": "firmware"},
            otp = ":otp_img_shutdown_output_{}".format(lc_state),
            rom = "//sw/device/silicon_creator/rom:mask_rom",
        ),
        ecdsa_key = ecdsa_key_for_lc_state(
            ECDSA_ONLY_KEY_STRUCTS,
            lc_state_val,
        ),
        exec_env = {
            "//hw/top_earlgrey:fpga_cw310_rom_with_fake_keys": None,
            "//hw/top_earlgrey:sim_dv": None,
        },
        fpga = fpga_params(
            binaries = {"//sw/device/silicon_creator/rom/e2e:new_empty_test_slot_a": "firmware"},
            exit_failure = MSG_PASS,
            exit_success = MSG_TEMPLATE_BFV_LCV.format(
                hex_digits(CONST.BFV.BOOT_POLICY.BAD_IDENTIFIER),
                hex_digits(lc_state_val),
            ),
            otp = ":otp_img_shutdown_output_{}".format(lc_state),
            tags = maybe_skip_in_ci(lc_state_val),
        ),
        manifest = ":manifest_bad_identifier",
    )
    for lc_state, lc_state_val in get_lc_items()
]

test_suite(
    name = "shutdown_output",
    tags = ["manual"],
    tests = ["shutdown_output_{}".format(lc_state) for lc_state, _ in get_lc_items()],
)
