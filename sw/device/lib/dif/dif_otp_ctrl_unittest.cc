// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
#include "sw/device/lib/dif/dif_otp_ctrl.h"

#include <cstring>
#include <limits>
#include <ostream>

#include "gtest/gtest.h"
#include "sw/device/lib/base/mmio.h"
#include "sw/device/lib/base/mock_mmio.h"
#include "sw/device/lib/dif/dif_test_base.h"

#include "otp_ctrl_regs.h"  // Generated.

namespace dif_otp_ctrl_unittest {
namespace {
using ::mock_mmio::LeInt;
using ::mock_mmio::MmioTest;
using ::mock_mmio::MockDevice;
using ::testing::Each;
using ::testing::ElementsAre;

class OtpTest : public testing::Test, public MmioTest {
 protected:
  dif_otp_ctrl_t otp_ = {.base_addr = dev().region()};
};

class DaiRegwenTest : public OtpTest {};

TEST_F(DaiRegwenTest, LockDai) {
  EXPECT_WRITE32(
      OTP_CTRL_DIRECT_ACCESS_REGWEN_REG_OFFSET,
      {{OTP_CTRL_DIRECT_ACCESS_REGWEN_DIRECT_ACCESS_REGWEN_BIT, false}});
  EXPECT_DIF_OK(dif_otp_ctrl_dai_lock(&otp_));
}

TEST_F(DaiRegwenTest, IsDaiLocked) {
  bool flag;

  EXPECT_READ32(
      OTP_CTRL_DIRECT_ACCESS_REGWEN_REG_OFFSET,
      {{OTP_CTRL_DIRECT_ACCESS_REGWEN_DIRECT_ACCESS_REGWEN_BIT, true}});
  EXPECT_DIF_OK(dif_otp_ctrl_dai_is_locked(&otp_, &flag));
  EXPECT_FALSE(flag);

  EXPECT_READ32(
      OTP_CTRL_DIRECT_ACCESS_REGWEN_REG_OFFSET,
      {{OTP_CTRL_DIRECT_ACCESS_REGWEN_DIRECT_ACCESS_REGWEN_BIT, false}});
  EXPECT_DIF_OK(dif_otp_ctrl_dai_is_locked(&otp_, &flag));
  EXPECT_TRUE(flag);
}

TEST_F(DaiRegwenTest, NullArgs) {
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_lock(nullptr));

  bool flag;
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_is_locked(nullptr, &flag));
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_is_locked(&otp_, nullptr));
}

class ConfigTest : public OtpTest {};

TEST_F(ConfigTest, Basic) {
  dif_otp_ctrl_config_t config = {
      .check_timeout = 100'000,
      .integrity_period_mask = 0x3'ffff,
      .consistency_period_mask = 0x3ff'ffff,
  };

  EXPECT_READ32(OTP_CTRL_CHECK_REGWEN_REG_OFFSET,
                {{OTP_CTRL_CHECK_REGWEN_CHECK_REGWEN_BIT, true}});

  EXPECT_WRITE32(OTP_CTRL_CHECK_TIMEOUT_REG_OFFSET, config.check_timeout);
  EXPECT_WRITE32(OTP_CTRL_INTEGRITY_CHECK_PERIOD_REG_OFFSET,
                 config.integrity_period_mask);
  EXPECT_WRITE32(OTP_CTRL_CONSISTENCY_CHECK_PERIOD_REG_OFFSET,
                 config.consistency_period_mask);

  EXPECT_DIF_OK(dif_otp_ctrl_configure(&otp_, config));
}

TEST_F(ConfigTest, Locked) {
  EXPECT_READ32(OTP_CTRL_CHECK_REGWEN_REG_OFFSET,
                {{OTP_CTRL_CHECK_REGWEN_CHECK_REGWEN_BIT, false}});

  EXPECT_EQ(dif_otp_ctrl_configure(&otp_, {}), kDifLocked);
}

TEST_F(ConfigTest, IsConfigLocked) {
  bool flag;

  EXPECT_READ32(OTP_CTRL_CHECK_REGWEN_REG_OFFSET,
                {{OTP_CTRL_CHECK_REGWEN_CHECK_REGWEN_BIT, true}});
  EXPECT_DIF_OK(dif_otp_ctrl_config_is_locked(&otp_, &flag));
  EXPECT_FALSE(flag);

  EXPECT_READ32(OTP_CTRL_CHECK_REGWEN_REG_OFFSET,
                {{OTP_CTRL_CHECK_REGWEN_CHECK_REGWEN_BIT, false}});
  EXPECT_DIF_OK(dif_otp_ctrl_config_is_locked(&otp_, &flag));
  EXPECT_TRUE(flag);
}

TEST_F(ConfigTest, LockConfig) {
  EXPECT_WRITE32(OTP_CTRL_CHECK_REGWEN_REG_OFFSET,
                 {{OTP_CTRL_CHECK_REGWEN_CHECK_REGWEN_BIT, false}});
  EXPECT_DIF_OK(dif_otp_ctrl_lock_config(&otp_));
}

TEST_F(ConfigTest, NullArgs) {
  EXPECT_DIF_BADARG(dif_otp_ctrl_configure(nullptr, {}));

  bool flag;
  EXPECT_DIF_BADARG(dif_otp_ctrl_config_is_locked(nullptr, &flag));
  EXPECT_DIF_BADARG(dif_otp_ctrl_config_is_locked(&otp_, nullptr));

  EXPECT_DIF_BADARG(dif_otp_ctrl_lock_config(nullptr));
}

class CheckTest : public OtpTest {};

