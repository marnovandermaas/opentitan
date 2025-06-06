// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>
#include <stdint.h>

#include "dt/dt_rstmgr.h"
#include "dt/dt_sram_ctrl.h"
#include "sw/device/lib/base/macros.h"
#include "sw/device/lib/base/mmio.h"
#include "sw/device/lib/base/multibits.h"
#include "sw/device/lib/base/stdasm.h"
#include "sw/device/lib/dif/dif_rstmgr.h"
#include "sw/device/lib/runtime/log.h"
#include "sw/device/lib/testing/rand_testutils.h"
#include "sw/device/lib/testing/rstmgr_testutils.h"
#include "sw/device/lib/testing/sram_ctrl_testutils.h"
#include "sw/device/lib/testing/test_framework/check.h"
#include "sw/device/lib/testing/test_framework/ottf_main.h"
#include "sw/device/lib/testing/test_framework/ottf_utils.h"

#include "rstmgr_regs.h"     // Generated.
#include "sram_ctrl_regs.h"  // Generated.

OTTF_DEFINE_TEST_CONFIG();

enum {
/**
 * Retention SRAM start address (inclusive).
 */
#if defined(OPENTITAN_IS_EARLGREY)
  kRetSramBaseAddr = TOP_EARLGREY_SRAM_CTRL_RET_AON_RAM_BASE_ADDR,
  kRetRamLastAddr =
      kRetSramBaseAddr + TOP_EARLGREY_SRAM_CTRL_RET_AON_RAM_SIZE_BYTES - 1,
#elif defined(OPENTITAN_IS_DARJEELING)
  kRetSramBaseAddr = TOP_DARJEELING_SRAM_CTRL_RET_AON_RAM_BASE_ADDR,
  kRetRamLastAddr =
      kRetSramBaseAddr + TOP_DARJEELING_SRAM_CTRL_RET_AON_RAM_SIZE_BYTES - 1,
#else
#error Unsupported top
#endif

  // See `sw/device/silicon_creator/lib/drivers/retention_sram.h`.
  kRetSramOwnerAddr = kRetSramBaseAddr + 4 + 2048,

  kTestBufferSizeWords = 16,
  kTestBufferSizeBytes = kTestBufferSizeWords * sizeof(uint32_t),

  /**
   * Note that there are `2^32` valid code words and that each non-valid code
   * word triggers an error. Therefore, the probability that a random 39-bit
   * word triggers an error is: `(2^39 - 2^32)/ 2^39 = 127/128`. Then the
   * probability that all `kPatternTestWords` triggers an errors is
   * `(127/128)^kPatternTestWords` after re-scrambling.
   *
   * The Generic formula:
   *
   *               (w-i)
   *             127
   * Pr(i) =  -------- x (w choose i)
   *                w
   *             128
   * Where:
   *      w = The number of words tested.
   *      i = The number of words that may not generate errors.
   *      Pr(i) = Probability that i words will not generate an ECC error.
   *
   * So for i in (0..3):
   *
   * ``` Python
   * from math import comb
   * w = 32
   * t = 0
   * for i in range(4):
   *    p = ((127**(w-i))/(128**w)) * comb(w,i)
   *    t += p
   *    print(f'Pr({i}): { round(p, 4)},\tsum{{Pr(0-{i})}}: {round(t, 6)}')
   * ```
   * ```
   * Pr(0): 0.778,   sum{Pr(0-0)}: 0.778037
   * Pr(1): 0.196,   sum{Pr(0-1)}: 0.974077
   * Pr(2): 0.0239,  sum{Pr(0-2)}: 0.998004
   * Pr(3): 0.0019,  sum{Pr(0-3)}: 0.999888
   * ```
   * So by choosing 3 as the floor limit we will a have probability of `1 -
   * 0.998004 = 0.1996%` that this test would fail randomly due to ECC errors
   * not being generated.
   *
   * Note: Although `kTestBufferSizeWords` is 16 we use 32 to compute the
   * probability since we perform two tests here RET SRAM and main SRAM.
   */

  kEccErrorsFalsePositiveFloorLimit = 3,
};

static_assert(kTestBufferSizeWords == 16,
              "kBackdoorTestWords changed, so "
              "kEccErrorsFalsePositiveFloorLimit should be "
              "computed again");

typedef struct {
  uint32_t pattern[kTestBufferSizeWords];
  uint32_t backdoor[kTestBufferSizeWords];
  uint32_t ecc_error_counter;
} scramble_test_frame;

static scramble_test_frame *scrambling_frame;
static scramble_test_frame *reference_frame;

