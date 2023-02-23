// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// This sequence writes a random values to combo detect registers including
// precondition settings and checks for the combo detect interrupt.
class sysrst_ctrl_combo_detect_with_pre_cond_vseq extends sysrst_ctrl_base_vseq;
  `uvm_object_utils(sysrst_ctrl_combo_detect_with_pre_cond_vseq)

  `uvm_object_new
  rand uvm_reg_data_t set_action[4];
  rand bit [4:0] trigger_combo[4], trigger_combo_precondition[4];
  rand uint16_t set_duration[4], set_duration_precondition[4];
  rand uint16_t cycles, cycles_precondition;
  rand uint16_t set_pulse_width, set_key_timer, num_trans_combo_detect;

  constraint num_trans_c {num_trans == 50;}

  constraint num_trans_combo_detect_c {num_trans_combo_detect == 10;}

  constraint trigger_combo_precondition_c {
    foreach (trigger_combo_precondition[i]) {
      trigger_combo_precondition[i] != 0;
    }
  }

  constraint trigger_combo_c {
     foreach (trigger_combo[i]) {
       trigger_combo[i] != 0;
       (trigger_combo[i] & trigger_combo_precondition[i]) == 0;
     }
   }

  constraint set_duration_precondition_c {
    foreach (set_duration_precondition[i]) {
      set_duration_precondition[i] dist {
        [10 : 50]  :/ 95,
        [51 : 300] :/ 5
      };
    }
  }

  constraint set_duration_c {
    foreach (set_duration[i]) {
      set_duration[i] dist {
        [10 : 50 ] :/ 95,
        [51 : 300] :/ 5
      };
    }
  }

  constraint set_pulse_width_c {
    set_pulse_width dist {
      [10 : 50]  :/ 95,
      [51 : 300] :/ 5
    };
    // ec_rst will trigger a pulse, we check all the action after 4 set_duration are done
    // make the set_pulse_width larger than any of the duration, so that we can still check it's
    // value.
    foreach (set_duration[i]) {
      if (cycles > set_duration[i] + set_key_timer) {
        set_pulse_width > cycles - (set_duration[i] + set_key_timer);
      }
    }
  }

  constraint set_key_timer_c {
    set_key_timer dist {
        [10 : 50 ]  :/ 95,
        [51 : 300]  :/ 5
    };
  }

  constraint cycles_c {
    foreach (set_duration[i]) {
      cycles dist {
        [1 : (set_duration[i] + set_key_timer) - 2] :/ 5,
        [(set_duration[i] + set_key_timer) + 5 : (set_duration[i] + set_key_timer) * 2] :/ 95
      };
      // Don't fall into this uncerntain period.
      !(cycles inside {[(set_duration[i] + set_key_timer) - 1 :
                        (set_duration[i] + set_key_timer) + 5]});
    }
  }

  constraint cycles_precondition_c {
    foreach (set_duration_precondition[i]) {
      solve set_duration_precondition[i] before cycles_precondition;
      cycles_precondition dist {
        [1 : (set_duration_precondition[i] + set_key_timer) - 2] :/ 5,
        [(set_duration_precondition[i]) + 5 :
                     (set_duration_precondition[i] + set_key_timer) * 2]   :/ 95
      };
      // Don't fall into this uncerntain period.
      !(cycles_precondition inside {[(set_duration_precondition[i] + set_key_timer) - 1 :
                        (set_duration_precondition[i] + set_key_timer) + 5]});
    }
  }

  // Check for a input singal transition that triggers the combo detection logic
  function bit get_combo_trigger(int index, bit [4:0] combo_input_prev);
    logic [4:0] in;
    int count_ones_prev, count_ones;
    in[0]           = cfg.vif.key0_in;
    in[1]           = cfg.vif.key1_in;
    in[2]           = cfg.vif.key2_in;
    in[3]           = cfg.vif.pwrb_in;
    in[4]           = cfg.vif.ac_present;
    count_ones      = $countones(in & trigger_combo[index]);
    count_ones_prev = $countones(combo_input_prev & trigger_combo[index]);
    `uvm_info(`gfn, $sformatf(
              {
                "DETECTION: i:%0d, in: %5b prev_in: %5b sel:%5b,",
                "count_ones = %0d count_ones_prev = %0d "
              },
              index,
              in,
              combo_input_prev,
              trigger_combo[index],
              count_ones,
              count_ones_prev
              ), UVM_MEDIUM)
    // Check for Or'd signal transition
    return ((count_ones_prev > 0) && (count_ones == 0)) && (trigger_combo[index] != 0) ?
             1'b1 : 1'b0;
  endfunction

  // Check for a input combination that triggers the combo precondition logic
  function bit get_combo_precondition_trigger(int index);
    logic [4:0] in;
    in[0] = cfg.vif.key0_in;
    in[1] = cfg.vif.key1_in;
    in[2] = cfg.vif.key2_in;
    in[3] = cfg.vif.pwrb_in;
    in[4] = cfg.vif.ac_present;
    `uvm_info(`gfn, $sformatf(
              "PRECONDITION: i:%0d, in: %5b sel:%5b", index, in, trigger_combo_precondition[index]),
              UVM_MEDIUM)
    return ((in & trigger_combo_precondition[index]) == 0);
  endfunction

  task check_ec_rst_inactive(int max_cycle);
    cfg.clk_aon_rst_vif.wait_clks($urandom_range(0, max_cycle));
    `DV_CHECK_EQ(cfg.vif.ec_rst_l_out, 1)
  endtask

  task config_register();
    // Select the inputs for precondition
    foreach (ral.com_pre_sel_ctl[i]) csr_wr(ral.com_pre_sel_ctl[i], trigger_combo_precondition[i]);

    // Set the duration for precondition keys to pressed
    foreach (ral.com_pre_det_ctl[i]) csr_wr(ral.com_pre_det_ctl[i], set_duration_precondition[i]);

    // Select the inputs for the combo
    foreach (ral.com_sel_ctl[i]) csr_wr(ral.com_sel_ctl[i], trigger_combo[i]);

    // Set the duration for combo to pressed
    foreach (ral.com_det_ctl[i]) csr_wr(ral.com_det_ctl[i], set_duration[i]);

    // Set the trigger action
    foreach (ral.com_out_ctl[i]) csr_wr(ral.com_out_ctl[i], set_action[i]);

    // Set the ec_rst_0 pulse width
    csr_wr(ral.ec_rst_ctl, set_pulse_width);
    `uvm_info(`gfn, $sformatf("Write data of ec_rst_ctl register:0x%0h", set_pulse_width), UVM_LOW);
    // Set the key triggered debounce timer
    csr_wr(ral.key_intr_debounce_ctl, set_key_timer);
    `uvm_info(`gfn, $sformatf("Write data of key_intr_debounce_ctl register:0x%0h", set_key_timer),
              UVM_LOW);
  endtask

  task monitor_bat_disable_L2H(int exp_cycles);
    int inactive_cycles = 0;
    int aon_period_ns = cfg.clk_aon_rst_vif.clk_period_ps / 1000;
    // Check bat_disable is low for exp_cycles. After exp_cycles+20, below will time out and fail.
    `DV_SPINWAIT(while(cfg.vif.bat_disable != 1) begin
                   cfg.clk_aon_rst_vif.wait_clks(1);
                   inactive_cycles++;
                 end, "time out waiting for bat_disable == 1",
                 aon_period_ns * (exp_cycles + 20))
    `DV_CHECK(inactive_cycles inside {[exp_cycles - 4 : exp_cycles + 4]},
           $sformatf("bat_disable_check: inact(%0d) vs exp(%0d) +/-4", inactive_cycles, exp_cycles))
  endtask

  task monitor_rst_req_L2H(int exp_cycles);
    int inactive_cycles = 0;
    int aon_period_ns = cfg.clk_aon_rst_vif.clk_period_ps / 1000;
    // Check bat_disable is low for exp_cycles. After exp_cycles+20, below will time out and fail.
    `DV_SPINWAIT(while(cfg.vif.rst_req != 1) begin
                   cfg.clk_aon_rst_vif.wait_clks(1);
                   inactive_cycles++;
                 end, "time out waiting for rst_req == 1",
                 aon_period_ns * (exp_cycles + 20))
    `DV_CHECK(inactive_cycles inside {[exp_cycles - 4 : exp_cycles + 4]},
            $sformatf("rst_req_check: inact(%0d) vs exp(%0d) +/-4", inactive_cycles, exp_cycles))
  endtask

  task body();

    `uvm_info(`gfn, "Starting the body from combo detect with precondition", UVM_LOW)

    // Start sequence by releaseing ec_rst_l_o. post reset ec_rst_l_o remains asserted,
    // and must be deasserted. This is to make sure during test, the H->L and L->H transitions
    // of ec_rst_l_o can be observed
    release_ec_rst_l_o();
    // Reset combo logic input
    reset_combo_inputs();
    // Configure combo logic registers
    config_register();

    `uvm_info(`gfn, $sformatf("Value of cycles_precondition:%0d", cycles_precondition), UVM_LOW)
    `uvm_info(`gfn, $sformatf("Value of cycles:%0d", cycles), UVM_LOW)
    // It takes 2-3 clock cycles to sync the register values
    cfg.clk_aon_rst_vif.wait_clks(3);
    repeat (num_trans) begin : main_block
      bit precond_detected_for_one_block = 1'b0;
      bit [3:0] precondition_detected = '{4{1'b0}};
      // Randomize input
      repeat ($urandom_range(1, 3)) cfg.vif.randomize_combo_input();
      // Wait for debounce + detect timer
      cfg.clk_aon_rst_vif.wait_clks(cycles_precondition);
      for (int i = 0; i < 4; i++) begin
        if(cycles_precondition > (set_duration_precondition[i] + set_key_timer) &&
              get_combo_precondition_trigger(
                i
            )) begin
          precondition_detected[i] = (trigger_combo[i] & trigger_combo_precondition[i]) == 0;
          precond_detected_for_one_block |= precondition_detected[i];
        end
      end

      if (precond_detected_for_one_block) begin : combo_detect_block
        uvm_reg_data_t  rdata;
        uint16_t       [3:0] get_duration;
        uvm_reg_data_t [3:0] get_action;
        bit [3:0]  combo_detected;
        bit [4:0]  combo_precondition_mask = 5'd0;
        bit [4:0]  get_trigger_combo[4];
        bit [4:0]  get_trigger_combo_pre[4];
        bit [4:0]  combo_input_prev;
        bit        combo_triggered[4] = '{1'b0, 1'b0, 1'b0, 1'b0};
        // Update combo_precondition_mask
        for (int i = 0; i < 4; i++) begin
          if (precondition_detected[i]) begin
            // Dont change the signals asserted for precondition
            combo_precondition_mask |= trigger_combo_precondition[i];
          end
        end
        combo_precondition_mask = ~combo_precondition_mask;
        `uvm_info(`gfn, $sformatf("precondition_detected= %0x", precondition_detected), UVM_MEDIUM)
        `uvm_info(`gfn, $sformatf("combo_precondition_mask= %0x", combo_precondition_mask),
                  UVM_MEDIUM)
        // Disable blocks in Idle state
        for (int i = 0; i < 4; i++) begin
          if (!precondition_detected[i]) begin
            csr_wr(ral.com_sel_ctl[i], 5'd0);
            csr_wr(ral.com_pre_sel_ctl[i], 5'd0);
          end else begin
            `uvm_info(`gfn, $sformatf("valid precondition detected for combo channel: %0d", i),
                      UVM_LOW)
          end
        end
        cfg.clk_aon_rst_vif.wait_clks(3);

        while (precondition_detected > 0 && (combo_detected != precondition_detected) &&
               num_trans_combo_detect>0)  begin : combo_action_check
          bit bat_act_triggered, ec_act_triggered, rst_act_triggered;
          bit [3:0] intr_actions, intr_actions_pre_reset;
          int ec_act_occur_cyc = 0;
          int bat_act_occur_cyc = 0;
          int rst_req_act_occur_cyc = 0;
          int ec_act_occur_cycles[$];
          int bat_act_occur_cycles[$];
          int rst_req_act_occur_cycles[$];
          int ec_rst_check_wait_cycles=0;
          int max_wait_till_next_iter = set_pulse_width;
          bit key0_in_sel_detect = 0;
          bit key1_in_sel_detect = 0;
          bit key2_in_sel_detect = 0;
          bit pwrb_in_sel_detect = 0;
          bit ac_present_sel_detect = 0;
          bit key0_in_sel_precond = 0;
          bit key1_in_sel_precond = 0;
          bit key2_in_sel_precond = 0;
          bit pwrb_in_sel_precond = 0;
          bit ac_present_sel_precond = 0;

          // Sample combo key inputs
          combo_input_prev = get_combo_input();

          // Sample the combo_intr_status covergroup to capture the trigger combo inputs
          // before randomizing the inputs
          if (cfg.en_cov) begin
            foreach (intr_actions_pre_reset[i]) begin
              intr_actions_pre_reset[i] =
                  get_field_val(ral.com_out_ctl[i].interrupt, get_action[i]);
            end
            cov.combo_intr_status.sysrst_ctrl_combo_intr_status_cg.sample(
              get_field_val(ral.combo_intr_status.combo0_h2l, rdata),
              get_field_val(ral.combo_intr_status.combo1_h2l, rdata),
              get_field_val(ral.combo_intr_status.combo2_h2l, rdata),
              get_field_val(ral.combo_intr_status.combo3_h2l, rdata),
              cfg.vif.key0_in,
              cfg.vif.key1_in,
              cfg.vif.key2_in,
              cfg.vif.pwrb_in,
              cfg.vif.ac_present,
              intr_actions_pre_reset);
          end

          // Read combo detection registers
          foreach (ral.com_det_ctl[i]) csr_rd(ral.com_det_ctl[i], get_duration[i]);
          foreach (ral.com_out_ctl[i]) csr_rd(ral.com_out_ctl[i], get_action[i]);
          foreach (ral.com_sel_ctl[i]) csr_rd(ral.com_sel_ctl[i], get_trigger_combo[i]);
          foreach (ral.com_sel_ctl[i]) csr_rd(ral.com_pre_sel_ctl[i], get_trigger_combo_pre[i]);

          // Randomize combo logic inputs except the ones asserted for precondition
          repeat ($urandom_range(1, 3)) cfg.vif.randomize_combo_input(combo_precondition_mask);

          // Wait for debounce + detect timer
          cfg.clk_aon_rst_vif.wait_clks(cycles);

          // Update trigger value of Combo channel
          foreach (combo_triggered[i]) begin
            if (precondition_detected[i])
              combo_triggered[i] = get_combo_trigger(i, combo_input_prev);
            if (combo_triggered[i])
              `uvm_info(`gfn, $sformatf("valid combo input transition detected for channel :%0d",i),
                                         UVM_LOW)
          end

          // Check if the interrupt has raised.
          // NOTE: The interrupt will only raise if the interrupt combo action is set.
          for (int i = 0; i < 4; i++) begin
            if ((cycles + set_pulse_width) > (get_duration[i] + set_key_timer) &&
               combo_triggered[i])
            begin
              int key_detect_time = get_duration[i] + set_key_timer;
              bit com_out_intr = get_field_val(ral.com_out_ctl[i].interrupt, get_action[i]);
              bit com_out_bat_disable = get_field_val(ral.com_out_ctl[i].bat_disable,
                                          get_action[i]);
              bit com_out_ec_rst = get_field_val(ral.com_out_ctl[i].ec_rst, get_action[i]);
              bit com_out_rst_req = get_field_val(ral.com_out_ctl[i].rst_req, get_action[i]);
              // Sample aggregate key selection
              key0_in_sel_detect     = get_field_val(ral.com_sel_ctl[i].key0_in_sel,
                                                     get_trigger_combo[i]);
              key1_in_sel_detect     = get_field_val(ral.com_sel_ctl[i].key1_in_sel,
                                                     get_trigger_combo[i]);
              key2_in_sel_detect     = get_field_val(ral.com_sel_ctl[i].key2_in_sel,
                                                     get_trigger_combo[i]);
              pwrb_in_sel_detect     = get_field_val(ral.com_sel_ctl[i].pwrb_in_sel,
                                                     get_trigger_combo[i]);
              ac_present_sel_detect  = get_field_val(ral.com_sel_ctl[i].ac_present_sel,
                                                     get_trigger_combo[i]);
              key0_in_sel_precond    = get_field_val(ral.com_pre_sel_ctl[i].key0_in_sel,
                                                     get_trigger_combo_pre[i]);
              key1_in_sel_precond    = get_field_val(ral.com_pre_sel_ctl[i].key1_in_sel,
                                                     get_trigger_combo_pre[i]);
              key2_in_sel_precond    = get_field_val(ral.com_pre_sel_ctl[i].key2_in_sel,
                                                     get_trigger_combo_pre[i]);
              pwrb_in_sel_precond    = get_field_val(ral.com_pre_sel_ctl[i].pwrb_in_sel,
                                                     get_trigger_combo_pre[i]);
              ac_present_sel_precond = get_field_val(ral.com_pre_sel_ctl[i].ac_present_sel,
                                                     get_trigger_combo_pre[i]);

              intr_actions[i]    = com_out_intr;
              bat_act_triggered |= com_out_bat_disable;
              ec_act_triggered  |= com_out_ec_rst;
              rst_act_triggered |= com_out_rst_req;
              // Sample the combo_intr_status covergroup to capture the trigger combo settings
              if (cfg.en_cov) begin
                for (int i = 0; i < 4; i++) begin
                  cov.combo_detect_action[i].sysrst_ctrl_combo_detect_action_cg.sample(
                    com_out_bat_disable, com_out_intr, com_out_ec_rst, com_out_rst_req,
                    key0_in_sel_detect,
                    key1_in_sel_detect,
                    key2_in_sel_detect,
                    pwrb_in_sel_detect,
                    ac_present_sel_detect,
                    key0_in_sel_precond,
                    key1_in_sel_precond,
                    key2_in_sel_precond,
                    pwrb_in_sel_precond,
                    ac_present_sel_precond);
                  // Sample key combination coverpoints
                  cov.combo_key_combinations.sysrst_ctrl_combo_key_combinations_cg.sample(
                    .bat_disable    ( com_out_bat_disable ),
                    .interrupt      ( com_out_intr ),
                    .ec_rst         ( com_out_rst_req ),
                    .rst_req        ( com_out_rst_req ),
                    .key0_in_sel    ( key0_in_sel_detect ),
                    .key1_in_sel    ( key1_in_sel_detect ),
                    .key2_in_sel    ( key2_in_sel_detect ),
                    .pwrb_in_sel    ( pwrb_in_sel_detect ),
                    .ac_present_sel ( ac_present_sel_detect ),
                    .precondition_key0_in_sel ( key0_in_sel_precond ),
                    .precondition_key1_in_sel ( key1_in_sel_precond ),
                    .precondition_key2_in_sel ( key2_in_sel_precond ),
                    .precondition_pwrb_in_sel ( pwrb_in_sel_precond ),
                    .precondition_ac_present_sel ( ac_present_sel_precond) );
                end
              end
              if (com_out_bat_disable) begin
                bat_act_occur_cycles.push_back(key_detect_time);
              end
              if (com_out_rst_req) begin
                rst_req_act_occur_cycles.push_back(key_detect_time);
              end
              if (com_out_ec_rst) begin
                if (key_detect_time > cycles && key_detect_time < (cycles + set_pulse_width)) begin
                  ec_rst_check_wait_cycles = ec_rst_check_wait_cycles > 0 ?
                                      min2(ec_rst_check_wait_cycles, key_detect_time - cycles):
                                      (key_detect_time - cycles);
                end
                // Record which cycle the ec_rst occurs, just need to know the last combo
                // that triggers the ec_rst
                ec_act_occur_cyc = max2(ec_act_occur_cyc, key_detect_time);
                ec_act_triggered = 1;
                combo_detected[i]= com_out_ec_rst | com_out_bat_disable | com_out_intr |
                                    com_out_rst_req;
              end
            end
            `uvm_info(`gfn, $sformatf("bat_act_triggered = %0b", bat_act_triggered), UVM_MEDIUM)
            `uvm_info(`gfn, $sformatf("rst_act_triggered = %0b", rst_act_triggered), UVM_MEDIUM)
            `uvm_info(`gfn, $sformatf("ec_act_triggered = %0b", ec_act_triggered), UVM_MEDIUM)
          end
          // Update start cycle of output assertion
          if(bat_act_occur_cycles.size()>0) begin
            bat_act_occur_cycles.sort();
            if(bat_act_occur_cycles[0] > cycles)
              bat_act_occur_cyc = bat_act_occur_cycles[0] - cycles;
          end
          if(rst_req_act_occur_cycles.size()>0) begin
            rst_req_act_occur_cycles.sort();
            if(rst_req_act_occur_cycles[0] > cycles)
              rst_req_act_occur_cyc = rst_req_act_occur_cycles[0] - cycles;
          end
          `uvm_info(`gfn, $sformatf("bat_act_occur_cyc = %0d", bat_act_occur_cyc), UVM_MEDIUM)
          `uvm_info(`gfn, $sformatf("rst_req_act_occur_cyc = %0d", rst_req_act_occur_cyc),
                                     UVM_MEDIUM)
          `uvm_info(`gfn, $sformatf("ec_rst_check_wait_cycles = %0d", ec_rst_check_wait_cycles),
                                     UVM_MEDIUM)
          `uvm_info(`gfn, $sformatf("ec_act_occur_cyc = %0d", ec_act_occur_cyc), UVM_MEDIUM)

          // Check for Combo output assertions
          fork
            begin
              // Wait till next iteration
              cfg.clk_aon_rst_vif.wait_clks(set_pulse_width);
            end
            begin : ec_rst_check
              if (ec_act_triggered) begin
                int past_cycles = 0;
                if (ec_rst_check_wait_cycles > 0) begin
                  cfg.clk_aon_rst_vif.wait_clks(ec_rst_check_wait_cycles);
                  past_cycles = 0;
                end else begin
                  // -2 is because cross clock domaim make take ~2 cycles.
                  past_cycles = cycles - ec_act_occur_cyc - 2;
                end
                // past_cycles indicates how many cycles the pulse has been active.
                monitor_ec_rst_low(set_pulse_width - past_cycles);
              end else begin
                check_ec_rst_inactive(set_pulse_width);
                `DV_CHECK_EQ(cfg.vif.ec_rst_l_out, 1);
              end
            end : ec_rst_check
            begin : bat_disable_check
              if (bat_act_triggered) begin
                if (bat_act_occur_cyc > 0) monitor_bat_disable_L2H(bat_act_occur_cyc);
                else
                  `DV_CHECK_EQ(cfg.vif.bat_disable, 1);
              end else begin
                `DV_CHECK_EQ(cfg.vif.bat_disable, 0);
              end
            end : bat_disable_check
            begin : rst_req_check
              if (rst_act_triggered) begin
                if (rst_req_act_occur_cyc > 0) monitor_rst_req_L2H(rst_req_act_occur_cyc);
                else
                  `DV_CHECK_EQ(cfg.vif.rst_req, 1);
              end else begin
                `DV_CHECK_EQ(cfg.vif.rst_req, 0);
              end
            end : rst_req_check
          join

          // Check for interrupt status after output check
          begin : intr_check
            cfg.clk_aon_rst_vif.wait_clks(1);
            csr_rd(ral.combo_intr_status, rdata);
            `DV_CHECK_EQ(rdata, intr_actions)
            if (intr_actions) begin
              check_interrupts(.interrupts(1 << IntrSysrstCtrl), .check_set(1));

              // Write to clear the interrupt.
              csr_wr(ral.combo_intr_status, rdata);

              cfg.clk_aon_rst_vif.wait_clks(5);
              // Check if the interrupt is cleared.
              csr_rd_check(ral.combo_intr_status, .compare_value(0));
              // Sample the combo intr status covergroup.
              // The combo_intr_status get updated only when the interrupt action is triggered.
              if (cfg.en_cov) begin
                cov.combo_intr_status.sysrst_ctrl_combo_intr_status_cg.sample(
                    get_field_val(ral.combo_intr_status.combo0_h2l, rdata), get_field_val(
                    ral.combo_intr_status.combo1_h2l, rdata), get_field_val(
                    ral.combo_intr_status.combo2_h2l, rdata), get_field_val(
                    ral.combo_intr_status.combo3_h2l, rdata), cfg.vif.key0_in, cfg.vif.key1_in,
                    cfg.vif.key2_in, cfg.vif.pwrb_in, cfg.vif.ac_present, intr_actions);
              end
            end else begin
              check_interrupts(.interrupts(1 << IntrSysrstCtrl), .check_set(0));
            end
          end : intr_check

          // Reset design for sticky combo output settings
          if (bat_act_triggered || rst_act_triggered) begin
            // Reset combo logic input
            reset_combo_inputs();
            apply_resets_concurrently();
            // Delay to avoid race condition when sending item and checking no item after
            // reset occur at the same time.
            #1ps;
            // Release ec_rst_l_o after reset
            release_ec_rst_l_o();
            // Apply_resets_concurrently will set the registers to their default values,
            // wait for sometime and reconfigure the registers for next iteration.
            config_register();
            // Reset internal variables
            precondition_detected = '{4{1'b0}};
            combo_detected = '{4{1'b0}};
            // Exit combo_action_check block
            break;
          end
          num_trans_combo_detect--;
        end : combo_action_check
      end : combo_detect_block
      // Reset combo inputs after iteration
      reset_combo_inputs();
      // Reprogram the combo selection registers
      for (int i = 0; i < 4; i++) begin
        csr_wr(ral.com_sel_ctl[i], trigger_combo[i]);
        csr_wr(ral.com_pre_sel_ctl[i], trigger_combo_precondition[i]);
      end
      cfg.clk_aon_rst_vif.wait_clks(3);
    end : main_block
  endtask : body

endclass : sysrst_ctrl_combo_detect_with_pre_cond_vseq