TEST_F(CheckTest, Integrity) {
  EXPECT_READ32(
      OTP_CTRL_CHECK_TRIGGER_REGWEN_REG_OFFSET,
      {{OTP_CTRL_CHECK_TRIGGER_REGWEN_CHECK_TRIGGER_REGWEN_BIT, true}});
  EXPECT_WRITE32(OTP_CTRL_CHECK_TRIGGER_REG_OFFSET,
                 {{OTP_CTRL_CHECK_TRIGGER_INTEGRITY_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_check_integrity(&otp_));
}

TEST_F(CheckTest, Consistency) {
  EXPECT_READ32(
      OTP_CTRL_CHECK_TRIGGER_REGWEN_REG_OFFSET,
      {{OTP_CTRL_CHECK_TRIGGER_REGWEN_CHECK_TRIGGER_REGWEN_BIT, true}});
  EXPECT_WRITE32(OTP_CTRL_CHECK_TRIGGER_REG_OFFSET,
                 {{OTP_CTRL_CHECK_TRIGGER_CONSISTENCY_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_check_consistency(&otp_));
}

TEST_F(CheckTest, LockTrigger) {
  EXPECT_WRITE32(
      OTP_CTRL_CHECK_TRIGGER_REGWEN_REG_OFFSET,
      {{OTP_CTRL_CHECK_TRIGGER_REGWEN_CHECK_TRIGGER_REGWEN_BIT, false}});
  EXPECT_DIF_OK(dif_otp_ctrl_lock_check_trigger(&otp_));
}

TEST_F(CheckTest, Locked) {
  EXPECT_READ32(
      OTP_CTRL_CHECK_TRIGGER_REGWEN_REG_OFFSET,
      {{OTP_CTRL_CHECK_TRIGGER_REGWEN_CHECK_TRIGGER_REGWEN_BIT, false}});
  EXPECT_EQ(dif_otp_ctrl_check_integrity(&otp_), kDifLocked);

  EXPECT_READ32(
      OTP_CTRL_CHECK_TRIGGER_REGWEN_REG_OFFSET,
      {{OTP_CTRL_CHECK_TRIGGER_REGWEN_CHECK_TRIGGER_REGWEN_BIT, false}});
  EXPECT_EQ(dif_otp_ctrl_check_consistency(&otp_), kDifLocked);
}

TEST_F(CheckTest, NullArgs) {
  EXPECT_DIF_BADARG(dif_otp_ctrl_check_integrity(nullptr));
  EXPECT_DIF_BADARG(dif_otp_ctrl_check_consistency(nullptr));
}

class ReadLockTest : public OtpTest {};

// Too many formatting variants in template code, so disabling clang-format.
// clang-format off
TEST_F(ReadLockTest, IsLocked) {
  bool flag;

  EXPECT_READ32(
      OTP_CTRL_VENDOR_TEST_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_VENDOR_TEST_READ_LOCK_VENDOR_TEST_READ_LOCK_BIT,
        true}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionVendorTest, &flag));
  EXPECT_FALSE(flag);

  EXPECT_READ32(
      OTP_CTRL_VENDOR_TEST_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_VENDOR_TEST_READ_LOCK_VENDOR_TEST_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionVendorTest, &flag));
  EXPECT_TRUE(flag);

  EXPECT_READ32(
      OTP_CTRL_CREATOR_SW_CFG_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_CREATOR_SW_CFG_READ_LOCK_CREATOR_SW_CFG_READ_LOCK_BIT,
        true}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionCreatorSwCfg, &flag));
  EXPECT_FALSE(flag);

  EXPECT_READ32(
      OTP_CTRL_CREATOR_SW_CFG_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_CREATOR_SW_CFG_READ_LOCK_CREATOR_SW_CFG_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionCreatorSwCfg, &flag));
  EXPECT_TRUE(flag);

  EXPECT_READ32(
      OTP_CTRL_OWNER_SW_CFG_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_OWNER_SW_CFG_READ_LOCK_OWNER_SW_CFG_READ_LOCK_BIT,
        true}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionOwnerSwCfg, &flag));
  EXPECT_FALSE(flag);

  EXPECT_READ32(
      OTP_CTRL_OWNER_SW_CFG_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_OWNER_SW_CFG_READ_LOCK_OWNER_SW_CFG_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionOwnerSwCfg, &flag));
  EXPECT_TRUE(flag);

  EXPECT_READ32(
      OTP_CTRL_ROT_CREATOR_AUTH_CODESIGN_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_ROT_CREATOR_AUTH_CODESIGN_READ_LOCK_ROT_CREATOR_AUTH_CODESIGN_READ_LOCK_BIT,
        true}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionRotCreatorAuthCodesign, &flag));
  EXPECT_FALSE(flag);

  EXPECT_READ32(
      OTP_CTRL_ROT_CREATOR_AUTH_CODESIGN_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_ROT_CREATOR_AUTH_CODESIGN_READ_LOCK_ROT_CREATOR_AUTH_CODESIGN_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionRotCreatorAuthCodesign, &flag));
  EXPECT_TRUE(flag);

  EXPECT_READ32(
      OTP_CTRL_ROT_CREATOR_AUTH_STATE_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_ROT_CREATOR_AUTH_STATE_READ_LOCK_ROT_CREATOR_AUTH_STATE_READ_LOCK_BIT,
        true}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionRotCreatorAuthState, &flag));
  EXPECT_FALSE(flag);

  EXPECT_READ32(
      OTP_CTRL_ROT_CREATOR_AUTH_STATE_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_ROT_CREATOR_AUTH_STATE_READ_LOCK_ROT_CREATOR_AUTH_STATE_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionRotCreatorAuthState, &flag));
  EXPECT_TRUE(flag);
}