static dif_sram_ctrl_t ret_sram;
static dif_rstmgr_t rstmgr;

/**
 * Test pattern to be written and read from SRAM.
 */
static const uint32_t kRamTestPattern1[kTestBufferSizeWords] = {
    0xA5A5A5A5, 0xA23DE94C, 0xD82A4FB0, 0xE3CA4D62, 0xA5A5A5A5, 0xA23DE94C,
    0xD82A4FB0, 0xE3CA4D62, 0xA5A5A5A5, 0xA23DE94C, 0xD82A4FB0, 0xE3CA4D62,
    0xA5A5A5A5, 0xA23DE94C, 0xD82A4FB0, 0xE3CA4D62,
};

/**
 * Test pattern to be written and read from SRAM.
 */
static const uint32_t kRamTestPattern2[kTestBufferSizeWords] = {
    0x5A5A5A5A, 0x3CFB4A77, 0x304C6528, 0xFAEFD5CC, 0x5A5A5A5A, 0x3CFB4A77,
    0x304C6528, 0xFAEFD5CC, 0x5A5A5A5A, 0x3CFB4A77, 0x304C6528, 0xFAEFD5CC,
    0x5A5A5A5A, 0x3CFB4A77, 0x304C6528, 0xFAEFD5CC,
};

/**
 * Expected data for the backdoor write test, to be written from the testbench.
 */
static const uint8_t kBackdoorExpectedBytes[kTestBufferSizeBytes];

/**
 * Performs scrambling, saves the test relevant data and resets the system.
 *
 * This code is written in assembly because MAIN SRAM addresses will be
 * scrambled, which has a similar effect to overwriting with pseudo-random
 * data. This will thrash the SRAM (including .bss, .data segments and the
 * stack), effectively rendering the C runtime environment invalid.
 *
 * This function saves contents of the `scrambling_frame` struct in the main
 * SRAM including the data written from the testbench to the RETENTION SRAM,
 * which is kept intact across the system reboot.
 */
static noreturn void main_sram_scramble(void) {
  // Copy only the reference pattern in most environments.
  uint32_t copy_len = sizeof(reference_frame->pattern);
  // Also copy the backdoor in DV sim. The DV-sim testbench magically corrects
  // the backdoor data so that its ECC is correct. This won't be true on other
  // platforms and the number of ECC errors will be doubled.
  if (kDeviceType == kDeviceSimDV) {
    copy_len += sizeof(reference_frame->backdoor);
  }

  asm volatile(
      // Save the tests frames addresses before the scrambling.
      "lw a2, 0(%[mainFrame])                                        \n"
      "lw a3, 0(%[retFrame])                                         \n"
      // Request the new scrambling key for MAIN SRAM.
      "li t0, 0x1                                                    \n"
      "sw t0, %[kSramCtrlOffset](%[kSramCtrlRegsBase])               \n"

      // Busy loop - waiting for scrambling to finish.
      ".L_scrambling_busy_loop:                                      \n"
      "  lw t0, %[kSramCtrlStatusOffset](%[kSramCtrlRegsBase])       \n"
      "  andi t0, t0, %[kSramCtrlKeyScrDone]                         \n"
      "  beqz t0, .L_scrambling_busy_loop                            \n"

      // Restore the tests frames addresses after the scrambling.
      "sw a2, 0(%[mainFrame])                                        \n"
      "sw a3, 0(%[retFrame])                                         \n"

      // Copy the backdoor and pattern buffers from main to the retention SRAM.
      "add t1, a3, %[kCopyLen]                                      \n"
      ".L_buffer_copy_loop:                                         \n"
      "  lw t0, 0(a2)                                               \n"
      "  sw t0, 0(a3)                                               \n"
      "  addi a2, a2, 4                                             \n"
      "  addi a3, a3, 4                                             \n"
      "  blt a3, t1, .L_buffer_copy_loop                            \n"

      // Trigger the software system reset via the Reset Manager.
      "li t0, %[kMultiBitTrue]                                      \n"
      "sw t0, %[kRstMgrResetReq](%[kRstMgrRegsBase])                \n"

      // Satisfy the `noreturn` promise to the compiler.
      ".L_infinite_loop:                                            \n"
      "  wfi                                                        \n"
      "  j .L_infinite_loop"
      : /* No outputs. */
      : [kMultiBitTrue] "I"(kMultiBitBool4True),

        [kSramCtrlRegsBase] "r"(
            dt_sram_ctrl_primary_reg_block(kDtSramCtrlMain)),
        [kSramCtrlOffset] "I"(SRAM_CTRL_CTRL_REG_OFFSET),
        [kSramCtrlStatusOffset] "I"(SRAM_CTRL_STATUS_REG_OFFSET),

        [kSramCtrlKeyScrDone] "I"(0x1 << SRAM_CTRL_STATUS_SCR_KEY_VALID_BIT),

        [mainFrame] "r"(&scrambling_frame), [retFrame] "r"(&reference_frame),
        [kCopyLen] "r"(copy_len),

        [kRstMgrRegsBase] "r"(dt_rstmgr_primary_reg_block(kDtRstmgrAon)),
        [kRstMgrResetReq] "I"(RSTMGR_RESET_REQ_REG_OFFSET)
      : "t0", "t1", "a2", "a3");

  OT_UNREACHABLE();
}

