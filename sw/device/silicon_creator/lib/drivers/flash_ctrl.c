// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/silicon_creator/lib/drivers/flash_ctrl.h"

#include <assert.h>

#include "dt/dt_flash_ctrl.h"
#include "sw/device/lib/base/abs_mmio.h"
#include "sw/device/lib/base/bitfield.h"
#include "sw/device/lib/base/hardened.h"
#include "sw/device/lib/base/macros.h"
#include "sw/device/lib/base/memory.h"
#include "sw/device/lib/base/multibits.h"
#include "sw/device/silicon_creator/lib/base/sec_mmio.h"
#include "sw/device/silicon_creator/lib/drivers/otp.h"
#include "sw/device/silicon_creator/lib/error.h"

#include "flash_ctrl_regs.h"
#include "otp_ctrl_regs.h"

static const dt_flash_ctrl_t kFlashCtrlDt = kDtFlashCtrl;

// Values of `flash_ctrl_partition_t` constants must be distinct from each
// other, and `kFlashCtrlRegionInfo* >> 1` should give the correct
// CONTROL.INFO_SEL value.
static_assert(kFlashCtrlPartitionData == 0,
              "Incorrect enum value for kFlashCtrlRegionData");
static_assert(kFlashCtrlPartitionInfo0 >> 1 == 0,
              "Incorrect enum value for kFlashCtrlRegionInfo0");
static_assert(kFlashCtrlPartitionInfo1 >> 1 == 1,
              "Incorrect enum value for kFlashCtrlRegionInfo1");
static_assert(kFlashCtrlPartitionInfo2 >> 1 == 2,
              "Incorrect enum value for kFlashCtrlRegionInfo2");

/**
 * Base address of the flash_ctrl registers.
 */
static inline uint32_t flash_ctrl_core_base(void) {
  return dt_flash_ctrl_primary_reg_block(kFlashCtrlDt);
}

/**
 * Flash transaction parameters.
 */
typedef struct transaction_params {
  /**
   * Start address of a flash transaction.
   *
   * Must be the full byte address. For read and write operations flash
   * controller will truncate to the closest 32-bit word aligned address. For
   * page erases, the controller will truncate to the closest lower page aligned
   * address. For bank erases, the controller will truncate to the closest lower
   * bank aligned address.
   */
  uint32_t addr;
  /**
   * Operation type.
   *
   * Must be set to one of FLASH_CTRL_CONTROL_OP_VALUE_*.
   */
  uint32_t op_type;
  /**
   * Whether to erase a bank or a single page.
   *
   * Only applies to erase operations.
   */
  flash_ctrl_erase_type_t erase_type;
  /**
   * Partition to operate on.
   */
  flash_ctrl_partition_t partition;
  /**
   * Number of 32-bit words.
   *
   * Only applies to read and write operations.
   */
  uint32_t word_count;
} transaction_params_t;

/**
 * Starts a flash transaction.
 *
 * @param params Transaction parameters, see `transaction_params_t`.
 * @return The result of the operation.
 */
static void transaction_start(transaction_params_t params) {
  // Set the address.
  abs_mmio_write32(flash_ctrl_core_base() + FLASH_CTRL_ADDR_REG_OFFSET,
                   params.addr);
  // Configure flash_ctrl and start the transaction.
  const bool is_info =
      bitfield_bit32_read(params.partition, FLASH_CTRL_PARTITION_BIT_IS_INFO);
  const uint32_t info_type = bitfield_field32_read(
      params.partition, FLASH_CTRL_PARTITION_FIELD_INFO_TYPE);
  bool bank_erase = true;
  switch (launder32(params.erase_type)) {
    case kFlashCtrlEraseTypeBank:
      HARDENED_CHECK_EQ(params.erase_type, kFlashCtrlEraseTypeBank);
      bank_erase = true;
      break;
    case kFlashCtrlEraseTypePage:
      HARDENED_CHECK_EQ(params.erase_type, kFlashCtrlEraseTypePage);
      bank_erase = false;
      break;
    default:
      HARDENED_TRAP();
  }
  uint32_t reg = bitfield_bit32_write(0, FLASH_CTRL_CONTROL_START_BIT, true);
  reg =
      bitfield_field32_write(reg, FLASH_CTRL_CONTROL_OP_FIELD, params.op_type);
  reg =
      bitfield_bit32_write(reg, FLASH_CTRL_CONTROL_PARTITION_SEL_BIT, is_info);
  reg =
      bitfield_field32_write(reg, FLASH_CTRL_CONTROL_INFO_SEL_FIELD, info_type);
  reg = bitfield_bit32_write(reg, FLASH_CTRL_CONTROL_ERASE_SEL_BIT, bank_erase);
  reg = bitfield_field32_write(reg, FLASH_CTRL_CONTROL_NUM_FIELD,
                               params.word_count - 1);
  abs_mmio_write32(flash_ctrl_core_base() + FLASH_CTRL_CONTROL_REG_OFFSET, reg);
}

