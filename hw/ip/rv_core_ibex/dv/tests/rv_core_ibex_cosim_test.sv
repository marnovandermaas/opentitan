// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

class rv_core_ibex_cosim_test #();
  `uvm_component_new

  task body();
    $system($sformatf("sh test.sh"))
  endtask : body

endclass : rv_core_ibex_cosim_test