TEST_F(ReadLockTest, Lock) {
  EXPECT_WRITE32(
      OTP_CTRL_VENDOR_TEST_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_VENDOR_TEST_READ_LOCK_VENDOR_TEST_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_lock_reading(
      &otp_, kDifOtpCtrlPartitionVendorTest));

  EXPECT_WRITE32(
      OTP_CTRL_CREATOR_SW_CFG_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_CREATOR_SW_CFG_READ_LOCK_CREATOR_SW_CFG_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_lock_reading(
      &otp_, kDifOtpCtrlPartitionCreatorSwCfg));

  EXPECT_WRITE32(
      OTP_CTRL_OWNER_SW_CFG_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_OWNER_SW_CFG_READ_LOCK_OWNER_SW_CFG_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_lock_reading(
      &otp_, kDifOtpCtrlPartitionOwnerSwCfg));

  EXPECT_WRITE32(
      OTP_CTRL_ROT_CREATOR_AUTH_CODESIGN_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_ROT_CREATOR_AUTH_CODESIGN_READ_LOCK_ROT_CREATOR_AUTH_CODESIGN_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_lock_reading(
      &otp_, kDifOtpCtrlPartitionRotCreatorAuthCodesign));

  EXPECT_WRITE32(
      OTP_CTRL_ROT_CREATOR_AUTH_STATE_READ_LOCK_REG_OFFSET,
      {{OTP_CTRL_ROT_CREATOR_AUTH_STATE_READ_LOCK_ROT_CREATOR_AUTH_STATE_READ_LOCK_BIT,
        false}});
  EXPECT_DIF_OK(dif_otp_ctrl_lock_reading(
      &otp_, kDifOtpCtrlPartitionRotCreatorAuthState));
}

TEST_F(ReadLockTest, NotLockablePartitions) {
  bool flag;
  EXPECT_DIF_BADARG(
      dif_otp_ctrl_lock_reading(&otp_, kDifOtpCtrlPartitionHwCfg0));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionHwCfg0, &flag));

  EXPECT_DIF_BADARG(
      dif_otp_ctrl_lock_reading(&otp_, kDifOtpCtrlPartitionHwCfg1));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionHwCfg1, &flag));

  EXPECT_DIF_BADARG(
      dif_otp_ctrl_lock_reading(&otp_, kDifOtpCtrlPartitionSecret0));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionSecret0, &flag));

  EXPECT_DIF_BADARG(
      dif_otp_ctrl_lock_reading(&otp_, kDifOtpCtrlPartitionSecret1));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionSecret1, &flag));

  EXPECT_DIF_BADARG(
      dif_otp_ctrl_lock_reading(&otp_, kDifOtpCtrlPartitionSecret2));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionSecret2, &flag));

  EXPECT_DIF_BADARG(
      dif_otp_ctrl_lock_reading(&otp_, kDifOtpCtrlPartitionLifeCycle));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionLifeCycle, &flag));
}
// clang-format on

TEST_F(ReadLockTest, NullArgs) {
  bool flag;
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      nullptr, kDifOtpCtrlPartitionVendorTest, &flag));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionVendorTest, nullptr));
  EXPECT_DIF_BADARG(
      dif_otp_ctrl_lock_reading(nullptr, kDifOtpCtrlPartitionVendorTest));

  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      nullptr, kDifOtpCtrlPartitionCreatorSwCfg, &flag));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionCreatorSwCfg, nullptr));
  EXPECT_DIF_BADARG(
      dif_otp_ctrl_lock_reading(nullptr, kDifOtpCtrlPartitionCreatorSwCfg));

  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      nullptr, kDifOtpCtrlPartitionOwnerSwCfg, &flag));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionOwnerSwCfg, nullptr));
  EXPECT_DIF_BADARG(
      dif_otp_ctrl_lock_reading(nullptr, kDifOtpCtrlPartitionOwnerSwCfg));

  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      nullptr, kDifOtpCtrlPartitionRotCreatorAuthCodesign, &flag));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionRotCreatorAuthCodesign, nullptr));
  EXPECT_DIF_BADARG(dif_otp_ctrl_lock_reading(
      nullptr, kDifOtpCtrlPartitionRotCreatorAuthCodesign));

  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      nullptr, kDifOtpCtrlPartitionRotCreatorAuthState, &flag));
  EXPECT_DIF_BADARG(dif_otp_ctrl_reading_is_locked(
      &otp_, kDifOtpCtrlPartitionRotCreatorAuthState, nullptr));
  EXPECT_DIF_BADARG(dif_otp_ctrl_lock_reading(
      nullptr, kDifOtpCtrlPartitionRotCreatorAuthState));
}

class StatusTest : public OtpTest {};

TEST_F(StatusTest, Idle) {
  dif_otp_ctrl_status_t status;

  EXPECT_READ32(OTP_CTRL_STATUS_REG_OFFSET,
                {{OTP_CTRL_STATUS_DAI_IDLE_BIT, true}});
  EXPECT_DIF_OK(dif_otp_ctrl_get_status(&otp_, &status));

  EXPECT_EQ(status.codes, 1 << kDifOtpCtrlStatusCodeDaiIdle);
  EXPECT_THAT(status.causes, Each(kDifOtpCtrlErrorOk));
}

TEST_F(StatusTest, Errors) {
  dif_otp_ctrl_status_t status;

  EXPECT_READ32(OTP_CTRL_STATUS_REG_OFFSET,
                {
                    {OTP_CTRL_STATUS_DAI_IDLE_BIT, true},
                    {OTP_CTRL_STATUS_HW_CFG0_ERROR_BIT, true},
                    {OTP_CTRL_STATUS_LCI_ERROR_BIT, true},
                });

  EXPECT_READ32(OTP_CTRL_ERR_CODE_5_REG_OFFSET,
                {{OTP_CTRL_ERR_CODE_0_ERR_CODE_0_OFFSET,
                  OTP_CTRL_ERR_CODE_0_ERR_CODE_0_VALUE_MACRO_ECC_CORR_ERROR}});
  EXPECT_READ32(OTP_CTRL_ERR_CODE_12_REG_OFFSET,
                {{OTP_CTRL_ERR_CODE_0_ERR_CODE_0_OFFSET,
                  OTP_CTRL_ERR_CODE_0_ERR_CODE_0_VALUE_MACRO_ERROR}});

  EXPECT_DIF_OK(dif_otp_ctrl_get_status(&otp_, &status));
  EXPECT_EQ(status.codes, (1 << kDifOtpCtrlStatusCodeDaiIdle) |
                              (1 << kDifOtpCtrlStatusCodeHwCfg0Error) |
                              (1 << kDifOtpCtrlStatusCodeLciError));
  EXPECT_EQ(status.causes[kDifOtpCtrlStatusCodeHwCfg0Error],
            kDifOtpCtrlErrorMacroRecoverableRead);
  EXPECT_EQ(status.causes[kDifOtpCtrlStatusCodeLciError],
            kDifOtpCtrlErrorMacroUnspecified);
}