/**
 * Copies `word_count` words from the read FIFO to the given buffer.
 *
 * Large reads may create back pressure.
 *
 * @param word_count Number of words to read from the FIFO.
 * @param[out] data Output buffer.
 */
static void fifo_read(size_t word_count, void *data) {
  size_t i = 0, r = word_count - 1;
  for (; launder32(i) < word_count && launder32(r) < word_count; ++i, --r) {
    write_32(
        abs_mmio_read32(flash_ctrl_core_base() + FLASH_CTRL_RD_FIFO_REG_OFFSET),
        data);
    data = (char *)data + sizeof(uint32_t);
  }
  HARDENED_CHECK_EQ(i, word_count);
  HARDENED_CHECK_EQ(r, SIZE_MAX);
}

/**
 * Copies `word_count` words from the given buffer to the program FIFO.
 *
 * Large writes may create back pressure.
 *
 * @param word_count Number of words to write to the FIFO.
 * @param data Input buffer.
 */
static void fifo_write(size_t word_count, const void *data) {
  size_t i = 0, r = word_count - 1;
  for (; launder32(i) < word_count && launder32(r) < word_count; ++i, --r) {
    abs_mmio_write32(flash_ctrl_core_base() + FLASH_CTRL_PROG_FIFO_REG_OFFSET,
                     read_32(data));
    data = (const char *)data + sizeof(uint32_t);
  }
  HARDENED_CHECK_EQ(i, word_count);
  HARDENED_CHECK_EQ(r, SIZE_MAX);
}

/**
 * Blocks until the current flash transaction is complete.
 *
 * @param error Error code to return in case of a flash controller error.
 * @return The result of the operation.
 */
OT_WARN_UNUSED_RESULT
static rom_error_t wait_for_done(rom_error_t error) {
  uint32_t op_status;
  do {
    op_status = abs_mmio_read32(flash_ctrl_core_base() +
                                FLASH_CTRL_OP_STATUS_REG_OFFSET);
  } while (!bitfield_bit32_read(op_status, FLASH_CTRL_OP_STATUS_DONE_BIT));
  abs_mmio_write32(flash_ctrl_core_base() + FLASH_CTRL_OP_STATUS_REG_OFFSET,
                   0u);

  if (bitfield_bit32_read(op_status, FLASH_CTRL_OP_STATUS_ERR_BIT)) {
    return error;
  }
  return kErrorOk;
}

/**
 * Writes data to the given partition.
 *
 * @param addr Full byte address to write to.
 * @param partition The partition to write to.
 * @param word_count Number of bus words to write.
 * @param data Data to write.
 * @param error Error code to return in case of a flash controller error.
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
static rom_error_t write(uint32_t addr, flash_ctrl_partition_t partition,
                         uint32_t word_count, const void *data,
                         rom_error_t error) {
  enum {
    kWindowWordCount = FLASH_CTRL_PARAM_REG_BUS_PGM_RES_BYTES / sizeof(uint32_t)
  };

  // Find the number of words that can be written in the first window.
  uint32_t window_word_count =
      kWindowWordCount - ((addr / sizeof(uint32_t)) % kWindowWordCount);
  while (word_count > 0) {
    // Program operations can't cross window boundaries.
    window_word_count =
        word_count < window_word_count ? word_count : window_word_count;

    transaction_start((transaction_params_t){
        .addr = addr,
        .op_type = FLASH_CTRL_CONTROL_OP_VALUE_PROG,
        .partition = partition,
        .word_count = window_word_count,
        // Does not apply to program transactions.
        .erase_type = kFlashCtrlEraseTypePage,
    });

    fifo_write(window_word_count, data);
    RETURN_IF_ERROR(wait_for_done(error));

    addr += window_word_count * sizeof(uint32_t);
    data = (const char *)data + window_word_count * sizeof(uint32_t);
    word_count -= window_word_count;
    window_word_count = kWindowWordCount;
  }

  return kErrorOk;
}

/**
 * Disables all access to a page until next reset.
 *
 * It's the responsibility of the caller to call `SEC_MMIO_WRITE_INCREMENT()`
 * with the correct value.
 *
 * @param info_page An info page.
 */
static void page_lockdown(const flash_ctrl_info_page_t *info_page) {
  sec_mmio_write32(flash_ctrl_core_base() + info_page->cfg_offset, 0);
  sec_mmio_write32(flash_ctrl_core_base() + info_page->cfg_wen_offset, 0);
}

