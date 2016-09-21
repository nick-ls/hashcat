/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#ifndef _USER_OPTIONS_H
#define _USER_OPTIONS_H

#include <getopt.h>

typedef enum user_options_defaults
{
  ATTACK_MODE             = 0,
  BENCHMARK               = 0,
  BITMAP_MAX              = 24,
  BITMAP_MIN              = 16,
  DEBUG_MODE              = 0,
  FORCE                   = 0,
  GPU_TEMP_ABORT          = 90,
  GPU_TEMP_DISABLE        = 0,
  GPU_TEMP_RETAIN         = 75,
  HASH_MODE               = 0,
  HEX_CHARSET             = 0,
  HEX_SALT                = 0,
  HEX_WORDLIST            = 0,
  INCREMENT               = 0,
  INCREMENT_MAX           = PW_MAX,
  INCREMENT_MIN           = 1,
  KERNEL_ACCEL            = 0,
  KERNEL_LOOPS            = 0,
  KEYSPACE                = 0,
  LEFT                    = 0,
  LIMIT                   = 0,
  LOGFILE_DISABLE         = 0,
  LOOPBACK                = 0,
  MACHINE_READABLE        = 0,
  MARKOV_CLASSIC          = 0,
  MARKOV_DISABLE          = 0,
  MARKOV_THRESHOLD        = 0,
  NVIDIA_SPIN_DAMP        = 100,
  OPENCL_VECTOR_WIDTH     = 0,
  OUTFILE_AUTOHEX         = 1,
  OUTFILE_CHECK_TIMER     = 5,
  OUTFILE_FORMAT          = 3,
  POTFILE_DISABLE         = 0,
  POWERTUNE_ENABLE        = 0,
  QUIET                   = 0,
  REMOVE                  = 0,
  REMOVE_TIMER            = 60,
  RESTORE                 = 0,
  RESTORE_DISABLE         = 0,
  RESTORE_TIMER           = 60,
  RP_GEN                  = 0,
  RP_GEN_FUNC_MAX         = 4,
  RP_GEN_FUNC_MIN         = 1,
  RP_GEN_SEED             = 0,
  RUNTIME                 = 0,
  SCRYPT_TMTO             = 0,
  SEGMENT_SIZE            = 33554432,
  SEPARATOR               = ':',
  SHOW                    = 0,
  SKIP                    = 0,
  STATUS                  = 0,
  STATUS_TIMER            = 10,
  STDOUT_FLAG             = 0,
  USAGE                   = 0,
  USERNAME                = 0,
  VERSION                 = 0,
  WEAK_HASH_THRESHOLD     = 100,
  WORKLOAD_PROFILE        = 2,

} user_options_defaults_t;