TEST_F(StatusTest, NullArgs) {
  dif_otp_ctrl_status_t status;

  EXPECT_DIF_BADARG(dif_otp_ctrl_get_status(nullptr, &status));
  EXPECT_DIF_BADARG(dif_otp_ctrl_get_status(&otp_, nullptr));
}

struct RelativeAddressParams {
  std::string name;
  dif_otp_ctrl_partition_t partition;
  uint32_t abs_address;
  dif_result_t expected_result;
  uint32_t expected_relative_address;
};

class RelativeAddress
    : public OtpTest,
      public testing::WithParamInterface<RelativeAddressParams> {};

TEST_P(RelativeAddress, RelativeAddress) {
  uint32_t got_relative_address;
  dif_result_t got_result = dif_otp_ctrl_relative_address(
      GetParam().partition, GetParam().abs_address, &got_relative_address);
  EXPECT_EQ(got_result, GetParam().expected_result);
  EXPECT_EQ(got_relative_address, GetParam().expected_relative_address);
}

INSTANTIATE_TEST_SUITE_P(
    AllPartitions, RelativeAddress,
    testing::Values(
        RelativeAddressParams{
            "VendorTestOkay",
            kDifOtpCtrlPartitionVendorTest,
            OTP_CTRL_PARAM_VENDOR_TEST_OFFSET + 4,
            kDifOk,
            4,
        },
        RelativeAddressParams{
            "VendorTestUnaligned",
            kDifOtpCtrlPartitionVendorTest,
            OTP_CTRL_PARAM_VENDOR_TEST_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "VendorTestOutOfRangePastEnd",
            kDifOtpCtrlPartitionVendorTest,
            OTP_CTRL_PARAM_VENDOR_TEST_OFFSET + OTP_CTRL_PARAM_VENDOR_TEST_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "CreatorSwCfgOkay",
            kDifOtpCtrlPartitionCreatorSwCfg,
            OTP_CTRL_PARAM_CREATOR_SW_CFG_OFFSET + 4,
            kDifOk,
            4,
        },
        RelativeAddressParams{
            "CreatorSwCfgUnaligned",
            kDifOtpCtrlPartitionCreatorSwCfg,
            OTP_CTRL_PARAM_CREATOR_SW_CFG_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "CreatorSwCfgOutOfRangeBeforeStart",
            kDifOtpCtrlPartitionCreatorSwCfg,
            OTP_CTRL_PARAM_CREATOR_SW_CFG_OFFSET - 4,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "CreatorSwCfgOutOfRangePastEnd",
            kDifOtpCtrlPartitionCreatorSwCfg,
            OTP_CTRL_PARAM_CREATOR_SW_CFG_OFFSET +
                OTP_CTRL_PARAM_CREATOR_SW_CFG_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "OwnerSwCfgOkay",
            kDifOtpCtrlPartitionOwnerSwCfg,
            OTP_CTRL_PARAM_OWNER_SW_CFG_OFFSET + 4,
            kDifOk,
            4,
        },
        RelativeAddressParams{
            "OwnerSwCfgUnaligned",
            kDifOtpCtrlPartitionOwnerSwCfg,
            OTP_CTRL_PARAM_OWNER_SW_CFG_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "OwnerSwCfgOutOfRangeBeforeStart",
            kDifOtpCtrlPartitionOwnerSwCfg,
            OTP_CTRL_PARAM_OWNER_SW_CFG_OFFSET - 4,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "OwnerSwCfgOutOfRangePastEnd",
            kDifOtpCtrlPartitionOwnerSwCfg,
            OTP_CTRL_PARAM_OWNER_SW_CFG_OFFSET +
                OTP_CTRL_PARAM_OWNER_SW_CFG_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "RotCreatorAuthCodesignOkay",
            kDifOtpCtrlPartitionRotCreatorAuthCodesign,
            OTP_CTRL_PARAM_ROT_CREATOR_AUTH_CODESIGN_OFFSET + 4,
            kDifOk,
            4,
        },
        RelativeAddressParams{
            "RotCreatorAuthCodesignUnaligned",
            kDifOtpCtrlPartitionRotCreatorAuthCodesign,
            OTP_CTRL_PARAM_ROT_CREATOR_AUTH_CODESIGN_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "RotCreatorAuthCodesignOutOfRangeBeforeStart",
            kDifOtpCtrlPartitionRotCreatorAuthCodesign,
            OTP_CTRL_PARAM_ROT_CREATOR_AUTH_CODESIGN_OFFSET - 4,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "RotCreatorAuthCodesignOutOfRangePastEnd",
            kDifOtpCtrlPartitionRotCreatorAuthCodesign,
            OTP_CTRL_PARAM_ROT_CREATOR_AUTH_CODESIGN_OFFSET +
                OTP_CTRL_PARAM_ROT_CREATOR_AUTH_CODESIGN_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "RotCreatorAuthStateOkay",
            kDifOtpCtrlPartitionRotCreatorAuthState,
            OTP_CTRL_PARAM_ROT_CREATOR_AUTH_STATE_OFFSET + 4,
            kDifOk,
            4,
        },
        RelativeAddressParams{
            "RotCreatorAuthStateUnaligned",
            kDifOtpCtrlPartitionRotCreatorAuthState,
            OTP_CTRL_PARAM_ROT_CREATOR_AUTH_STATE_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "RotCreatorAuthStateOutOfRangeBeforeStart",
            kDifOtpCtrlPartitionRotCreatorAuthState,
            OTP_CTRL_PARAM_ROT_CREATOR_AUTH_STATE_OFFSET - 4,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "RotCreatorAuthStateOutOfRangePastEnd",
            kDifOtpCtrlPartitionRotCreatorAuthState,
            OTP_CTRL_PARAM_ROT_CREATOR_AUTH_STATE_OFFSET +
                OTP_CTRL_PARAM_ROT_CREATOR_AUTH_STATE_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "HwCfg0Okay",
            kDifOtpCtrlPartitionHwCfg0,
            OTP_CTRL_PARAM_HW_CFG0_OFFSET + 4,
            kDifOk,
            4,
        },
        RelativeAddressParams{
            "HwCfg0Unaligned",
            kDifOtpCtrlPartitionHwCfg0,
            OTP_CTRL_PARAM_HW_CFG0_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "HwCfg0OutOfRangeBeforeStart",
            kDifOtpCtrlPartitionHwCfg0,
            OTP_CTRL_PARAM_HW_CFG0_OFFSET - 4,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "HwCfg0OutOfRangePastEnd",
            kDifOtpCtrlPartitionHwCfg0,
            OTP_CTRL_PARAM_HW_CFG0_OFFSET + OTP_CTRL_PARAM_HW_CFG0_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "HwCfg1Okay",
            kDifOtpCtrlPartitionHwCfg1,
            OTP_CTRL_PARAM_HW_CFG1_OFFSET + 4,
            kDifOk,
            4,
        },
        RelativeAddressParams{
            "HwCfg1Unaligned",
            kDifOtpCtrlPartitionHwCfg1,
            OTP_CTRL_PARAM_HW_CFG1_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "HwCfg1OutOfRangeBeforeStart",
            kDifOtpCtrlPartitionHwCfg1,
            OTP_CTRL_PARAM_HW_CFG1_OFFSET - 4,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "HwCfg1OutOfRangePastEnd",
            kDifOtpCtrlPartitionHwCfg1,
            OTP_CTRL_PARAM_HW_CFG1_OFFSET + OTP_CTRL_PARAM_HW_CFG1_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "Secret0Okay",
            kDifOtpCtrlPartitionSecret0,
            OTP_CTRL_PARAM_SECRET0_OFFSET + 8,
            kDifOk,
            8,
        },
        RelativeAddressParams{
            "Secret0Unaligned",
            kDifOtpCtrlPartitionSecret0,
            OTP_CTRL_PARAM_SECRET0_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "Secret0OutOfRangeBeforeStart",
            kDifOtpCtrlPartitionSecret0,
            OTP_CTRL_PARAM_SECRET0_OFFSET - 8,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "Secret0OutOfRangePastEnd",
            kDifOtpCtrlPartitionSecret0,
            OTP_CTRL_PARAM_SECRET0_OFFSET + OTP_CTRL_PARAM_SECRET0_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "Secret1Okay",
            kDifOtpCtrlPartitionSecret1,
            OTP_CTRL_PARAM_SECRET1_OFFSET + 8,
            kDifOk,
            8,
        },
        RelativeAddressParams{
            "Secret1Unaligned",
            kDifOtpCtrlPartitionSecret1,
            OTP_CTRL_PARAM_SECRET1_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "Secret1OutOfRangeBeforeStart",
            kDifOtpCtrlPartitionSecret1,
            OTP_CTRL_PARAM_SECRET1_OFFSET - 8,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "Secret1OutOfRangePastEnd",
            kDifOtpCtrlPartitionSecret1,
            OTP_CTRL_PARAM_SECRET1_OFFSET + OTP_CTRL_PARAM_SECRET1_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "Secret2Okay",
            kDifOtpCtrlPartitionSecret2,
            OTP_CTRL_PARAM_SECRET2_OFFSET + 8,
            kDifOk,
            8,
        },
        RelativeAddressParams{
            "Secret2Unaligned",
            kDifOtpCtrlPartitionSecret2,
            OTP_CTRL_PARAM_SECRET2_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "Secret2OutOfRangeBeforeStart",
            kDifOtpCtrlPartitionSecret2,
            OTP_CTRL_PARAM_SECRET2_OFFSET - 8,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "Secret2OutOfRangePastEnd",
            kDifOtpCtrlPartitionSecret2,
            OTP_CTRL_PARAM_SECRET2_OFFSET + OTP_CTRL_PARAM_SECRET2_SIZE,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "LifeCycleOkay",
            kDifOtpCtrlPartitionLifeCycle,
            OTP_CTRL_PARAM_LIFE_CYCLE_OFFSET + 4,
            kDifOk,
            4,
        },
        RelativeAddressParams{
            "LifeCycleUnaligned",
            kDifOtpCtrlPartitionLifeCycle,
            OTP_CTRL_PARAM_LIFE_CYCLE_OFFSET + 1,
            kDifUnaligned,
            0,
        },
        RelativeAddressParams{
            "LifeCycleOutOfRangeBeforeStart",
            kDifOtpCtrlPartitionLifeCycle,
            OTP_CTRL_PARAM_LIFE_CYCLE_OFFSET - 4,
            kDifOutOfRange,
            0,
        },
        RelativeAddressParams{
            "LifeCycleOutOfRangePastEnd",
            kDifOtpCtrlPartitionLifeCycle,
            OTP_CTRL_PARAM_LIFE_CYCLE_OFFSET + OTP_CTRL_PARAM_LIFE_CYCLE_SIZE,
            kDifOutOfRange,
            0,
        }),
    [](const testing::TestParamInfo<RelativeAddress::ParamType> &info) {
      return info.param.name;
    });