void flash_ctrl_init(void) {
  SEC_MMIO_ASSERT_WRITE_INCREMENT(
      kFlashCtrlSecMmioInit,
      kFlashCtrlSecMmioDataDefaultCfgSet + 2 * kFlashCtrlSecMmioInfoCfgSet);

  // Set `HW_INFO_CFG_OVERRIDE` register if needed. This must be done before
  // initializing the flash_ctrl.
  uint32_t reg_val = FLASH_CTRL_HW_INFO_CFG_OVERRIDE_REG_RESVAL;
  uint32_t otp_val = otp_read32(
      OTP_CTRL_PARAM_CREATOR_SW_CFG_FLASH_HW_INFO_CFG_OVERRIDE_OFFSET);
  multi_bit_bool_t scramble_dis = bitfield_field32_read(
      otp_val, FLASH_CTRL_OTP_FIELD_HW_INFO_CFG_OVERRIDE_SCRAMBLE_DIS);
  if (scramble_dis == kMultiBitBool4True) {
    reg_val = bitfield_field32_write(
        reg_val, FLASH_CTRL_HW_INFO_CFG_OVERRIDE_SCRAMBLE_DIS_FIELD,
        scramble_dis);
  }
  multi_bit_bool_t ecc_dis = bitfield_field32_read(
      otp_val, FLASH_CTRL_OTP_FIELD_HW_INFO_CFG_OVERRIDE_ECC_DIS);
  if (ecc_dis == kMultiBitBool4True) {
    reg_val = bitfield_field32_write(
        reg_val, FLASH_CTRL_HW_INFO_CFG_OVERRIDE_ECC_DIS_FIELD, ecc_dis);
  }
  if (reg_val != FLASH_CTRL_HW_INFO_CFG_OVERRIDE_REG_RESVAL) {
    sec_mmio_write32(
        flash_ctrl_core_base() + FLASH_CTRL_HW_INFO_CFG_OVERRIDE_REG_OFFSET,
        reg_val);
  }

  // Initialize the flash controller.
  abs_mmio_write32(flash_ctrl_core_base() + FLASH_CTRL_INIT_REG_OFFSET,
                   bitfield_bit32_write(0, FLASH_CTRL_INIT_VAL_BIT, true));
  // Configure default scrambling, ECC, and HE settings for the data partition.
  otp_val =
      otp_read32(OTP_CTRL_PARAM_CREATOR_SW_CFG_FLASH_DATA_DEFAULT_CFG_OFFSET);
  flash_ctrl_cfg_t data_default_cfg = {
      .scrambling =
          bitfield_field32_read(otp_val, FLASH_CTRL_OTP_FIELD_SCRAMBLING),
      .ecc = bitfield_field32_read(otp_val, FLASH_CTRL_OTP_FIELD_ECC),
      .he = bitfield_field32_read(otp_val, FLASH_CTRL_OTP_FIELD_HE),
  };
  flash_ctrl_data_default_cfg_set(data_default_cfg);
  // Configure scrambling, ECC, and HE for `boot_data` pages.
  otp_val =
      otp_read32(OTP_CTRL_PARAM_CREATOR_SW_CFG_FLASH_INFO_BOOT_DATA_CFG_OFFSET);
  flash_ctrl_cfg_t boot_data_cfg = {
      .scrambling =
          bitfield_field32_read(otp_val, FLASH_CTRL_OTP_FIELD_SCRAMBLING),
      .ecc = bitfield_field32_read(otp_val, FLASH_CTRL_OTP_FIELD_ECC),
      .he = bitfield_field32_read(otp_val, FLASH_CTRL_OTP_FIELD_HE),
  };
  flash_ctrl_info_cfg_set(&kFlashCtrlInfoPageBootData0, boot_data_cfg);
  flash_ctrl_info_cfg_set(&kFlashCtrlInfoPageBootData1, boot_data_cfg);
}

void flash_ctrl_status_get(flash_ctrl_status_t *status) {
  // Read flash controller status.
  uint32_t fc_status =
      abs_mmio_read32(flash_ctrl_core_base() + FLASH_CTRL_STATUS_REG_OFFSET);

  // Extract flash controller status bits.
  status->rd_full =
      bitfield_bit32_read(fc_status, FLASH_CTRL_STATUS_RD_FULL_BIT);
  status->rd_empty =
      bitfield_bit32_read(fc_status, FLASH_CTRL_STATUS_RD_EMPTY_BIT);
  status->prog_full =
      bitfield_bit32_read(fc_status, FLASH_CTRL_STATUS_PROG_FULL_BIT);
  status->prog_empty =
      bitfield_bit32_read(fc_status, FLASH_CTRL_STATUS_PROG_EMPTY_BIT);
  status->init_wip =
      bitfield_bit32_read(fc_status, FLASH_CTRL_STATUS_INIT_WIP_BIT);
}