/**
 * Prepares the buffers.
 *
 * Makes sure that both buffers can be read and written to, and are initialized
 * to the opposite patterns.
 */
static void prepare_sram_for_scrambling(void) {
  LOG_INFO("Writing to addr 0x%x", scrambling_frame->pattern);
  // Make sure we can write and read the buffer in SRAM under test.
  sram_ctrl_testutils_write(
      (uint32_t)scrambling_frame->pattern,
      (sram_ctrl_testutils_data_t){.words = kRamTestPattern2,
                                   .len = kTestBufferSizeWords});
  sram_ctrl_testutils_write(
      (uint32_t)scrambling_frame->pattern,
      (sram_ctrl_testutils_data_t){.words = kRamTestPattern1,
                                   .len = kTestBufferSizeWords});

  LOG_INFO("Checking addr 0x%x", scrambling_frame->pattern);
  CHECK_ARRAYS_EQ(scrambling_frame->pattern, kRamTestPattern1,
                  kTestBufferSizeWords);

  LOG_INFO("Writing to addr 0x%x", reference_frame->pattern);
  // Make sure we can write and read to the reference SRAM.
  sram_ctrl_testutils_write(
      (uint32_t)reference_frame->pattern,
      (sram_ctrl_testutils_data_t){.words = kRamTestPattern1,
                                   .len = kTestBufferSizeWords});
  sram_ctrl_testutils_write(
      (uint32_t)reference_frame->pattern,
      (sram_ctrl_testutils_data_t){.words = kRamTestPattern2,
                                   .len = kTestBufferSizeWords});
  LOG_INFO("Checking addr 0x%x", reference_frame->pattern);
  CHECK_ARRAYS_EQ(reference_frame->pattern, kRamTestPattern2,
                  kTestBufferSizeWords);
}

static void execute_main_sram_test(void) {
  LOG_INFO("ut_backdoor: %x,ut_pattern: %x,ut_ecc_error_counter: %x",
           scrambling_frame->backdoor, scrambling_frame->pattern,
           &scrambling_frame->ecc_error_counter);
  LOG_INFO("ref_backdoor: %x,ref_pattern: %x,ref_ecc_error_counter: %x",
           reference_frame->backdoor, reference_frame->pattern,
           &reference_frame->ecc_error_counter);
  // Reset the Ecc error count.
  reference_frame->ecc_error_counter = 0;

  LOG_INFO("Preparing test...");
  prepare_sram_for_scrambling();
  LOG_INFO("Scrambling...");
  main_sram_scramble();
}