class DaiReadTest : public OtpTest {};

TEST_F(DaiReadTest, Read32) {
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_MANUF_STATE_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_RD_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_read_start(&otp_, kDifOtpCtrlPartitionHwCfg0,
                                            /*address=*/0x20));

  EXPECT_READ32(OTP_CTRL_DIRECT_ACCESS_RDATA_0_REG_OFFSET, 0x12345678);

  uint32_t val;
  EXPECT_DIF_OK(dif_otp_ctrl_dai_read32_end(&otp_, &val));
  EXPECT_EQ(val, 0x12345678);
}

TEST_F(DaiReadTest, Read64) {
  uint64_t val;
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_SECRET0_OFFSET + 0x8);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_RD_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_read_start(&otp_, kDifOtpCtrlPartitionSecret0,
                                            /*address=*/0x8));

  EXPECT_READ32(OTP_CTRL_DIRECT_ACCESS_RDATA_1_REG_OFFSET, 0x12345678);
  EXPECT_READ32(OTP_CTRL_DIRECT_ACCESS_RDATA_0_REG_OFFSET, 0x90abcdef);

  EXPECT_DIF_OK(dif_otp_ctrl_dai_read64_end(&otp_, &val));
  EXPECT_EQ(val, 0x1234567890abcdef);

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_SECRET1_OFFSET + 0x8);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_RD_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_read_start(&otp_, kDifOtpCtrlPartitionSecret1,
                                            /*address=*/0x8));

  EXPECT_READ32(OTP_CTRL_DIRECT_ACCESS_RDATA_1_REG_OFFSET, 0x12345678);
  EXPECT_READ32(OTP_CTRL_DIRECT_ACCESS_RDATA_0_REG_OFFSET, 0x90abcdef);

  EXPECT_DIF_OK(dif_otp_ctrl_dai_read64_end(&otp_, &val));
  EXPECT_EQ(val, 0x1234567890abcdef);

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_SECRET2_OFFSET + 0x8);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_RD_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_read_start(&otp_, kDifOtpCtrlPartitionSecret2,
                                            /*address=*/0x8));

  EXPECT_READ32(OTP_CTRL_DIRECT_ACCESS_RDATA_1_REG_OFFSET, 0x12345678);
  EXPECT_READ32(OTP_CTRL_DIRECT_ACCESS_RDATA_0_REG_OFFSET, 0x90abcdef);

  EXPECT_DIF_OK(dif_otp_ctrl_dai_read64_end(&otp_, &val));
  EXPECT_EQ(val, 0x1234567890abcdef);
}

