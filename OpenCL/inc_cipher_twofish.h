/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

DECLSPEC u32 mds_rem (u32 p0, u32 p1);
DECLSPEC u32 h_fun128 (u32 *sk, u32 *lk, const u32 x, const u32 *key);
DECLSPEC void twofish128_set_key (u32 *sk, u32 *lk, const u32 *ukey);
DECLSPEC void twofish128_encrypt (const u32 *sk, const u32 *lk, const u32 *in, u32 *out);
DECLSPEC void twofish128_decrypt (const u32 *sk, const u32 *lk, const u32 *in, u32 *out);
DECLSPEC u32 h_fun256 (u32 *sk, u32 *lk, const u32 x, const u32 *key);
DECLSPEC void twofish256_set_key (u32 *sk, u32 *lk, const u32 *ukey);
DECLSPEC void twofish256_encrypt (const u32 *sk, const u32 *lk, const u32 *in, u32 *out);
DECLSPEC void twofish256_decrypt (const u32 *sk, const u32 *lk, const u32 *in, u32 *out);