static void check_sram_data(scramble_test_frame *mem_frame) {
  LOG_INFO("Checking addr 0x%x", mem_frame->pattern);
  uint32_t tmp_buffer[kTestBufferSizeWords];
  memcpy(tmp_buffer, (const uint8_t *)mem_frame->pattern, sizeof(tmp_buffer));

  CHECK_ARRAYS_NE((uint32_t *)tmp_buffer, kRamTestPattern1,
                  kTestBufferSizeWords);
  CHECK_ARRAYS_NE((uint32_t *)tmp_buffer, kRamTestPattern2,
                  kTestBufferSizeWords);

  // Decide whether to perform ECC error count checks after memory is scrambled.
  //
  // This is not done on CW305/CW310 FPGAs because interrupts for ECC errors are
  // only triggered when the SecureIbex parameter is enabled. This parameter is
  // disabled for these boards due to resource constraints. On CW340 and the
  // other targets, this parameter is enabled.
  bool check_ecc_errors = false;
  switch (kDeviceType) {
    case kDeviceFpgaCw305:
    case kDeviceFpgaCw310:
      check_ecc_errors = false;
      break;
    case kDeviceFpgaCw340:
    case kDeviceSilicon:
    case kDeviceSimDV:
    case kDeviceSimVerilator:
      check_ecc_errors = true;
      break;
    default:
      CHECK(false, "Device type not handled: %d", kDeviceType);
      return;
  }

  if (check_ecc_errors) {
    LOG_INFO("Checking ECC error count of %d",
             reference_frame->ecc_error_counter);
    CHECK(reference_frame->ecc_error_counter <= kTestBufferSizeWords);

    // Statistically there is always a chance that after changing the scrambling
    // key the ECC bits are correct and no IRQ is triggered. So we tolerate a
    // minimum of false positives.
    uint32_t false_positives =
        kTestBufferSizeWords - reference_frame->ecc_error_counter;

    CHECK(false_positives <= kEccErrorsFalsePositiveFloorLimit,
          "Too many expected ECC errors failed to trigger (%d > %d)",
          false_positives, kEccErrorsFalsePositiveFloorLimit);

    if (false_positives > 0) {
      LOG_INFO("Passing with remark: %d words didn't give expected ECC errors",
               false_positives);
    }
  }

  if (kDeviceType == kDeviceSimDV) {
    // Reading before comparing just to make sure it will always read all the
    // words and the right number of ECC errors will be generated.
    LOG_INFO("Checking backdoor  0x%x", mem_frame->backdoor);
    uint32_t kBackdoorExpectedWords[kTestBufferSizeWords];
    memcpy(kBackdoorExpectedWords, kBackdoorExpectedBytes,
           kTestBufferSizeBytes);

    CHECK_ARRAYS_EQ(mem_frame->backdoor, kBackdoorExpectedWords,
                    kTestBufferSizeWords);
  } else {
    // Outside of DV SIM we can only check that the pattern was scrambled.
    CHECK_ARRAYS_NE(mem_frame->pattern, kRamTestPattern2, kTestBufferSizeWords);
  }
}

static void execute_retention_sram_test(void) {
  LOG_INFO("Wiping retention sram...");
  CHECK_DIF_OK(dif_sram_ctrl_wipe(&ret_sram));

  LOG_INFO("Preparing test...");
  prepare_sram_for_scrambling();

  LOG_INFO("Scrambling...");
  CHECK_STATUS_OK(sram_ctrl_testutils_scramble(&ret_sram));
}

/**
 * Override internal IRQ interrupt service routine to count
 * the number of integrity exceptions.
 */
void ottf_internal_isr(uint32_t *exc_info) {
  reference_frame->ecc_error_counter++;
}

typedef enum test_phases {
  kTestPhaseSetup = 0,
  kTestPhaseMainSramScramble,
  kTestPhaseMainSramCheck,
  kTestPhaseRetSramScramble,
  kTestPhaseRetSramCheck,
  kTestPhaseDone,
} test_phases_t;

// Test phase written by testbench.
static volatile const uint8_t kTestPhase = kTestPhaseSetup;
const uint32_t kTestPhaseTimeoutUsec = 2500;

/**
 * Synchronise the current test phase with the test bench.
 *
 * Note that this is a no-op outside of the DV SIM environment where this test
 * does not have a test bench.
 *
 * @param prior_phase The phase before the sync.
 * @param next_phase The phase expected after the sync.
 * @return The status of the synchronisation.
 */
static status_t sync_testbench(uint8_t prior_phase, uint8_t next_phase) {
  if (kDeviceType == kDeviceSimDV) {
    // Set WFI status for testbench synchronization,
    // no actual WFI instruction is issued.
    test_status_set(kTestStatusInWfi);
    test_status_set(kTestStatusInTest);

    IBEX_TRY_SPIN_FOR(OTTF_BACKDOOR_READ(kTestPhase) != prior_phase,
                      kTestPhaseTimeoutUsec);

    TRY_CHECK(kTestPhase == next_phase);
  }

  return OK_STATUS();
}