TEST_F(DaiReadTest, Unaligned) {
  EXPECT_EQ(dif_otp_ctrl_dai_read_start(&otp_, kDifOtpCtrlPartitionHwCfg0,
                                        /*address=*/0b01),
            kDifUnaligned);
  EXPECT_EQ(dif_otp_ctrl_dai_read_start(&otp_, kDifOtpCtrlPartitionSecret2,
                                        /*address=*/0b100),
            kDifUnaligned);
}

TEST_F(DaiReadTest, OutOfRange) {
  EXPECT_EQ(dif_otp_ctrl_dai_read_start(&otp_, kDifOtpCtrlPartitionHwCfg0,
                                        /*address=*/0x100),
            kDifOutOfRange);
}

TEST_F(DaiReadTest, NullArgs) {
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_read_start(nullptr,
                                                kDifOtpCtrlPartitionHwCfg0,
                                                /*address=*/0x0));

  uint32_t val32;
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_read32_end(nullptr, &val32));
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_read32_end(&otp_, nullptr));

  uint64_t val64;
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_read64_end(nullptr, &val64));
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_read64_end(&otp_, nullptr));
}

class DaiProgramTest : public OtpTest {};

TEST_F(DaiProgramTest, Program32) {
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_MANUF_STATE_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_0_REG_OFFSET, 0x12345678);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_WR_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_program32(&otp_, kDifOtpCtrlPartitionHwCfg0,
                                           /*address=*/0x20,
                                           /*value=*/0x12345678));
}

TEST_F(DaiProgramTest, Program64) {
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_SECRET2_OFFSET + 0x8);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_0_REG_OFFSET, 0x90abcdef);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_1_REG_OFFSET, 0x12345678);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_WR_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_program64(&otp_, kDifOtpCtrlPartitionSecret2,
                                           /*address=*/0x8,
                                           /*value=*/0x1234567890abcdef));
}

TEST_F(DaiProgramTest, BadPartition) {
  EXPECT_EQ(dif_otp_ctrl_dai_program32(&otp_, kDifOtpCtrlPartitionSecret1,
                                       /*address=*/0x0, /*value=*/42),
            kDifError);
  EXPECT_EQ(dif_otp_ctrl_dai_program64(&otp_, kDifOtpCtrlPartitionHwCfg0,
                                       /*address=*/0x0, /*value=*/42),
            kDifError);

  // LC is never writeable.
  EXPECT_EQ(dif_otp_ctrl_dai_program32(&otp_, kDifOtpCtrlPartitionLifeCycle,
                                       /*address=*/0x0, /*value=*/42),
            kDifError);
}

TEST_F(DaiProgramTest, Unaligned) {
  EXPECT_EQ(dif_otp_ctrl_dai_program32(&otp_, kDifOtpCtrlPartitionHwCfg0,
                                       /*address=*/0b01, /*value=*/42),
            kDifUnaligned);
  EXPECT_EQ(dif_otp_ctrl_dai_program64(&otp_, kDifOtpCtrlPartitionSecret2,
                                       /*address=*/0b100, /*value=*/42),
            kDifUnaligned);
}

TEST_F(DaiProgramTest, OutOfRange) {
  // Check that we can't write a digest directly.
  EXPECT_EQ(dif_otp_ctrl_dai_program32(
                &otp_, kDifOtpCtrlPartitionCreatorSwCfg,
                /*address=*/OTP_CTRL_PARAM_CREATOR_SW_CFG_DIGEST_OFFSET,
                /*value=*/42),
            kDifOutOfRange);

  // Same digest check for 64-bit.
  EXPECT_EQ(dif_otp_ctrl_dai_program64(
                &otp_, kDifOtpCtrlPartitionSecret2,
                /*address=*/OTP_CTRL_PARAM_SECRET2_DIGEST_OFFSET, /*value=*/42),
            kDifOutOfRange);
}

TEST_F(DaiProgramTest, NullArgs) {
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_program32(nullptr,
                                               kDifOtpCtrlPartitionHwCfg0,
                                               /*address=*/0x0, /*value=*/42));
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_program64(nullptr,
                                               kDifOtpCtrlPartitionSecret0,
                                               /*address=*/0x0, /*value=*/42));
}