void flash_ctrl_error_code_get(flash_ctrl_error_code_t *error_code) {
  // Read flash error code.
  uint32_t code =
      abs_mmio_read32(flash_ctrl_core_base() + FLASH_CTRL_ERR_CODE_REG_OFFSET);

  // Extract flash controller error code bits.
  error_code->macro_err =
      bitfield_bit32_read(code, FLASH_CTRL_ERR_CODE_MACRO_ERR_BIT);
  error_code->update_err =
      bitfield_bit32_read(code, FLASH_CTRL_ERR_CODE_UPDATE_ERR_BIT);
  error_code->prog_type_err =
      bitfield_bit32_read(code, FLASH_CTRL_ERR_CODE_PROG_TYPE_ERR_BIT);
  error_code->prog_win_err =
      bitfield_bit32_read(code, FLASH_CTRL_ERR_CODE_PROG_WIN_ERR_BIT);
  error_code->prog_err =
      bitfield_bit32_read(code, FLASH_CTRL_ERR_CODE_PROG_ERR_BIT);
  error_code->rd_err =
      bitfield_bit32_read(code, FLASH_CTRL_ERR_CODE_RD_ERR_BIT);
  error_code->mp_err =
      bitfield_bit32_read(code, FLASH_CTRL_ERR_CODE_MP_ERR_BIT);
  error_code->op_err =
      bitfield_bit32_read(code, FLASH_CTRL_ERR_CODE_OP_ERR_BIT);
}

rom_error_t flash_ctrl_data_read(uint32_t addr, uint32_t word_count,
                                 void *data) {
  transaction_start((transaction_params_t){
      .addr = addr,
      .op_type = FLASH_CTRL_CONTROL_OP_VALUE_READ,
      .partition = kFlashCtrlPartitionData,
      .word_count = word_count,
      // Does not apply to read transactions.
      .erase_type = kFlashCtrlEraseTypePage,
  });
  fifo_read(word_count, data);
  return wait_for_done(kErrorFlashCtrlDataRead);
}

rom_error_t flash_ctrl_info_read(const flash_ctrl_info_page_t *info_page,
                                 uint32_t offset, uint32_t word_count,
                                 void *data) {
  transaction_start((transaction_params_t){
      .addr = info_page->base_addr + offset,
      .op_type = FLASH_CTRL_CONTROL_OP_VALUE_READ,
      .partition = kFlashCtrlPartitionInfo0,
      .word_count = word_count,
      // Does not apply to read transactions.
      .erase_type = kFlashCtrlEraseTypePage,
  });
  fifo_read(word_count, data);
  return wait_for_done(kErrorFlashCtrlInfoRead);
}

rom_error_t flash_ctrl_info_read_zeros_on_read_error(
    const flash_ctrl_info_page_t *info_page, uint32_t offset,
    uint32_t word_count, void *data) {
  rom_error_t err = flash_ctrl_info_read(info_page, offset, word_count, data);
  if (err != kErrorOk) {
    flash_ctrl_error_code_t flash_ctrl_err_code;
    flash_ctrl_error_code_get(&flash_ctrl_err_code);
    if (flash_ctrl_err_code.rd_err) {
      // If we encountered a read error, return all 0s.
      memset(data, 0, word_count * sizeof(uint32_t));
      return kErrorOk;
    }
  }
  return err;
}

void flash_ctrl_info_lock(const flash_ctrl_info_page_t *info_page) {
  abs_mmio_write32(flash_ctrl_core_base() + info_page->cfg_wen_offset, 0);
}

rom_error_t flash_ctrl_data_write(uint32_t addr, uint32_t word_count,
                                  const void *data) {
  return write(addr, kFlashCtrlPartitionData, word_count, data,
               kErrorFlashCtrlDataWrite);
}

rom_error_t flash_ctrl_info_write(const flash_ctrl_info_page_t *info_page,
                                  uint32_t offset, uint32_t word_count,
                                  const void *data) {
  const uint32_t addr = info_page->base_addr + offset;
  return write(addr, kFlashCtrlPartitionInfo0, word_count, data,
               kErrorFlashCtrlInfoWrite);
}

rom_error_t flash_ctrl_data_erase(uint32_t addr,
                                  flash_ctrl_erase_type_t erase_type) {
  transaction_start((transaction_params_t){
      .addr = addr,
      .op_type = FLASH_CTRL_CONTROL_OP_VALUE_ERASE,
      .erase_type = erase_type,
      .partition = kFlashCtrlPartitionData,
      // Does not apply to erase transactions.
      .word_count = 1,
  });
  return wait_for_done(kErrorFlashCtrlDataErase);
}

