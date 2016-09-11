/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#define _WPA_

#define NEW_SIMD_CODE

#include "inc_vendor.cl"
#include "inc_hash_constants.h"
#include "inc_hash_functions.cl"
#include "inc_types.cl"
#include "inc_common.cl"
#include "inc_simd.cl"

#define COMPARE_S "inc_comp_single.cl"
#define COMPARE_M "inc_comp_multi.cl"

void md5_transform_S (const u32 w0[4], const u32 w1[4], const u32 w2[4], const u32 w3[4], u32 digest[4])
{
  u32 a = digest[0];
  u32 b = digest[1];
  u32 c = digest[2];
  u32 d = digest[3];

  u32 w0_t = w0[0];
  u32 w1_t = w0[1];
  u32 w2_t = w0[2];
  u32 w3_t = w0[3];
  u32 w4_t = w1[0];
  u32 w5_t = w1[1];
  u32 w6_t = w1[2];
  u32 w7_t = w1[3];
  u32 w8_t = w2[0];
  u32 w9_t = w2[1];
  u32 wa_t = w2[2];
  u32 wb_t = w2[3];
  u32 wc_t = w3[0];
  u32 wd_t = w3[1];
  u32 we_t = w3[2];
  u32 wf_t = w3[3];

  MD5_STEP_S (MD5_Fo, a, b, c, d, w0_t, MD5C00, MD5S00);
  MD5_STEP_S (MD5_Fo, d, a, b, c, w1_t, MD5C01, MD5S01);
  MD5_STEP_S (MD5_Fo, c, d, a, b, w2_t, MD5C02, MD5S02);
  MD5_STEP_S (MD5_Fo, b, c, d, a, w3_t, MD5C03, MD5S03);
  MD5_STEP_S (MD5_Fo, a, b, c, d, w4_t, MD5C04, MD5S00);
  MD5_STEP_S (MD5_Fo, d, a, b, c, w5_t, MD5C05, MD5S01);
  MD5_STEP_S (MD5_Fo, c, d, a, b, w6_t, MD5C06, MD5S02);
  MD5_STEP_S (MD5_Fo, b, c, d, a, w7_t, MD5C07, MD5S03);
  MD5_STEP_S (MD5_Fo, a, b, c, d, w8_t, MD5C08, MD5S00);
  MD5_STEP_S (MD5_Fo, d, a, b, c, w9_t, MD5C09, MD5S01);
  MD5_STEP_S (MD5_Fo, c, d, a, b, wa_t, MD5C0a, MD5S02);
  MD5_STEP_S (MD5_Fo, b, c, d, a, wb_t, MD5C0b, MD5S03);
  MD5_STEP_S (MD5_Fo, a, b, c, d, wc_t, MD5C0c, MD5S00);
  MD5_STEP_S (MD5_Fo, d, a, b, c, wd_t, MD5C0d, MD5S01);
  MD5_STEP_S (MD5_Fo, c, d, a, b, we_t, MD5C0e, MD5S02);
  MD5_STEP_S (MD5_Fo, b, c, d, a, wf_t, MD5C0f, MD5S03);

  MD5_STEP_S (MD5_Go, a, b, c, d, w1_t, MD5C10, MD5S10);
  MD5_STEP_S (MD5_Go, d, a, b, c, w6_t, MD5C11, MD5S11);
  MD5_STEP_S (MD5_Go, c, d, a, b, wb_t, MD5C12, MD5S12);
  MD5_STEP_S (MD5_Go, b, c, d, a, w0_t, MD5C13, MD5S13);
  MD5_STEP_S (MD5_Go, a, b, c, d, w5_t, MD5C14, MD5S10);
  MD5_STEP_S (MD5_Go, d, a, b, c, wa_t, MD5C15, MD5S11);
  MD5_STEP_S (MD5_Go, c, d, a, b, wf_t, MD5C16, MD5S12);
  MD5_STEP_S (MD5_Go, b, c, d, a, w4_t, MD5C17, MD5S13);
  MD5_STEP_S (MD5_Go, a, b, c, d, w9_t, MD5C18, MD5S10);
  MD5_STEP_S (MD5_Go, d, a, b, c, we_t, MD5C19, MD5S11);
  MD5_STEP_S (MD5_Go, c, d, a, b, w3_t, MD5C1a, MD5S12);
  MD5_STEP_S (MD5_Go, b, c, d, a, w8_t, MD5C1b, MD5S13);
  MD5_STEP_S (MD5_Go, a, b, c, d, wd_t, MD5C1c, MD5S10);
  MD5_STEP_S (MD5_Go, d, a, b, c, w2_t, MD5C1d, MD5S11);
  MD5_STEP_S (MD5_Go, c, d, a, b, w7_t, MD5C1e, MD5S12);
  MD5_STEP_S (MD5_Go, b, c, d, a, wc_t, MD5C1f, MD5S13);

  MD5_STEP_S (MD5_H , a, b, c, d, w5_t, MD5C20, MD5S20);
  MD5_STEP_S (MD5_H , d, a, b, c, w8_t, MD5C21, MD5S21);
  MD5_STEP_S (MD5_H , c, d, a, b, wb_t, MD5C22, MD5S22);
  MD5_STEP_S (MD5_H , b, c, d, a, we_t, MD5C23, MD5S23);
  MD5_STEP_S (MD5_H , a, b, c, d, w1_t, MD5C24, MD5S20);
  MD5_STEP_S (MD5_H , d, a, b, c, w4_t, MD5C25, MD5S21);
  MD5_STEP_S (MD5_H , c, d, a, b, w7_t, MD5C26, MD5S22);
  MD5_STEP_S (MD5_H , b, c, d, a, wa_t, MD5C27, MD5S23);
  MD5_STEP_S (MD5_H , a, b, c, d, wd_t, MD5C28, MD5S20);
  MD5_STEP_S (MD5_H , d, a, b, c, w0_t, MD5C29, MD5S21);
  MD5_STEP_S (MD5_H , c, d, a, b, w3_t, MD5C2a, MD5S22);
  MD5_STEP_S (MD5_H , b, c, d, a, w6_t, MD5C2b, MD5S23);
  MD5_STEP_S (MD5_H , a, b, c, d, w9_t, MD5C2c, MD5S20);
  MD5_STEP_S (MD5_H , d, a, b, c, wc_t, MD5C2d, MD5S21);
  MD5_STEP_S (MD5_H , c, d, a, b, wf_t, MD5C2e, MD5S22);
  MD5_STEP_S (MD5_H , b, c, d, a, w2_t, MD5C2f, MD5S23);

  MD5_STEP_S (MD5_I , a, b, c, d, w0_t, MD5C30, MD5S30);
  MD5_STEP_S (MD5_I , d, a, b, c, w7_t, MD5C31, MD5S31);
  MD5_STEP_S (MD5_I , c, d, a, b, we_t, MD5C32, MD5S32);
  MD5_STEP_S (MD5_I , b, c, d, a, w5_t, MD5C33, MD5S33);
  MD5_STEP_S (MD5_I , a, b, c, d, wc_t, MD5C34, MD5S30);
  MD5_STEP_S (MD5_I , d, a, b, c, w3_t, MD5C35, MD5S31);
  MD5_STEP_S (MD5_I , c, d, a, b, wa_t, MD5C36, MD5S32);
  MD5_STEP_S (MD5_I , b, c, d, a, w1_t, MD5C37, MD5S33);
  MD5_STEP_S (MD5_I , a, b, c, d, w8_t, MD5C38, MD5S30);
  MD5_STEP_S (MD5_I , d, a, b, c, wf_t, MD5C39, MD5S31);
  MD5_STEP_S (MD5_I , c, d, a, b, w6_t, MD5C3a, MD5S32);
  MD5_STEP_S (MD5_I , b, c, d, a, wd_t, MD5C3b, MD5S33);
  MD5_STEP_S (MD5_I , a, b, c, d, w4_t, MD5C3c, MD5S30);
  MD5_STEP_S (MD5_I , d, a, b, c, wb_t, MD5C3d, MD5S31);
  MD5_STEP_S (MD5_I , c, d, a, b, w2_t, MD5C3e, MD5S32);
  MD5_STEP_S (MD5_I , b, c, d, a, w9_t, MD5C3f, MD5S33);

  digest[0] += a;
  digest[1] += b;
  digest[2] += c;
  digest[3] += d;
}

