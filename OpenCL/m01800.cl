/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#include "inc_vendor.cl"
#include "inc_hash_constants.h"
#include "inc_hash_functions.cl"
#include "inc_types.cl"
#include "inc_common.cl"
#include "inc_hash_sha512.cl"

#define COMPARE_S "inc_comp_single.cl"
#define COMPARE_M "inc_comp_multi.cl"

__kernel void m01800_init (__global pw_t *pws, __global const kernel_rule_t *rules_buf, __global const pw_t *combs_buf, __global const bf_t *bfs_buf, __global sha512crypt_tmp_t *tmps, __global void *hooks, __global const u32 *bitmaps_buf_s1_a, __global const u32 *bitmaps_buf_s1_b, __global const u32 *bitmaps_buf_s1_c, __global const u32 *bitmaps_buf_s1_d, __global const u32 *bitmaps_buf_s2_a, __global const u32 *bitmaps_buf_s2_b, __global const u32 *bitmaps_buf_s2_c, __global const u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global const digest_t *digests_buf, __global u32 *hashes_shown, __global const salt_t *salt_bufs, __global const void *esalt_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  /**
   * base
   */

  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  /**
   * init
   */

  const u32 pw_len = pws[gid].pw_len;

  const u32 pw_lenv = ceil ((float) pw_len / 4);

  u32 w[64] = { 0 };

  for (int idx = 0; idx < pw_lenv; idx++)
  {
    w[idx] = pws[gid].i[idx];
  }

  for (int idx = 0; idx < pw_lenv; idx++)
  {
    w[idx] = swap32 (w[idx]);
  }

  const u32 salt_len = salt_bufs[salt_pos].salt_len;

  const u32 salt_lenv = ceil ((float) salt_len / 4);

  u32 s[64] = { 0 };

  for (int idx = 0; idx < salt_lenv; idx++)
  {
    s[idx] = salt_bufs[salt_pos].salt_buf[idx];
  }

  for (int idx = 0; idx < salt_lenv; idx++)
  {
    s[idx] = swap32 (s[idx]);
  }

  /**
   * prepare
   */

  sha512_ctx_t ctx;

  sha512_init (&ctx);

  sha512_update (&ctx, w, pw_len);

  sha512_update (&ctx, s, salt_len);

  sha512_update (&ctx, w, pw_len);

  sha512_final (&ctx);

  u32 final[32] = { 0 };

  final[ 0] = h32_from_64_S (ctx.h[0]);
  final[ 1] = l32_from_64_S (ctx.h[0]);
  final[ 2] = h32_from_64_S (ctx.h[1]);
  final[ 3] = l32_from_64_S (ctx.h[1]);
  final[ 4] = h32_from_64_S (ctx.h[2]);
  final[ 5] = l32_from_64_S (ctx.h[2]);
  final[ 6] = h32_from_64_S (ctx.h[3]);
  final[ 7] = l32_from_64_S (ctx.h[3]);
  final[ 8] = h32_from_64_S (ctx.h[4]);
  final[ 9] = l32_from_64_S (ctx.h[4]);
  final[10] = h32_from_64_S (ctx.h[5]);
  final[11] = l32_from_64_S (ctx.h[5]);
  final[12] = h32_from_64_S (ctx.h[6]);
  final[13] = l32_from_64_S (ctx.h[6]);
  final[14] = h32_from_64_S (ctx.h[7]);
  final[15] = l32_from_64_S (ctx.h[7]);

  // alt_result

  sha512_init (&ctx);

  sha512_update (&ctx, w, pw_len);

  sha512_update (&ctx, s, salt_len);

  int pl;

  for (pl = pw_len; pl > 64; pl -= 64)
  {
    sha512_update (&ctx, final, 64);
  }

  u32 t_final[32] = { 0 };

  #ifdef _unroll
  #pragma unroll
  #endif
  for (int i = 0; i < 16; i++) t_final[i] = final[i];

  truncate_block_16x4_be (t_final + 0, t_final + 4, t_final + 8, t_final + 12, pl);

  sha512_update (&ctx, t_final, pl);

  for (int cnt = pw_len; cnt > 0; cnt >>= 1)
  {
    if ((cnt & 1) != 0)
    {
      sha512_update (&ctx, final, 64);
    }
    else
    {
      sha512_update (&ctx, w, pw_len);
    }
  }

  sha512_final (&ctx);

  tmps[gid].alt_result[ 0] = h32_from_64_S (ctx.h[0]);
  tmps[gid].alt_result[ 1] = l32_from_64_S (ctx.h[0]);
  tmps[gid].alt_result[ 2] = h32_from_64_S (ctx.h[1]);
  tmps[gid].alt_result[ 3] = l32_from_64_S (ctx.h[1]);
  tmps[gid].alt_result[ 4] = h32_from_64_S (ctx.h[2]);
  tmps[gid].alt_result[ 5] = l32_from_64_S (ctx.h[2]);
  tmps[gid].alt_result[ 6] = h32_from_64_S (ctx.h[3]);
  tmps[gid].alt_result[ 7] = l32_from_64_S (ctx.h[3]);
  tmps[gid].alt_result[ 8] = h32_from_64_S (ctx.h[4]);
  tmps[gid].alt_result[ 9] = l32_from_64_S (ctx.h[4]);
  tmps[gid].alt_result[10] = h32_from_64_S (ctx.h[5]);
  tmps[gid].alt_result[11] = l32_from_64_S (ctx.h[5]);
  tmps[gid].alt_result[12] = h32_from_64_S (ctx.h[6]);
  tmps[gid].alt_result[13] = l32_from_64_S (ctx.h[6]);
  tmps[gid].alt_result[14] = h32_from_64_S (ctx.h[7]);
  tmps[gid].alt_result[15] = l32_from_64_S (ctx.h[7]);

  // p_bytes

  sha512_init (&ctx);

  for (u32 j = 0; j < pw_len; j++)
  {
    sha512_update (&ctx, w, pw_len);
  }

  sha512_final (&ctx);

  final[ 0] = h32_from_64_S (ctx.h[0]);
  final[ 1] = l32_from_64_S (ctx.h[0]);
  final[ 2] = h32_from_64_S (ctx.h[1]);
  final[ 3] = l32_from_64_S (ctx.h[1]);
  final[ 4] = h32_from_64_S (ctx.h[2]);
  final[ 5] = l32_from_64_S (ctx.h[2]);
  final[ 6] = h32_from_64_S (ctx.h[3]);
  final[ 7] = l32_from_64_S (ctx.h[3]);
  final[ 8] = h32_from_64_S (ctx.h[4]);
  final[ 9] = l32_from_64_S (ctx.h[4]);
  final[10] = h32_from_64_S (ctx.h[5]);
  final[11] = l32_from_64_S (ctx.h[5]);
  final[12] = h32_from_64_S (ctx.h[6]);
  final[13] = l32_from_64_S (ctx.h[6]);
  final[14] = h32_from_64_S (ctx.h[7]);
  final[15] = l32_from_64_S (ctx.h[7]);

  u32 p_final[64] = { 0 };

  int idx;

  for (pl = pw_len, idx = 0; pl > 64; pl -= 64, idx += 16)
  {
    p_final[idx +  0] = final[ 0];
    p_final[idx +  1] = final[ 1];
    p_final[idx +  2] = final[ 2];
    p_final[idx +  3] = final[ 3];
    p_final[idx +  4] = final[ 4];
    p_final[idx +  5] = final[ 5];
    p_final[idx +  6] = final[ 6];
    p_final[idx +  7] = final[ 7];
    p_final[idx +  8] = final[ 8];
    p_final[idx +  9] = final[ 9];
    p_final[idx + 10] = final[10];
    p_final[idx + 11] = final[11];
    p_final[idx + 12] = final[12];
    p_final[idx + 13] = final[13];
    p_final[idx + 14] = final[14];
    p_final[idx + 15] = final[15];
  }

  truncate_block_16x4_be (final + 0, final + 4, final + 8, final + 12, pl);

  p_final[idx +  0] = final[ 0];
  p_final[idx +  1] = final[ 1];
  p_final[idx +  2] = final[ 2];
  p_final[idx +  3] = final[ 3];
  p_final[idx +  4] = final[ 4];
  p_final[idx +  5] = final[ 5];
  p_final[idx +  6] = final[ 6];
  p_final[idx +  7] = final[ 7];
  p_final[idx +  8] = final[ 8];
  p_final[idx +  9] = final[ 9];
  p_final[idx + 10] = final[10];
  p_final[idx + 11] = final[11];
  p_final[idx + 12] = final[12];
  p_final[idx + 13] = final[13];
  p_final[idx + 14] = final[14];
  p_final[idx + 15] = final[15];

  #ifdef _unroll
  #pragma unroll
  #endif
  for (int i = 0; i < 64; i++) tmps[gid].p_bytes[i] = p_final[i];

  // s_bytes

  sha512_init (&ctx);

  for (u32 j = 0; j < 16 + (tmps[gid].alt_result[0] >> 24); j++)
  {
    sha512_update (&ctx, s, salt_len);
  }

  sha512_final (&ctx);

  final[ 0] = h32_from_64_S (ctx.h[0]);
  final[ 1] = l32_from_64_S (ctx.h[0]);
  final[ 2] = h32_from_64_S (ctx.h[1]);
  final[ 3] = l32_from_64_S (ctx.h[1]);
  final[ 4] = h32_from_64_S (ctx.h[2]);
  final[ 5] = l32_from_64_S (ctx.h[2]);
  final[ 6] = h32_from_64_S (ctx.h[3]);
  final[ 7] = l32_from_64_S (ctx.h[3]);
  final[ 8] = h32_from_64_S (ctx.h[4]);
  final[ 9] = l32_from_64_S (ctx.h[4]);
  final[10] = h32_from_64_S (ctx.h[5]);
  final[11] = l32_from_64_S (ctx.h[5]);
  final[12] = h32_from_64_S (ctx.h[6]);
  final[13] = l32_from_64_S (ctx.h[6]);
  final[14] = h32_from_64_S (ctx.h[7]);
  final[15] = l32_from_64_S (ctx.h[7]);

  u32 s_final[64] = { 0 };

  for (pl = salt_len, idx = 0; pl > 64; pl -= 64, idx += 16)
  {
    s_final[idx +  0] = final[ 0];
    s_final[idx +  1] = final[ 1];
    s_final[idx +  2] = final[ 2];
    s_final[idx +  3] = final[ 3];
    s_final[idx +  4] = final[ 4];
    s_final[idx +  5] = final[ 5];
    s_final[idx +  6] = final[ 6];
    s_final[idx +  7] = final[ 7];
    s_final[idx +  8] = final[ 8];
    s_final[idx +  9] = final[ 9];
    s_final[idx + 10] = final[10];
    s_final[idx + 11] = final[11];
    s_final[idx + 12] = final[12];
    s_final[idx + 13] = final[13];
    s_final[idx + 14] = final[14];
    s_final[idx + 15] = final[15];
  }

  truncate_block_16x4_be (final + 0, final + 4, final + 8, final + 12, pl);

  s_final[idx +  0] = final[ 0];
  s_final[idx +  1] = final[ 1];
  s_final[idx +  2] = final[ 2];
  s_final[idx +  3] = final[ 3];
  s_final[idx +  4] = final[ 4];
  s_final[idx +  5] = final[ 5];
  s_final[idx +  6] = final[ 6];
  s_final[idx +  7] = final[ 7];
  s_final[idx +  8] = final[ 8];
  s_final[idx +  9] = final[ 9];
  s_final[idx + 10] = final[10];
  s_final[idx + 11] = final[11];
  s_final[idx + 12] = final[12];
  s_final[idx + 13] = final[13];
  s_final[idx + 14] = final[14];
  s_final[idx + 15] = final[15];

  #ifdef _unroll
  #pragma unroll
  #endif
  for (int i = 0; i < 64; i++) tmps[gid].s_bytes[i] = s_final[i];
}

