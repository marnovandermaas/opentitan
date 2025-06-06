CAPI=2:
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
name: ${instance_vlnv("lowrisc:ip:pwrmgr_pkg:0.1")}
description: "Power manager package"

filesets:
  files_rtl:
    depend:
      - lowrisc:tlul:headers
  % if wait_for_external_reset:
      - lowrisc:ip:lc_ctrl_pkg
      - lowrisc:ip:rom_ctrl_pkg
  % endif
    files:
      - rtl/pwrmgr_reg_pkg.sv
      - rtl/pwrmgr_pkg.sv
    file_type: systemVerilogSource

  files_verilator_waiver:
    depend:
      # common waivers
      - lowrisc:lint:common
      - lowrisc:lint:comportable
    files:
      - lint/pwrmgr_pkg.vlt
    file_type: vlt

targets:
  default:
    filesets:
      - tool_verilator   ? (files_verilator_waiver)
      - files_rtl