void hmac_md5_pad_S (u32 w0[4], u32 w1[4], u32 w2[4], u32 w3[4], u32 ipad[4], u32 opad[4])
{
  w0[0] = w0[0] ^ 0x36363636;
  w0[1] = w0[1] ^ 0x36363636;
  w0[2] = w0[2] ^ 0x36363636;
  w0[3] = w0[3] ^ 0x36363636;
  w1[0] = w1[0] ^ 0x36363636;
  w1[1] = w1[1] ^ 0x36363636;
  w1[2] = w1[2] ^ 0x36363636;
  w1[3] = w1[3] ^ 0x36363636;
  w2[0] = w2[0] ^ 0x36363636;
  w2[1] = w2[1] ^ 0x36363636;
  w2[2] = w2[2] ^ 0x36363636;
  w2[3] = w2[3] ^ 0x36363636;
  w3[0] = w3[0] ^ 0x36363636;
  w3[1] = w3[1] ^ 0x36363636;
  w3[2] = w3[2] ^ 0x36363636;
  w3[3] = w3[3] ^ 0x36363636;

  ipad[0] = MD5M_A;
  ipad[1] = MD5M_B;
  ipad[2] = MD5M_C;
  ipad[3] = MD5M_D;

  md5_transform_S (w0, w1, w2, w3, ipad);

  w0[0] = w0[0] ^ 0x6a6a6a6a;
  w0[1] = w0[1] ^ 0x6a6a6a6a;
  w0[2] = w0[2] ^ 0x6a6a6a6a;
  w0[3] = w0[3] ^ 0x6a6a6a6a;
  w1[0] = w1[0] ^ 0x6a6a6a6a;
  w1[1] = w1[1] ^ 0x6a6a6a6a;
  w1[2] = w1[2] ^ 0x6a6a6a6a;
  w1[3] = w1[3] ^ 0x6a6a6a6a;
  w2[0] = w2[0] ^ 0x6a6a6a6a;
  w2[1] = w2[1] ^ 0x6a6a6a6a;
  w2[2] = w2[2] ^ 0x6a6a6a6a;
  w2[3] = w2[3] ^ 0x6a6a6a6a;
  w3[0] = w3[0] ^ 0x6a6a6a6a;
  w3[1] = w3[1] ^ 0x6a6a6a6a;
  w3[2] = w3[2] ^ 0x6a6a6a6a;
  w3[3] = w3[3] ^ 0x6a6a6a6a;

  opad[0] = MD5M_A;
  opad[1] = MD5M_B;
  opad[2] = MD5M_C;
  opad[3] = MD5M_D;

  md5_transform_S (w0, w1, w2, w3, opad);
}

void hmac_md5_run_S (u32 w0[4], u32 w1[4], u32 w2[4], u32 w3[4], u32 ipad[4], u32 opad[4], u32 digest[4])
{
  digest[0] = ipad[0];
  digest[1] = ipad[1];
  digest[2] = ipad[2];
  digest[3] = ipad[3];

  md5_transform_S (w0, w1, w2, w3, digest);

  w0[0] = digest[0];
  w0[1] = digest[1];
  w0[2] = digest[2];
  w0[3] = digest[3];
  w1[0] = 0x80;
  w1[1] = 0;
  w1[2] = 0;
  w1[3] = 0;
  w2[0] = 0;
  w2[1] = 0;
  w2[2] = 0;
  w2[3] = 0;
  w3[0] = 0;
  w3[1] = 0;
  w3[2] = (64 + 16) * 8;
  w3[3] = 0;

  digest[0] = opad[0];
  digest[1] = opad[1];
  digest[2] = opad[2];
  digest[3] = opad[3];

  md5_transform_S (w0, w1, w2, w3, digest);
}

