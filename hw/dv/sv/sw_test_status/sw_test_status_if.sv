// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

interface sw_test_status_if #(
  parameter int AddrWidth = 32
) (
  input logic clk_i,
  input logic rst_ni,               // Under reset.
  input logic fetch_en,             // Fetch enabled.
  input logic wr_valid,             // Qualified write access.
  input logic [AddrWidth-1:0] addr, // Incoming addr.
  input logic [15:0] data           // Incoming data.
);

  import sw_test_status_pkg::*;
  `include "dv_macros.svh"

  // Address to which the test status is written to. This is set by the testbench.
  logic [AddrWidth-1:0] sw_test_status_addr;

  // Enforce that the SW test pass indication is made only from the SwTestStatusInTest state.
  bit can_pass_only_in_test = 1'b1;

  // Validate the incoming write address.
  logic data_valid;
  assign data_valid = wr_valid && (addr == sw_test_status_addr);

  // SW test status indication.
  sw_test_status_e sw_test_status;
  sw_test_status_e sw_test_status_prev;

  // If the sw_test_status reaches the terminal states, assert that we are done.
  bit in_terminal_state;
  bit sw_test_done;
  bit sw_test_passed;

  // The test seq may reboot the CPU multiple times, so it may cycle through the SW test states
  // multiple times.
  int num_iterations = 1;

  function automatic void set_num_iterations(int value);
    num_iterations = value;
  endfunction

  always @(posedge clk_i) begin
    if (!rst_ni) begin
      sw_test_status_prev = sw_test_status;
      sw_test_status = SwTestStatusUnderReset;
    end
    else if (sw_test_status == SwTestStatusUnderReset && fetch_en) begin
      sw_test_status_prev = sw_test_status;
      sw_test_status = SwTestStatusBooted;
    end
    else if (data_valid) begin
      sw_test_status_prev = sw_test_status;
      sw_test_status = sw_test_status_e'(data);

      // Do further processing only on status transitions.
      if (sw_test_status_prev != sw_test_status) begin
        `dv_info($sformatf("SW test transitioned to %s.", sw_test_status.name()))

        // SW MUST not transition to any other state, if it is already done.
        if (sw_test_done) begin
          `dv_error("SW test status must not change after reaching the pass or fail state.")
          `dv_error("==== SW TEST FAILED ====")
        end

        in_terminal_state = sw_test_status inside {SwTestStatusPassed, SwTestStatusFailed};
        if (in_terminal_state) num_iterations--;
        sw_test_done |= in_terminal_state && (num_iterations == 0);

        // Exit only when all iterations of the SW test are finished.
        if (sw_test_done) begin
          if (sw_test_status == SwTestStatusPassed) begin
            if (can_pass_only_in_test && sw_test_status_prev != SwTestStatusInTest) begin
              `dv_error($sformatf("SW test transitioned to %s from an illegal state: %s.",
                                  sw_test_status.name(), sw_test_status_prev.name()))
              `dv_error("==== SW TEST FAILED ====")
            end else begin
              sw_test_passed = 1'b1;
              `dv_info("==== SW TEST PASSED ====")
            end
          end else begin
            `dv_error("==== SW TEST FAILED ====")
          end
        end
      end
    end
  end

endinterface