__kernel void m01800_loop (__global pw_t *pws, __global const kernel_rule_t *rules_buf, __global const pw_t *combs_buf, __global const bf_t *bfs_buf, __global sha512crypt_tmp_t *tmps, __global void *hooks, __global const u32 *bitmaps_buf_s1_a, __global const u32 *bitmaps_buf_s1_b, __global const u32 *bitmaps_buf_s1_c, __global const u32 *bitmaps_buf_s1_d, __global const u32 *bitmaps_buf_s2_a, __global const u32 *bitmaps_buf_s2_b, __global const u32 *bitmaps_buf_s2_c, __global const u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global const digest_t *digests_buf, __global u32 *hashes_shown, __global const salt_t *salt_bufs, __global const void *esalt_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  /**
   * base
   */

  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  const u32 pw_len = pws[gid].pw_len;

  const u32 salt_len = salt_bufs[salt_pos].salt_len;

  u32 alt_result[32] = { 0 };

  #ifdef _unroll
  #pragma unroll
  #endif
  for (int i = 0; i < 16; i++) alt_result[i] = tmps[gid].alt_result[i];

  /* Repeatedly run the collected hash value through SHA512 to burn
     CPU cycles.  */

  for (u32 i = 0, j = loop_pos; i < loop_cnt; i++, j++)
  {
    sha512_ctx_t ctx;

    sha512_init (&ctx);

    if (j & 1)
    {
      sha512_update_global (&ctx, tmps[gid].p_bytes, pw_len);
    }
    else
    {
      sha512_update (&ctx, alt_result, 64);
    }

    if (j % 3)
    {
      sha512_update_global (&ctx, tmps[gid].s_bytes, salt_len);
    }

    if (j % 7)
    {
      sha512_update_global (&ctx, tmps[gid].p_bytes, pw_len);
    }

    if (j & 1)
    {
      sha512_update (&ctx, alt_result, 64);
    }
    else
    {
      sha512_update_global (&ctx, tmps[gid].p_bytes, pw_len);
    }

    sha512_final (&ctx);

    alt_result[ 0] = h32_from_64_S (ctx.h[0]);
    alt_result[ 1] = l32_from_64_S (ctx.h[0]);
    alt_result[ 2] = h32_from_64_S (ctx.h[1]);
    alt_result[ 3] = l32_from_64_S (ctx.h[1]);
    alt_result[ 4] = h32_from_64_S (ctx.h[2]);
    alt_result[ 5] = l32_from_64_S (ctx.h[2]);
    alt_result[ 6] = h32_from_64_S (ctx.h[3]);
    alt_result[ 7] = l32_from_64_S (ctx.h[3]);
    alt_result[ 8] = h32_from_64_S (ctx.h[4]);
    alt_result[ 9] = l32_from_64_S (ctx.h[4]);
    alt_result[10] = h32_from_64_S (ctx.h[5]);
    alt_result[11] = l32_from_64_S (ctx.h[5]);
    alt_result[12] = h32_from_64_S (ctx.h[6]);
    alt_result[13] = l32_from_64_S (ctx.h[6]);
    alt_result[14] = h32_from_64_S (ctx.h[7]);
    alt_result[15] = l32_from_64_S (ctx.h[7]);
  }

  #ifdef _unroll
  #pragma unroll
  #endif
  for (int i = 0; i < 16; i++) tmps[gid].alt_result[i] = alt_result[i];
}