typedef enum user_options_map
{
  IDX_ATTACK_MODE              = 'a',
  IDX_BENCHMARK                = 'b',
  IDX_BITMAP_MAX               = 0xff00,
  IDX_BITMAP_MIN               = 0xff01,
  IDX_CPU_AFFINITY             = 0xff02,
  IDX_CUSTOM_CHARSET_1         = '1',
  IDX_CUSTOM_CHARSET_2         = '2',
  IDX_CUSTOM_CHARSET_3         = '3',
  IDX_CUSTOM_CHARSET_4         = '4',
  IDX_DEBUG_FILE               = 0xff03,
  IDX_DEBUG_MODE               = 0xff04,
  IDX_FORCE                    = 0xff05,
  IDX_GPU_TEMP_ABORT           = 0xff06,
  IDX_GPU_TEMP_DISABLE         = 0xff07,
  IDX_GPU_TEMP_RETAIN          = 0xff08,
  IDX_HASH_MODE                = 'm',
  IDX_HELP                     = 'h',
  IDX_HEX_CHARSET              = 0xff09,
  IDX_HEX_SALT                 = 0xff0a,
  IDX_HEX_WORDLIST             = 0xff0b,
  IDX_INCREMENT                = 'i',
  IDX_INCREMENT_MAX            = 0xff0c,
  IDX_INCREMENT_MIN            = 0xff0d,
  IDX_INDUCTION_DIR            = 0xff0e,
  IDX_KERNEL_ACCEL             = 'n',
  IDX_KERNEL_LOOPS             = 'u',
  IDX_KEYSPACE                 = 0xff0f,
  IDX_LEFT                     = 0xff10,
  IDX_LIMIT                    = 'l',
  IDX_LOGFILE_DISABLE          = 0xff11,
  IDX_LOOPBACK                 = 0xff12,
  IDX_MACHINE_READABLE         = 0xff13,
  IDX_MARKOV_CLASSIC           = 0xff14,
  IDX_MARKOV_DISABLE           = 0xff15,
  IDX_MARKOV_HCSTAT            = 0xff16,
  IDX_MARKOV_THRESHOLD         = 't',
  IDX_NVIDIA_SPIN_DAMP         = 0xff17,
  IDX_OPENCL_DEVICES           = 'd',
  IDX_OPENCL_DEVICE_TYPES      = 'D',
  IDX_OPENCL_INFO              = 'I',
  IDX_OPENCL_PLATFORMS         = 0xff18,
  IDX_OPENCL_VECTOR_WIDTH      = 0xff19,
  IDX_OUTFILE_AUTOHEX_DISABLE  = 0xff1a,
  IDX_OUTFILE_CHECK_DIR        = 0xff1b,
  IDX_OUTFILE_CHECK_TIMER      = 0xff1c,
  IDX_OUTFILE_FORMAT           = 0xff1d,
  IDX_OUTFILE                  = 'o',
  IDX_POTFILE_DISABLE          = 0xff1e,
  IDX_POTFILE_PATH             = 0xff1f,
  IDX_POWERTUNE_ENABLE         = 0xff20,
  IDX_QUIET                    = 0xff21,
  IDX_REMOVE                   = 0xff22,
  IDX_REMOVE_TIMER             = 0xff23,
  IDX_RESTORE                  = 0xff24,
  IDX_RESTORE_DISABLE          = 0xff25,
  IDX_RP_FILE                  = 'r',
  IDX_RP_GEN_FUNC_MAX          = 0xff26,
  IDX_RP_GEN_FUNC_MIN          = 0xff27,
  IDX_RP_GEN                   = 'g',
  IDX_RP_GEN_SEED              = 0xff28,
  IDX_RULE_BUF_L               = 'j',
  IDX_RULE_BUF_R               = 'k',
  IDX_RUNTIME                  = 0xff29,
  IDX_SCRYPT_TMTO              = 0xff2a,
  IDX_SEGMENT_SIZE             = 'c',
  IDX_SEPARATOR                = 'p',
  IDX_SESSION                  = 0xff2b,
  IDX_SHOW                     = 0xff2c,
  IDX_SKIP                     = 's',
  IDX_STATUS                   = 0xff2d,
  IDX_STATUS_TIMER             = 0xff2e,
  IDX_STDOUT_FLAG              = 0xff2f,
  IDX_TRUECRYPT_KEYFILES       = 0xff30,
  IDX_USERNAME                 = 0xff31,
  IDX_VERACRYPT_KEYFILES       = 0xff32,
  IDX_VERACRYPT_PIM            = 0xff33,
  IDX_VERSION_LOWER            = 'v',
  IDX_VERSION                  = 'V',
  IDX_WEAK_HASH_THRESHOLD      = 0xff34,
  IDX_WORKLOAD_PROFILE         = 'w'

} user_options_map_t;

static const char short_options[] = "hVvm:a:r:j:k:g:o:t:d:D:n:u:c:p:s:l:1:2:3:4:iIbw:";

