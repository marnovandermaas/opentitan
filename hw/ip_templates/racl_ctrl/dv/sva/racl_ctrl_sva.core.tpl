CAPI=2:
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
#
# Register assertions and bind file. This needs to be templated because it
# needs the register layout (from racl_ctrl.hjson) to know what registers exist.

name: ${instance_vlnv(f"lowrisc:dv:{module_instance_name}_sva:0.1")}
description: "RACL_CTRL assertion modules and bind file."
filesets:
  files_dv:
    depend:
      - lowrisc:tlul:headers
      - lowrisc:fpv:csr_assert_gen
    files:
      - racl_ctrl_bind.sv
    file_type: systemVerilogSource

  files_formal:
    depend:
      - ${instance_vlnv(f"lowrisc:ip:{module_instance_name}")}

generate:
  csr_assert_gen:
    generator: csr_assert_gen
    parameters:
      spec: ../../data/${module_instance_name}.hjson

targets:
  default: &default_target
    filesets:
      - files_dv
    generate:
      - csr_assert_gen
  formal:
    <<: *default_target
    filesets:
      - files_formal
      - files_dv
    toplevel: ${module_instance_name}