__kernel void m01800_comp (__global pw_t *pws, __global const kernel_rule_t *rules_buf, __global const pw_t *combs_buf, __global const bf_t *bfs_buf, __global sha512crypt_tmp_t *tmps, __global void *hooks, __global const u32 *bitmaps_buf_s1_a, __global const u32 *bitmaps_buf_s1_b, __global const u32 *bitmaps_buf_s1_c, __global const u32 *bitmaps_buf_s1_d, __global const u32 *bitmaps_buf_s2_a, __global const u32 *bitmaps_buf_s2_b, __global const u32 *bitmaps_buf_s2_c, __global const u32 *bitmaps_buf_s2_d, __global plain_t *plains_buf, __global const digest_t *digests_buf, __global u32 *hashes_shown, __global const salt_t *salt_bufs, __global const void *esalt_bufs, __global u32 *d_return_buf, __global u32 *d_scryptV0_buf, __global u32 *d_scryptV1_buf, __global u32 *d_scryptV2_buf, __global u32 *d_scryptV3_buf, const u32 bitmap_mask, const u32 bitmap_shift1, const u32 bitmap_shift2, const u32 salt_pos, const u32 loop_pos, const u32 loop_cnt, const u32 il_cnt, const u32 digests_cnt, const u32 digests_offset, const u32 combs_mode, const u32 gid_max)
{
  /**
   * base
   */

  const u32 gid = get_global_id (0);

  if (gid >= gid_max) return;

  const u32 lid = get_local_id (0);

  const u32 r0 = swap32 (tmps[gid].alt_result[0]);
  const u32 r1 = swap32 (tmps[gid].alt_result[1]);
  const u32 r2 = swap32 (tmps[gid].alt_result[2]);
  const u32 r3 = swap32 (tmps[gid].alt_result[3]);

  #define il_pos 0

  #include COMPARE_M
}
