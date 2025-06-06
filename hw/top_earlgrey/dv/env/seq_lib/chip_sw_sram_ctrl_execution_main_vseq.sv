// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

class chip_sw_sram_ctrl_execution_main_vseq extends chip_sw_base_vseq;
  `uvm_object_utils(chip_sw_sram_ctrl_execution_main_vseq)

  `uvm_object_new

  localparam logic [255:0] DEVICE_ID =
      256'hFA53B8058E157CB69F1F413E87242971B6B52A656A1CAB7FEBF21E5BF1F45EDD;
  localparam logic [255:0] MANUF_STATE =
      256'h41389646B3968A3B128F4AF0AFFC1AAC77ADEFF42376E09D523D5C06786AAC34;
  localparam logic [7:0] MUBI8TRUE = prim_mubi_pkg::MuBi8True;
  localparam logic [7:0] MUBI8FALSE = prim_mubi_pkg::MuBi8False;
  localparam logic [7:0] EN_CSRNG_SW_APP_READ = MUBI8FALSE;
  localparam logic [7:0] DIS_RV_DM_LATE_DEBUG = MUBI8FALSE;

  virtual task do_test(logic [7:0] en_sram_ifetch, bit set_prod_lc);

    apply_reset();

    if (set_prod_lc) begin
      otp_write_lc_partition_state(cfg.mem_bkdr_util_h[Otp], LcStProd);
    end
    otp_write_hw_cfg0_partition(
        .mem_bkdr_util_h(cfg.mem_bkdr_util_h[Otp]),
        .device_id(DEVICE_ID), .manuf_state(MANUF_STATE));
    otp_write_hw_cfg1_partition(
        .mem_bkdr_util_h(cfg.mem_bkdr_util_h[Otp]),
        .en_sram_ifetch(en_sram_ifetch),
        .en_csrng_sw_app_read(EN_CSRNG_SW_APP_READ),
        .dis_rv_dm_late_debug(DIS_RV_DM_LATE_DEBUG));

    `DV_WAIT(cfg.sw_test_status_vif.sw_test_status == SwTestStatusInTest)
    `DV_WAIT(cfg.sw_test_status_vif.sw_test_status == SwTestStatusInWfi)
  endtask

  virtual task body();

    super.body();

    // State 0 - SRAM_IFETCH = False, LC_STATE = LcStRma (default).
    do_test(MUBI8FALSE, 0);
    // State 1 - SRAM_IFETCH = True, LC_STATE = LcStRma (default).
    do_test(MUBI8TRUE, 0);
    // State 2 - SRAM_IFETCH = False, LC_STATE = LcStProd.
    do_test(MUBI8FALSE, 1);
    // State 3 - SRAM_IFETCH = True, LC_STATE = LcStProd.
    do_test(MUBI8TRUE, 1);

    override_test_status_and_finish(.passed(1'b 1));

  endtask

endclass