class DaiDigestTest : public OtpTest {};

TEST_F(DaiDigestTest, DigestSw) {
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_VENDOR_TEST_DIGEST_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_0_REG_OFFSET, 0x00abcdef);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_1_REG_OFFSET, 0xabcdef00);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_WR_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionVendorTest,
                                        /*digest=*/0xabcdef0000abcdef));

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_CREATOR_SW_CFG_DIGEST_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_0_REG_OFFSET, 0x00abcdef);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_1_REG_OFFSET, 0xabcdef00);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_WR_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionCreatorSwCfg,
                                        /*digest=*/0xabcdef0000abcdef));

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_OWNER_SW_CFG_DIGEST_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_0_REG_OFFSET, 0x00abcdef);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_1_REG_OFFSET, 0xabcdef00);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_WR_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionOwnerSwCfg,
                                        /*digest=*/0xabcdef0000abcdef));

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_ROT_CREATOR_AUTH_CODESIGN_DIGEST_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_0_REG_OFFSET, 0x00abcdef);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_1_REG_OFFSET, 0xabcdef00);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_WR_BIT, true}});

  EXPECT_DIF_OK(
      dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionRotCreatorAuthCodesign,
                              /*digest=*/0xabcdef0000abcdef));

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_ROT_CREATOR_AUTH_STATE_DIGEST_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_0_REG_OFFSET, 0x00abcdef);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_WDATA_1_REG_OFFSET, 0xabcdef00);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_WR_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_digest(&otp_,
                                        kDifOtpCtrlPartitionRotCreatorAuthState,
                                        /*digest=*/0xabcdef0000abcdef));
}

TEST_F(DaiDigestTest, DigestHw) {
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_HW_CFG0_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_DIGEST_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionHwCfg0,
                                        /*digest=*/0));

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_HW_CFG1_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_DIGEST_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionHwCfg1,
                                        /*digest=*/0));

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_SECRET0_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_DIGEST_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionSecret0,
                                        /*digest=*/0));

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_SECRET1_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_DIGEST_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionSecret1,
                                        /*digest=*/0));

  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_ADDRESS_REG_OFFSET,
                 OTP_CTRL_PARAM_SECRET2_OFFSET);
  EXPECT_WRITE32(OTP_CTRL_DIRECT_ACCESS_CMD_REG_OFFSET,
                 {{OTP_CTRL_DIRECT_ACCESS_CMD_DIGEST_BIT, true}});

  EXPECT_DIF_OK(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionSecret2,
                                        /*digest=*/0));
}

TEST_F(DaiDigestTest, BadPartition) {
  EXPECT_EQ(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionLifeCycle,
                                    /*digest=*/0),
            kDifError);
}

TEST_F(DaiDigestTest, BadDigest) {
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_digest(&otp_, kDifOtpCtrlPartitionHwCfg0,
                                            /*digest=*/0xabcdef0000abcdef));
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_digest(&otp_,
                                            kDifOtpCtrlPartitionCreatorSwCfg,
                                            /*digest=*/0));
}

TEST_F(DaiDigestTest, NullArgs) {
  EXPECT_DIF_BADARG(dif_otp_ctrl_dai_digest(nullptr,
                                            kDifOtpCtrlPartitionCreatorSwCfg,
                                            /*digest=*/0xabcdef0000abcdef));
}

class IsDigestComputed : public OtpTest {};

TEST_F(IsDigestComputed, NullArgs) {
  bool is_computed;
  EXPECT_DIF_BADARG(dif_otp_ctrl_is_digest_computed(
      nullptr, kDifOtpCtrlPartitionSecret2, &is_computed));
  EXPECT_DIF_BADARG(dif_otp_ctrl_is_digest_computed(
      &otp_, kDifOtpCtrlPartitionSecret2, nullptr));
  EXPECT_DIF_BADARG(dif_otp_ctrl_is_digest_computed(
      nullptr, kDifOtpCtrlPartitionSecret2, nullptr));
}

TEST_F(IsDigestComputed, BadPartition) {
  bool is_computed;
  EXPECT_DIF_BADARG(dif_otp_ctrl_is_digest_computed(
      &otp_, kDifOtpCtrlPartitionLifeCycle, &is_computed));
}

TEST_F(IsDigestComputed, Success) {
  bool is_computed;

  EXPECT_READ32(OTP_CTRL_SECRET2_DIGEST_1_REG_OFFSET, 0x98abcdef);
  EXPECT_READ32(OTP_CTRL_SECRET2_DIGEST_0_REG_OFFSET, 0xabcdef01);
  EXPECT_DIF_OK(dif_otp_ctrl_is_digest_computed(
      &otp_, kDifOtpCtrlPartitionSecret2, &is_computed));
  EXPECT_TRUE(is_computed);

  EXPECT_READ32(OTP_CTRL_SECRET2_DIGEST_1_REG_OFFSET, 0);
  EXPECT_READ32(OTP_CTRL_SECRET2_DIGEST_0_REG_OFFSET, 0);
  EXPECT_DIF_OK(dif_otp_ctrl_is_digest_computed(
      &otp_, kDifOtpCtrlPartitionSecret2, &is_computed));
  EXPECT_FALSE(is_computed);
}

struct DigestParams {
  dif_otp_ctrl_partition_t partition;
  bool has_digest;
  ptrdiff_t reg0, reg1;
};

class GetDigest : public OtpTest,
                  public testing::WithParamInterface<DigestParams> {};

TEST_P(GetDigest, GetDigest) {
  if (!GetParam().has_digest) {
    uint64_t digest;
    EXPECT_DIF_BADARG(
        dif_otp_ctrl_get_digest(&otp_, GetParam().partition, &digest));
    return;
  }

  EXPECT_READ32(GetParam().reg1, 0xabcdef99);
  EXPECT_READ32(GetParam().reg0, 0x99abcdef);

  uint64_t digest;
  EXPECT_DIF_OK(dif_otp_ctrl_get_digest(&otp_, GetParam().partition, &digest));
  EXPECT_EQ(digest, 0xabcdef9999abcdef);
}

