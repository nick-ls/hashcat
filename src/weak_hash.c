/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#include "common.h"
#include "types.h"
#include "logging.h"
#include "opencl.h"
#include "hashes.h"
#include "weak_hash.h"

void weak_hash_check (opencl_ctx_t *opencl_ctx, hc_device_param_t *device_param, const user_options_t *user_options, const user_options_extra_t *user_options_extra, const straight_ctx_t *straight_ctx, const combinator_ctx_t *combinator_ctx, hashconfig_t *hashconfig, hashes_t *hashes, cpt_ctx_t *cpt_ctx, status_ctx_t *status_ctx, const uint salt_pos)
{
  salt_t *salt_buf = &hashes->salts_buf[salt_pos];

  device_param->kernel_params_buf32[27] = salt_pos;
  device_param->kernel_params_buf32[30] = 1;
  device_param->kernel_params_buf32[31] = salt_buf->digests_cnt;
  device_param->kernel_params_buf32[32] = salt_buf->digests_offset;
  device_param->kernel_params_buf32[33] = 0;
  device_param->kernel_params_buf32[34] = 1;

  uint cmd0_rule_old = straight_ctx->kernel_rules_buf[0].cmds[0];

  straight_ctx->kernel_rules_buf[0].cmds[0] = 0;

  /**
   * run the kernel
   */

  if (hashconfig->attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
  {
    run_kernel (KERN_RUN_1, opencl_ctx, device_param, 1, false, 0, hashconfig, user_options, status_ctx);
  }
  else
  {
    run_kernel (KERN_RUN_1, opencl_ctx, device_param, 1, false, 0, hashconfig, user_options, status_ctx);

    uint loop_step = 16;

    const uint iter = salt_buf->salt_iter;

    for (uint loop_pos = 0; loop_pos < iter; loop_pos += loop_step)
    {
      uint loop_left = iter - loop_pos;

      loop_left = MIN (loop_left, loop_step);

      device_param->kernel_params_buf32[28] = loop_pos;
      device_param->kernel_params_buf32[29] = loop_left;

      run_kernel (KERN_RUN_2, opencl_ctx, device_param, 1, false, 0, hashconfig, user_options, status_ctx);
    }

    run_kernel (KERN_RUN_3, opencl_ctx, device_param, 1, false, 0, hashconfig, user_options, status_ctx);
  }

  /**
   * result
   */

  check_cracked (opencl_ctx, device_param, user_options, user_options_extra, straight_ctx, combinator_ctx, hashconfig, hashes, cpt_ctx, status_ctx, salt_pos);

  /**
   * cleanup
   */

  device_param->kernel_params_buf32[27] = 0;
  device_param->kernel_params_buf32[28] = 0;
  device_param->kernel_params_buf32[29] = 0;
  device_param->kernel_params_buf32[30] = 0;
  device_param->kernel_params_buf32[31] = 0;
  device_param->kernel_params_buf32[32] = 0;
  device_param->kernel_params_buf32[33] = 0;
  device_param->kernel_params_buf32[34] = 0;

  straight_ctx->kernel_rules_buf[0].cmds[0] = cmd0_rule_old;
}