rom_error_t flash_ctrl_data_erase_verify(uint32_t addr,
                                         flash_ctrl_erase_type_t erase_type) {
  static_assert(__builtin_popcount(FLASH_CTRL_PARAM_BYTES_PER_BANK) == 1,
                "Bytes per bank must be a power of two.");
  static_assert(__builtin_popcount(FLASH_CTRL_PARAM_BYTES_PER_PAGE) == 1,
                "Bytes per page must be a power of two.");

  size_t byte_count = 0;
  rom_error_t error = kErrorFlashCtrlDataEraseVerify;
  switch (launder32(erase_type)) {
    case kFlashCtrlEraseTypeBank:
      HARDENED_CHECK_EQ(erase_type, kFlashCtrlEraseTypeBank);
      byte_count = FLASH_CTRL_PARAM_BYTES_PER_BANK;
      error = kErrorOk ^ (byte_count - 1);
      break;
    case kFlashCtrlEraseTypePage:
      HARDENED_CHECK_EQ(erase_type, kFlashCtrlEraseTypePage);
      byte_count = FLASH_CTRL_PARAM_BYTES_PER_PAGE;
      error = kErrorOk ^ (byte_count - 1);
      break;
    default:
      HARDENED_TRAP();
      byte_count = 0U;
      break;
  }

  // Truncate to the closest lower bank/page aligned address.
  addr &= ~byte_count + 1;
  uint32_t mask = kFlashCtrlErasedWord;
  size_t i = 0, r = byte_count - 1;
  uint32_t mem_base =
      dt_flash_ctrl_reg_block(kFlashCtrlDt, kDtFlashCtrlRegBlockMem);
  for (; launder32(i) < byte_count && launder32(r) < byte_count;
       i += sizeof(uint32_t), r -= sizeof(uint32_t)) {
    uint32_t word = abs_mmio_read32(mem_base + addr + i);
    mask &= word;
    error &= word;
  }
  HARDENED_CHECK_EQ(i, byte_count);
  HARDENED_CHECK_EQ(r, SIZE_MAX);

  if (launder32(mask) == kFlashCtrlErasedWord) {
    HARDENED_CHECK_EQ(mask, kFlashCtrlErasedWord);
    return error ^ (byte_count - 1);
  }

  return kErrorFlashCtrlDataEraseVerify;
}

rom_error_t flash_ctrl_info_erase(const flash_ctrl_info_page_t *info_page,
                                  flash_ctrl_erase_type_t erase_type) {
  transaction_start((transaction_params_t){
      .addr = info_page->base_addr,
      .op_type = FLASH_CTRL_CONTROL_OP_VALUE_ERASE,
      .erase_type = erase_type,
      .partition = kFlashCtrlPartitionInfo0,
      // Does not apply to erase transactions.
      .word_count = 1,
  });
  return wait_for_done(kErrorFlashCtrlInfoErase);
}

void flash_ctrl_exec_set(uint32_t exec_val) {
  SEC_MMIO_ASSERT_WRITE_INCREMENT(kFlashCtrlSecMmioExecSet, 1);
  sec_mmio_write32(flash_ctrl_core_base() + FLASH_CTRL_EXEC_REG_OFFSET,
                   exec_val);
}

void flash_ctrl_data_default_perms_set(flash_ctrl_perms_t perms) {
  SEC_MMIO_ASSERT_WRITE_INCREMENT(kFlashCtrlSecMmioDataDefaultPermsSet, 1);

  // Read first to preserve ECC, scrambling, and high endurance bits.
  uint32_t reg = sec_mmio_read32(flash_ctrl_core_base() +
                                 FLASH_CTRL_DEFAULT_REGION_REG_OFFSET);
  reg = bitfield_field32_write(reg, FLASH_CTRL_DEFAULT_REGION_RD_EN_FIELD,
                               perms.read);
  reg = bitfield_field32_write(reg, FLASH_CTRL_DEFAULT_REGION_PROG_EN_FIELD,
                               perms.write);
  reg = bitfield_field32_write(reg, FLASH_CTRL_DEFAULT_REGION_ERASE_EN_FIELD,
                               perms.erase);
  sec_mmio_write32(
      flash_ctrl_core_base() + FLASH_CTRL_DEFAULT_REGION_REG_OFFSET, reg);
}

void flash_ctrl_info_perms_set(const flash_ctrl_info_page_t *info_page,
                               flash_ctrl_perms_t perms) {
  SEC_MMIO_ASSERT_WRITE_INCREMENT(kFlashCtrlSecMmioInfoPermsSet, 1);

  // Read first to preserve ECC, scrambling, and high endurance bits.
  uint32_t reg =
      sec_mmio_read32(flash_ctrl_core_base() + info_page->cfg_offset);
  reg = bitfield_field32_write(
      reg, FLASH_CTRL_BANK0_INFO0_PAGE_CFG_0_EN_0_FIELD, kMultiBitBool4True);
  reg = bitfield_field32_write(
      reg, FLASH_CTRL_BANK0_INFO0_PAGE_CFG_0_RD_EN_0_FIELD, perms.read);
  reg = bitfield_field32_write(
      reg, FLASH_CTRL_BANK0_INFO0_PAGE_CFG_0_PROG_EN_0_FIELD, perms.write);
  reg = bitfield_field32_write(
      reg, FLASH_CTRL_BANK0_INFO0_PAGE_CFG_0_ERASE_EN_0_FIELD, perms.erase);
  sec_mmio_write32(flash_ctrl_core_base() + info_page->cfg_offset, reg);
}