void sha1_transform_S (const u32 w0[4], const u32 w1[4], const u32 w2[4], const u32 w3[4], u32 digest[5])
{
  u32 A = digest[0];
  u32 B = digest[1];
  u32 C = digest[2];
  u32 D = digest[3];
  u32 E = digest[4];

  u32 w0_t = w0[0];
  u32 w1_t = w0[1];
  u32 w2_t = w0[2];
  u32 w3_t = w0[3];
  u32 w4_t = w1[0];
  u32 w5_t = w1[1];
  u32 w6_t = w1[2];
  u32 w7_t = w1[3];
  u32 w8_t = w2[0];
  u32 w9_t = w2[1];
  u32 wa_t = w2[2];
  u32 wb_t = w2[3];
  u32 wc_t = w3[0];
  u32 wd_t = w3[1];
  u32 we_t = w3[2];
  u32 wf_t = w3[3];

  #undef K
  #define K SHA1C00

  SHA1_STEP_S (SHA1_F0o, A, B, C, D, E, w0_t);
  SHA1_STEP_S (SHA1_F0o, E, A, B, C, D, w1_t);
  SHA1_STEP_S (SHA1_F0o, D, E, A, B, C, w2_t);
  SHA1_STEP_S (SHA1_F0o, C, D, E, A, B, w3_t);
  SHA1_STEP_S (SHA1_F0o, B, C, D, E, A, w4_t);
  SHA1_STEP_S (SHA1_F0o, A, B, C, D, E, w5_t);
  SHA1_STEP_S (SHA1_F0o, E, A, B, C, D, w6_t);
  SHA1_STEP_S (SHA1_F0o, D, E, A, B, C, w7_t);
  SHA1_STEP_S (SHA1_F0o, C, D, E, A, B, w8_t);
  SHA1_STEP_S (SHA1_F0o, B, C, D, E, A, w9_t);
  SHA1_STEP_S (SHA1_F0o, A, B, C, D, E, wa_t);
  SHA1_STEP_S (SHA1_F0o, E, A, B, C, D, wb_t);
  SHA1_STEP_S (SHA1_F0o, D, E, A, B, C, wc_t);
  SHA1_STEP_S (SHA1_F0o, C, D, E, A, B, wd_t);
  SHA1_STEP_S (SHA1_F0o, B, C, D, E, A, we_t);
  SHA1_STEP_S (SHA1_F0o, A, B, C, D, E, wf_t);
  w0_t = rotl32_S ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP_S (SHA1_F0o, E, A, B, C, D, w0_t);
  w1_t = rotl32_S ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP_S (SHA1_F0o, D, E, A, B, C, w1_t);
  w2_t = rotl32_S ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP_S (SHA1_F0o, C, D, E, A, B, w2_t);
  w3_t = rotl32_S ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP_S (SHA1_F0o, B, C, D, E, A, w3_t);

  #undef K
  #define K SHA1C01

  w4_t = rotl32_S ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP_S (SHA1_F1, A, B, C, D, E, w4_t);
  w5_t = rotl32_S ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP_S (SHA1_F1, E, A, B, C, D, w5_t);
  w6_t = rotl32_S ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP_S (SHA1_F1, D, E, A, B, C, w6_t);
  w7_t = rotl32_S ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP_S (SHA1_F1, C, D, E, A, B, w7_t);
  w8_t = rotl32_S ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP_S (SHA1_F1, B, C, D, E, A, w8_t);
  w9_t = rotl32_S ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP_S (SHA1_F1, A, B, C, D, E, w9_t);
  wa_t = rotl32_S ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP_S (SHA1_F1, E, A, B, C, D, wa_t);
  wb_t = rotl32_S ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP_S (SHA1_F1, D, E, A, B, C, wb_t);
  wc_t = rotl32_S ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP_S (SHA1_F1, C, D, E, A, B, wc_t);
  wd_t = rotl32_S ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP_S (SHA1_F1, B, C, D, E, A, wd_t);
  we_t = rotl32_S ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP_S (SHA1_F1, A, B, C, D, E, we_t);
  wf_t = rotl32_S ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP_S (SHA1_F1, E, A, B, C, D, wf_t);
  w0_t = rotl32_S ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP_S (SHA1_F1, D, E, A, B, C, w0_t);
  w1_t = rotl32_S ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP_S (SHA1_F1, C, D, E, A, B, w1_t);
  w2_t = rotl32_S ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP_S (SHA1_F1, B, C, D, E, A, w2_t);
  w3_t = rotl32_S ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP_S (SHA1_F1, A, B, C, D, E, w3_t);
  w4_t = rotl32_S ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP_S (SHA1_F1, E, A, B, C, D, w4_t);
  w5_t = rotl32_S ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP_S (SHA1_F1, D, E, A, B, C, w5_t);
  w6_t = rotl32_S ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP_S (SHA1_F1, C, D, E, A, B, w6_t);
  w7_t = rotl32_S ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP_S (SHA1_F1, B, C, D, E, A, w7_t);

  #undef K
  #define K SHA1C02

  w8_t = rotl32_S ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP_S (SHA1_F2o, A, B, C, D, E, w8_t);
  w9_t = rotl32_S ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP_S (SHA1_F2o, E, A, B, C, D, w9_t);
  wa_t = rotl32_S ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP_S (SHA1_F2o, D, E, A, B, C, wa_t);
  wb_t = rotl32_S ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP_S (SHA1_F2o, C, D, E, A, B, wb_t);
  wc_t = rotl32_S ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP_S (SHA1_F2o, B, C, D, E, A, wc_t);
  wd_t = rotl32_S ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP_S (SHA1_F2o, A, B, C, D, E, wd_t);
  we_t = rotl32_S ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP_S (SHA1_F2o, E, A, B, C, D, we_t);
  wf_t = rotl32_S ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP_S (SHA1_F2o, D, E, A, B, C, wf_t);
  w0_t = rotl32_S ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP_S (SHA1_F2o, C, D, E, A, B, w0_t);
  w1_t = rotl32_S ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP_S (SHA1_F2o, B, C, D, E, A, w1_t);
  w2_t = rotl32_S ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP_S (SHA1_F2o, A, B, C, D, E, w2_t);
  w3_t = rotl32_S ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP_S (SHA1_F2o, E, A, B, C, D, w3_t);
  w4_t = rotl32_S ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP_S (SHA1_F2o, D, E, A, B, C, w4_t);
  w5_t = rotl32_S ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP_S (SHA1_F2o, C, D, E, A, B, w5_t);
  w6_t = rotl32_S ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP_S (SHA1_F2o, B, C, D, E, A, w6_t);
  w7_t = rotl32_S ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP_S (SHA1_F2o, A, B, C, D, E, w7_t);
  w8_t = rotl32_S ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP_S (SHA1_F2o, E, A, B, C, D, w8_t);
  w9_t = rotl32_S ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP_S (SHA1_F2o, D, E, A, B, C, w9_t);
  wa_t = rotl32_S ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP_S (SHA1_F2o, C, D, E, A, B, wa_t);
  wb_t = rotl32_S ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP_S (SHA1_F2o, B, C, D, E, A, wb_t);

  #undef K
  #define K SHA1C03

  wc_t = rotl32_S ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP_S (SHA1_F1, A, B, C, D, E, wc_t);
  wd_t = rotl32_S ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP_S (SHA1_F1, E, A, B, C, D, wd_t);
  we_t = rotl32_S ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP_S (SHA1_F1, D, E, A, B, C, we_t);
  wf_t = rotl32_S ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP_S (SHA1_F1, C, D, E, A, B, wf_t);
  w0_t = rotl32_S ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP_S (SHA1_F1, B, C, D, E, A, w0_t);
  w1_t = rotl32_S ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP_S (SHA1_F1, A, B, C, D, E, w1_t);
  w2_t = rotl32_S ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP_S (SHA1_F1, E, A, B, C, D, w2_t);
  w3_t = rotl32_S ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP_S (SHA1_F1, D, E, A, B, C, w3_t);
  w4_t = rotl32_S ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP_S (SHA1_F1, C, D, E, A, B, w4_t);
  w5_t = rotl32_S ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP_S (SHA1_F1, B, C, D, E, A, w5_t);
  w6_t = rotl32_S ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP_S (SHA1_F1, A, B, C, D, E, w6_t);
  w7_t = rotl32_S ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP_S (SHA1_F1, E, A, B, C, D, w7_t);
  w8_t = rotl32_S ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP_S (SHA1_F1, D, E, A, B, C, w8_t);
  w9_t = rotl32_S ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP_S (SHA1_F1, C, D, E, A, B, w9_t);
  wa_t = rotl32_S ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP_S (SHA1_F1, B, C, D, E, A, wa_t);
  wb_t = rotl32_S ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP_S (SHA1_F1, A, B, C, D, E, wb_t);
  wc_t = rotl32_S ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP_S (SHA1_F1, E, A, B, C, D, wc_t);
  wd_t = rotl32_S ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP_S (SHA1_F1, D, E, A, B, C, wd_t);
  we_t = rotl32_S ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP_S (SHA1_F1, C, D, E, A, B, we_t);
  wf_t = rotl32_S ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP_S (SHA1_F1, B, C, D, E, A, wf_t);

  digest[0] += A;
  digest[1] += B;
  digest[2] += C;
  digest[3] += D;
  digest[4] += E;
}

