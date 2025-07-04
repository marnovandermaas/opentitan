// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/tests/penetrationtests/firmware/sca/aes_sca.h"

#include "sw/device/lib/base/memory.h"
#include "sw/device/lib/base/status.h"
#include "sw/device/lib/dif/dif_aes.h"
#include "sw/device/lib/runtime/log.h"
#include "sw/device/lib/testing/test_framework/ottf_test_config.h"
#include "sw/device/lib/testing/test_framework/ujson_ottf.h"
#include "sw/device/lib/ujson/ujson.h"
#include "sw/device/sca/lib/aes.h"
#include "sw/device/sca/lib/prng.h"
#include "sw/device/tests/penetrationtests/firmware/lib/pentest_lib.h"
#include "sw/device/tests/penetrationtests/json/aes_sca_commands.h"
#include "sw/device/tests/penetrationtests/json/commands.h"

#ifndef OPENTITAN_IS_ENGLISHBREAKFAST
#include "sw/device/lib/testing/aes_testutils.h"
#endif

#include "hw/top_earlgrey/sw/autogen/top_earlgrey.h"

/**
 * Enable FPGA mode.
 */
static bool fpga_mode = false;

enum {
  kAesKeyLengthMax = 32,
  kAesKeyLength = 16,
  kAesTextLength = 16,
  kTestTimeout = (1000 * 1000),
  /**
   * Number of cycles (at `kClockFreqCpuHz`) that Ibex should sleep to minimize
   * noise during AES operations. Caution: This number should be chosen to
   * provide enough time. Otherwise, Ibex might wake up while AES is still busy
   * and disturb the capture. Currently, we use a start trigger delay of 320
   * clock cycles and the scope captures 60 clock cycles at kClockFreqCpuHz.
   */
  kIbexAesSleepCycles = 680,
  /**
   * The maximum number of encryptions to do per batch. The ChipWhisperer Husky
   * scope determines how many encryptions (capture segments) it wants to record
   * per batch based on the number of samples per segment. As the plaintexts
   * and keys are generated in advance for fixed-vs-random batch captures, we
   * need to make sure the corresponding buffers are sufficiently large. Note
   * that on both CW305 and CW310, the main SRAM has a size of 128 kBytes. So it
   * should be fine to allocate space for 256 segments (2 * 16 Bytes * 256 = 8
   * kBytes).
   */
  kNumBatchOpsMax = 256,
  /**
   * Max number of encryptions that can be captured before we rewrite the key to
   * reset the internal block counter. Otherwise, the AES peripheral might
   * trigger the reseeding of the internal masking PRNG which disturbs SCA
   * measurements.
   */
  kBlockCtrMax = 8191,
};

/**
 * An array of keys to be used in a batch.
 */
uint8_t batch_keys[kNumBatchOpsMax][kAesKeyLength];

/**
 * An array of plaintexts to be used in a batch.
 */
uint8_t batch_plaintexts[kNumBatchOpsMax][kAesTextLength];

/**
 * Key selection between fixed and random key during the batch capture.
 */
bool sample_fixed = true;

/**
 * An array to store pre-computed round keys derived from the generation key.
 * The generation key (key_gen) is specified in [DTR] Section 5.1.
 * This key is used for generating all pseudo-random data for batch captures.
 * kKeyGen[kAesKeyLength] = {0x12, 0x34, 0x56, 0x78,
 *                           0x9a, 0xbc, 0xde, 0xf1,
 *                           0x23, 0x45, 0x67, 0x89,
 *                           0xab, 0xcd, 0xe0, 0xf0};
 */