void flash_ctrl_data_default_cfg_set(flash_ctrl_cfg_t cfg) {
  SEC_MMIO_ASSERT_WRITE_INCREMENT(kFlashCtrlSecMmioDataDefaultCfgSet, 1);

  // Read first to preserve permission bits.
  uint32_t reg = sec_mmio_read32(flash_ctrl_core_base() +
                                 FLASH_CTRL_DEFAULT_REGION_REG_OFFSET);
  reg = bitfield_field32_write(reg, FLASH_CTRL_DEFAULT_REGION_SCRAMBLE_EN_FIELD,
                               cfg.scrambling);
  reg = bitfield_field32_write(reg, FLASH_CTRL_DEFAULT_REGION_ECC_EN_FIELD,
                               cfg.ecc);
  reg = bitfield_field32_write(reg, FLASH_CTRL_DEFAULT_REGION_HE_EN_FIELD,
                               cfg.he);
  sec_mmio_write32(
      flash_ctrl_core_base() + FLASH_CTRL_DEFAULT_REGION_REG_OFFSET, reg);
}

flash_ctrl_cfg_t flash_ctrl_data_default_cfg_get(void) {
  const uint32_t default_region = sec_mmio_read32(
      flash_ctrl_core_base() + FLASH_CTRL_DEFAULT_REGION_REG_OFFSET);
  return (flash_ctrl_cfg_t){
      .scrambling = bitfield_field32_read(
          default_region, FLASH_CTRL_DEFAULT_REGION_SCRAMBLE_EN_FIELD),
      .ecc = bitfield_field32_read(default_region,
                                   FLASH_CTRL_DEFAULT_REGION_ECC_EN_FIELD),
      .he = bitfield_field32_read(default_region,
                                  FLASH_CTRL_DEFAULT_REGION_HE_EN_FIELD),
  };
}

// This X macro helps to generate code that operates on each of the flash_ctrl
// memory protection regions.
#define FLASH_CTRL_MP_REGIONS(X) \
  X(0)                           \
  X(1)                           \
  X(2)                           \
  X(3)                           \
  X(4)                           \
  X(5)                           \
  X(6)                           \
  X(7)

// Defines the bounds of the given memory protection region by setting the
// MP_REGION_${region} register.
static void flash_ctrl_mp_region_write(flash_ctrl_region_index_t region,
                                       uint32_t page_offset,
                                       uint32_t num_pages) {
#define FLASH_CTRL_MP_REGION_WRITE_(region_macro_arg)                              \
  case ((region_macro_arg)): {                                                     \
    HARDENED_CHECK_EQ(region, (region_macro_arg));                                 \
    uint32_t mp_region = FLASH_CTRL_MP_REGION_##region_macro_arg##_REG_RESVAL;     \
    /* Write the region's base address into the bitfield. */                       \
    mp_region = bitfield_field32_write(                                            \
        mp_region,                                                                 \
        FLASH_CTRL_MP_REGION_##region_macro_arg##_BASE_##region_macro_arg##_FIELD, \
        page_offset);                                                              \
    /* Write the region's size in pages into the bitfield. */                      \
    mp_region = bitfield_field32_write(                                            \
        mp_region,                                                                 \
        FLASH_CTRL_MP_REGION_##region_macro_arg##_SIZE_##region_macro_arg##_FIELD, \
        num_pages);                                                                \
    /* Write the bitfield to the MP_REGION_${region} register. */                  \
    sec_mmio_write32(flash_ctrl_core_base() +                                      \
                         FLASH_CTRL_MP_REGION_##region_macro_arg##_REG_OFFSET,     \
                     mp_region);                                                   \
    return;                                                                        \
  }

  switch (launder32(region)) {
    FLASH_CTRL_MP_REGIONS(FLASH_CTRL_MP_REGION_WRITE_)
    default:
      OT_UNREACHABLE();
  }

#undef FLASH_CTRL_MP_REGION_WRITE_
}

