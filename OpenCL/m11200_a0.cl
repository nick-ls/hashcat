/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

//#define NEW_SIMD_CODE

#include "inc_vendor.cl"
#include "inc_hash_constants.h"
#include "inc_hash_functions.cl"
#include "inc_types.cl"
#include "inc_common.cl"
#include "inc_rp_optimized.h"
#include "inc_rp_optimized.cl"
#include "inc_scalar.cl"
#include "inc_hash_sha1.cl"

__kernel void m11200_mxx (__global pw_t *pws, __global const kernel_rule_t *rules_buf, __global const pw_t *combs_buf, __global const bf_t *bfs_buf, __global void *tmps, __global void *hooks, __global const u32 *bitmaps_buf_s1_a, __global const u32 *bitmaps_buf_s1_b, __global const u32 *bitmaps_buf_s1_c, __global const u32 *bitmaps_buf_s1_d, __global const u32 *bitmaps_buf_s2_a, __global const u32 *bitmaps_buf_s2_b, __global const u32 *bitmaps_buf_s2_c, __global const u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global const digest_t *digests_buf, __global u32 *hashes_shown, __global const salt_t *salt_bufs, __global const void *esalt_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  /**
   * modifier
   */

  const u32 lid = get_local_id (0);
  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  /**
   * base
   */

  const u32 pw_len = pws[gid].pw_len;

  const u32 pw_lenv = ceil ((float) pw_len / 4);

  u32 w[64] = { 0 };

  for (int idx = 0; idx < pw_lenv; idx++)
  {
    w[idx] = pws[gid].i[idx];
  }

  sha1_ctx_t ctx0;

  sha1_init (&ctx0);

  sha1_update_global_swap (&ctx0, salt_bufs[salt_pos].salt_buf, salt_bufs[salt_pos].salt_len);

  /**
   * loop
   */

  for (u32 il_pos = 0; il_pos < il_cnt; il_pos++)
  {
    // todo: add rules engine

    sha1_ctx_t ctx2;

    sha1_init (&ctx2);

    sha1_update_swap (&ctx2, w, pw_len);

    sha1_final (&ctx2);

    u32 a = ctx2.h[0];
    u32 b = ctx2.h[1];
    u32 c = ctx2.h[2];
    u32 d = ctx2.h[3];
    u32 e = ctx2.h[4];

    const u32 a_sav = a;
    const u32 b_sav = b;
    const u32 c_sav = c;
    const u32 d_sav = d;
    const u32 e_sav = e;

    sha1_ctx_t ctx1;

    sha1_init (&ctx1);

    ctx1.w0[0] = a;
    ctx1.w0[1] = b;
    ctx1.w0[2] = c;
    ctx1.w0[3] = d;
    ctx1.w1[0] = e;

    ctx1.len = 20;

    sha1_final (&ctx1);

    a = ctx1.h[0];
    b = ctx1.h[1];
    c = ctx1.h[2];
    d = ctx1.h[3];
    e = ctx1.h[4];

    sha1_ctx_t ctx = ctx0;

    u32 w0[4];
    u32 w1[4];
    u32 w2[4];
    u32 w3[4];

    w0[0] = a;
    w0[1] = b;
    w0[2] = c;
    w0[3] = d;
    w1[0] = e;
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

    sha1_update_64 (&ctx, w0, w1, w2, w3, 20);

    sha1_final (&ctx);

    ctx.h[0] ^= a_sav;
    ctx.h[1] ^= b_sav;
    ctx.h[2] ^= c_sav;
    ctx.h[3] ^= d_sav;
    ctx.h[4] ^= e_sav;

    const u32 r0 = ctx.h[DGST_R0];
    const u32 r1 = ctx.h[DGST_R1];
    const u32 r2 = ctx.h[DGST_R2];
    const u32 r3 = ctx.h[DGST_R3];

    COMPARE_M_SCALAR (r0, r1, r2, r3);
  }
}

__kernel void m11200_sxx (__global pw_t *pws, __global const kernel_rule_t *rules_buf, __global const pw_t *combs_buf, __global const bf_t *bfs_buf, __global void *tmps, __global void *hooks, __global const u32 *bitmaps_buf_s1_a, __global const u32 *bitmaps_buf_s1_b, __global const u32 *bitmaps_buf_s1_c, __global const u32 *bitmaps_buf_s1_d, __global const u32 *bitmaps_buf_s2_a, __global const u32 *bitmaps_buf_s2_b, __global const u32 *bitmaps_buf_s2_c, __global const u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global const digest_t *digests_buf, __global u32 *hashes_shown, __global const salt_t *salt_bufs, __global const void *esalt_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  /**
   * modifier
   */

  const u32 lid = get_local_id (0);
  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  /**
   * digest
   */

  const u32 search[4] =
  {
    digests_buf[digests_offset].digest_buf[DGST_R0],
    digests_buf[digests_offset].digest_buf[DGST_R1],
    digests_buf[digests_offset].digest_buf[DGST_R2],
    digests_buf[digests_offset].digest_buf[DGST_R3]
  };

  /**
   * base
   */

  const u32 pw_len = pws[gid].pw_len;

  const u32 pw_lenv = ceil ((float) pw_len / 4);

  u32 w[64] = { 0 };

  for (int idx = 0; idx < pw_lenv; idx++)
  {
    w[idx] = pws[gid].i[idx];
  }

  sha1_ctx_t ctx0;

  sha1_init (&ctx0);

  sha1_update_global_swap (&ctx0, salt_bufs[salt_pos].salt_buf, salt_bufs[salt_pos].salt_len);

  /**
   * loop
   */

  for (u32 il_pos = 0; il_pos < il_cnt; il_pos++)
  {
    // todo: add rules engine

    sha1_ctx_t ctx2;

    sha1_init (&ctx2);

    sha1_update_swap (&ctx2, w, pw_len);

    sha1_final (&ctx2);

    u32 a = ctx2.h[0];
    u32 b = ctx2.h[1];
    u32 c = ctx2.h[2];
    u32 d = ctx2.h[3];
    u32 e = ctx2.h[4];

    const u32 a_sav = a;
    const u32 b_sav = b;
    const u32 c_sav = c;
    const u32 d_sav = d;
    const u32 e_sav = e;

    sha1_ctx_t ctx1;

    sha1_init (&ctx1);

    ctx1.w0[0] = a;
    ctx1.w0[1] = b;
    ctx1.w0[2] = c;
    ctx1.w0[3] = d;
    ctx1.w1[0] = e;

    ctx1.len = 20;

    sha1_final (&ctx1);

    a = ctx1.h[0];
    b = ctx1.h[1];
    c = ctx1.h[2];
    d = ctx1.h[3];
    e = ctx1.h[4];

    sha1_ctx_t ctx = ctx0;

    u32 w0[4];
    u32 w1[4];
    u32 w2[4];
    u32 w3[4];

    w0[0] = a;
    w0[1] = b;
    w0[2] = c;
    w0[3] = d;
    w1[0] = e;
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

    sha1_update_64 (&ctx, w0, w1, w2, w3, 20);

    sha1_final (&ctx);

    ctx.h[0] ^= a_sav;
    ctx.h[1] ^= b_sav;
    ctx.h[2] ^= c_sav;
    ctx.h[3] ^= d_sav;
    ctx.h[4] ^= e_sav;

    const u32 r0 = ctx.h[DGST_R0];
    const u32 r1 = ctx.h[DGST_R1];
    const u32 r2 = ctx.h[DGST_R2];
    const u32 r3 = ctx.h[DGST_R3];

    COMPARE_S_SCALAR (r0, r1, r2, r3);
  }
}