static const uint32_t kKeyGenRoundKeys[(kAesKeyLength / 4) * 11] = {
    0xab239a12, 0xcd45bc34, 0xe067de56, 0xf089f178, 0xbc1734ae, 0xe12c69d5,
    0x836304da, 0x9262eb1a, 0xcb776054, 0x9d7c5039, 0x71f29195, 0x64f6947f,
    0xd2196e0e, 0x2bb6ca9a, 0xc4b547d6, 0x6602f460, 0x528099f7, 0xd1fa4c86,
    0xd317a2e5, 0x452321d5, 0x92c040d9, 0x8756ace0, 0xed3e298b, 0x92d7f4d5,
    0xfc6eaeee, 0xc84f19b5, 0x3ed3edc4, 0x2bb96e9a, 0x7a86e846, 0x99511e07,
    0x350bd835, 0xd6fd442a, 0x3c46c028, 0x47de8f91, 0x25101bc3, 0x9f49b4f0,
    0x29155393, 0xb8ff21ae, 0x36130318, 0x79e6af1b, 0xa68f9ac9, 0xcd758aab,
    0x88beadae, 0x8ef711be};

/**
 * Plaintext of the fixed set of fixed-vs-random-key TVLA
 */
static uint8_t plaintext_fixed[kAesTextLength] = {
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
/**
 * Key of the of the fixed set of fixed-vs-random-key TVLA
 */
static uint8_t key_fixed[kAesTextLength] = {0x81, 0x1E, 0x37, 0x31, 0xB0, 0x12,
                                            0x0A, 0x78, 0x42, 0x78, 0x1E, 0x22,
                                            0xB2, 0x5C, 0xDD, 0xF9};
/**
 * Plaintext of the random set of fixed-vs-random-key TVLA
 */
static uint8_t plaintext_random[kAesTextLength] = {
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc};
/**
 * Key of the random set of fixed-vs-random-key TVLA
 */
static uint8_t key_random[kAesTextLength] = {0x53, 0x53, 0x53, 0x53, 0x53, 0x53,
                                             0x53, 0x53, 0x53, 0x53, 0x53, 0x53,
                                             0x53, 0x53, 0x53, 0x53};
/**
 * Temp ciphertext variable
 */
static uint8_t ciphertext_temp[kAesTextLength];

/**
 * batch_plaintext for batch capture to initially set it using command.
 */
static uint8_t batch_plaintext[kAesTextLength];

/**
 * Block counter variable for manually handling reseeding operations of the
 * masking PRNG inside the AES peripheral.
 */
static uint32_t block_ctr;

static dif_aes_t aes;

dif_aes_transaction_t transaction = {
    .operation = kDifAesOperationEncrypt,
    .mode = kDifAesModeEcb,
    .key_len = kDifAesKey128,
    .manual_operation = kDifAesManualOperationManual,
    .key_provider = kDifAesKeySoftwareProvided,
    .mask_reseeding = kDifAesReseedPer8kBlock,
    .reseed_on_key_change = false,
    .force_masks = false,
    .ctrl_aux_lock = false,
};

/**
 * Load fixed seed into AES.
 *
 * Before calling this function, use
 * aes_testutils_masking_prng_zero_output_seed() to initialize the entropy
 * complex for performing AES SCA measurements with masking switched off. This
 * function then loads the fixed seed into the AES, allowing the disable the
 * masking.
 *
 * @param key Key.
 * @param key_len Key length.
 */
static status_t aes_sca_load_fixed_seed(void) {
  AES_TESTUTILS_WAIT_FOR_STATUS(&aes, kDifAesStatusIdle, true, kTestTimeout);
  // Load magic seed such that masking is turned off. We need to do this after
  // dif_aes_start() as then the force_masks is correctly set.
  TRY(dif_aes_trigger(&aes, kDifAesTriggerPrngReseed));
  AES_TESTUTILS_WAIT_FOR_STATUS(&aes, kDifAesStatusIdle, true, kTestTimeout);

  return OK_STATUS();
}

/**
 * Mask and configure key.
 *
 * This function masks the provided key using a software LFSR and then
 * configures the key into the AES peripheral. The masking can be disabled by
 * initializing the LFSR to 0 (see `aes_serial_seed_lfsr()`). The key must be
 * `kAesKeyLength` bytes long.
 *
 * @param key Key.
 * @param key_len Key length.
 */
static status_t aes_key_mask_and_config(const uint8_t *key, size_t key_len) {
  if (key_len != kAesKeyLength) {
    return ABORTED();
  }
  dif_aes_key_share_t key_shares;
  // Mask the provided key.
  for (int i = 0; i < key_len / 4; ++i) {
    key_shares.share1[i] = pentest_non_linear_layer(
        pentest_linear_layer(pentest_next_lfsr(1, kPentestLfsrMasking)));
    key_shares.share0[i] = *((uint32_t *)key + i) ^ key_shares.share1[i];
  }
  // Provide random shares for unused key bits.
  for (size_t i = key_len / 4; i < kAesKeyLengthMax / 4; ++i) {
    key_shares.share1[i] =
        pentest_non_linear_layer(pentest_next_lfsr(1, kPentestLfsrMasking));
    key_shares.share0[i] =
        pentest_non_linear_layer(pentest_next_lfsr(1, kPentestLfsrMasking));
  }
  TRY(dif_aes_start(&aes, &transaction, &key_shares, NULL));

#ifndef OPENTITAN_IS_ENGLISHBREAKFAST
  if (transaction.force_masks) {
    // Disable masking. Force the masking PRNG output value to 0.
    TRY(aes_sca_load_fixed_seed());
  }
#endif

  return OK_STATUS();
}

/**
 * Callback wrapper for AES manual trigger function.
 */
static void aes_manual_trigger(void) {
  (void)dif_aes_trigger(&aes, kDifAesTriggerStart);
}

/**
 * Encrypts a plaintext using the AES peripheral.
 *
 * This function uses `pentest_call_and_sleep()` from the pentest library to put
 * Ibex to sleep in order to minimize noise during captures. The plaintext must
 * be `kAesTextLength` bytes long.
 *
 * @param plaintext Plaintext.
 * @param plaintext_len Length of the plaintext.
 */
static status_t aes_encrypt(const uint8_t *plaintext, size_t plaintext_len) {
  bool ready = false;
  do {
    TRY(dif_aes_get_status(&aes, kDifAesStatusInputReady, &ready));
  } while (!ready);

  dif_aes_data_t data;
  if (plaintext_len != sizeof(data.data)) {
    return ABORTED();
  }
  memcpy(data.data, plaintext, plaintext_len);
  TRY(dif_aes_load_data(&aes, data));

  // Start AES operation (this triggers the capture) and go to sleep.
  if (fpga_mode) {
    // On FPGA, the trigger is AND-ed with AES !IDLE and creates a LO-HI-LO per.
    // Activate the GPIO by setting the GPIO.
    pentest_set_trigger_high();
    pentest_call_and_sleep(aes_manual_trigger, kIbexAesSleepCycles, false,
                           false);
    pentest_set_trigger_low();
  } else {
    // On the chip, we need to manually set and unset the trigger. This is done
    // in this function to have the trigger as close as possible to the AES
    // operation.
    pentest_call_and_sleep(aes_manual_trigger, kIbexAesSleepCycles, true,
                           false);
  }
  return OK_STATUS();
}

/**
 * Wait until AES output is valid and then get ciphertext and send it over
 * serial communication.
 *
 * @param only_first_word Send only the first word of the ciphertext.
 */
static status_t aes_send_ciphertext(bool only_first_word, ujson_t *uj) {
  bool ready = false;
  do {
    TRY(dif_aes_get_status(&aes, kDifAesStatusOutputValid, &ready));
  } while (!ready);

  dif_aes_data_t ciphertext;
  if (dif_aes_read_output(&aes, &ciphertext) != kDifOk) {
    return OUT_OF_RANGE();
  }

  aes_sca_ciphertext_t uj_output;
  memset(uj_output.ciphertext, 0, AESSCA_CMD_MAX_DATA_BYTES);
  uj_output.ciphertext_length = kAesTextLength;
  if (only_first_word) {
    uj_output.ciphertext_length = 4;
  }
  memcpy(uj_output.ciphertext, (uint8_t *)ciphertext.data,
         uj_output.ciphertext_length);
  RESP_OK(ujson_serialize_aes_sca_ciphertext_t, uj, &uj_output);
  return OK_STATUS();
}

/**
 * Advances data for fvsr-key TVLA - fixed set.
 *
 * This function updates plaintext_fixed for fvsr-key TVLA, according
 * to DTR recommendations.
 */
static void aes_serial_advance_fixed(void) {
  aes_sw_encrypt_block(plaintext_fixed, kKeyGenRoundKeys, ciphertext_temp);
  memcpy(plaintext_fixed, ciphertext_temp, kAesTextLength);
}

/**
 * Advances data for fvsr-key TVLA - random set.
 *
 * This function updates plaintext_random and key_random for fvsr-key and
 * random TVLA, according to DTR recommendations.
 */
static void aes_serial_advance_random(void) {
  aes_sw_encrypt_block(plaintext_random, kKeyGenRoundKeys, ciphertext_temp);
  memcpy(plaintext_random, ciphertext_temp, kAesTextLength);
  aes_sw_encrypt_block(key_random, kKeyGenRoundKeys, ciphertext_temp);
  memcpy(key_random, ciphertext_temp, kAesTextLength);
}

/**
 * Advances data for fvsr-data TVLA - random set.
 *
 * This function updates plaintext_random for fvsr-data and
 * TVLA, according to DTR recommendations, Section 5.1.
 */
static void aes_serial_advance_random_data(void) {
  aes_sw_encrypt_block(plaintext_random, kKeyGenRoundKeys, ciphertext_temp);
  memcpy(plaintext_random, ciphertext_temp, kAesTextLength);
}

/**
 * Fixed vs random key batch generate command handler.
 *
 * This command generates random plaintexts and fixed or random keys using PRNG
 * for AES fixed vs random key batch capture in order to remove fake leakage.
 * Fixed or random key sequence is also determined here by using the lsb bit of
 * the plaintext. In order to simplify the analysis, the first encryption has to
 * use fixed key. The data collection method is based on the derived test
 * requirements (DTR) for TVLA:
 * https://www.rambus.com/wp-content/uploads/2015/08/TVLA-DTR-with-AES.pdf
 * The measurements are taken by using either fixed or randomly selected keys.
 * In addition, a PRNG is used for random key and plaintext generation instead
 * of AES algorithm as specified in the TVLA DTR.
 *
 * Packet payload must be a `uint32_t` representation of the number of
 * encryptions to perform. Number of operations of a batch should not be greater
 * than the 'kNumBatchOpsMax' value.
 *
 * The PRNG should be initialized using the 's' (seed PRNG) command before
 * starting batch captures. In addition, the fixed key should also be set
 * using 't' (fvsr key set) command before starting batch captures.
 *
 * The uJSON data contains:
 *  - data: The number of encryptions.
 *
 * @param uj The received uJSON data.
 */
static status_t aes_sca_fvsr_key_batch_generate(
    penetrationtest_num_enc_t uj_data) {
  if (uj_data.num_enc > kNumBatchOpsMax) {
    return OUT_OF_RANGE();
  }

  for (uint32_t i = 0; i < uj_data.num_enc; ++i) {
    if (sample_fixed) {
      memcpy(batch_keys[i], key_fixed, kAesKeyLength);
      memcpy(batch_plaintexts[i], plaintext_fixed, kAesKeyLength);
      aes_serial_advance_fixed();
    } else {
      memcpy(batch_keys[i], key_random, kAesKeyLength);
      memcpy(batch_plaintexts[i], plaintext_random, kAesKeyLength);
      aes_serial_advance_random();
    }
    sample_fixed = batch_plaintexts[i][0] & 0x1;
  }

  return OK_STATUS();
}

status_t handle_aes_sca_batch_alternative_encrypt(ujson_t *uj) {
  penetrationtest_num_enc_t uj_data;
  TRY(ujson_deserialize_penetrationtest_num_enc_t(uj, &uj_data));

  // Add to current block_ctr to check if > kBlockCtrMax
  block_ctr += uj_data.num_enc;
  // Rewrite the key to reset the internal block counter. Otherwise, the AES
  // peripheral might trigger the reseeding of the internal masking PRNG which
  // disturbs SCA measurements.
  if (block_ctr > kBlockCtrMax) {
    TRY(aes_key_mask_and_config(key_fixed, kAesKeyLength));
    block_ctr = uj_data.num_enc;
  }

  // First plaintext has been set through command into batch_plaintext

  dif_aes_data_t ciphertext;
  for (uint32_t i = 0; i < uj_data.num_enc; ++i) {
    // Encrypt
    TRY(aes_encrypt(batch_plaintext, kAesTextLength));

    // Get ciphertext
    bool ready = false;
    do {
      TRY(dif_aes_get_status(&aes, kDifAesStatusOutputValid, &ready));
    } while (!ready);

    if (dif_aes_read_output(&aes, &ciphertext)) {
      return ABORTED();
    }

    // Use ciphertext as next plaintext (incl. next call to this function)
    memcpy(batch_plaintext, ciphertext.data, kAesTextLength);
  }

  // send last ciphertext
  aes_sca_ciphertext_t uj_output;
  uj_output.ciphertext_length = kAesTextLength;
  memcpy(uj_output.ciphertext, (uint8_t *)ciphertext.data, kAesTextLength);
  RESP_OK(ujson_serialize_aes_sca_ciphertext_t, uj, &uj_output);

  return OK_STATUS();
}

status_t handle_aes_sca_batch_encrypt(ujson_t *uj) {
  penetrationtest_num_enc_t uj_data;
  TRY(ujson_deserialize_penetrationtest_num_enc_t(uj, &uj_data));

  block_ctr += uj_data.num_enc;
  // Rewrite the key to reset the internal block counter. Otherwise, the AES
  // peripheral might trigger the reseeding of the internal masking PRNG which
  // disturbs SCA measurements.
  if (block_ctr > kBlockCtrMax) {
    TRY(aes_key_mask_and_config(key_fixed, kAesKeyLength));
    block_ctr = uj_data.num_enc;
  }

  for (uint32_t i = 0; i < uj_data.num_enc; ++i) {
    TRY(aes_encrypt(plaintext_random, kAesTextLength));
    aes_serial_advance_random();
  }

  TRY(aes_send_ciphertext(true, uj));

  return OK_STATUS();
}

status_t handle_aes_sca_batch_encrypt_random(ujson_t *uj) {
  penetrationtest_num_enc_t uj_data;
  TRY(ujson_deserialize_penetrationtest_num_enc_t(uj, &uj_data));

  block_ctr += uj_data.num_enc;
  // Rewrite the key to reset the internal block counter. Otherwise, the AES
  // peripheral might trigger the reseeding of the internal masking PRNG which
  // disturbs SCA measurements.
  if (block_ctr > kBlockCtrMax) {
    TRY(aes_key_mask_and_config(key_random, kAesKeyLength));
    block_ctr = uj_data.num_enc;
  }

  for (uint32_t i = 0; i < uj_data.num_enc; ++i) {
    TRY(aes_key_mask_and_config(key_random, kAesKeyLength));
    TRY(aes_encrypt(plaintext_random, kAesTextLength));
    aes_serial_advance_random();
  }

  TRY(aes_send_ciphertext(true, uj));

  return OK_STATUS();
}

status_t handle_aes_sca_batch_plaintext_set(ujson_t *uj) {
  aes_sca_text_t uj_data;
  TRY(ujson_deserialize_aes_sca_text_t(uj, &uj_data));

  if (uj_data.text_length != kAesTextLength) {
    return OUT_OF_RANGE();
  }
  memcpy(batch_plaintext, uj_data.text, uj_data.text_length);

  return OK_STATUS();
}

status_t handle_aes_sca_fvsr_data_batch_encrypt(ujson_t *uj) {
  penetrationtest_num_enc_t uj_data;
  TRY(ujson_deserialize_penetrationtest_num_enc_t(uj, &uj_data));

  if (uj_data.num_enc > kNumBatchOpsMax) {
    return OUT_OF_RANGE();
  }

  for (uint32_t i = 0; i < uj_data.num_enc; ++i) {
    memcpy(batch_keys[i], key_fixed, kAesKeyLength);
    if (sample_fixed) {
      memcpy(batch_plaintexts[i], plaintext_fixed, kAesKeyLength);
    } else {
      memcpy(batch_plaintexts[i], plaintext_random, kAesKeyLength);
      aes_serial_advance_random_data();
    }
    sample_fixed = pentest_next_lfsr(1, kPentestLfsrOrder) & 0x1;
  }

  for (uint32_t i = 0; i < uj_data.num_enc; ++i) {
    TRY(aes_key_mask_and_config(batch_keys[i], kAesKeyLength));
    TRY(aes_encrypt(batch_plaintexts[i], kAesTextLength));
  }

  TRY(aes_send_ciphertext(false, uj));

  return OK_STATUS();
}

status_t handle_aes_sca_fvsr_key_batch_encrypt(ujson_t *uj) {
  penetrationtest_num_enc_t uj_data;
  TRY(ujson_deserialize_penetrationtest_num_enc_t(uj, &uj_data));

  if (uj_data.num_enc > kNumBatchOpsMax) {
    return OUT_OF_RANGE();
  }

  for (uint32_t i = 0; i < uj_data.num_enc; ++i) {
    TRY(aes_key_mask_and_config(batch_keys[i], kAesKeyLength));
    TRY(aes_encrypt(batch_plaintexts[i], kAesTextLength));
  }

  TRY(aes_send_ciphertext(false, uj));

  // Start to generate random keys and plaintexts for the next batch when the
  // waves are getting from scope by the host to increase capture rate.
  return aes_sca_fvsr_key_batch_generate(uj_data);
}

status_t handle_aes_sca_fvsr_key_batch_generate(ujson_t *uj) {
  penetrationtest_num_enc_t uj_data;
  TRY(ujson_deserialize_penetrationtest_num_enc_t(uj, &uj_data));

  return aes_sca_fvsr_key_batch_generate(uj_data);
}

status_t handle_aes_sca_fvsr_key_set(ujson_t *uj) {
  aes_sca_key_t uj_key_data;
  TRY(ujson_deserialize_aes_sca_key_t(uj, &uj_key_data));

  if (uj_key_data.key_length != kAesKeyLength) {
    return OUT_OF_RANGE();
  }
  memcpy(key_fixed, uj_key_data.key, uj_key_data.key_length);
  return OK_STATUS();
}

status_t handle_aes_sca_fvsr_key_start_batch_generate(ujson_t *uj) {
  aes_sca_cmd_t uj_data;
  TRY(ujson_deserialize_aes_sca_cmd_t(uj, &uj_data));

  static const uint8_t kPlaintextFixedStartFvsrKey[kAesTextLength] = {
      0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
      0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
  static const uint8_t kKeyFixedStartFvsrKey[kAesTextLength] = {
      0x81, 0x1E, 0x37, 0x31, 0xB0, 0x12, 0x0A, 0x78,
      0x42, 0x78, 0x1E, 0x22, 0xB2, 0x5C, 0xDD, 0xF9};
  static const uint8_t kPlaintextRandomStartFvsrKey[kAesTextLength] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc};
  static const uint8_t kKeyRandomStartFvsrKey[kAesTextLength] = {
      0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53,
      0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53};
  // Starting constants for fixed-vs-random data, DTR Section 5.1
  static const uint8_t kPlaintextFixedStartFvsrData[kAesTextLength] = {
      0xDA, 0x39, 0xA3, 0xEE, 0x5E, 0x6B, 0x4B, 0x0D,
      0x32, 0x55, 0xBF, 0xEF, 0x95, 0x60, 0x18, 0x90};
  static const uint8_t kPlaintextRandomStartFvsrData[kAesTextLength] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  static const uint8_t kKeyStartFvsrData[kAesTextLength] = {
      0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
      0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};

  // Initial state of the prng
  static const uint32_t kPrngInitialState = 0x99999999;

  // If fixed-vs-random key analysis
  if (uj_data.cmd == 1) {
    memcpy(plaintext_fixed, kPlaintextFixedStartFvsrKey, kAesTextLength);
    memcpy(key_fixed, kKeyFixedStartFvsrKey, kAesKeyLength);
    memcpy(plaintext_random, kPlaintextRandomStartFvsrKey, kAesTextLength);
    memcpy(key_random, kKeyRandomStartFvsrKey, kAesKeyLength);
  }

  // If fixed-vs-random data analysis
  if (uj_data.cmd == 2) {
    memcpy(plaintext_fixed, kPlaintextFixedStartFvsrData, kAesTextLength);
    memcpy(key_fixed, kKeyStartFvsrData, kAesKeyLength);
    memcpy(plaintext_random, kPlaintextRandomStartFvsrData, kAesTextLength);
  }

  pentest_seed_lfsr(kPrngInitialState, kPentestLfsrOrder);

  return OK_STATUS();
}

status_t handle_aes_pentest_init(ujson_t *uj) {
  // Read mode. FPGA or discrete.
  aes_sca_fpga_mode_t uj_data;
  TRY(ujson_deserialize_aes_sca_fpga_mode_t(uj, &uj_data));
  if (uj_data.fpga_mode == 0x01) {
    fpga_mode = true;
  }

  penetrationtest_cpuctrl_t uj_cpuctrl_data;
  TRY(ujson_deserialize_penetrationtest_cpuctrl_t(uj, &uj_cpuctrl_data));
  penetrationtest_sensor_config_t uj_sensor_data;
  TRY(ujson_deserialize_penetrationtest_sensor_config_t(uj, &uj_sensor_data));

  pentest_init(kPentestTriggerSourceAes,
               kPentestPeripheralIoDiv4 | kPentestPeripheralAes,
               uj_sensor_data.sensor_ctrl_enable,
               uj_sensor_data.sensor_ctrl_en_fatal);

  if (dif_aes_init(mmio_region_from_addr(TOP_EARLGREY_AES_BASE_ADDR), &aes) !=
      kDifOk) {
    return ABORTED();
  }

  if (dif_aes_reset(&aes) != kDifOk) {
    return ABORTED();
  }

  // Configure the CPU for the pentest.
  penetrationtest_device_info_t uj_output;
  TRY(pentest_configure_cpu(
      uj_cpuctrl_data.enable_icache, &uj_output.icache_en,
      uj_cpuctrl_data.enable_dummy_instr, &uj_output.dummy_instr_en,
      uj_cpuctrl_data.dummy_instr_count, uj_cpuctrl_data.enable_jittery_clock,
      uj_cpuctrl_data.enable_sram_readback, &uj_output.clock_jitter_locked,
      &uj_output.clock_jitter_en, &uj_output.sram_main_readback_locked,
      &uj_output.sram_ret_readback_locked, &uj_output.sram_main_readback_en,
      &uj_output.sram_ret_readback_en));

  // Read device ID and return to host.
  TRY(pentest_read_device_id(uj_output.device_id));
  RESP_OK(ujson_serialize_penetrationtest_device_info_t, uj, &uj_output);

  // Read different SKU config fields and return to host.
  TRY(pentest_send_sku_config(uj));

  return OK_STATUS();
}

status_t handle_aes_sca_key_set(ujson_t *uj) {
  aes_sca_key_t uj_key_data;
  TRY(ujson_deserialize_aes_sca_key_t(uj, &uj_key_data));

  memcpy(key_fixed, uj_key_data.key, uj_key_data.key_length);
  block_ctr = 0;
  TRY(aes_key_mask_and_config(key_fixed, uj_key_data.key_length));
  return OK_STATUS();
}

status_t handle_aes_pentest_seed_lfsr(ujson_t *uj) {
  aes_sca_lfsr_t uj_lfsr_data;
  TRY(ujson_deserialize_aes_sca_lfsr_t(uj, &uj_lfsr_data));

  uint32_t seed_local = read_32(uj_lfsr_data.seed);
  if (seed_local == 0) {
    // disable masking
    transaction.force_masks = true;
  } else {
    // enable masking
    transaction.force_masks = false;
  }
  pentest_seed_lfsr(seed_local, kPentestLfsrMasking);

#ifndef OPENTITAN_IS_ENGLISHBREAKFAST
  if (transaction.force_masks) {
    LOG_INFO("Disabling masks.");
    const dif_csrng_t csrng = {
        .base_addr = mmio_region_from_addr(TOP_EARLGREY_CSRNG_BASE_ADDR)};
    const dif_edn_t edn0 = {
        .base_addr = mmio_region_from_addr(TOP_EARLGREY_EDN0_BASE_ADDR)};

    status_t res = aes_testutils_masking_prng_zero_output_seed(&csrng, &edn0);
    if (res.value != 0) {
      return ABORTED();
    }
    // Load the magic seed into the PRNG. After this, the PRNG outputs
    // an all-zero vector.
    TRY(aes_sca_load_fixed_seed());
  }
#endif

  TRY(dif_aes_trigger(&aes, kDifAesTriggerDataOutClear));
  AES_TESTUTILS_WAIT_FOR_STATUS(&aes, kDifAesStatusIdle, true, kTestTimeout);
  return OK_STATUS();
}

status_t handle_aes_pentest_seed_lfsr_order(ujson_t *uj) {
  aes_sca_lfsr_t uj_lfsr_data;
  TRY(ujson_deserialize_aes_sca_lfsr_t(uj, &uj_lfsr_data));

  uint32_t seed_local = read_32(uj_lfsr_data.seed);
  pentest_seed_lfsr(seed_local, kPentestLfsrOrder);

  return OK_STATUS();
}

status_t handle_aes_sca_single_encrypt(ujson_t *uj) {
  aes_sca_text_t uj_data;
  TRY(ujson_deserialize_aes_sca_text_t(uj, &uj_data));
  if (uj_data.text_length != kAesTextLength) {
    return OUT_OF_RANGE();
  }

  block_ctr++;
  // Rewrite the key to reset the internal block counter. Otherwise, the AES
  // peripheral might trigger the reseeding of the internal masking PRNG which
  // disturbs SCA measurements.
  if (block_ctr > kBlockCtrMax) {
    TRY(aes_key_mask_and_config(key_fixed, kAesKeyLength));
    block_ctr = 1;
  }

  TRY(aes_encrypt(uj_data.text, uj_data.text_length));

  TRY(aes_send_ciphertext(false, uj));
  return OK_STATUS();
}

status_t handle_aes_sca(ujson_t *uj) {
  aes_sca_subcommand_t cmd;
  TRY(ujson_deserialize_aes_sca_subcommand_t(uj, &cmd));
  switch (cmd) {
    case kAesScaSubcommandBatchAlternativeEncrypt:
      return handle_aes_sca_batch_alternative_encrypt(uj);
    case kAesScaSubcommandBatchEncrypt:
      return handle_aes_sca_batch_encrypt(uj);
    case kAesScaSubcommandBatchEncryptRandom:
      return handle_aes_sca_batch_encrypt_random(uj);
    case kAesScaSubcommandBatchPlaintextSet:
      return handle_aes_sca_batch_plaintext_set(uj);
    case kAesScaSubcommandFvsrDataBatchEncrypt:
      return handle_aes_sca_fvsr_data_batch_encrypt(uj);
    case kAesScaSubcommandFvsrKeyBatchEncrypt:
      return handle_aes_sca_fvsr_key_batch_encrypt(uj);
    case kAesScaSubcommandFvsrKeyBatchGenerate:
      return handle_aes_sca_fvsr_key_batch_generate(uj);
    case kAesScaSubcommandFvsrKeySet:
      return handle_aes_sca_fvsr_key_set(uj);
    case kAesScaSubcommandFvsrKeyStartBatchGenerate:
      return handle_aes_sca_fvsr_key_start_batch_generate(uj);
    case kAesScaSubcommandInit:
      return handle_aes_pentest_init(uj);
    case kAesScaSubcommandKeySet:
      return handle_aes_sca_key_set(uj);
    case kAesScaSubcommandSeedLfsr:
      return handle_aes_pentest_seed_lfsr(uj);
    case kAesScaSubcommandSeedLfsrOrder:
      return handle_aes_pentest_seed_lfsr_order(uj);
    case kAesScaSubcommandSingleEncrypt:
      return handle_aes_sca_single_encrypt(uj);
    default:
      LOG_ERROR("Unrecognized AES SCA subcommand: %d", cmd);
      return INVALID_ARGUMENT();
  }
  return OK_STATUS();
}