// Resets the given region's memory protection config by resetting the
// MP_REGION_CFG_${region} register, which implicitly disables the region.
static void flash_ctrl_mp_region_cfg_reset(flash_ctrl_region_index_t region) {
#define FLASH_CTRL_MP_REGION_CFG_RESET_(region_macro_arg)                               \
  case ((region_macro_arg)): {                                                          \
    HARDENED_CHECK_EQ(region, (region_macro_arg));                                      \
    static_assert(                                                                      \
        (FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_REG_RESVAL &                     \
         FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_EN_##region_macro_arg##_MASK) == \
            kMultiBitBool4False,                                                        \
        "FLASH_CTRL_MP_REGION_CFG_" #region_macro_arg                                   \
        "'s reset value should disable the region");                                    \
    /* Reset the MP_REGION_CFG_${region} register. */                                   \
    sec_mmio_write32(                                                                   \
        flash_ctrl_core_base() +                                                        \
            FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_REG_OFFSET,                   \
        FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_REG_RESVAL);                      \
    return;                                                                             \
  }

  switch (launder32(region)) {
    FLASH_CTRL_MP_REGIONS(FLASH_CTRL_MP_REGION_CFG_RESET_)
    default:
      OT_UNREACHABLE();
  }

#undef FLASH_CTRL_MP_CFG_RESET_
}

// Configures permissions for the given MP region by setting the appropriate
// MP_REGION_CFG register.
static void flash_ctrl_mp_region_cfg_write(flash_ctrl_region_index_t region,
                                           flash_ctrl_cfg_t cfg,
                                           flash_ctrl_perms_t perms,
                                           multi_bit_bool_t en,
                                           hardened_bool_t lock) {
#define FLASH_CTRL_MP_REGION_CFG_WRITE_(region_macro_arg)                                     \
  case ((region_macro_arg)): {                                                                \
    HARDENED_CHECK_EQ(region, (region_macro_arg));                                            \
    uint32_t mp_region_cfg =                                                                  \
        FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_REG_RESVAL;                             \
    mp_region_cfg = bitfield_field32_write(                                                   \
        mp_region_cfg,                                                                        \
        FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_HE_EN_##region_macro_arg##_FIELD,       \
        cfg.he);                                                                              \
    mp_region_cfg = bitfield_field32_write(                                                   \
        mp_region_cfg,                                                                        \
        FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_ECC_EN_##region_macro_arg##_FIELD,      \
        cfg.ecc);                                                                             \
    mp_region_cfg = bitfield_field32_write(                                                   \
        mp_region_cfg,                                                                        \
        FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_SCRAMBLE_EN_##region_macro_arg##_FIELD, \
        cfg.scrambling);                                                                      \
    mp_region_cfg = bitfield_field32_write(                                                   \
        mp_region_cfg,                                                                        \
        FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_ERASE_EN_##region_macro_arg##_FIELD,    \
        perms.erase);                                                                         \
    mp_region_cfg = bitfield_field32_write(                                                   \
        mp_region_cfg,                                                                        \
        FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_PROG_EN_##region_macro_arg##_FIELD,     \
        perms.write);                                                                         \
    mp_region_cfg = bitfield_field32_write(                                                   \
        mp_region_cfg,                                                                        \
        FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_RD_EN_##region_macro_arg##_FIELD,       \
        perms.read);                                                                          \
    mp_region_cfg = bitfield_field32_write(                                                   \
        mp_region_cfg,                                                                        \
        FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_EN_##region_macro_arg##_FIELD,          \
        en);                                                                                  \
    sec_mmio_write32(                                                                         \
        flash_ctrl_core_base() +                                                              \
            FLASH_CTRL_MP_REGION_CFG_##region_macro_arg##_REG_OFFSET,                         \
        mp_region_cfg);                                                                       \
    if (lock != kHardenedBoolFalse) {                                                         \
      sec_mmio_write32(                                                                       \
          flash_ctrl_core_base() +                                                            \
              FLASH_CTRL_REGION_CFG_REGWEN_##region_macro_arg##_REG_OFFSET,                   \
          0);                                                                                 \
    }                                                                                         \
    return;                                                                                   \
  }

  switch (launder32(region)) {
    FLASH_CTRL_MP_REGIONS(FLASH_CTRL_MP_REGION_CFG_WRITE_)
    default:
      OT_UNREACHABLE();
  }

#undef FLASH_CTRL_MP_REGION_CFG_WRITE_
}

void flash_ctrl_data_region_protect(flash_ctrl_region_index_t region,
                                    uint32_t page_offset, uint32_t num_pages,
                                    flash_ctrl_perms_t perms,
                                    flash_ctrl_cfg_t cfg,
                                    hardened_bool_t lock) {
  // Reset the region's configuration via the MP_REGION_CFG_${region} register.
  // This temporarily disables memory protection for the region.
  flash_ctrl_mp_region_cfg_reset(region);

  // Set the region's bounds in the MP_REGION_${region} register.
  flash_ctrl_mp_region_write(region, page_offset, num_pages);

  // Write the new value of MP_REGION_CFG_${region}.
  flash_ctrl_mp_region_cfg_write(region, cfg, perms,
                                 /*en=*/kMultiBitBool4True, lock);
}

void flash_ctrl_info_cfg_set(const flash_ctrl_info_page_t *info_page,
                             flash_ctrl_cfg_t cfg) {
  SEC_MMIO_ASSERT_WRITE_INCREMENT(kFlashCtrlSecMmioInfoCfgSet, 1);

  // Read first to preserve permission bits.
  uint32_t reg =
      sec_mmio_read32(flash_ctrl_core_base() + info_page->cfg_offset);
  reg = bitfield_field32_write(
      reg, FLASH_CTRL_BANK0_INFO0_PAGE_CFG_0_EN_0_FIELD, kMultiBitBool4True);
  reg = bitfield_field32_write(
      reg, FLASH_CTRL_BANK0_INFO0_PAGE_CFG_0_SCRAMBLE_EN_0_FIELD,
      cfg.scrambling);
  reg = bitfield_field32_write(
      reg, FLASH_CTRL_BANK0_INFO0_PAGE_CFG_0_ECC_EN_0_FIELD, cfg.ecc);
  reg = bitfield_field32_write(
      reg, FLASH_CTRL_BANK0_INFO0_PAGE_CFG_0_HE_EN_0_FIELD, cfg.he);
  sec_mmio_write32(flash_ctrl_core_base() + info_page->cfg_offset, reg);
}

void flash_ctrl_info_cfg_lock(const flash_ctrl_info_page_t *info_page) {
  sec_mmio_write32(flash_ctrl_core_base() + info_page->cfg_wen_offset, 0);
}

void flash_ctrl_bank_erase_perms_set(hardened_bool_t enable) {
  uint32_t reg = 0;
  switch (launder32(enable)) {
    case kHardenedBoolTrue:
      HARDENED_CHECK_EQ(enable, kHardenedBoolTrue);
      reg = bitfield_bit32_write(
          0, FLASH_CTRL_MP_BANK_CFG_SHADOWED_ERASE_EN_0_BIT, true);
      reg = bitfield_bit32_write(
          reg, FLASH_CTRL_MP_BANK_CFG_SHADOWED_ERASE_EN_1_BIT, true);
      break;
    case kHardenedBoolFalse:
      HARDENED_CHECK_EQ(enable, kHardenedBoolFalse);
      reg = 0U;
      break;
    default:
      HARDENED_TRAP();
      reg = 0U;
      break;
  }
  sec_mmio_write32_shadowed(
      flash_ctrl_core_base() + FLASH_CTRL_MP_BANK_CFG_SHADOWED_REG_OFFSET, reg);
}

/**
 * Information pages that should be locked by ROM_EXT before handing over
 * execution to the first owner boot stage. See
 * `flash_ctrl_creator_info_pages_lockdown()`.
 */
static const flash_ctrl_info_page_t *kInfoPagesNoOwnerAccess[] = {
    // Bank 0
    &kFlashCtrlInfoPageFactoryId,
    &kFlashCtrlInfoPageCreatorSecret,
    &kFlashCtrlInfoPageOwnerSecret,
    &kFlashCtrlInfoPageWaferAuthSecret,
    // Bank 1
    &kFlashCtrlInfoPageBootData0,
    &kFlashCtrlInfoPageBootData1,
    &kFlashCtrlInfoPageCreatorReserved0,
};

enum {
  kInfoPagesNoOwnerAccessCount = ARRAYSIZE(kInfoPagesNoOwnerAccess),
};

void flash_ctrl_creator_info_pages_lockdown(void) {
  SEC_MMIO_ASSERT_WRITE_INCREMENT(kFlashCtrlSecMmioCreatorInfoPagesLockdown,
                                  2 * kInfoPagesNoOwnerAccessCount);
  size_t i = 0, r = kInfoPagesNoOwnerAccessCount - 1;
  for (; launder32(i) < kInfoPagesNoOwnerAccessCount &&
         launder32(r) < kInfoPagesNoOwnerAccessCount;
       ++i, --r) {
    page_lockdown(kInfoPagesNoOwnerAccess[i]);
  }
  HARDENED_CHECK_EQ(i, kInfoPagesNoOwnerAccessCount);
  HARDENED_CHECK_EQ(r, SIZE_MAX);
}

const flash_ctrl_cfg_t kCertificateInfoPageCfg = {
    .scrambling = kMultiBitBool4True,
    .ecc = kMultiBitBool4True,
    .he = kMultiBitBool4False,
};
const flash_ctrl_perms_t kCertificateInfoPageCreatorAccess = {
    .read = kMultiBitBool4True,
    .write = kMultiBitBool4True,
    .erase = kMultiBitBool4True,
};
const flash_ctrl_perms_t kCertificateInfoPageOwnerAccess = {
    .read = kMultiBitBool4True,
    .write = kMultiBitBool4False,
    .erase = kMultiBitBool4False,
};

void flash_ctrl_cert_info_page_creator_cfg(
    const flash_ctrl_info_page_t *info_page) {
  SEC_MMIO_ASSERT_WRITE_INCREMENT(kFlashCtrlSecMmioCertInfoPageCreatorCfg, 2);
  flash_ctrl_info_cfg_set(info_page, kCertificateInfoPageCfg);
  flash_ctrl_info_perms_set(info_page, kCertificateInfoPageCreatorAccess);
}

void flash_ctrl_cert_info_page_owner_restrict(
    const flash_ctrl_info_page_t *info_page) {
  SEC_MMIO_ASSERT_WRITE_INCREMENT(kFlashCtrlSecMmioCertInfoPageOwnerRestrict,
                                  2);
  flash_ctrl_info_perms_set(info_page, kCertificateInfoPageOwnerAccess);
  sec_mmio_write32(flash_ctrl_core_base() + info_page->cfg_wen_offset, 0);
}