void hmac_sha1_pad_S (u32 w0[4], u32 w1[4], u32 w2[4], u32 w3[4], u32 ipad[5], u32 opad[5])
{
  w0[0] = w0[0] ^ 0x36363636;
  w0[1] = w0[1] ^ 0x36363636;
  w0[2] = w0[2] ^ 0x36363636;
  w0[3] = w0[3] ^ 0x36363636;
  w1[0] = w1[0] ^ 0x36363636;
  w1[1] = w1[1] ^ 0x36363636;
  w1[2] = w1[2] ^ 0x36363636;
  w1[3] = w1[3] ^ 0x36363636;
  w2[0] = w2[0] ^ 0x36363636;
  w2[1] = w2[1] ^ 0x36363636;
  w2[2] = w2[2] ^ 0x36363636;
  w2[3] = w2[3] ^ 0x36363636;
  w3[0] = w3[0] ^ 0x36363636;
  w3[1] = w3[1] ^ 0x36363636;
  w3[2] = w3[2] ^ 0x36363636;
  w3[3] = w3[3] ^ 0x36363636;

  ipad[0] = SHA1M_A;
  ipad[1] = SHA1M_B;
  ipad[2] = SHA1M_C;
  ipad[3] = SHA1M_D;
  ipad[4] = SHA1M_E;

  sha1_transform_S (w0, w1, w2, w3, ipad);

  w0[0] = w0[0] ^ 0x6a6a6a6a;
  w0[1] = w0[1] ^ 0x6a6a6a6a;
  w0[2] = w0[2] ^ 0x6a6a6a6a;
  w0[3] = w0[3] ^ 0x6a6a6a6a;
  w1[0] = w1[0] ^ 0x6a6a6a6a;
  w1[1] = w1[1] ^ 0x6a6a6a6a;
  w1[2] = w1[2] ^ 0x6a6a6a6a;
  w1[3] = w1[3] ^ 0x6a6a6a6a;
  w2[0] = w2[0] ^ 0x6a6a6a6a;
  w2[1] = w2[1] ^ 0x6a6a6a6a;
  w2[2] = w2[2] ^ 0x6a6a6a6a;
  w2[3] = w2[3] ^ 0x6a6a6a6a;
  w3[0] = w3[0] ^ 0x6a6a6a6a;
  w3[1] = w3[1] ^ 0x6a6a6a6a;
  w3[2] = w3[2] ^ 0x6a6a6a6a;
  w3[3] = w3[3] ^ 0x6a6a6a6a;

  opad[0] = SHA1M_A;
  opad[1] = SHA1M_B;
  opad[2] = SHA1M_C;
  opad[3] = SHA1M_D;
  opad[4] = SHA1M_E;

  sha1_transform_S (w0, w1, w2, w3, opad);
}

void hmac_sha1_run_S (u32 w0[4], u32 w1[4], u32 w2[4], u32 w3[4], u32 ipad[5], u32 opad[5], u32 digest[5])
{
  digest[0] = ipad[0];
  digest[1] = ipad[1];
  digest[2] = ipad[2];
  digest[3] = ipad[3];
  digest[4] = ipad[4];

  sha1_transform_S (w0, w1, w2, w3, digest);

  w0[0] = digest[0];
  w0[1] = digest[1];
  w0[2] = digest[2];
  w0[3] = digest[3];
  w1[0] = digest[4];
  w1[1] = 0x80000000;
  w1[2] = 0;
  w1[3] = 0;
  w2[0] = 0;
  w2[1] = 0;
  w2[2] = 0;
  w2[3] = 0;
  w3[0] = 0;
  w3[1] = 0;
  w3[2] = 0;
  w3[3] = (64 + 20) * 8;

  digest[0] = opad[0];
  digest[1] = opad[1];
  digest[2] = opad[2];
  digest[3] = opad[3];
  digest[4] = opad[4];

  sha1_transform_S (w0, w1, w2, w3, digest);
}