/**
 * Executes the MAIN SRAM and RET SRAM scrambling test.
 *
 * - Set the retention SRAM address to the Owner space range.
 * - Set a random address to the main SRAM in between the heap and stack.
 * - Set the reference memory as the retention SRAM and the scrambling as the
 *   main SRAM.
 * - Inform the address to the testbench using `INFO_LOG`.
 * - Prepare the main and retention memory for the test by writing a pattern to
 *   them. In both cases, we write two patterns and double check that only the
 *   second pattern is actually stored in the memory.
 * - Save the reference and scrambling frames pointers from the registers.
 * - Request a new scrambling key for the main memory. This will only
 *   re-scramble the main memory - the retention memory will remain intact!
 * - Restore the reference and scrambling frames pointers to registers.
 * - The backdoor sequence triggers once the new scrambling key becomes valid,
 *   and writes random, but correctly scrambled and ECC encoded data to the main
 *   memory.
 * - Copy the contents of the `scrambling_frame` to the `reference_frame` except
 *   the `ecc_error_counter` to be verified later.
 * - Reset the chip to restore the c runtime.
 * - We check that the `reference_frame` does not match any of the test
 *   patterns.
 * - Check the ECC error counter.
 * - Check that the backdoor written data in the `reference_frame`, matches with
 *   the data supplied by the testbench.
 * - Pick a random address in the retention SRAM range.
 * - Set the reference memory as the main SRAM and the scrambling as the ret
 *   SRAM and repeat the test except that it is neither necessary to copy the
 *   `scrambling_frame` to the `reference_frame` nor reset the chip before the
 *   checking.
 *
 * The control flow between this test software and the testbench is:
 *
 * +-----------------------------+------------------------------+
 * | Software                    | Testbench                    |
 * |-----------------------------|------------------------------|
 * | Send addresses over UART  ---> Receive addresses over UART |
 * |---------------------------SYNC-----------------------------|
 * | Execute main SRAM test      | Write expected data          |
 * | Reset                       |                              |
 * |---------------------------SYNC-----------------------------|
 * | Check main against expected |                              |
 * | Send addresses over UART  ---> Receive addresses over UART |
 * |---------------------------SYNC-----------------------------|
 * | Execute ret SRAM test       | Write expected data          |
 * |---------------------------SYNC-----------------------------|
 * | Check ret against expected  |                              |
 * +-----------------------------+------------------------------+
 */
uint32_t main_sram_addr;
uint32_t ret_sram_addr;

extern uint8_t _stack_start[];
extern uint8_t _freertos_heap_start[];

bool test_main(void) {
  CHECK_DIF_OK(dif_rstmgr_init_from_dt(kDtRstmgrAon, &rstmgr));

  CHECK_DIF_OK(dif_sram_ctrl_init_from_dt(kDtSramCtrlRetAon, &ret_sram));

  main_sram_addr = OT_ALIGN_MEM(rand_testutils_gen32_range(
      (uintptr_t)_freertos_heap_start,
      (uintptr_t)_stack_start - sizeof(scramble_test_frame)));

  // Note: Any other address range in the ret SRAM may be written during the
  // boot, which will invalidate the test.
  ret_sram_addr = OT_ALIGN_MEM(kRetSramOwnerAddr);

  scrambling_frame = (scramble_test_frame *)main_sram_addr;
  reference_frame = (scramble_test_frame *)ret_sram_addr;

  dif_rstmgr_reset_info_bitfield_t info = rstmgr_testutils_reason_get();
  uint8_t current_phase = kTestPhase;
  if (info == kDifRstmgrResetInfoPor) {
    LOG_INFO("RET_SRAM addr: %x MAIN_SRAM addr: %x", ret_sram_addr,
             main_sram_addr);
    CHECK_STATUS_OK(sync_testbench(current_phase, kTestPhaseMainSramScramble));
    LOG_INFO("First boot, testing main sram");
    // First boot, start with ret sram.
    execute_main_sram_test();

  } else if (info == kDifRstmgrResetInfoSw) {
    CHECK_STATUS_OK(sync_testbench(current_phase, kTestPhaseMainSramCheck));
    LOG_INFO("Second boot, checking main sram");

    check_sram_data(reference_frame);

    LOG_INFO("Testing Retention sram");
    ret_sram_addr = OT_ALIGN_MEM(rand_testutils_gen32_range(
        kRetSramBaseAddr, kRetRamLastAddr - sizeof(scramble_test_frame)));
    LOG_INFO("RET_SRAM addr: %x MAIN_SRAM addr: %x", ret_sram_addr,
             main_sram_addr);
    CHECK_STATUS_OK(sync_testbench(current_phase, kTestPhaseRetSramScramble));

    scrambling_frame = (scramble_test_frame *)ret_sram_addr;
    reference_frame = (scramble_test_frame *)main_sram_addr;

    execute_retention_sram_test();

    CHECK_STATUS_OK(sync_testbench(current_phase, kTestPhaseRetSramCheck));
    LOG_INFO("Checking retention sram");

    // Reset the Ecc error count that lies on the main sram.
    reference_frame->ecc_error_counter = 0;
    check_sram_data(scrambling_frame);

    return true;
  }

  return false;
}
