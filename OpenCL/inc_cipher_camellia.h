/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#ifndef _INC_CIPHER_CAMELLIA_H
#define _INC_CIPHER_CAMELLIA_H

CONSTANT_AS CONSTSPEC u32a c_sbox[256] =
{
  0x70, 0x82, 0x2c, 0xec, 0xb3, 0x27, 0xc0, 0xe5,
  0xe4, 0x85, 0x57, 0x35, 0xea, 0x0c, 0xae, 0x41,
  0x23, 0xef, 0x6b, 0x93, 0x45, 0x19, 0xa5, 0x21,
  0xed, 0x0e, 0x4f, 0x4e, 0x1d, 0x65, 0x92, 0xbd,
  0x86, 0xb8, 0xaf, 0x8f, 0x7c, 0xeb, 0x1f, 0xce,
  0x3e, 0x30, 0xdc, 0x5f, 0x5e, 0xc5, 0x0b, 0x1a,
  0xa6, 0xe1, 0x39, 0xca, 0xd5, 0x47, 0x5d, 0x3d,
  0xd9, 0x01, 0x5a, 0xd6, 0x51, 0x56, 0x6c, 0x4d,
  0x8b, 0x0d, 0x9a, 0x66, 0xfb, 0xcc, 0xb0, 0x2d,
  0x74, 0x12, 0x2b, 0x20, 0xf0, 0xb1, 0x84, 0x99,
  0xdf, 0x4c, 0xcb, 0xc2, 0x34, 0x7e, 0x76, 0x05,
  0x6d, 0xb7, 0xa9, 0x31, 0xd1, 0x17, 0x04, 0xd7,
  0x14, 0x58, 0x3a, 0x61, 0xde, 0x1b, 0x11, 0x1c,
  0x32, 0x0f, 0x9c, 0x16, 0x53, 0x18, 0xf2, 0x22,
  0xfe, 0x44, 0xcf, 0xb2, 0xc3, 0xb5, 0x7a, 0x91,
  0x24, 0x08, 0xe8, 0xa8, 0x60, 0xfc, 0x69, 0x50,
  0xaa, 0xd0, 0xa0, 0x7d, 0xa1, 0x89, 0x62, 0x97,
  0x54, 0x5b, 0x1e, 0x95, 0xe0, 0xff, 0x64, 0xd2,
  0x10, 0xc4, 0x00, 0x48, 0xa3, 0xf7, 0x75, 0xdb,
  0x8a, 0x03, 0xe6, 0xda, 0x09, 0x3f, 0xdd, 0x94,
  0x87, 0x5c, 0x83, 0x02, 0xcd, 0x4a, 0x90, 0x33,
  0x73, 0x67, 0xf6, 0xf3, 0x9d, 0x7f, 0xbf, 0xe2,
  0x52, 0x9b, 0xd8, 0x26, 0xc8, 0x37, 0xc6, 0x3b,
  0x81, 0x96, 0x6f, 0x4b, 0x13, 0xbe, 0x63, 0x2e,
  0xe9, 0x79, 0xa7, 0x8c, 0x9f, 0x6e, 0xbc, 0x8e,
  0x29, 0xf5, 0xf9, 0xb6, 0x2f, 0xfd, 0xb4, 0x59,
  0x78, 0x98, 0x06, 0x6a, 0xe7, 0x46, 0x71, 0xba,
  0xd4, 0x25, 0xab, 0x42, 0x88, 0xa2, 0x8d, 0xfa,
  0x72, 0x07, 0xb9, 0x55, 0xf8, 0xee, 0xac, 0x0a,
  0x36, 0x49, 0x2a, 0x68, 0x3c, 0x38, 0xf1, 0xa4,
  0x40, 0x28, 0xd3, 0x7b, 0xbb, 0xc9, 0x43, 0xc1,
  0x15, 0xe3, 0xad, 0xf4, 0x77, 0xc7, 0x80, 0x9e
};

DECLSPEC void cam_feistel (const u32 *x, const u32 *k, u32 *y);
DECLSPEC void cam_fl (u32 *x, const u32 *kl, const u32 *kr);
DECLSPEC void camellia256_set_key (u32 *ks, const u32 *ukey);
DECLSPEC void camellia256_encrypt (const u32 *ks, const u32 *in, u32 *out);
DECLSPEC void camellia256_decrypt (const u32 *ks, const u32 *in, u32 *out);

#endif // _INC_CIPHER_CAMELLIA_H