void sha1_transform_V (const u32x w0[4], const u32x w1[4], const u32x w2[4], const u32x w3[4], u32x digest[5])
{
  u32x A = digest[0];
  u32x B = digest[1];
  u32x C = digest[2];
  u32x D = digest[3];
  u32x E = digest[4];

  u32x w0_t = w0[0];
  u32x w1_t = w0[1];
  u32x w2_t = w0[2];
  u32x w3_t = w0[3];
  u32x w4_t = w1[0];
  u32x w5_t = w1[1];
  u32x w6_t = w1[2];
  u32x w7_t = w1[3];
  u32x w8_t = w2[0];
  u32x w9_t = w2[1];
  u32x wa_t = w2[2];
  u32x wb_t = w2[3];
  u32x wc_t = w3[0];
  u32x wd_t = w3[1];
  u32x we_t = w3[2];
  u32x wf_t = w3[3];

  #undef K
  #define K SHA1C00

  SHA1_STEP (SHA1_F0o, A, B, C, D, E, w0_t);
  SHA1_STEP (SHA1_F0o, E, A, B, C, D, w1_t);
  SHA1_STEP (SHA1_F0o, D, E, A, B, C, w2_t);
  SHA1_STEP (SHA1_F0o, C, D, E, A, B, w3_t);
  SHA1_STEP (SHA1_F0o, B, C, D, E, A, w4_t);
  SHA1_STEP (SHA1_F0o, A, B, C, D, E, w5_t);
  SHA1_STEP (SHA1_F0o, E, A, B, C, D, w6_t);
  SHA1_STEP (SHA1_F0o, D, E, A, B, C, w7_t);
  SHA1_STEP (SHA1_F0o, C, D, E, A, B, w8_t);
  SHA1_STEP (SHA1_F0o, B, C, D, E, A, w9_t);
  SHA1_STEP (SHA1_F0o, A, B, C, D, E, wa_t);
  SHA1_STEP (SHA1_F0o, E, A, B, C, D, wb_t);
  SHA1_STEP (SHA1_F0o, D, E, A, B, C, wc_t);
  SHA1_STEP (SHA1_F0o, C, D, E, A, B, wd_t);
  SHA1_STEP (SHA1_F0o, B, C, D, E, A, we_t);
  SHA1_STEP (SHA1_F0o, A, B, C, D, E, wf_t);
  w0_t = rotl32 ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP (SHA1_F0o, E, A, B, C, D, w0_t);
  w1_t = rotl32 ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP (SHA1_F0o, D, E, A, B, C, w1_t);
  w2_t = rotl32 ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP (SHA1_F0o, C, D, E, A, B, w2_t);
  w3_t = rotl32 ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP (SHA1_F0o, B, C, D, E, A, w3_t);

  #undef K
  #define K SHA1C01

  w4_t = rotl32 ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w4_t);
  w5_t = rotl32 ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, w5_t);
  w6_t = rotl32 ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w6_t);
  w7_t = rotl32 ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w7_t);
  w8_t = rotl32 ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w8_t);
  w9_t = rotl32 ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w9_t);
  wa_t = rotl32 ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, wa_t);
  wb_t = rotl32 ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, wb_t);
  wc_t = rotl32 ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, wc_t);
  wd_t = rotl32 ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, wd_t);
  we_t = rotl32 ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, we_t);
  wf_t = rotl32 ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, wf_t);
  w0_t = rotl32 ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w0_t);
  w1_t = rotl32 ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w1_t);
  w2_t = rotl32 ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w2_t);
  w3_t = rotl32 ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w3_t);
  w4_t = rotl32 ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, w4_t);
  w5_t = rotl32 ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w5_t);
  w6_t = rotl32 ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w6_t);
  w7_t = rotl32 ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w7_t);

  #undef K
  #define K SHA1C02

  w8_t = rotl32 ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP (SHA1_F2o, A, B, C, D, E, w8_t);
  w9_t = rotl32 ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP (SHA1_F2o, E, A, B, C, D, w9_t);
  wa_t = rotl32 ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP (SHA1_F2o, D, E, A, B, C, wa_t);
  wb_t = rotl32 ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP (SHA1_F2o, C, D, E, A, B, wb_t);
  wc_t = rotl32 ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP (SHA1_F2o, B, C, D, E, A, wc_t);
  wd_t = rotl32 ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP (SHA1_F2o, A, B, C, D, E, wd_t);
  we_t = rotl32 ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP (SHA1_F2o, E, A, B, C, D, we_t);
  wf_t = rotl32 ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP (SHA1_F2o, D, E, A, B, C, wf_t);
  w0_t = rotl32 ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP (SHA1_F2o, C, D, E, A, B, w0_t);
  w1_t = rotl32 ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP (SHA1_F2o, B, C, D, E, A, w1_t);
  w2_t = rotl32 ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP (SHA1_F2o, A, B, C, D, E, w2_t);
  w3_t = rotl32 ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP (SHA1_F2o, E, A, B, C, D, w3_t);
  w4_t = rotl32 ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP (SHA1_F2o, D, E, A, B, C, w4_t);
  w5_t = rotl32 ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP (SHA1_F2o, C, D, E, A, B, w5_t);
  w6_t = rotl32 ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP (SHA1_F2o, B, C, D, E, A, w6_t);
  w7_t = rotl32 ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP (SHA1_F2o, A, B, C, D, E, w7_t);
  w8_t = rotl32 ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP (SHA1_F2o, E, A, B, C, D, w8_t);
  w9_t = rotl32 ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP (SHA1_F2o, D, E, A, B, C, w9_t);
  wa_t = rotl32 ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP (SHA1_F2o, C, D, E, A, B, wa_t);
  wb_t = rotl32 ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP (SHA1_F2o, B, C, D, E, A, wb_t);

  #undef K
  #define K SHA1C03

  wc_t = rotl32 ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, wc_t);
  wd_t = rotl32 ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, wd_t);
  we_t = rotl32 ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, we_t);
  wf_t = rotl32 ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, wf_t);
  w0_t = rotl32 ((wd_t ^ w8_t ^ w2_t ^ w0_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w0_t);
  w1_t = rotl32 ((we_t ^ w9_t ^ w3_t ^ w1_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w1_t);
  w2_t = rotl32 ((wf_t ^ wa_t ^ w4_t ^ w2_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, w2_t);
  w3_t = rotl32 ((w0_t ^ wb_t ^ w5_t ^ w3_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w3_t);
  w4_t = rotl32 ((w1_t ^ wc_t ^ w6_t ^ w4_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w4_t);
  w5_t = rotl32 ((w2_t ^ wd_t ^ w7_t ^ w5_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, w5_t);
  w6_t = rotl32 ((w3_t ^ we_t ^ w8_t ^ w6_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, w6_t);
  w7_t = rotl32 ((w4_t ^ wf_t ^ w9_t ^ w7_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, w7_t);
  w8_t = rotl32 ((w5_t ^ w0_t ^ wa_t ^ w8_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, w8_t);
  w9_t = rotl32 ((w6_t ^ w1_t ^ wb_t ^ w9_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, w9_t);
  wa_t = rotl32 ((w7_t ^ w2_t ^ wc_t ^ wa_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, wa_t);
  wb_t = rotl32 ((w8_t ^ w3_t ^ wd_t ^ wb_t), 1u); SHA1_STEP (SHA1_F1, A, B, C, D, E, wb_t);
  wc_t = rotl32 ((w9_t ^ w4_t ^ we_t ^ wc_t), 1u); SHA1_STEP (SHA1_F1, E, A, B, C, D, wc_t);
  wd_t = rotl32 ((wa_t ^ w5_t ^ wf_t ^ wd_t), 1u); SHA1_STEP (SHA1_F1, D, E, A, B, C, wd_t);
  we_t = rotl32 ((wb_t ^ w6_t ^ w0_t ^ we_t), 1u); SHA1_STEP (SHA1_F1, C, D, E, A, B, we_t);
  wf_t = rotl32 ((wc_t ^ w7_t ^ w1_t ^ wf_t), 1u); SHA1_STEP (SHA1_F1, B, C, D, E, A, wf_t);

  digest[0] += A;
  digest[1] += B;
  digest[2] += C;
  digest[3] += D;
  digest[4] += E;
}

void hmac_sha1_run_V (u32x w0[4], u32x w1[4], u32x w2[4], u32x w3[4], u32x ipad[5], u32x opad[5], u32x digest[5])
{
  digest[0] = ipad[0];
  digest[1] = ipad[1];
  digest[2] = ipad[2];
  digest[3] = ipad[3];
  digest[4] = ipad[4];

  sha1_transform_V (w0, w1, w2, w3, digest);

  w0[0] = digest[0];
  w0[1] = digest[1];
  w0[2] = digest[2];
  w0[3] = digest[3];
  w1[0] = digest[4];
  w1[1] = 0x80000000;
  w1[2] = 0;
  w1[3] = 0;
  w2[0] = 0;
  w2[1] = 0;
  w2[2] = 0;
  w2[3] = 0;
  w3[0] = 0;
  w3[1] = 0;
  w3[2] = 0;
  w3[3] = (64 + 20) * 8;

  digest[0] = opad[0];
  digest[1] = opad[1];
  digest[2] = opad[2];
  digest[3] = opad[3];
  digest[4] = opad[4];

  sha1_transform_V (w0, w1, w2, w3, digest);
}

__kernel void m02500_init (__global pw_t *pws, __global kernel_rule_t *rules_buf, __global comb_t *combs_buf, __global bf_t *bfs_buf, __global wpa_tmp_t *tmps, __global void *hooks, __global u32 *bitmaps_buf_s1_a, __global u32 *bitmaps_buf_s1_b, __global u32 *bitmaps_buf_s1_c, __global u32 *bitmaps_buf_s1_d, __global u32 *bitmaps_buf_s2_a, __global u32 *bitmaps_buf_s2_b, __global u32 *bitmaps_buf_s2_c, __global u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global digest_t *digests_buf, __global u32 *hashes_shown, __global salt_t *salt_bufs, __global wpa_t *wpa_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  /**
   * base
   */

  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  u32 w0[4];

  w0[0] = pws[gid].i[ 0];
  w0[1] = pws[gid].i[ 1];
  w0[2] = pws[gid].i[ 2];
  w0[3] = pws[gid].i[ 3];

  u32 w1[4];

  w1[0] = pws[gid].i[ 4];
  w1[1] = pws[gid].i[ 5];
  w1[2] = pws[gid].i[ 6];
  w1[3] = pws[gid].i[ 7];

  u32 w2[4];

  w2[0] = pws[gid].i[ 8];
  w2[1] = pws[gid].i[ 9];
  w2[2] = pws[gid].i[10];
  w2[3] = pws[gid].i[11];

  u32 w3[4];

  w3[0] = pws[gid].i[12];
  w3[1] = pws[gid].i[13];
  w3[2] = pws[gid].i[14];
  w3[3] = pws[gid].i[15];

  /**
   * salt
   */

  u32 salt_len = salt_bufs[salt_pos].salt_len;

  u32 salt_buf0[4];
  u32 salt_buf1[4];

  salt_buf0[0] = salt_bufs[salt_pos].salt_buf[0];
  salt_buf0[1] = salt_bufs[salt_pos].salt_buf[1];
  salt_buf0[2] = salt_bufs[salt_pos].salt_buf[2];
  salt_buf0[3] = salt_bufs[salt_pos].salt_buf[3];
  salt_buf1[0] = salt_bufs[salt_pos].salt_buf[4];
  salt_buf1[1] = salt_bufs[salt_pos].salt_buf[5];
  salt_buf1[2] = salt_bufs[salt_pos].salt_buf[6];
  salt_buf1[3] = salt_bufs[salt_pos].salt_buf[7];

  /**
   * pads
   */

  w0[0] = swap32_S (w0[0]);
  w0[1] = swap32_S (w0[1]);
  w0[2] = swap32_S (w0[2]);
  w0[3] = swap32_S (w0[3]);
  w1[0] = swap32_S (w1[0]);
  w1[1] = swap32_S (w1[1]);
  w1[2] = swap32_S (w1[2]);
  w1[3] = swap32_S (w1[3]);
  w2[0] = swap32_S (w2[0]);
  w2[1] = swap32_S (w2[1]);
  w2[2] = swap32_S (w2[2]);
  w2[3] = swap32_S (w2[3]);
  w3[0] = swap32_S (w3[0]);
  w3[1] = swap32_S (w3[1]);
  w3[2] = swap32_S (w3[2]);
  w3[3] = swap32_S (w3[3]);

  u32 ipad[5];
  u32 opad[5];

  hmac_sha1_pad_S (w0, w1, w2, w3, ipad, opad);

  tmps[gid].ipad[0] = ipad[0];
  tmps[gid].ipad[1] = ipad[1];
  tmps[gid].ipad[2] = ipad[2];
  tmps[gid].ipad[3] = ipad[3];
  tmps[gid].ipad[4] = ipad[4];

  tmps[gid].opad[0] = opad[0];
  tmps[gid].opad[1] = opad[1];
  tmps[gid].opad[2] = opad[2];
  tmps[gid].opad[3] = opad[3];
  tmps[gid].opad[4] = opad[4];

  for (u32 i = 0, j = 1; i < 8; i += 5, j += 1)
  {
    w0[0] = salt_buf0[0];
    w0[1] = salt_buf0[1];
    w0[2] = salt_buf0[2];
    w0[3] = salt_buf0[3];
    w1[0] = salt_buf1[0];
    w1[1] = salt_buf1[1];
    w1[2] = salt_buf1[2];
    w1[3] = salt_buf1[3];
    w2[0] = 0;
    w2[1] = 0;
    w2[2] = 0;
    w2[3] = 0;
    w3[0] = 0;
    w3[1] = 0;
    w3[2] = 0;
    w3[3] = 0;

    if (j == 1)
      append_0x01_3x4_S (w0, w1, w2, salt_len + 3);
    else
      append_0x02_3x4_S (w0, w1, w2, salt_len + 3);

    append_0x80_3x4_S (w0, w1, w2, salt_len + 4);

    w0[0] = swap32_S (w0[0]);
    w0[1] = swap32_S (w0[1]);
    w0[2] = swap32_S (w0[2]);
    w0[3] = swap32_S (w0[3]);
    w1[0] = swap32_S (w1[0]);
    w1[1] = swap32_S (w1[1]);
    w1[2] = swap32_S (w1[2]);
    w1[3] = swap32_S (w1[3]);
    w2[0] = swap32_S (w2[0]);
    w2[1] = swap32_S (w2[1]);
    w2[2] = 0;
    w2[3] = 0;
    w3[0] = 0;
    w3[1] = 0;
    w3[2] = 0;
    w3[3] = (64 + salt_len + 4) * 8;

    u32 dgst[5];

    hmac_sha1_run_S (w0, w1, w2, w3, ipad, opad, dgst);

    tmps[gid].dgst[i + 0] = dgst[0];
    tmps[gid].dgst[i + 1] = dgst[1];
    tmps[gid].dgst[i + 2] = dgst[2];
    tmps[gid].dgst[i + 3] = dgst[3];
    tmps[gid].dgst[i + 4] = dgst[4];

    tmps[gid].out[i + 0] = dgst[0];
    tmps[gid].out[i + 1] = dgst[1];
    tmps[gid].out[i + 2] = dgst[2];
    tmps[gid].out[i + 3] = dgst[3];
    tmps[gid].out[i + 4] = dgst[4];
  }
}

__kernel void m02500_loop (__global pw_t *pws, __global kernel_rule_t *rules_buf, __global comb_t *combs_buf, __global bf_t *bfs_buf, __global wpa_tmp_t *tmps, __global void *hooks, __global u32 *bitmaps_buf_s1_a, __global u32 *bitmaps_buf_s1_b, __global u32 *bitmaps_buf_s1_c, __global u32 *bitmaps_buf_s1_d, __global u32 *bitmaps_buf_s2_a, __global u32 *bitmaps_buf_s2_b, __global u32 *bitmaps_buf_s2_c, __global u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global digest_t *digests_buf, __global u32 *hashes_shown, __global salt_t *salt_bufs, __global wpa_t *wpa_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  u32x ipad[5];
  u32x opad[5];

  ipad[0] = packv (tmps, ipad, gid, 0);
  ipad[1] = packv (tmps, ipad, gid, 1);
  ipad[2] = packv (tmps, ipad, gid, 2);
  ipad[3] = packv (tmps, ipad, gid, 3);
  ipad[4] = packv (tmps, ipad, gid, 4);

  opad[0] = packv (tmps, opad, gid, 0);
  opad[1] = packv (tmps, opad, gid, 1);
  opad[2] = packv (tmps, opad, gid, 2);
  opad[3] = packv (tmps, opad, gid, 3);
  opad[4] = packv (tmps, opad, gid, 4);

  for (u32 i = 0; i < 8; i += 5)
  {
    u32x dgst[5];
    u32x out[5];

    dgst[0] = packv (tmps, dgst, gid, i + 0);
    dgst[1] = packv (tmps, dgst, gid, i + 1);
    dgst[2] = packv (tmps, dgst, gid, i + 2);
    dgst[3] = packv (tmps, dgst, gid, i + 3);
    dgst[4] = packv (tmps, dgst, gid, i + 4);

    out[0] = packv (tmps, out, gid, i + 0);
    out[1] = packv (tmps, out, gid, i + 1);
    out[2] = packv (tmps, out, gid, i + 2);
    out[3] = packv (tmps, out, gid, i + 3);
    out[4] = packv (tmps, out, gid, i + 4);

    for (u32 j = 0; j < loop_cnt; j++)
    {
      u32x w0[4];
      u32x w1[4];
      u32x w2[4];
      u32x w3[4];

      w0[0] = dgst[0];
      w0[1] = dgst[1];
      w0[2] = dgst[2];
      w0[3] = dgst[3];
      w1[0] = dgst[4];
      w1[1] = 0x80000000;
      w1[2] = 0;
      w1[3] = 0;
      w2[0] = 0;
      w2[1] = 0;
      w2[2] = 0;
      w2[3] = 0;
      w3[0] = 0;
      w3[1] = 0;
      w3[2] = 0;
      w3[3] = (64 + 20) * 8;

      hmac_sha1_run_V (w0, w1, w2, w3, ipad, opad, dgst);

      out[0] ^= dgst[0];
      out[1] ^= dgst[1];
      out[2] ^= dgst[2];
      out[3] ^= dgst[3];
      out[4] ^= dgst[4];
    }

    unpackv (tmps, dgst, gid, i + 0, dgst[0]);
    unpackv (tmps, dgst, gid, i + 1, dgst[1]);
    unpackv (tmps, dgst, gid, i + 2, dgst[2]);
    unpackv (tmps, dgst, gid, i + 3, dgst[3]);
    unpackv (tmps, dgst, gid, i + 4, dgst[4]);

    unpackv (tmps, out, gid, i + 0, out[0]);
    unpackv (tmps, out, gid, i + 1, out[1]);
    unpackv (tmps, out, gid, i + 2, out[2]);
    unpackv (tmps, out, gid, i + 3, out[3]);
    unpackv (tmps, out, gid, i + 4, out[4]);
  }
}

__kernel void m02500_comp (__global pw_t *pws, __global kernel_rule_t *rules_buf, __global comb_t *combs_buf, __global bf_t *bfs_buf, __global wpa_tmp_t *tmps, __global void *hooks, __global u32 *bitmaps_buf_s1_a, __global u32 *bitmaps_buf_s1_b, __global u32 *bitmaps_buf_s1_c, __global u32 *bitmaps_buf_s1_d, __global u32 *bitmaps_buf_s2_a, __global u32 *bitmaps_buf_s2_b, __global u32 *bitmaps_buf_s2_c, __global u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global digest_t *digests_buf, __global u32 *hashes_shown, __global salt_t *salt_bufs, __global wpa_t *wpa_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  const u32 lid = get_local_id (0);

  u32 w0[4];
  u32 w1[4];
  u32 w2[4];
  u32 w3[4];

  w0[0] = tmps[gid].out[0];
  w0[1] = tmps[gid].out[1];
  w0[2] = tmps[gid].out[2];
  w0[3] = tmps[gid].out[3];
  w1[0] = tmps[gid].out[4];
  w1[1] = tmps[gid].out[5];
  w1[2] = tmps[gid].out[6];
  w1[3] = tmps[gid].out[7];
  w2[0] = 0;
  w2[1] = 0;
  w2[2] = 0;
  w2[3] = 0;
  w3[0] = 0;
  w3[1] = 0;
  w3[2] = 0;
  w3[3] = 0;

  u32 ipad[5];
  u32 opad[5];

  hmac_sha1_pad_S (w0, w1, w2, w3, ipad, opad);

  w0[0] = wpa_bufs[salt_pos].pke[ 0];
  w0[1] = wpa_bufs[salt_pos].pke[ 1];
  w0[2] = wpa_bufs[salt_pos].pke[ 2];
  w0[3] = wpa_bufs[salt_pos].pke[ 3];
  w1[0] = wpa_bufs[salt_pos].pke[ 4];
  w1[1] = wpa_bufs[salt_pos].pke[ 5];
  w1[2] = wpa_bufs[salt_pos].pke[ 6];
  w1[3] = wpa_bufs[salt_pos].pke[ 7];
  w2[0] = wpa_bufs[salt_pos].pke[ 8];
  w2[1] = wpa_bufs[salt_pos].pke[ 9];
  w2[2] = wpa_bufs[salt_pos].pke[10];
  w2[3] = wpa_bufs[salt_pos].pke[11];
  w3[0] = wpa_bufs[salt_pos].pke[12];
  w3[1] = wpa_bufs[salt_pos].pke[13];
  w3[2] = wpa_bufs[salt_pos].pke[14];
  w3[3] = wpa_bufs[salt_pos].pke[15];

  sha1_transform_S (w0, w1, w2, w3, ipad);

  w0[0] = wpa_bufs[salt_pos].pke[16];
  w0[1] = wpa_bufs[salt_pos].pke[17];
  w0[2] = wpa_bufs[salt_pos].pke[18];
  w0[3] = wpa_bufs[salt_pos].pke[19];
  w1[0] = wpa_bufs[salt_pos].pke[20];
  w1[1] = wpa_bufs[salt_pos].pke[21];
  w1[2] = wpa_bufs[salt_pos].pke[22];
  w1[3] = wpa_bufs[salt_pos].pke[23];
  w2[0] = wpa_bufs[salt_pos].pke[24];
  w2[1] = 0x80000000;
  w2[2] = 0;
  w2[3] = 0;
  w3[0] = 0;
  w3[1] = 0;
  w3[2] = 0;
  w3[3] = (64 + 100) * 8;

  u32 digest[5];

  hmac_sha1_run_S (w0, w1, w2, w3, ipad, opad, digest);

  {
    w0[0] = swap32_S (digest[0]);
    w0[1] = swap32_S (digest[1]);
    w0[2] = swap32_S (digest[2]);
    w0[3] = swap32_S (digest[3]);
    w1[0] = 0;
    w1[1] = 0;
    w1[2] = 0;
    w1[3] = 0;
    w2[0] = 0;
    w2[1] = 0;
    w2[2] = 0;
    w2[3] = 0;
    w3[0] = 0;
    w3[1] = 0;
    w3[2] = 0;
    w3[3] = 0;

    hmac_md5_pad_S (w0, w1, w2, w3, ipad, opad);

    int eapol_size = wpa_bufs[salt_pos].eapol_size;

    int eapol_left;
    int eapol_off;

    for (eapol_left = eapol_size, eapol_off = 0; eapol_left >= 56; eapol_left -= 64, eapol_off += 16)
    {
      w0[0] = wpa_bufs[salt_pos].eapol[eapol_off +  0];
      w0[1] = wpa_bufs[salt_pos].eapol[eapol_off +  1];
      w0[2] = wpa_bufs[salt_pos].eapol[eapol_off +  2];
      w0[3] = wpa_bufs[salt_pos].eapol[eapol_off +  3];
      w1[0] = wpa_bufs[salt_pos].eapol[eapol_off +  4];
      w1[1] = wpa_bufs[salt_pos].eapol[eapol_off +  5];
      w1[2] = wpa_bufs[salt_pos].eapol[eapol_off +  6];
      w1[3] = wpa_bufs[salt_pos].eapol[eapol_off +  7];
      w2[0] = wpa_bufs[salt_pos].eapol[eapol_off +  8];
      w2[1] = wpa_bufs[salt_pos].eapol[eapol_off +  9];
      w2[2] = wpa_bufs[salt_pos].eapol[eapol_off + 10];
      w2[3] = wpa_bufs[salt_pos].eapol[eapol_off + 11];
      w3[0] = wpa_bufs[salt_pos].eapol[eapol_off + 12];
      w3[1] = wpa_bufs[salt_pos].eapol[eapol_off + 13];
      w3[2] = wpa_bufs[salt_pos].eapol[eapol_off + 14];
      w3[3] = wpa_bufs[salt_pos].eapol[eapol_off + 15];

      md5_transform_S (w0, w1, w2, w3, ipad);
    }

    w0[0] = wpa_bufs[salt_pos].eapol[eapol_off +  0];
    w0[1] = wpa_bufs[salt_pos].eapol[eapol_off +  1];
    w0[2] = wpa_bufs[salt_pos].eapol[eapol_off +  2];
    w0[3] = wpa_bufs[salt_pos].eapol[eapol_off +  3];
    w1[0] = wpa_bufs[salt_pos].eapol[eapol_off +  4];
    w1[1] = wpa_bufs[salt_pos].eapol[eapol_off +  5];
    w1[2] = wpa_bufs[salt_pos].eapol[eapol_off +  6];
    w1[3] = wpa_bufs[salt_pos].eapol[eapol_off +  7];
    w2[0] = wpa_bufs[salt_pos].eapol[eapol_off +  8];
    w2[1] = wpa_bufs[salt_pos].eapol[eapol_off +  9];
    w2[2] = wpa_bufs[salt_pos].eapol[eapol_off + 10];
    w2[3] = wpa_bufs[salt_pos].eapol[eapol_off + 11];
    w3[0] = wpa_bufs[salt_pos].eapol[eapol_off + 12];
    w3[1] = wpa_bufs[salt_pos].eapol[eapol_off + 13];
    w3[2] = (64 + eapol_size) * 8;
    w3[3] = 0;

    u32 digest1[4];

    hmac_md5_run_S (w0, w1, w2, w3, ipad, opad, digest1);

    /**
     * base
     */

    #define il_pos 0

    const u32 r0 = digest1[DGST_R0];
    const u32 r1 = digest1[DGST_R1];
    const u32 r2 = digest1[DGST_R2];
    const u32 r3 = digest1[DGST_R3];

    #include COMPARE_M
  }

  {
    w0[0] = digest[0];
    w0[1] = digest[1];
    w0[2] = digest[2];
    w0[3] = digest[3];
    w1[0] = 0;
    w1[1] = 0;
    w1[2] = 0;
    w1[3] = 0;
    w2[0] = 0;
    w2[1] = 0;
    w2[2] = 0;
    w2[3] = 0;
    w3[0] = 0;
    w3[1] = 0;
    w3[2] = 0;
    w3[3] = 0;

    hmac_sha1_pad_S (w0, w1, w2, w3, ipad, opad);

    int eapol_size = wpa_bufs[salt_pos].eapol_size;

    int eapol_left;
    int eapol_off;

    for (eapol_left = eapol_size, eapol_off = 0; eapol_left >= 56; eapol_left -= 64, eapol_off += 16)
    {
      w0[0] = wpa_bufs[salt_pos].eapol[eapol_off +  0];
      w0[1] = wpa_bufs[salt_pos].eapol[eapol_off +  1];
      w0[2] = wpa_bufs[salt_pos].eapol[eapol_off +  2];
      w0[3] = wpa_bufs[salt_pos].eapol[eapol_off +  3];
      w1[0] = wpa_bufs[salt_pos].eapol[eapol_off +  4];
      w1[1] = wpa_bufs[salt_pos].eapol[eapol_off +  5];
      w1[2] = wpa_bufs[salt_pos].eapol[eapol_off +  6];
      w1[3] = wpa_bufs[salt_pos].eapol[eapol_off +  7];
      w2[0] = wpa_bufs[salt_pos].eapol[eapol_off +  8];
      w2[1] = wpa_bufs[salt_pos].eapol[eapol_off +  9];
      w2[2] = wpa_bufs[salt_pos].eapol[eapol_off + 10];
      w2[3] = wpa_bufs[salt_pos].eapol[eapol_off + 11];
      w3[0] = wpa_bufs[salt_pos].eapol[eapol_off + 12];
      w3[1] = wpa_bufs[salt_pos].eapol[eapol_off + 13];
      w3[2] = wpa_bufs[salt_pos].eapol[eapol_off + 14];
      w3[3] = wpa_bufs[salt_pos].eapol[eapol_off + 15];

      sha1_transform_S (w0, w1, w2, w3, ipad);
    }

    w0[0] = wpa_bufs[salt_pos].eapol[eapol_off +  0];
    w0[1] = wpa_bufs[salt_pos].eapol[eapol_off +  1];
    w0[2] = wpa_bufs[salt_pos].eapol[eapol_off +  2];
    w0[3] = wpa_bufs[salt_pos].eapol[eapol_off +  3];
    w1[0] = wpa_bufs[salt_pos].eapol[eapol_off +  4];
    w1[1] = wpa_bufs[salt_pos].eapol[eapol_off +  5];
    w1[2] = wpa_bufs[salt_pos].eapol[eapol_off +  6];
    w1[3] = wpa_bufs[salt_pos].eapol[eapol_off +  7];
    w2[0] = wpa_bufs[salt_pos].eapol[eapol_off +  8];
    w2[1] = wpa_bufs[salt_pos].eapol[eapol_off +  9];
    w2[2] = wpa_bufs[salt_pos].eapol[eapol_off + 10];
    w2[3] = wpa_bufs[salt_pos].eapol[eapol_off + 11];
    w3[0] = wpa_bufs[salt_pos].eapol[eapol_off + 12];
    w3[1] = wpa_bufs[salt_pos].eapol[eapol_off + 13];
    w3[2] = 0;
    w3[3] = (64 + eapol_size) * 8;

    u32 digest2[5];

    hmac_sha1_run_S (w0, w1, w2, w3, ipad, opad, digest2);

    /**
     * base
     */

    #define il_pos 0

    const u32 r0 = digest2[DGST_R0];
    const u32 r1 = digest2[DGST_R1];
    const u32 r2 = digest2[DGST_R2];
    const u32 r3 = digest2[DGST_R3];

    #include COMPARE_M
  }
}
