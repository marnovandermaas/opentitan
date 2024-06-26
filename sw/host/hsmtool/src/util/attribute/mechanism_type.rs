// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// This file was autogenerated by `//sw/host/hsmtool/scripts/pkcs11_consts.py`.
// Do not edit.'

use cryptoki_sys::*;
use num_enum::{FromPrimitive, IntoPrimitive};
use std::convert::TryFrom;

use crate::util::attribute::{AttrData, AttributeError};

#[derive(
    Clone,
    Copy,
    Debug,
    PartialEq,
    Eq,
    Hash,
    IntoPrimitive,
    FromPrimitive,
    serde::Serialize,
    serde::Deserialize,
)]
#[repr(u64)]
pub enum MechanismType {
    #[serde(rename = "CKM_RSA_PKCS_KEY_PAIR_GEN")]
    RsaPkcsKeyPairGen = CKM_RSA_PKCS_KEY_PAIR_GEN,
    #[serde(rename = "CKM_RSA_PKCS")]
    RsaPkcs = CKM_RSA_PKCS,
    #[serde(rename = "CKM_RSA_9796")]
    Rsa9796 = CKM_RSA_9796,
    #[serde(rename = "CKM_RSA_X_509")]
    RsaX509 = CKM_RSA_X_509,
    #[serde(rename = "CKM_MD2_RSA_PKCS")]
    Md2RsaPkcs = CKM_MD2_RSA_PKCS,
    #[serde(rename = "CKM_MD5_RSA_PKCS")]
    Md5RsaPkcs = CKM_MD5_RSA_PKCS,
    #[serde(rename = "CKM_SHA1_RSA_PKCS")]
    Sha1RsaPkcs = CKM_SHA1_RSA_PKCS,
    #[serde(rename = "CKM_RIPEMD128_RSA_PKCS")]
    Ripemd128RsaPkcs = CKM_RIPEMD128_RSA_PKCS,
    #[serde(rename = "CKM_RIPEMD160_RSA_PKCS")]
    Ripemd160RsaPkcs = CKM_RIPEMD160_RSA_PKCS,
    #[serde(rename = "CKM_RSA_PKCS_OAEP")]
    RsaPkcsOaep = CKM_RSA_PKCS_OAEP,
    #[serde(rename = "CKM_RSA_X9_31_KEY_PAIR_GEN")]
    RsaX931KeyPairGen = CKM_RSA_X9_31_KEY_PAIR_GEN,
    #[serde(rename = "CKM_RSA_X9_31")]
    RsaX931 = CKM_RSA_X9_31,
    #[serde(rename = "CKM_SHA1_RSA_X9_31")]
    Sha1RsaX931 = CKM_SHA1_RSA_X9_31,
    #[serde(rename = "CKM_RSA_PKCS_PSS")]
    RsaPkcsPss = CKM_RSA_PKCS_PSS,
    #[serde(rename = "CKM_SHA1_RSA_PKCS_PSS")]
    Sha1RsaPkcsPss = CKM_SHA1_RSA_PKCS_PSS,
    #[serde(rename = "CKM_DSA_KEY_PAIR_GEN")]
    DsaKeyPairGen = CKM_DSA_KEY_PAIR_GEN,
    #[serde(rename = "CKM_DSA")]
    Dsa = CKM_DSA,
    #[serde(rename = "CKM_DSA_SHA1")]
    DsaSha1 = CKM_DSA_SHA1,
    #[serde(rename = "CKM_DSA_SHA224")]
    DsaSha224 = CKM_DSA_SHA224,
    #[serde(rename = "CKM_DSA_SHA256")]
    DsaSha256 = CKM_DSA_SHA256,
    #[serde(rename = "CKM_DSA_SHA384")]
    DsaSha384 = CKM_DSA_SHA384,
    #[serde(rename = "CKM_DSA_SHA512")]
    DsaSha512 = CKM_DSA_SHA512,
    #[serde(rename = "CKM_DH_PKCS_KEY_PAIR_GEN")]
    DhPkcsKeyPairGen = CKM_DH_PKCS_KEY_PAIR_GEN,
    #[serde(rename = "CKM_DH_PKCS_DERIVE")]
    DhPkcsDerive = CKM_DH_PKCS_DERIVE,
    #[serde(rename = "CKM_X9_42_DH_KEY_PAIR_GEN")]
    X942DhKeyPairGen = CKM_X9_42_DH_KEY_PAIR_GEN,
    #[serde(rename = "CKM_X9_42_DH_DERIVE")]
    X942DhDerive = CKM_X9_42_DH_DERIVE,
    #[serde(rename = "CKM_X9_42_DH_HYBRID_DERIVE")]
    X942DhHybridDerive = CKM_X9_42_DH_HYBRID_DERIVE,
    #[serde(rename = "CKM_X9_42_MQV_DERIVE")]
    X942MqvDerive = CKM_X9_42_MQV_DERIVE,
    #[serde(rename = "CKM_SHA256_RSA_PKCS")]
    Sha256RsaPkcs = CKM_SHA256_RSA_PKCS,
    #[serde(rename = "CKM_SHA384_RSA_PKCS")]
    Sha384RsaPkcs = CKM_SHA384_RSA_PKCS,
    #[serde(rename = "CKM_SHA512_RSA_PKCS")]
    Sha512RsaPkcs = CKM_SHA512_RSA_PKCS,
    #[serde(rename = "CKM_SHA256_RSA_PKCS_PSS")]
    Sha256RsaPkcsPss = CKM_SHA256_RSA_PKCS_PSS,
    #[serde(rename = "CKM_SHA384_RSA_PKCS_PSS")]
    Sha384RsaPkcsPss = CKM_SHA384_RSA_PKCS_PSS,
    #[serde(rename = "CKM_SHA512_RSA_PKCS_PSS")]
    Sha512RsaPkcsPss = CKM_SHA512_RSA_PKCS_PSS,
    #[serde(rename = "CKM_SHA512_224")]
    Sha512224 = CKM_SHA512_224,
    #[serde(rename = "CKM_SHA512_224_HMAC")]
    Sha512224Hmac = CKM_SHA512_224_HMAC,
    #[serde(rename = "CKM_SHA512_224_HMAC_GENERAL")]
    Sha512224HmacGeneral = CKM_SHA512_224_HMAC_GENERAL,
    #[serde(rename = "CKM_SHA512_224_KEY_DERIVATION")]
    Sha512224KeyDerivation = CKM_SHA512_224_KEY_DERIVATION,
    #[serde(rename = "CKM_SHA512_256")]
    Sha512256 = CKM_SHA512_256,
    #[serde(rename = "CKM_SHA512_256_HMAC")]
    Sha512256Hmac = CKM_SHA512_256_HMAC,
    #[serde(rename = "CKM_SHA512_256_HMAC_GENERAL")]
    Sha512256HmacGeneral = CKM_SHA512_256_HMAC_GENERAL,
    #[serde(rename = "CKM_SHA512_256_KEY_DERIVATION")]
    Sha512256KeyDerivation = CKM_SHA512_256_KEY_DERIVATION,
    #[serde(rename = "CKM_SHA512_T")]
    Sha512T = CKM_SHA512_T,
    #[serde(rename = "CKM_SHA512_T_HMAC")]
    Sha512THmac = CKM_SHA512_T_HMAC,
    #[serde(rename = "CKM_SHA512_T_HMAC_GENERAL")]
    Sha512THmacGeneral = CKM_SHA512_T_HMAC_GENERAL,
    #[serde(rename = "CKM_SHA512_T_KEY_DERIVATION")]
    Sha512TKeyDerivation = CKM_SHA512_T_KEY_DERIVATION,
    #[serde(rename = "CKM_RC2_KEY_GEN")]
    Rc2KeyGen = CKM_RC2_KEY_GEN,
    #[serde(rename = "CKM_RC2_ECB")]
    Rc2Ecb = CKM_RC2_ECB,
    #[serde(rename = "CKM_RC2_CBC")]
    Rc2Cbc = CKM_RC2_CBC,
    #[serde(rename = "CKM_RC2_MAC")]
    Rc2Mac = CKM_RC2_MAC,
    #[serde(rename = "CKM_RC2_MAC_GENERAL")]
    Rc2MacGeneral = CKM_RC2_MAC_GENERAL,
    #[serde(rename = "CKM_RC2_CBC_PAD")]
    Rc2CbcPad = CKM_RC2_CBC_PAD,
    #[serde(rename = "CKM_RC4_KEY_GEN")]
    Rc4KeyGen = CKM_RC4_KEY_GEN,
    #[serde(rename = "CKM_RC4")]
    Rc4 = CKM_RC4,
    #[serde(rename = "CKM_DES_KEY_GEN")]
    DesKeyGen = CKM_DES_KEY_GEN,
    #[serde(rename = "CKM_DES_ECB")]
    DesEcb = CKM_DES_ECB,
    #[serde(rename = "CKM_DES_CBC")]
    DesCbc = CKM_DES_CBC,
    #[serde(rename = "CKM_DES_MAC")]
    DesMac = CKM_DES_MAC,
    #[serde(rename = "CKM_DES_MAC_GENERAL")]
    DesMacGeneral = CKM_DES_MAC_GENERAL,
    #[serde(rename = "CKM_DES_CBC_PAD")]
    DesCbcPad = CKM_DES_CBC_PAD,
    #[serde(rename = "CKM_DES2_KEY_GEN")]
    Des2KeyGen = CKM_DES2_KEY_GEN,
    #[serde(rename = "CKM_DES3_KEY_GEN")]
    Des3KeyGen = CKM_DES3_KEY_GEN,
    #[serde(rename = "CKM_DES3_ECB")]
    Des3Ecb = CKM_DES3_ECB,
    #[serde(rename = "CKM_DES3_CBC")]
    Des3Cbc = CKM_DES3_CBC,
    #[serde(rename = "CKM_DES3_MAC")]
    Des3Mac = CKM_DES3_MAC,
    #[serde(rename = "CKM_DES3_MAC_GENERAL")]
    Des3MacGeneral = CKM_DES3_MAC_GENERAL,
    #[serde(rename = "CKM_DES3_CBC_PAD")]
    Des3CbcPad = CKM_DES3_CBC_PAD,
    #[serde(rename = "CKM_DES3_CMAC_GENERAL")]
    Des3CmacGeneral = CKM_DES3_CMAC_GENERAL,
    #[serde(rename = "CKM_DES3_CMAC")]
    Des3Cmac = CKM_DES3_CMAC,
    #[serde(rename = "CKM_CDMF_KEY_GEN")]
    CdmfKeyGen = CKM_CDMF_KEY_GEN,
    #[serde(rename = "CKM_CDMF_ECB")]
    CdmfEcb = CKM_CDMF_ECB,
    #[serde(rename = "CKM_CDMF_CBC")]
    CdmfCbc = CKM_CDMF_CBC,
    #[serde(rename = "CKM_CDMF_MAC")]
    CdmfMac = CKM_CDMF_MAC,
    #[serde(rename = "CKM_CDMF_MAC_GENERAL")]
    CdmfMacGeneral = CKM_CDMF_MAC_GENERAL,
    #[serde(rename = "CKM_CDMF_CBC_PAD")]
    CdmfCbcPad = CKM_CDMF_CBC_PAD,
    #[serde(rename = "CKM_DES_OFB64")]
    DesOfb64 = CKM_DES_OFB64,
    #[serde(rename = "CKM_DES_OFB8")]
    DesOfb8 = CKM_DES_OFB8,
    #[serde(rename = "CKM_DES_CFB64")]
    DesCfb64 = CKM_DES_CFB64,
    #[serde(rename = "CKM_DES_CFB8")]
    DesCfb8 = CKM_DES_CFB8,
    #[serde(rename = "CKM_MD2")]
    Md2 = CKM_MD2,
    #[serde(rename = "CKM_MD2_HMAC")]
    Md2Hmac = CKM_MD2_HMAC,
    #[serde(rename = "CKM_MD2_HMAC_GENERAL")]
    Md2HmacGeneral = CKM_MD2_HMAC_GENERAL,
    #[serde(rename = "CKM_MD5")]
    Md5 = CKM_MD5,
    #[serde(rename = "CKM_MD5_HMAC")]
    Md5Hmac = CKM_MD5_HMAC,
    #[serde(rename = "CKM_MD5_HMAC_GENERAL")]
    Md5HmacGeneral = CKM_MD5_HMAC_GENERAL,
    #[serde(rename = "CKM_SHA_1")]
    Sha1 = CKM_SHA_1,
    #[serde(rename = "CKM_SHA_1_HMAC")]
    Sha1Hmac = CKM_SHA_1_HMAC,
    #[serde(rename = "CKM_SHA_1_HMAC_GENERAL")]
    Sha1HmacGeneral = CKM_SHA_1_HMAC_GENERAL,
    #[serde(rename = "CKM_RIPEMD128")]
    Ripemd128 = CKM_RIPEMD128,
    #[serde(rename = "CKM_RIPEMD128_HMAC")]
    Ripemd128Hmac = CKM_RIPEMD128_HMAC,
    #[serde(rename = "CKM_RIPEMD128_HMAC_GENERAL")]
    Ripemd128HmacGeneral = CKM_RIPEMD128_HMAC_GENERAL,
    #[serde(rename = "CKM_RIPEMD160")]
    Ripemd160 = CKM_RIPEMD160,
    #[serde(rename = "CKM_RIPEMD160_HMAC")]
    Ripemd160Hmac = CKM_RIPEMD160_HMAC,
    #[serde(rename = "CKM_RIPEMD160_HMAC_GENERAL")]
    Ripemd160HmacGeneral = CKM_RIPEMD160_HMAC_GENERAL,
    #[serde(rename = "CKM_SHA256")]
    Sha256 = CKM_SHA256,
    #[serde(rename = "CKM_SHA256_HMAC")]
    Sha256Hmac = CKM_SHA256_HMAC,
    #[serde(rename = "CKM_SHA256_HMAC_GENERAL")]
    Sha256HmacGeneral = CKM_SHA256_HMAC_GENERAL,
    #[serde(rename = "CKM_SHA384")]
    Sha384 = CKM_SHA384,
    #[serde(rename = "CKM_SHA384_HMAC")]
    Sha384Hmac = CKM_SHA384_HMAC,
    #[serde(rename = "CKM_SHA384_HMAC_GENERAL")]
    Sha384HmacGeneral = CKM_SHA384_HMAC_GENERAL,
    #[serde(rename = "CKM_SHA512")]
    Sha512 = CKM_SHA512,
    #[serde(rename = "CKM_SHA512_HMAC")]
    Sha512Hmac = CKM_SHA512_HMAC,
    #[serde(rename = "CKM_SHA512_HMAC_GENERAL")]
    Sha512HmacGeneral = CKM_SHA512_HMAC_GENERAL,
    #[serde(rename = "CKM_SECURID_KEY_GEN")]
    SecuridKeyGen = CKM_SECURID_KEY_GEN,
    #[serde(rename = "CKM_SECURID")]
    Securid = CKM_SECURID,
    #[serde(rename = "CKM_HOTP_KEY_GEN")]
    HotpKeyGen = CKM_HOTP_KEY_GEN,
    #[serde(rename = "CKM_HOTP")]
    Hotp = CKM_HOTP,
    #[serde(rename = "CKM_ACTI")]
    Acti = CKM_ACTI,
    #[serde(rename = "CKM_ACTI_KEY_GEN")]
    ActiKeyGen = CKM_ACTI_KEY_GEN,
    #[serde(rename = "CKM_CAST_KEY_GEN")]
    CastKeyGen = CKM_CAST_KEY_GEN,
    #[serde(rename = "CKM_CAST_ECB")]
    CastEcb = CKM_CAST_ECB,
    #[serde(rename = "CKM_CAST_CBC")]
    CastCbc = CKM_CAST_CBC,
    #[serde(rename = "CKM_CAST_MAC")]
    CastMac = CKM_CAST_MAC,
    #[serde(rename = "CKM_CAST_MAC_GENERAL")]
    CastMacGeneral = CKM_CAST_MAC_GENERAL,
    #[serde(rename = "CKM_CAST_CBC_PAD")]
    CastCbcPad = CKM_CAST_CBC_PAD,
    #[serde(rename = "CKM_CAST3_KEY_GEN")]
    Cast3KeyGen = CKM_CAST3_KEY_GEN,
    #[serde(rename = "CKM_CAST3_ECB")]
    Cast3Ecb = CKM_CAST3_ECB,
    #[serde(rename = "CKM_CAST3_CBC")]
    Cast3Cbc = CKM_CAST3_CBC,
    #[serde(rename = "CKM_CAST3_MAC")]
    Cast3Mac = CKM_CAST3_MAC,
    #[serde(rename = "CKM_CAST3_MAC_GENERAL")]
    Cast3MacGeneral = CKM_CAST3_MAC_GENERAL,
    #[serde(rename = "CKM_CAST3_CBC_PAD")]
    Cast3CbcPad = CKM_CAST3_CBC_PAD,
    #[serde(rename = "CKM_CAST128_KEY_GEN")]
    Cast128KeyGen = CKM_CAST128_KEY_GEN,
    #[serde(rename = "CKM_CAST128_ECB")]
    Cast128Ecb = CKM_CAST128_ECB,
    #[serde(rename = "CKM_CAST128_CBC")]
    Cast128Cbc = CKM_CAST128_CBC,
    #[serde(rename = "CKM_CAST128_MAC")]
    Cast128Mac = CKM_CAST128_MAC,
    #[serde(rename = "CKM_CAST128_MAC_GENERAL")]
    Cast128MacGeneral = CKM_CAST128_MAC_GENERAL,
    #[serde(rename = "CKM_CAST128_CBC_PAD")]
    Cast128CbcPad = CKM_CAST128_CBC_PAD,
    #[serde(rename = "CKM_RC5_KEY_GEN")]
    Rc5KeyGen = CKM_RC5_KEY_GEN,
    #[serde(rename = "CKM_RC5_ECB")]
    Rc5Ecb = CKM_RC5_ECB,
    #[serde(rename = "CKM_RC5_CBC")]
    Rc5Cbc = CKM_RC5_CBC,
    #[serde(rename = "CKM_RC5_MAC")]
    Rc5Mac = CKM_RC5_MAC,
    #[serde(rename = "CKM_RC5_MAC_GENERAL")]
    Rc5MacGeneral = CKM_RC5_MAC_GENERAL,
    #[serde(rename = "CKM_RC5_CBC_PAD")]
    Rc5CbcPad = CKM_RC5_CBC_PAD,
    #[serde(rename = "CKM_IDEA_KEY_GEN")]
    IdeaKeyGen = CKM_IDEA_KEY_GEN,
    #[serde(rename = "CKM_IDEA_ECB")]
    IdeaEcb = CKM_IDEA_ECB,
    #[serde(rename = "CKM_IDEA_CBC")]
    IdeaCbc = CKM_IDEA_CBC,
    #[serde(rename = "CKM_IDEA_MAC")]
    IdeaMac = CKM_IDEA_MAC,
    #[serde(rename = "CKM_IDEA_MAC_GENERAL")]
    IdeaMacGeneral = CKM_IDEA_MAC_GENERAL,
    #[serde(rename = "CKM_IDEA_CBC_PAD")]
    IdeaCbcPad = CKM_IDEA_CBC_PAD,
    #[serde(rename = "CKM_GENERIC_SECRET_KEY_GEN")]
    GenericSecretKeyGen = CKM_GENERIC_SECRET_KEY_GEN,
    #[serde(rename = "CKM_CONCATENATE_BASE_AND_KEY")]
    ConcatenateBaseAndKey = CKM_CONCATENATE_BASE_AND_KEY,
    #[serde(rename = "CKM_CONCATENATE_BASE_AND_DATA")]
    ConcatenateBaseAndData = CKM_CONCATENATE_BASE_AND_DATA,
    #[serde(rename = "CKM_CONCATENATE_DATA_AND_BASE")]
    ConcatenateDataAndBase = CKM_CONCATENATE_DATA_AND_BASE,
    #[serde(rename = "CKM_XOR_BASE_AND_DATA")]
    XorBaseAndData = CKM_XOR_BASE_AND_DATA,
    #[serde(rename = "CKM_EXTRACT_KEY_FROM_KEY")]
    ExtractKeyFromKey = CKM_EXTRACT_KEY_FROM_KEY,
    #[serde(rename = "CKM_SSL3_PRE_MASTER_KEY_GEN")]
    Ssl3PreMasterKeyGen = CKM_SSL3_PRE_MASTER_KEY_GEN,
    #[serde(rename = "CKM_SSL3_MASTER_KEY_DERIVE")]
    Ssl3MasterKeyDerive = CKM_SSL3_MASTER_KEY_DERIVE,
    #[serde(rename = "CKM_SSL3_KEY_AND_MAC_DERIVE")]
    Ssl3KeyAndMacDerive = CKM_SSL3_KEY_AND_MAC_DERIVE,
    #[serde(rename = "CKM_SSL3_MASTER_KEY_DERIVE_DH")]
    Ssl3MasterKeyDeriveDh = CKM_SSL3_MASTER_KEY_DERIVE_DH,
    #[serde(rename = "CKM_TLS_PRE_MASTER_KEY_GEN")]
    TlsPreMasterKeyGen = CKM_TLS_PRE_MASTER_KEY_GEN,
    #[serde(rename = "CKM_TLS_MASTER_KEY_DERIVE")]
    TlsMasterKeyDerive = CKM_TLS_MASTER_KEY_DERIVE,
    #[serde(rename = "CKM_TLS_KEY_AND_MAC_DERIVE")]
    TlsKeyAndMacDerive = CKM_TLS_KEY_AND_MAC_DERIVE,
    #[serde(rename = "CKM_TLS_MASTER_KEY_DERIVE_DH")]
    TlsMasterKeyDeriveDh = CKM_TLS_MASTER_KEY_DERIVE_DH,
    #[serde(rename = "CKM_TLS_PRF")]
    TlsPrf = CKM_TLS_PRF,
    #[serde(rename = "CKM_SSL3_MD5_MAC")]
    Ssl3Md5Mac = CKM_SSL3_MD5_MAC,
    #[serde(rename = "CKM_SSL3_SHA1_MAC")]
    Ssl3Sha1Mac = CKM_SSL3_SHA1_MAC,
    #[serde(rename = "CKM_MD5_KEY_DERIVATION")]
    Md5KeyDerivation = CKM_MD5_KEY_DERIVATION,
    #[serde(rename = "CKM_MD2_KEY_DERIVATION")]
    Md2KeyDerivation = CKM_MD2_KEY_DERIVATION,
    #[serde(rename = "CKM_SHA1_KEY_DERIVATION")]
    Sha1KeyDerivation = CKM_SHA1_KEY_DERIVATION,
    #[serde(rename = "CKM_SHA256_KEY_DERIVATION")]
    Sha256KeyDerivation = CKM_SHA256_KEY_DERIVATION,
    #[serde(rename = "CKM_SHA384_KEY_DERIVATION")]
    Sha384KeyDerivation = CKM_SHA384_KEY_DERIVATION,
    #[serde(rename = "CKM_SHA512_KEY_DERIVATION")]
    Sha512KeyDerivation = CKM_SHA512_KEY_DERIVATION,
    #[serde(rename = "CKM_PBE_MD2_DES_CBC")]
    PbeMd2DesCbc = CKM_PBE_MD2_DES_CBC,
    #[serde(rename = "CKM_PBE_MD5_DES_CBC")]
    PbeMd5DesCbc = CKM_PBE_MD5_DES_CBC,
    #[serde(rename = "CKM_PBE_MD5_CAST_CBC")]
    PbeMd5CastCbc = CKM_PBE_MD5_CAST_CBC,
    #[serde(rename = "CKM_PBE_MD5_CAST3_CBC")]
    PbeMd5Cast3Cbc = CKM_PBE_MD5_CAST3_CBC,
    #[serde(rename = "CKM_PBE_MD5_CAST128_CBC")]
    PbeMd5Cast128Cbc = CKM_PBE_MD5_CAST128_CBC,
    #[serde(rename = "CKM_PBE_SHA1_CAST128_CBC")]
    PbeSha1Cast128Cbc = CKM_PBE_SHA1_CAST128_CBC,
    #[serde(rename = "CKM_PBE_SHA1_RC4_128")]
    PbeSha1Rc4128 = CKM_PBE_SHA1_RC4_128,
    #[serde(rename = "CKM_PBE_SHA1_RC4_40")]
    PbeSha1Rc440 = CKM_PBE_SHA1_RC4_40,
    #[serde(rename = "CKM_PBE_SHA1_DES3_EDE_CBC")]
    PbeSha1Des3EdeCbc = CKM_PBE_SHA1_DES3_EDE_CBC,
    #[serde(rename = "CKM_PBE_SHA1_DES2_EDE_CBC")]
    PbeSha1Des2EdeCbc = CKM_PBE_SHA1_DES2_EDE_CBC,
    #[serde(rename = "CKM_PBE_SHA1_RC2_128_CBC")]
    PbeSha1Rc2128Cbc = CKM_PBE_SHA1_RC2_128_CBC,
    #[serde(rename = "CKM_PBE_SHA1_RC2_40_CBC")]
    PbeSha1Rc240Cbc = CKM_PBE_SHA1_RC2_40_CBC,
    #[serde(rename = "CKM_PKCS5_PBKD2")]
    Pkcs5Pbkd2 = CKM_PKCS5_PBKD2,
    #[serde(rename = "CKM_PBA_SHA1_WITH_SHA1_HMAC")]
    PbaSha1WithSha1Hmac = CKM_PBA_SHA1_WITH_SHA1_HMAC,
    #[serde(rename = "CKM_WTLS_PRE_MASTER_KEY_GEN")]
    WtlsPreMasterKeyGen = CKM_WTLS_PRE_MASTER_KEY_GEN,
    #[serde(rename = "CKM_WTLS_MASTER_KEY_DERIVE")]
    WtlsMasterKeyDerive = CKM_WTLS_MASTER_KEY_DERIVE,
    #[serde(rename = "CKM_WTLS_MASTER_KEY_DERIVE_DH_ECC")]
    WtlsMasterKeyDeriveDhEcc = CKM_WTLS_MASTER_KEY_DERIVE_DH_ECC,
    #[serde(rename = "CKM_WTLS_PRF")]
    WtlsPrf = CKM_WTLS_PRF,
    #[serde(rename = "CKM_WTLS_SERVER_KEY_AND_MAC_DERIVE")]
    WtlsServerKeyAndMacDerive = CKM_WTLS_SERVER_KEY_AND_MAC_DERIVE,
    #[serde(rename = "CKM_WTLS_CLIENT_KEY_AND_MAC_DERIVE")]
    WtlsClientKeyAndMacDerive = CKM_WTLS_CLIENT_KEY_AND_MAC_DERIVE,
    #[serde(rename = "CKM_TLS10_MAC_SERVER")]
    Tls10MacServer = CKM_TLS10_MAC_SERVER,
    #[serde(rename = "CKM_TLS10_MAC_CLIENT")]
    Tls10MacClient = CKM_TLS10_MAC_CLIENT,
    #[serde(rename = "CKM_TLS12_MAC")]
    Tls12Mac = CKM_TLS12_MAC,
    #[serde(rename = "CKM_TLS12_KDF")]
    Tls12Kdf = CKM_TLS12_KDF,
    #[serde(rename = "CKM_TLS12_MASTER_KEY_DERIVE")]
    Tls12MasterKeyDerive = CKM_TLS12_MASTER_KEY_DERIVE,
    #[serde(rename = "CKM_TLS12_KEY_AND_MAC_DERIVE")]
    Tls12KeyAndMacDerive = CKM_TLS12_KEY_AND_MAC_DERIVE,
    #[serde(rename = "CKM_TLS12_MASTER_KEY_DERIVE_DH")]
    Tls12MasterKeyDeriveDh = CKM_TLS12_MASTER_KEY_DERIVE_DH,
    #[serde(rename = "CKM_TLS12_KEY_SAFE_DERIVE")]
    Tls12KeySafeDerive = CKM_TLS12_KEY_SAFE_DERIVE,
    #[serde(rename = "CKM_TLS_MAC")]
    TlsMac = CKM_TLS_MAC,
    #[serde(rename = "CKM_TLS_KDF")]
    TlsKdf = CKM_TLS_KDF,
    #[serde(rename = "CKM_KEY_WRAP_LYNKS")]
    KeyWrapLynks = CKM_KEY_WRAP_LYNKS,
    #[serde(rename = "CKM_KEY_WRAP_SET_OAEP")]
    KeyWrapSetOaep = CKM_KEY_WRAP_SET_OAEP,
    #[serde(rename = "CKM_CMS_SIG")]
    CmsSig = CKM_CMS_SIG,
    #[serde(rename = "CKM_KIP_DERIVE")]
    KipDerive = CKM_KIP_DERIVE,
    #[serde(rename = "CKM_KIP_WRAP")]
    KipWrap = CKM_KIP_WRAP,
    #[serde(rename = "CKM_KIP_MAC")]
    KipMac = CKM_KIP_MAC,
    #[serde(rename = "CKM_CAMELLIA_KEY_GEN")]
    CamelliaKeyGen = CKM_CAMELLIA_KEY_GEN,
    #[serde(rename = "CKM_CAMELLIA_CTR")]
    CamelliaCtr = CKM_CAMELLIA_CTR,
    #[serde(rename = "CKM_ARIA_KEY_GEN")]
    AriaKeyGen = CKM_ARIA_KEY_GEN,
    #[serde(rename = "CKM_ARIA_ECB")]
    AriaEcb = CKM_ARIA_ECB,
    #[serde(rename = "CKM_ARIA_CBC")]
    AriaCbc = CKM_ARIA_CBC,
    #[serde(rename = "CKM_ARIA_MAC")]
    AriaMac = CKM_ARIA_MAC,
    #[serde(rename = "CKM_ARIA_MAC_GENERAL")]
    AriaMacGeneral = CKM_ARIA_MAC_GENERAL,
    #[serde(rename = "CKM_ARIA_CBC_PAD")]
    AriaCbcPad = CKM_ARIA_CBC_PAD,
    #[serde(rename = "CKM_ARIA_ECB_ENCRYPT_DATA")]
    AriaEcbEncryptData = CKM_ARIA_ECB_ENCRYPT_DATA,
    #[serde(rename = "CKM_ARIA_CBC_ENCRYPT_DATA")]
    AriaCbcEncryptData = CKM_ARIA_CBC_ENCRYPT_DATA,
    #[serde(rename = "CKM_SEED_KEY_GEN")]
    SeedKeyGen = CKM_SEED_KEY_GEN,
    #[serde(rename = "CKM_SEED_ECB")]
    SeedEcb = CKM_SEED_ECB,
    #[serde(rename = "CKM_SEED_CBC")]
    SeedCbc = CKM_SEED_CBC,
    #[serde(rename = "CKM_SEED_MAC")]
    SeedMac = CKM_SEED_MAC,
    #[serde(rename = "CKM_SEED_MAC_GENERAL")]
    SeedMacGeneral = CKM_SEED_MAC_GENERAL,
    #[serde(rename = "CKM_SEED_CBC_PAD")]
    SeedCbcPad = CKM_SEED_CBC_PAD,
    #[serde(rename = "CKM_SEED_ECB_ENCRYPT_DATA")]
    SeedEcbEncryptData = CKM_SEED_ECB_ENCRYPT_DATA,
    #[serde(rename = "CKM_SEED_CBC_ENCRYPT_DATA")]
    SeedCbcEncryptData = CKM_SEED_CBC_ENCRYPT_DATA,
    #[serde(rename = "CKM_SKIPJACK_KEY_GEN")]
    SkipjackKeyGen = CKM_SKIPJACK_KEY_GEN,
    #[serde(rename = "CKM_SKIPJACK_ECB64")]
    SkipjackEcb64 = CKM_SKIPJACK_ECB64,
    #[serde(rename = "CKM_SKIPJACK_CBC64")]
    SkipjackCbc64 = CKM_SKIPJACK_CBC64,
    #[serde(rename = "CKM_SKIPJACK_OFB64")]
    SkipjackOfb64 = CKM_SKIPJACK_OFB64,
    #[serde(rename = "CKM_SKIPJACK_CFB64")]
    SkipjackCfb64 = CKM_SKIPJACK_CFB64,
    #[serde(rename = "CKM_SKIPJACK_CFB32")]
    SkipjackCfb32 = CKM_SKIPJACK_CFB32,
    #[serde(rename = "CKM_SKIPJACK_CFB16")]
    SkipjackCfb16 = CKM_SKIPJACK_CFB16,
    #[serde(rename = "CKM_SKIPJACK_CFB8")]
    SkipjackCfb8 = CKM_SKIPJACK_CFB8,
    #[serde(rename = "CKM_SKIPJACK_WRAP")]
    SkipjackWrap = CKM_SKIPJACK_WRAP,
    #[serde(rename = "CKM_SKIPJACK_PRIVATE_WRAP")]
    SkipjackPrivateWrap = CKM_SKIPJACK_PRIVATE_WRAP,
    #[serde(rename = "CKM_SKIPJACK_RELAYX")]
    SkipjackRelayx = CKM_SKIPJACK_RELAYX,
    #[serde(rename = "CKM_KEA_KEY_PAIR_GEN")]
    KeaKeyPairGen = CKM_KEA_KEY_PAIR_GEN,
    #[serde(rename = "CKM_KEA_KEY_DERIVE")]
    KeaKeyDerive = CKM_KEA_KEY_DERIVE,
    #[serde(rename = "CKM_FORTEZZA_TIMESTAMP")]
    FortezzaTimestamp = CKM_FORTEZZA_TIMESTAMP,
    #[serde(rename = "CKM_BATON_KEY_GEN")]
    BatonKeyGen = CKM_BATON_KEY_GEN,
    #[serde(rename = "CKM_BATON_ECB128")]
    BatonEcb128 = CKM_BATON_ECB128,
    #[serde(rename = "CKM_BATON_ECB96")]
    BatonEcb96 = CKM_BATON_ECB96,
    #[serde(rename = "CKM_BATON_CBC128")]
    BatonCbc128 = CKM_BATON_CBC128,
    #[serde(rename = "CKM_BATON_COUNTER")]
    BatonCounter = CKM_BATON_COUNTER,
    #[serde(rename = "CKM_BATON_SHUFFLE")]
    BatonShuffle = CKM_BATON_SHUFFLE,
    #[serde(rename = "CKM_BATON_WRAP")]
    BatonWrap = CKM_BATON_WRAP,
    #[serde(rename = "CKM_EC_KEY_PAIR_GEN")]
    EcKeyPairGen = CKM_EC_KEY_PAIR_GEN,
    #[serde(rename = "CKM_ECDSA")]
    Ecdsa = CKM_ECDSA,
    #[serde(rename = "CKM_ECDSA_SHA1")]
    EcdsaSha1 = CKM_ECDSA_SHA1,
    #[serde(rename = "CKM_ECDSA_SHA224")]
    EcdsaSha224 = CKM_ECDSA_SHA224,
    #[serde(rename = "CKM_ECDSA_SHA256")]
    EcdsaSha256 = CKM_ECDSA_SHA256,
    #[serde(rename = "CKM_ECDSA_SHA384")]
    EcdsaSha384 = CKM_ECDSA_SHA384,
    #[serde(rename = "CKM_ECDSA_SHA512")]
    EcdsaSha512 = CKM_ECDSA_SHA512,
    #[serde(rename = "CKM_ECDH1_DERIVE")]
    Ecdh1Derive = CKM_ECDH1_DERIVE,
    #[serde(rename = "CKM_ECDH1_COFACTOR_DERIVE")]
    Ecdh1CofactorDerive = CKM_ECDH1_COFACTOR_DERIVE,
    #[serde(rename = "CKM_ECMQV_DERIVE")]
    EcmqvDerive = CKM_ECMQV_DERIVE,
    #[serde(rename = "CKM_ECDH_AES_KEY_WRAP")]
    EcdhAesKeyWrap = CKM_ECDH_AES_KEY_WRAP,
    #[serde(rename = "CKM_RSA_AES_KEY_WRAP")]
    RsaAesKeyWrap = CKM_RSA_AES_KEY_WRAP,
    #[serde(rename = "CKM_JUNIPER_KEY_GEN")]
    JuniperKeyGen = CKM_JUNIPER_KEY_GEN,
    #[serde(rename = "CKM_JUNIPER_ECB128")]
    JuniperEcb128 = CKM_JUNIPER_ECB128,
    #[serde(rename = "CKM_JUNIPER_CBC128")]
    JuniperCbc128 = CKM_JUNIPER_CBC128,
    #[serde(rename = "CKM_JUNIPER_COUNTER")]
    JuniperCounter = CKM_JUNIPER_COUNTER,
    #[serde(rename = "CKM_JUNIPER_SHUFFLE")]
    JuniperShuffle = CKM_JUNIPER_SHUFFLE,
    #[serde(rename = "CKM_JUNIPER_WRAP")]
    JuniperWrap = CKM_JUNIPER_WRAP,
    #[serde(rename = "CKM_FASTHASH")]
    Fasthash = CKM_FASTHASH,
    #[serde(rename = "CKM_AES_KEY_GEN")]
    AesKeyGen = CKM_AES_KEY_GEN,
    #[serde(rename = "CKM_AES_ECB")]
    AesEcb = CKM_AES_ECB,
    #[serde(rename = "CKM_AES_CBC")]
    AesCbc = CKM_AES_CBC,
    #[serde(rename = "CKM_AES_MAC")]
    AesMac = CKM_AES_MAC,
    #[serde(rename = "CKM_AES_MAC_GENERAL")]
    AesMacGeneral = CKM_AES_MAC_GENERAL,
    #[serde(rename = "CKM_AES_CBC_PAD")]
    AesCbcPad = CKM_AES_CBC_PAD,
    #[serde(rename = "CKM_AES_CTR")]
    AesCtr = CKM_AES_CTR,
    #[serde(rename = "CKM_AES_GCM")]
    AesGcm = CKM_AES_GCM,
    #[serde(rename = "CKM_AES_CCM")]
    AesCcm = CKM_AES_CCM,
    #[serde(rename = "CKM_AES_CTS")]
    AesCts = CKM_AES_CTS,
    #[serde(rename = "CKM_AES_CMAC")]
    AesCmac = CKM_AES_CMAC,
    #[serde(rename = "CKM_AES_CMAC_GENERAL")]
    AesCmacGeneral = CKM_AES_CMAC_GENERAL,
    #[serde(rename = "CKM_AES_XCBC_MAC")]
    AesXcbcMac = CKM_AES_XCBC_MAC,
    #[serde(rename = "CKM_AES_XCBC_MAC_96")]
    AesXcbcMac96 = CKM_AES_XCBC_MAC_96,
    #[serde(rename = "CKM_AES_GMAC")]
    AesGmac = CKM_AES_GMAC,
    #[serde(rename = "CKM_BLOWFISH_KEY_GEN")]
    BlowfishKeyGen = CKM_BLOWFISH_KEY_GEN,
    #[serde(rename = "CKM_BLOWFISH_CBC")]
    BlowfishCbc = CKM_BLOWFISH_CBC,
    #[serde(rename = "CKM_TWOFISH_KEY_GEN")]
    TwofishKeyGen = CKM_TWOFISH_KEY_GEN,
    #[serde(rename = "CKM_TWOFISH_CBC")]
    TwofishCbc = CKM_TWOFISH_CBC,
    #[serde(rename = "CKM_BLOWFISH_CBC_PAD")]
    BlowfishCbcPad = CKM_BLOWFISH_CBC_PAD,
    #[serde(rename = "CKM_TWOFISH_CBC_PAD")]
    TwofishCbcPad = CKM_TWOFISH_CBC_PAD,
    #[serde(rename = "CKM_DES_ECB_ENCRYPT_DATA")]
    DesEcbEncryptData = CKM_DES_ECB_ENCRYPT_DATA,
    #[serde(rename = "CKM_DES_CBC_ENCRYPT_DATA")]
    DesCbcEncryptData = CKM_DES_CBC_ENCRYPT_DATA,
    #[serde(rename = "CKM_DES3_ECB_ENCRYPT_DATA")]
    Des3EcbEncryptData = CKM_DES3_ECB_ENCRYPT_DATA,
    #[serde(rename = "CKM_DES3_CBC_ENCRYPT_DATA")]
    Des3CbcEncryptData = CKM_DES3_CBC_ENCRYPT_DATA,
    #[serde(rename = "CKM_AES_ECB_ENCRYPT_DATA")]
    AesEcbEncryptData = CKM_AES_ECB_ENCRYPT_DATA,
    #[serde(rename = "CKM_AES_CBC_ENCRYPT_DATA")]
    AesCbcEncryptData = CKM_AES_CBC_ENCRYPT_DATA,
    #[serde(rename = "CKM_GOSTR3410_KEY_PAIR_GEN")]
    Gostr3410KeyPairGen = CKM_GOSTR3410_KEY_PAIR_GEN,
    #[serde(rename = "CKM_GOSTR3410")]
    Gostr3410 = CKM_GOSTR3410,
    #[serde(rename = "CKM_GOSTR3410_WITH_GOSTR3411")]
    Gostr3410WithGostr3411 = CKM_GOSTR3410_WITH_GOSTR3411,
    #[serde(rename = "CKM_GOSTR3410_KEY_WRAP")]
    Gostr3410KeyWrap = CKM_GOSTR3410_KEY_WRAP,
    #[serde(rename = "CKM_GOSTR3410_DERIVE")]
    Gostr3410Derive = CKM_GOSTR3410_DERIVE,
    #[serde(rename = "CKM_GOSTR3411")]
    Gostr3411 = CKM_GOSTR3411,
    #[serde(rename = "CKM_GOSTR3411_HMAC")]
    Gostr3411Hmac = CKM_GOSTR3411_HMAC,
    #[serde(rename = "CKM_GOST28147_KEY_GEN")]
    Gost28147KeyGen = CKM_GOST28147_KEY_GEN,
    #[serde(rename = "CKM_GOST28147_ECB")]
    Gost28147Ecb = CKM_GOST28147_ECB,
    #[serde(rename = "CKM_GOST28147")]
    Gost28147 = CKM_GOST28147,
    #[serde(rename = "CKM_GOST28147_MAC")]
    Gost28147Mac = CKM_GOST28147_MAC,
    #[serde(rename = "CKM_GOST28147_KEY_WRAP")]
    Gost28147KeyWrap = CKM_GOST28147_KEY_WRAP,
    #[serde(rename = "CKM_DSA_PARAMETER_GEN")]
    DsaParameterGen = CKM_DSA_PARAMETER_GEN,
    #[serde(rename = "CKM_DH_PKCS_PARAMETER_GEN")]
    DhPkcsParameterGen = CKM_DH_PKCS_PARAMETER_GEN,
    #[serde(rename = "CKM_X9_42_DH_PARAMETER_GEN")]
    X942DhParameterGen = CKM_X9_42_DH_PARAMETER_GEN,
    #[serde(rename = "CKM_DSA_PROBABLISTIC_PARAMETER_GEN")]
    DsaProbablisticParameterGen = CKM_DSA_PROBABLISTIC_PARAMETER_GEN,
    #[serde(rename = "CKM_DSA_SHAWE_TAYLOR_PARAMETER_GEN")]
    DsaShaweTaylorParameterGen = CKM_DSA_SHAWE_TAYLOR_PARAMETER_GEN,
    #[serde(rename = "CKM_AES_OFB")]
    AesOfb = CKM_AES_OFB,
    #[serde(rename = "CKM_AES_CFB64")]
    AesCfb64 = CKM_AES_CFB64,
    #[serde(rename = "CKM_AES_CFB8")]
    AesCfb8 = CKM_AES_CFB8,
    #[serde(rename = "CKM_AES_CFB128")]
    AesCfb128 = CKM_AES_CFB128,
    #[serde(rename = "CKM_AES_CFB1")]
    AesCfb1 = CKM_AES_CFB1,
    #[serde(rename = "CKM_SHA224")]
    Sha224 = CKM_SHA224,
    #[serde(rename = "CKM_SHA224_HMAC")]
    Sha224Hmac = CKM_SHA224_HMAC,
    #[serde(rename = "CKM_SHA224_HMAC_GENERAL")]
    Sha224HmacGeneral = CKM_SHA224_HMAC_GENERAL,
    #[serde(rename = "CKM_SHA224_RSA_PKCS")]
    Sha224RsaPkcs = CKM_SHA224_RSA_PKCS,
    #[serde(rename = "CKM_SHA224_RSA_PKCS_PSS")]
    Sha224RsaPkcsPss = CKM_SHA224_RSA_PKCS_PSS,
    #[serde(rename = "CKM_SHA224_KEY_DERIVATION")]
    Sha224KeyDerivation = CKM_SHA224_KEY_DERIVATION,
    #[serde(rename = "CKM_CAMELLIA_ECB")]
    CamelliaEcb = CKM_CAMELLIA_ECB,
    #[serde(rename = "CKM_CAMELLIA_CBC")]
    CamelliaCbc = CKM_CAMELLIA_CBC,
    #[serde(rename = "CKM_CAMELLIA_MAC")]
    CamelliaMac = CKM_CAMELLIA_MAC,
    #[serde(rename = "CKM_CAMELLIA_MAC_GENERAL")]
    CamelliaMacGeneral = CKM_CAMELLIA_MAC_GENERAL,
    #[serde(rename = "CKM_CAMELLIA_CBC_PAD")]
    CamelliaCbcPad = CKM_CAMELLIA_CBC_PAD,
    #[serde(rename = "CKM_CAMELLIA_ECB_ENCRYPT_DATA")]
    CamelliaEcbEncryptData = CKM_CAMELLIA_ECB_ENCRYPT_DATA,
    #[serde(rename = "CKM_CAMELLIA_CBC_ENCRYPT_DATA")]
    CamelliaCbcEncryptData = CKM_CAMELLIA_CBC_ENCRYPT_DATA,
    #[serde(rename = "CKM_AES_KEY_WRAP")]
    AesKeyWrap = CKM_AES_KEY_WRAP,
    #[serde(rename = "CKM_AES_KEY_WRAP_PAD")]
    AesKeyWrapPad = CKM_AES_KEY_WRAP_PAD,
    #[serde(rename = "CKM_RSA_PKCS_TPM_1_1")]
    RsaPkcsTpm1_1 = CKM_RSA_PKCS_TPM_1_1,
    #[serde(rename = "CKM_RSA_PKCS_OAEP_TPM_1_1")]
    RsaPkcsOaepTpm1_1 = CKM_RSA_PKCS_OAEP_TPM_1_1,
    #[serde(rename = "CKM_EC_EDWARDS_KEY_PAIR_GEN")]
    EcEdwardsKeyPairGen = CKM_EC_EDWARDS_KEY_PAIR_GEN,
    #[serde(rename = "CKM_EC_MONTGOMERY_KEY_PAIR_GEN")]
    EcMontgomeryKeyPairGen = CKM_EC_MONTGOMERY_KEY_PAIR_GEN,
    #[serde(rename = "CKM_EDDSA")]
    Eddsa = CKM_EDDSA,
    #[serde(rename = "CKM_VENDOR_DEFINED")]
    VendorDefined = CKM_VENDOR_DEFINED,
    #[num_enum(catch_all)]
    UnknownMechanismType(u64) = u64::MAX,
}

impl From<cryptoki::mechanism::MechanismType> for MechanismType {
    fn from(val: cryptoki::mechanism::MechanismType) -> Self {
        let val = CK_MECHANISM_TYPE::from(val);
        Self::from(val)
    }
}

impl TryFrom<MechanismType> for cryptoki::mechanism::MechanismType {
    type Error = cryptoki::error::Error;
    fn try_from(val: MechanismType) -> Result<Self, Self::Error> {
        let val = CK_MECHANISM_TYPE::from(val);
        cryptoki::mechanism::MechanismType::try_from(val)
    }
}

impl TryFrom<&AttrData> for MechanismType {
    type Error = AttributeError;
    fn try_from(val: &AttrData) -> Result<Self, Self::Error> {
        match val {
            AttrData::MechanismType(x) => Ok(*x),
            _ => Err(AttributeError::EncodingError),
        }
    }
}

impl From<MechanismType> for AttrData {
    fn from(val: MechanismType) -> Self {
        AttrData::MechanismType(val)
    }
}