static const struct option long_options[] =
{
  {"help",                      no_argument,       0, IDX_HELP},
  {"version",                   no_argument,       0, IDX_VERSION},
  {"quiet",                     no_argument,       0, IDX_QUIET},
  {"show",                      no_argument,       0, IDX_SHOW},
  {"left",                      no_argument,       0, IDX_LEFT},
  {"username",                  no_argument,       0, IDX_USERNAME},
  {"remove",                    no_argument,       0, IDX_REMOVE},
  {"remove-timer",              required_argument, 0, IDX_REMOVE_TIMER},
  {"skip",                      required_argument, 0, IDX_SKIP},
  {"limit",                     required_argument, 0, IDX_LIMIT},
  {"keyspace",                  no_argument,       0, IDX_KEYSPACE},
  {"potfile-disable",           no_argument,       0, IDX_POTFILE_DISABLE},
  {"potfile-path",              required_argument, 0, IDX_POTFILE_PATH},
  {"debug-mode",                required_argument, 0, IDX_DEBUG_MODE},
  {"debug-file",                required_argument, 0, IDX_DEBUG_FILE},
  {"induction-dir",             required_argument, 0, IDX_INDUCTION_DIR},
  {"outfile-check-dir",         required_argument, 0, IDX_OUTFILE_CHECK_DIR},
  {"force",                     no_argument,       0, IDX_FORCE},
  {"benchmark",                 no_argument,       0, IDX_BENCHMARK},
  {"stdout",                    no_argument,       0, IDX_STDOUT_FLAG},
  {"restore",                   no_argument,       0, IDX_RESTORE},
  {"restore-disable",           no_argument,       0, IDX_RESTORE_DISABLE},
  {"status",                    no_argument,       0, IDX_STATUS},
  {"status-timer",              required_argument, 0, IDX_STATUS_TIMER},
  {"machine-readable",          no_argument,       0, IDX_MACHINE_READABLE},
  {"loopback",                  no_argument,       0, IDX_LOOPBACK},
  {"weak-hash-threshold",       required_argument, 0, IDX_WEAK_HASH_THRESHOLD},
  {"session",                   required_argument, 0, IDX_SESSION},
  {"runtime",                   required_argument, 0, IDX_RUNTIME},
  {"generate-rules",            required_argument, 0, IDX_RP_GEN},
  {"generate-rules-func-min",   required_argument, 0, IDX_RP_GEN_FUNC_MIN},
  {"generate-rules-func-max",   required_argument, 0, IDX_RP_GEN_FUNC_MAX},
  {"generate-rules-seed",       required_argument, 0, IDX_RP_GEN_SEED},
  {"rule-left",                 required_argument, 0, IDX_RULE_BUF_L},
  {"rule-right",                required_argument, 0, IDX_RULE_BUF_R},
  {"hash-type",                 required_argument, 0, IDX_HASH_MODE},
  {"attack-mode",               required_argument, 0, IDX_ATTACK_MODE},
  {"rules-file",                required_argument, 0, IDX_RP_FILE},
  {"outfile",                   required_argument, 0, IDX_OUTFILE},
  {"outfile-format",            required_argument, 0, IDX_OUTFILE_FORMAT},
  {"outfile-autohex-disable",   no_argument,       0, IDX_OUTFILE_AUTOHEX_DISABLE},
  {"outfile-check-timer",       required_argument, 0, IDX_OUTFILE_CHECK_TIMER},
  {"hex-charset",               no_argument,       0, IDX_HEX_CHARSET},
  {"hex-salt",                  no_argument,       0, IDX_HEX_SALT},
  {"hex-wordlist",              no_argument,       0, IDX_HEX_WORDLIST},
  {"markov-disable",            no_argument,       0, IDX_MARKOV_DISABLE},
  {"markov-classic",            no_argument,       0, IDX_MARKOV_CLASSIC},
  {"markov-threshold",          required_argument, 0, IDX_MARKOV_THRESHOLD},
  {"markov-hcstat",             required_argument, 0, IDX_MARKOV_HCSTAT},
  {"cpu-affinity",              required_argument, 0, IDX_CPU_AFFINITY},
  {"opencl-info",               no_argument,       0, IDX_OPENCL_INFO},
  {"opencl-devices",            required_argument, 0, IDX_OPENCL_DEVICES},
  {"opencl-platforms",          required_argument, 0, IDX_OPENCL_PLATFORMS},
  {"opencl-device-types",       required_argument, 0, IDX_OPENCL_DEVICE_TYPES},
  {"opencl-vector-width",       required_argument, 0, IDX_OPENCL_VECTOR_WIDTH},
  {"workload-profile",          required_argument, 0, IDX_WORKLOAD_PROFILE},
  {"kernel-accel",              required_argument, 0, IDX_KERNEL_ACCEL},
  {"kernel-loops",              required_argument, 0, IDX_KERNEL_LOOPS},
  {"nvidia-spin-damp",          required_argument, 0, IDX_NVIDIA_SPIN_DAMP},
  {"gpu-temp-disable",          no_argument,       0, IDX_GPU_TEMP_DISABLE},
  #if defined (HAVE_HWMON)
  {"gpu-temp-abort",            required_argument, 0, IDX_GPU_TEMP_ABORT},
  {"gpu-temp-retain",           required_argument, 0, IDX_GPU_TEMP_RETAIN},
  {"powertune-enable",          no_argument,       0, IDX_POWERTUNE_ENABLE},
  #endif // HAVE_HWMON
  {"logfile-disable",           no_argument,       0, IDX_LOGFILE_DISABLE},
  {"truecrypt-keyfiles",        required_argument, 0, IDX_TRUECRYPT_KEYFILES},
  {"veracrypt-keyfiles",        required_argument, 0, IDX_VERACRYPT_KEYFILES},
  {"veracrypt-pim",             required_argument, 0, IDX_VERACRYPT_PIM},
  {"segment-size",              required_argument, 0, IDX_SEGMENT_SIZE},
  {"scrypt-tmto",               required_argument, 0, IDX_SCRYPT_TMTO},
  {"seperator",                 required_argument, 0, IDX_SEPARATOR},
  {"separator",                 required_argument, 0, IDX_SEPARATOR},
  {"bitmap-min",                required_argument, 0, IDX_BITMAP_MIN},
  {"bitmap-max",                required_argument, 0, IDX_BITMAP_MAX},
  {"increment",                 no_argument,       0, IDX_INCREMENT},
  {"increment-min",             required_argument, 0, IDX_INCREMENT_MIN},
  {"increment-max",             required_argument, 0, IDX_INCREMENT_MAX},
  {"custom-charset1",           required_argument, 0, IDX_CUSTOM_CHARSET_1},
  {"custom-charset2",           required_argument, 0, IDX_CUSTOM_CHARSET_2},
  {"custom-charset3",           required_argument, 0, IDX_CUSTOM_CHARSET_3},
  {"custom-charset4",           required_argument, 0, IDX_CUSTOM_CHARSET_4},
  {0, 0, 0, 0}
};

void user_options_init (user_options_t *user_options, int myargc, char **myargv);

void user_options_destroy (user_options_t *user_options);

int user_options_parse (user_options_t *user_options, int myargc, char **myargv);

int user_options_sanity (user_options_t *user_options, int myargc, char **myargv, user_options_extra_t *user_options_extra);

int user_options_extra_init (user_options_t *user_options, int myargc, char **myargv, user_options_extra_t *user_options_extra);

#endif // _USER_OPTIONS_H