TEST_P(GetDigest, BadDigest) {
  if (!GetParam().has_digest) {
    return;
  }

  EXPECT_READ32(GetParam().reg1, 0x0);
  EXPECT_READ32(GetParam().reg0, 0x0);

  uint64_t digest;
  EXPECT_EQ(dif_otp_ctrl_get_digest(&otp_, GetParam().partition, &digest),
            kDifError);
}

TEST_P(GetDigest, NullArgs) {
  uint64_t digest;
  EXPECT_DIF_BADARG(
      dif_otp_ctrl_get_digest(nullptr, GetParam().partition, &digest));
  EXPECT_DIF_BADARG(
      dif_otp_ctrl_get_digest(&otp_, GetParam().partition, nullptr));
}

// This depends on the maximum length of partition names, which will
// be changing, so turn formatting off.
// clang-format off
INSTANTIATE_TEST_SUITE_P(
    AllDigests, GetDigest,
    testing::Values(
        DigestParams{
            kDifOtpCtrlPartitionVendorTest,
            true,
            OTP_CTRL_VENDOR_TEST_DIGEST_0_REG_OFFSET,
            OTP_CTRL_VENDOR_TEST_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionCreatorSwCfg,
            true,
            OTP_CTRL_CREATOR_SW_CFG_DIGEST_0_REG_OFFSET,
            OTP_CTRL_CREATOR_SW_CFG_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionOwnerSwCfg,
            true,
            OTP_CTRL_OWNER_SW_CFG_DIGEST_0_REG_OFFSET,
            OTP_CTRL_OWNER_SW_CFG_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionRotCreatorAuthCodesign,
            true,
            OTP_CTRL_ROT_CREATOR_AUTH_CODESIGN_DIGEST_0_REG_OFFSET,
            OTP_CTRL_ROT_CREATOR_AUTH_CODESIGN_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionRotCreatorAuthState,
            true,
            OTP_CTRL_ROT_CREATOR_AUTH_STATE_DIGEST_0_REG_OFFSET,
            OTP_CTRL_ROT_CREATOR_AUTH_STATE_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionHwCfg0,
            true,
            OTP_CTRL_HW_CFG0_DIGEST_0_REG_OFFSET,
            OTP_CTRL_HW_CFG0_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionHwCfg1,
            true,
            OTP_CTRL_HW_CFG1_DIGEST_0_REG_OFFSET,
            OTP_CTRL_HW_CFG1_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionSecret0,
            true,
            OTP_CTRL_SECRET0_DIGEST_0_REG_OFFSET,
            OTP_CTRL_SECRET0_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionSecret1,
            true,
            OTP_CTRL_SECRET1_DIGEST_0_REG_OFFSET,
            OTP_CTRL_SECRET1_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionSecret2,
            true,
            OTP_CTRL_SECRET2_DIGEST_0_REG_OFFSET,
            OTP_CTRL_SECRET2_DIGEST_1_REG_OFFSET,
        },
        DigestParams{
            kDifOtpCtrlPartitionLifeCycle,
            false,
            0,
            0,
        }));
// clang-format on

class BlockingIoTest : public OtpTest {
 protected:
  static constexpr size_t kWords = 4;
};

TEST_F(BlockingIoTest, Read) {
  for (size_t i = 0; i < kWords; ++i) {
    auto offset =
        OTP_CTRL_PARAM_OWNER_SW_CFG_OFFSET + 0x10 + i * sizeof(uint32_t);
    EXPECT_READ32(OTP_CTRL_SW_CFG_WINDOW_REG_OFFSET + offset, i + 1);
  }

  std::vector<uint32_t> buf(kWords);
  EXPECT_DIF_OK(dif_otp_ctrl_read_blocking(
      &otp_, kDifOtpCtrlPartitionOwnerSwCfg, 0x10, buf.data(), buf.size()));
  EXPECT_THAT(buf, ElementsAre(1, 2, 3, 4));
}

TEST_F(BlockingIoTest, BadPartition) {
  std::vector<uint32_t> buf(kWords);
  EXPECT_EQ(dif_otp_ctrl_read_blocking(&otp_, kDifOtpCtrlPartitionHwCfg0, 0x10,
                                       buf.data(), buf.size()),
            kDifError);
}

TEST_F(BlockingIoTest, Unaligned) {
  std::vector<uint32_t> buf(kWords);
  EXPECT_EQ(dif_otp_ctrl_read_blocking(&otp_, kDifOtpCtrlPartitionOwnerSwCfg,
                                       0x11, buf.data(), buf.size()),
            kDifUnaligned);
}

TEST_F(BlockingIoTest, OutOfRange) {
  std::vector<uint32_t> buf(0x2f0);
  EXPECT_EQ(dif_otp_ctrl_read_blocking(&otp_, kDifOtpCtrlPartitionOwnerSwCfg,
                                       0x300, buf.data(), buf.size()),
            kDifOutOfRange);
  EXPECT_EQ(dif_otp_ctrl_read_blocking(&otp_, kDifOtpCtrlPartitionOwnerSwCfg,
                                       0x10, buf.data(), 0x330),
            kDifOutOfRange);
}

TEST_F(BlockingIoTest, NullArgs) {
  std::vector<uint32_t> buf(kWords);
  EXPECT_DIF_BADARG(dif_otp_ctrl_read_blocking(
      nullptr, kDifOtpCtrlPartitionOwnerSwCfg, 0x10, buf.data(), buf.size()));
  EXPECT_DIF_BADARG(dif_otp_ctrl_read_blocking(
      &otp_, kDifOtpCtrlPartitionOwnerSwCfg, 0x10, nullptr, buf.size()));
}

}  // namespace
}  // namespace dif_otp_ctrl_unittest
