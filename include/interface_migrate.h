
typedef enum kern_type
{
  KERN_TYPE_MD5_SLTPW               = 20,
  KERN_TYPE_MD5_PWUSLT              = 30,
  KERN_TYPE_HMACMD5_PW              = 50,
  KERN_TYPE_HMACMD5_SLT             = 60,
  KERN_TYPE_SHA1_SLTPW              = 120,
  KERN_TYPE_SHA1_PWUSLT             = 130,
  KERN_TYPE_SHA1_SLTPWU             = 140,
  KERN_TYPE_HMACSHA1_PW             = 150,
  KERN_TYPE_HMACSHA1_SLT            = 160,
  KERN_TYPE_SHA256_PWSLT            = 1410,
  KERN_TYPE_SHA256_SLTPW            = 1420,
  KERN_TYPE_SHA256_PWUSLT           = 1430,
  KERN_TYPE_SHA256_SLTPWU           = 1440,
  KERN_TYPE_HMACSHA256_PW           = 1450,
  KERN_TYPE_HMACSHA256_SLT          = 1460,
  KERN_TYPE_APR1CRYPT               = 1600,
  KERN_TYPE_SHA512_PWSLT            = 1710,
  KERN_TYPE_SHA512_SLTPW            = 1720,
  KERN_TYPE_SHA512_PWSLTU           = 1730,
  KERN_TYPE_SHA512_SLTPWU           = 1740,
  KERN_TYPE_HMACSHA512_PW           = 1750,
  KERN_TYPE_HMACSHA512_SLT          = 1760,
  KERN_TYPE_MD55                    = 2600,
  KERN_TYPE_MD55_PWSLT1             = 2610,
  KERN_TYPE_MD55_PWSLT2             = 2710,
  KERN_TYPE_MD55_SLTPW              = 2810,
  KERN_TYPE_MD5_SLT_MD5_PW          = 3710,
  KERN_TYPE_MD5_SLT_PW_SLT          = 3800,
  KERN_TYPE_MD5_SLT_MD5_SLT_PW      = 4010,
  KERN_TYPE_MD5_SLT_MD5_PW_SLT      = 4110,
  KERN_TYPE_MD5U5                   = 4300,
  KERN_TYPE_MD5U5_PWSLT1            = 4310,
  KERN_TYPE_MD5_SHA1                = 4400,
  KERN_TYPE_SHA11                   = 4500,
  KERN_TYPE_SHA1_SLT_SHA1_PW        = 4520,
  KERN_TYPE_SHA1_MD5                = 4700,
  KERN_TYPE_SHA1_SLT_PW_SLT         = 4900,
  KERN_TYPE_RIPEMD160               = 6000,
  KERN_TYPE_WHIRLPOOL               = 6100,
  KERN_TYPE_SIPHASH                 = 10100,
  KERN_TYPE_STREEBOG_256            = 11700,
  KERN_TYPE_HMAC_STREEBOG_256_PW    = 11750,
  KERN_TYPE_HMAC_STREEBOG_256_SLT   = 11760,
  KERN_TYPE_STREEBOG_512            = 11800,
  KERN_TYPE_HMAC_STREEBOG_512_PW    = 11850,
  KERN_TYPE_HMAC_STREEBOG_512_SLT   = 11860,
  KERN_TYPE_OPENCART                = 13900,

} kern_type_t;

