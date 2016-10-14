/**
 * Author......: See docs/credits.txt
 * License.....: MIT
 */

#include "common.h"
#include "types.h"
#include "memory.h"
#include "locking.h"
#include "thread.h"
#include "timer.h"
#include "tuningdb.h"
#include "rp_cpu.h"
#include "terminal.h"
#include "mpsp.h"
#include "straight.h"
#include "combinator.h"
#include "convert.h"
#include "stdout.h"
#include "filehandling.h"
#include "interface.h"
#include "wordlist.h"
#include "shared.h"
#include "hashes.h"
#include "cpu_md5.h"
#include "event.h"
#include "dynloader.h"
#include "opencl.h"

static const u32 full01 = 0x01010101;
static const u32 full80 = 0x80808080;

static double TARGET_MS_PROFILE[4] = { 2, 12, 96, 480 };

static void generate_source_kernel_filename (const u32 attack_exec, const u32 attack_kern, const u32 kern_type, char *shared_dir, char *source_file)
{
  if (attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
  {
    if (attack_kern == ATTACK_KERN_STRAIGHT)
      snprintf (source_file, 255, "%s/OpenCL/m%05d_a0.cl", shared_dir, (int) kern_type);
    else if (attack_kern == ATTACK_KERN_COMBI)
      snprintf (source_file, 255, "%s/OpenCL/m%05d_a1.cl", shared_dir, (int) kern_type);
    else if (attack_kern == ATTACK_KERN_BF)
      snprintf (source_file, 255, "%s/OpenCL/m%05d_a3.cl", shared_dir, (int) kern_type);
  }
  else
    snprintf (source_file, 255, "%s/OpenCL/m%05d.cl", shared_dir, (int) kern_type);
}

static void generate_cached_kernel_filename (const u32 attack_exec, const u32 attack_kern, const u32 kern_type, char *profile_dir, const char *device_name_chksum, char *cached_file)
{
  if (attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
  {
    if (attack_kern == ATTACK_KERN_STRAIGHT)
      snprintf (cached_file, 255, "%s/kernels/m%05d_a0.%s.kernel", profile_dir, (int) kern_type, device_name_chksum);
    else if (attack_kern == ATTACK_KERN_COMBI)
      snprintf (cached_file, 255, "%s/kernels/m%05d_a1.%s.kernel", profile_dir, (int) kern_type, device_name_chksum);
    else if (attack_kern == ATTACK_KERN_BF)
      snprintf (cached_file, 255, "%s/kernels/m%05d_a3.%s.kernel", profile_dir, (int) kern_type, device_name_chksum);
  }
  else
  {
    snprintf (cached_file, 255, "%s/kernels/m%05d.%s.kernel", profile_dir, (int) kern_type, device_name_chksum);
  }
}

static void generate_source_kernel_mp_filename (const u32 opti_type, const u32 opts_type, char *shared_dir, char *source_file)
{
  if ((opti_type & OPTI_TYPE_BRUTE_FORCE) && (opts_type & OPTS_TYPE_PT_GENERATE_BE))
  {
    snprintf (source_file, 255, "%s/OpenCL/markov_be.cl", shared_dir);
  }
  else
  {
    snprintf (source_file, 255, "%s/OpenCL/markov_le.cl", shared_dir);
  }
}

static void generate_cached_kernel_mp_filename (const u32 opti_type, const u32 opts_type, char *profile_dir, const char *device_name_chksum, char *cached_file)
{
  if ((opti_type & OPTI_TYPE_BRUTE_FORCE) && (opts_type & OPTS_TYPE_PT_GENERATE_BE))
  {
    snprintf (cached_file, 255, "%s/kernels/markov_be.%s.kernel", profile_dir, device_name_chksum);
  }
  else
  {
    snprintf (cached_file, 255, "%s/kernels/markov_le.%s.kernel", profile_dir, device_name_chksum);
  }
}

static void generate_source_kernel_amp_filename (const u32 attack_kern, char *shared_dir, char *source_file)
{
  snprintf (source_file, 255, "%s/OpenCL/amp_a%d.cl", shared_dir, attack_kern);
}

static void generate_cached_kernel_amp_filename (const u32 attack_kern, char *profile_dir, const char *device_name_chksum, char *cached_file)
{
  snprintf (cached_file, 255, "%s/kernels/amp_a%d.%s.kernel", profile_dir, attack_kern, device_name_chksum);
}

static int setup_opencl_platforms_filter (hashcat_ctx_t *hashcat_ctx, const char *opencl_platforms, u32 *out)
{
  u32 opencl_platforms_filter = 0;

  if (opencl_platforms)
  {
    char *platforms = hcstrdup (hashcat_ctx, opencl_platforms);

    char *next = strtok (platforms, ",");

    do
    {
      int platform = atoi (next);

      if (platform < 1 || platform > 32)
      {
        event_log_error (hashcat_ctx, "Invalid OpenCL platform %u specified", platform);

        return -1;
      }

      opencl_platforms_filter |= 1u << (platform - 1);

    } while ((next = strtok (NULL, ",")) != NULL);

    hcfree (platforms);
  }
  else
  {
    opencl_platforms_filter = -1u;
  }

  *out = opencl_platforms_filter;

  return 0;
}

static int setup_devices_filter (hashcat_ctx_t *hashcat_ctx, const char *opencl_devices, u32 *out)
{
  u32 devices_filter = 0;

  if (opencl_devices)
  {
    char *devices = hcstrdup (hashcat_ctx, opencl_devices);

    char *next = strtok (devices, ",");

    do
    {
      int device_id = atoi (next);

      if (device_id < 1 || device_id > 32)
      {
        event_log_error (hashcat_ctx, "Invalid device_id %u specified", device_id);

        return -1;
      }

      devices_filter |= 1u << (device_id - 1);

    } while ((next = strtok (NULL, ",")) != NULL);

    hcfree (devices);
  }
  else
  {
    devices_filter = -1u;
  }

  *out = devices_filter;

  return 0;
}

static int setup_device_types_filter (hashcat_ctx_t *hashcat_ctx, const char *opencl_device_types, cl_device_type *out)
{
  cl_device_type device_types_filter = 0;

  if (opencl_device_types)
  {
    char *device_types = hcstrdup (hashcat_ctx, opencl_device_types);

    char *next = strtok (device_types, ",");

    do
    {
      int device_type = atoi (next);

      if (device_type < 1 || device_type > 3)
      {
        event_log_error (hashcat_ctx, "Invalid device_type %u specified", device_type);

        return -1;
      }

      device_types_filter |= 1u << device_type;

    } while ((next = strtok (NULL, ",")) != NULL);

    hcfree (device_types);
  }
  else
  {
    // Do not use CPU by default, this often reduces GPU performance because
    // the CPU is too busy to handle GPU synchronization

    device_types_filter = CL_DEVICE_TYPE_ALL & ~CL_DEVICE_TYPE_CPU;
  }

  *out = device_types_filter;

  return 0;
}

static int read_kernel_binary (hashcat_ctx_t *hashcat_ctx, const char *kernel_file, int num_devices, size_t *kernel_lengths, char **kernel_sources)
{
  FILE *fp = fopen (kernel_file, "rb");

  if (fp != NULL)
  {
    struct stat st;

    memset (&st, 0, sizeof (st));

    stat (kernel_file, &st);

    char *buf = (char *) hcmalloc (hashcat_ctx, st.st_size + 1); VERIFY_PTR (buf);

    size_t num_read = fread (buf, sizeof (char), st.st_size, fp);

    if (num_read != (size_t) st.st_size)
    {
      event_log_error (hashcat_ctx, "%s: %s", kernel_file, strerror (errno));

      return -1;
    }

    fclose (fp);

    buf[st.st_size] = 0;

    for (int i = 0; i < num_devices; i++)
    {
      kernel_lengths[i] = (size_t) st.st_size;

      kernel_sources[i] = buf;
    }
  }
  else
  {
    event_log_error (hashcat_ctx, "%s: %s", kernel_file, strerror (errno));

    return -1;
  }

  return 0;
}

static int write_kernel_binary (hashcat_ctx_t *hashcat_ctx, char *kernel_file, char *binary, size_t binary_size)
{
  if (binary_size > 0)
  {
    FILE *fp = fopen (kernel_file, "wb");

    if (fp == NULL)
    {
      event_log_error (hashcat_ctx, "%s: %s", kernel_file, strerror (errno));

      return -1;
    }

    const int rc = lock_file (fp);

    if (rc == -1) return -1;

    fwrite (binary, sizeof (char), binary_size, fp);
    fflush (fp);
    fclose (fp);

    unlock_file (fp);
  }

  return 0;
}

int ocl_init (hashcat_ctx_t *hashcat_ctx)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  memset (ocl, 0, sizeof (OCL_PTR));

  #if   defined(_WIN)
  ocl->lib = hc_dlopen ("OpenCL");
  #elif defined(__APPLE__)
  ocl->lib = hc_dlopen ("/System/Library/Frameworks/OpenCL.framework/OpenCL", RTLD_NOW);
  #else
  ocl->lib = hc_dlopen ("libOpenCL.so", RTLD_NOW);

  if (ocl->lib == NULL) ocl->lib = hc_dlopen ("libOpenCL.so.1", RTLD_NOW);
  #endif

  if (ocl->lib == NULL)
  {
    event_log_warning (hashcat_ctx,
      "Can't find OpenCL ICD loader library" EOL
      ""                                     EOL
      #if defined (__linux__)
      "You're probably missing the \"ocl-icd-libopencl1\" package (Debian/Ubuntu)" EOL
      "Run: sudo apt-get install ocl-icd-libopencl1"                               EOL
      ""                                                                           EOL
      #elif defined (_WIN)
      "You're probably missing the OpenCL runtime installation"                              EOL
      "* AMD users require AMD drivers 14.9 or later (recommended 15.12 or later)"           EOL
      "* Intel users require Intel OpenCL Runtime 14.2 or later (recommended 15.1 or later)" EOL
      "* NVidia users require NVidia drivers 346.59 or later (recommended 361.x or later)"   EOL
      ""                                                                                     EOL
      #endif
    );

    return -1;
  }

  HC_LOAD_FUNC(ocl, clBuildProgram, OCL_CLBUILDPROGRAM, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clCreateBuffer, OCL_CLCREATEBUFFER, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clCreateCommandQueue, OCL_CLCREATECOMMANDQUEUE, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clCreateContext, OCL_CLCREATECONTEXT, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clCreateKernel, OCL_CLCREATEKERNEL, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clCreateProgramWithBinary, OCL_CLCREATEPROGRAMWITHBINARY, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clCreateProgramWithSource, OCL_CLCREATEPROGRAMWITHSOURCE, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clEnqueueCopyBuffer, OCL_CLENQUEUECOPYBUFFER, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clEnqueueMapBuffer, OCL_CLENQUEUEMAPBUFFER, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clEnqueueNDRangeKernel, OCL_CLENQUEUENDRANGEKERNEL, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clEnqueueReadBuffer, OCL_CLENQUEUEREADBUFFER, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clEnqueueUnmapMemObject, OCL_CLENQUEUEUNMAPMEMOBJECT, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clEnqueueWriteBuffer, OCL_CLENQUEUEWRITEBUFFER, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clFinish, OCL_CLFINISH, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clFlush, OCL_CLFLUSH, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clGetDeviceIDs, OCL_CLGETDEVICEIDS, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clGetDeviceInfo, OCL_CLGETDEVICEINFO, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clGetEventInfo, OCL_CLGETEVENTINFO, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clGetKernelWorkGroupInfo, OCL_CLGETKERNELWORKGROUPINFO, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clGetPlatformIDs, OCL_CLGETPLATFORMIDS, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clGetPlatformInfo, OCL_CLGETPLATFORMINFO, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clGetProgramBuildInfo, OCL_CLGETPROGRAMBUILDINFO, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clGetProgramInfo, OCL_CLGETPROGRAMINFO, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clReleaseCommandQueue, OCL_CLRELEASECOMMANDQUEUE, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clReleaseContext, OCL_CLRELEASECONTEXT, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clReleaseKernel, OCL_CLRELEASEKERNEL, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clReleaseMemObject, OCL_CLRELEASEMEMOBJECT, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clReleaseProgram, OCL_CLRELEASEPROGRAM, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clSetKernelArg, OCL_CLSETKERNELARG, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clWaitForEvents, OCL_CLWAITFOREVENTS, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clGetEventProfilingInfo, OCL_CLGETEVENTPROFILINGINFO, OpenCL, 1)
  HC_LOAD_FUNC(ocl, clReleaseEvent, OCL_CLRELEASEEVENT, OpenCL, 1)

  return 0;
}

void ocl_close (hashcat_ctx_t *hashcat_ctx)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  if (ocl)
  {
    if (ocl->lib)
    {
      hc_dlclose (ocl->lib);
    }
  }
}

int hc_clEnqueueNDRangeKernel (hashcat_ctx_t *hashcat_ctx, cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clEnqueueNDRangeKernel (command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clEnqueueNDRangeKernel(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clGetEventInfo (hashcat_ctx_t *hashcat_ctx, cl_event event, cl_event_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clGetEventInfo (event, param_name, param_value_size, param_value, param_value_size_ret);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clGetEventInfo(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clFlush (hashcat_ctx_t *hashcat_ctx, cl_command_queue command_queue)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clFlush (command_queue);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clFlush(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clFinish (hashcat_ctx_t *hashcat_ctx, cl_command_queue command_queue)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clFinish (command_queue);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clFinish(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clSetKernelArg (hashcat_ctx_t *hashcat_ctx, cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clSetKernelArg (kernel, arg_index, arg_size, arg_value);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clSetKernelArg(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clEnqueueWriteBuffer (hashcat_ctx_t *hashcat_ctx, cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t cb, const void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clEnqueueWriteBuffer (command_queue, buffer, blocking_write, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clEnqueueWriteBuffer(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clEnqueueCopyBuffer (hashcat_ctx_t *hashcat_ctx, cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, size_t src_offset, size_t dst_offset, size_t cb, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clEnqueueCopyBuffer (command_queue, src_buffer, dst_buffer, src_offset, dst_offset, cb, num_events_in_wait_list, event_wait_list, event);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clEnqueueCopyBuffer(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clEnqueueReadBuffer (hashcat_ctx_t *hashcat_ctx, cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t cb, void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clEnqueueReadBuffer (command_queue, buffer, blocking_read, offset, cb, ptr, num_events_in_wait_list, event_wait_list, event);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clEnqueueReadBuffer(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clGetPlatformIDs (hashcat_ctx_t *hashcat_ctx, cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clGetPlatformIDs (num_entries, platforms, num_platforms);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clGetPlatformIDs(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clGetPlatformInfo (hashcat_ctx_t *hashcat_ctx, cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clGetPlatformInfo (platform, param_name, param_value_size, param_value, param_value_size_ret);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clGetPlatformInfo(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clGetDeviceIDs (hashcat_ctx_t *hashcat_ctx, cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clGetDeviceIDs (platform, device_type, num_entries, devices, num_devices);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clGetDeviceIDs(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clGetDeviceInfo (hashcat_ctx_t *hashcat_ctx, cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clGetDeviceInfo (device, param_name, param_value_size, param_value, param_value_size_ret);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clGetDeviceInfo(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clCreateContext (hashcat_ctx_t *hashcat_ctx, cl_context_properties *properties, cl_uint num_devices, const cl_device_id *devices, void (CL_CALLBACK *pfn_notify) (const char *, const void *, size_t, void *), void *user_data, cl_context *context)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  cl_int CL_err;

  *context = ocl->clCreateContext (properties, num_devices, devices, pfn_notify, user_data, &CL_err);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clCreateContext(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clCreateCommandQueue (hashcat_ctx_t *hashcat_ctx, cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_command_queue *command_queue)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  cl_int CL_err;

  *command_queue = ocl->clCreateCommandQueue (context, device, properties, &CL_err);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clCreateCommandQueue(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clCreateBuffer (hashcat_ctx_t *hashcat_ctx, cl_context context, cl_mem_flags flags, size_t size, void *host_ptr, cl_mem *mem)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  cl_int CL_err;

  *mem = ocl->clCreateBuffer (context, flags, size, host_ptr, &CL_err);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clCreateBuffer(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clCreateProgramWithSource (hashcat_ctx_t *hashcat_ctx, cl_context context, cl_uint count, const char **strings, const size_t *lengths, cl_program *program)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  cl_int CL_err;

  *program = ocl->clCreateProgramWithSource (context, count, strings, lengths, &CL_err);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clCreateProgramWithSource(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clCreateProgramWithBinary (hashcat_ctx_t *hashcat_ctx, cl_context context, cl_uint num_devices, const cl_device_id *device_list, const size_t *lengths, const unsigned char **binaries, cl_int *binary_status, cl_program *program)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  cl_int CL_err;

  *program = ocl->clCreateProgramWithBinary (context, num_devices, device_list, lengths, binaries, binary_status, &CL_err);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clCreateProgramWithBinary(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clBuildProgram (hashcat_ctx_t *hashcat_ctx, cl_program program, cl_uint num_devices, const cl_device_id *device_list, const char *options, void (CL_CALLBACK *pfn_notify) (cl_program program, void *user_data), void *user_data)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clBuildProgram (program, num_devices, device_list, options, pfn_notify, user_data);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clBuildProgram(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clCreateKernel (hashcat_ctx_t *hashcat_ctx, cl_program program, const char *kernel_name, cl_kernel *kernel)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  cl_int CL_err;

  *kernel = ocl->clCreateKernel (program, kernel_name, &CL_err);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clCreateKernel(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clReleaseMemObject (hashcat_ctx_t *hashcat_ctx, cl_mem mem)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clReleaseMemObject (mem);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clReleaseMemObject(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clReleaseKernel (hashcat_ctx_t *hashcat_ctx, cl_kernel kernel)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clReleaseKernel (kernel);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clReleaseKernel(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clReleaseProgram (hashcat_ctx_t *hashcat_ctx, cl_program program)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clReleaseProgram (program);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clReleaseProgram(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clReleaseCommandQueue (hashcat_ctx_t *hashcat_ctx, cl_command_queue command_queue)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clReleaseCommandQueue (command_queue);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clReleaseCommandQueue(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clReleaseContext (hashcat_ctx_t *hashcat_ctx, cl_context context)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clReleaseContext (context);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clReleaseContext(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clEnqueueMapBuffer (hashcat_ctx_t *hashcat_ctx, cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, cl_map_flags map_flags, size_t offset, size_t cb, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event, void **buf)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  cl_int CL_err;

  *buf = ocl->clEnqueueMapBuffer (command_queue, buffer, blocking_read, map_flags, offset, cb, num_events_in_wait_list, event_wait_list, event, &CL_err);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clEnqueueMapBuffer(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clEnqueueUnmapMemObject (hashcat_ctx_t *hashcat_ctx, cl_command_queue command_queue, cl_mem memobj, void *mapped_ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clEnqueueUnmapMemObject (command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clEnqueueUnmapMemObject(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clGetKernelWorkGroupInfo (hashcat_ctx_t *hashcat_ctx, cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clGetKernelWorkGroupInfo (kernel, device, param_name, param_value_size, param_value, param_value_size_ret);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clGetKernelWorkGroupInfo(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clGetProgramBuildInfo (hashcat_ctx_t *hashcat_ctx, cl_program program, cl_device_id device, cl_program_build_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clGetProgramBuildInfo (program, device, param_name, param_value_size, param_value, param_value_size_ret);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clGetProgramBuildInfo(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clGetProgramInfo (hashcat_ctx_t *hashcat_ctx, cl_program program, cl_program_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clGetProgramInfo (program, param_name, param_value_size, param_value, param_value_size_ret);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clGetProgramInfo(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clWaitForEvents (hashcat_ctx_t *hashcat_ctx, cl_uint num_events, const cl_event *event_list)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clWaitForEvents (num_events, event_list);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clWaitForEvents(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clGetEventProfilingInfo (hashcat_ctx_t *hashcat_ctx, cl_event event, cl_profiling_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clGetEventProfilingInfo (event, param_name, param_value_size, param_value, param_value_size_ret);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clGetEventProfilingInfo(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int hc_clReleaseEvent (hashcat_ctx_t *hashcat_ctx, cl_event event)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  OCL_PTR *ocl = opencl_ctx->ocl;

  const cl_int CL_err = ocl->clReleaseEvent (event);

  if (CL_err != CL_SUCCESS)
  {
    event_log_error (hashcat_ctx, "clReleaseEvent(): %s", val2cstr_cl (CL_err));

    return -1;
  }

  return 0;
}

int gidd_to_pw_t (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param, const u64 gidd, pw_t *pw)
{
  cl_int CL_err = hc_clEnqueueReadBuffer (hashcat_ctx, device_param->command_queue, device_param->d_pws_buf, CL_TRUE, gidd * sizeof (pw_t), sizeof (pw_t), pw, 0, NULL, NULL);

  if (CL_err != CL_SUCCESS) return -1;

  return 0;
}

int choose_kernel (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param, const u32 highest_pw_len, const u32 pws_cnt, const u32 fast_iteration, const u32 salt_pos)
{
  hashconfig_t   *hashconfig   = hashcat_ctx->hashconfig;
  hashes_t       *hashes       = hashcat_ctx->hashes;
  status_ctx_t   *status_ctx   = hashcat_ctx->status_ctx;
  user_options_t *user_options = hashcat_ctx->user_options;

  cl_int CL_err = CL_SUCCESS;

  if (hashconfig->hash_mode == 2000)
  {
    return process_stdout (hashcat_ctx, device_param, pws_cnt);
  }

  if (hashconfig->attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
  {
    if (user_options->attack_mode == ATTACK_MODE_BF)
    {
      if (hashconfig->opts_type & OPTS_TYPE_PT_BITSLICE)
      {
        const u32 size_tm = 32 * sizeof (bs_word_t);

        run_kernel_bzero (hashcat_ctx, device_param, device_param->d_tm_c, size_tm);

        run_kernel_tm (hashcat_ctx, device_param);

        CL_err = hc_clEnqueueCopyBuffer (hashcat_ctx, device_param->command_queue, device_param->d_tm_c, device_param->d_bfs_c, 0, 0, size_tm, 0, NULL, NULL);

        if (CL_err == -1) return -1;
      }
    }

    if (highest_pw_len < 16)
    {
      run_kernel (hashcat_ctx, device_param, KERN_RUN_1, pws_cnt, true, fast_iteration);
    }
    else if (highest_pw_len < 32)
    {
      run_kernel (hashcat_ctx, device_param, KERN_RUN_2, pws_cnt, true, fast_iteration);
    }
    else
    {
      run_kernel (hashcat_ctx, device_param, KERN_RUN_3,pws_cnt, true, fast_iteration);
    }
  }
  else
  {
    run_kernel_amp (hashcat_ctx, device_param, pws_cnt);

    run_kernel (hashcat_ctx, device_param, KERN_RUN_1, pws_cnt, false, 0);

    if (hashconfig->opts_type & OPTS_TYPE_HOOK12)
    {
      run_kernel (hashcat_ctx, device_param, KERN_RUN_12, pws_cnt, false, 0);

      CL_err = hc_clEnqueueReadBuffer (hashcat_ctx, device_param->command_queue, device_param->d_hooks, CL_TRUE, 0, device_param->size_hooks, device_param->hooks_buf, 0, NULL, NULL);

      if (CL_err == -1) return -1;

      // do something with data

      CL_err = hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_hooks, CL_TRUE, 0, device_param->size_hooks, device_param->hooks_buf, 0, NULL, NULL);

      if (CL_err == -1) return -1;
    }

    u32 iter = hashes->salts_buf[salt_pos].salt_iter;

    u32 loop_step = device_param->kernel_loops;

    for (u32 loop_pos = 0, slow_iteration = 0; loop_pos < iter; loop_pos += loop_step, slow_iteration++)
    {
      u32 loop_left = iter - loop_pos;

      loop_left = MIN (loop_left, loop_step);

      device_param->kernel_params_buf32[28] = loop_pos;
      device_param->kernel_params_buf32[29] = loop_left;

      run_kernel (hashcat_ctx, device_param, KERN_RUN_2, pws_cnt, true, slow_iteration);

      while (status_ctx->run_thread_level2 == false) break;

      /**
       * speed
       */

      const float iter_part = (float) (loop_pos + loop_left) / iter;

      const u64 perf_sum_all = (u64) (pws_cnt * iter_part);

      double speed_ms = hc_timer_get (device_param->timer_speed);

      const u32 speed_pos = device_param->speed_pos;

      device_param->speed_cnt[speed_pos] = perf_sum_all;

      device_param->speed_ms[speed_pos] = speed_ms;

      if (user_options->speed_only == true)
      {
        if (speed_ms > 4096) return -1;
      }
    }

    if (hashconfig->opts_type & OPTS_TYPE_HOOK23)
    {
      run_kernel (hashcat_ctx, device_param, KERN_RUN_23, pws_cnt, false, 0);

      CL_err = hc_clEnqueueReadBuffer (hashcat_ctx, device_param->command_queue, device_param->d_hooks, CL_TRUE, 0, device_param->size_hooks, device_param->hooks_buf, 0, NULL, NULL);

      if (CL_err == -1) return -1;

      // do something with data

      CL_err = hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_hooks, CL_TRUE, 0, device_param->size_hooks, device_param->hooks_buf, 0, NULL, NULL);

      if (CL_err == -1) return -1;
    }

    run_kernel (hashcat_ctx, device_param, KERN_RUN_3, pws_cnt, false, 0);
  }

  return 0;
}

int run_kernel (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param, const u32 kern_run, const u32 num, const u32 event_update, const u32 iteration)
{
  hashconfig_t   *hashconfig   = hashcat_ctx->hashconfig;
  status_ctx_t   *status_ctx   = hashcat_ctx->status_ctx;
  user_options_t *user_options = hashcat_ctx->user_options;

  cl_int CL_err = CL_SUCCESS;

  u32 num_elements = num;

  device_param->kernel_params_buf32[34] = num;

  u32 kernel_threads = device_param->kernel_threads;

  while (num_elements % kernel_threads) num_elements++;

  cl_kernel kernel = NULL;

  switch (kern_run)
  {
    case KERN_RUN_1:    kernel = device_param->kernel1;     break;
    case KERN_RUN_12:   kernel = device_param->kernel12;    break;
    case KERN_RUN_2:    kernel = device_param->kernel2;     break;
    case KERN_RUN_23:   kernel = device_param->kernel23;    break;
    case KERN_RUN_3:    kernel = device_param->kernel3;     break;
  }

  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 24, sizeof (cl_uint), device_param->kernel_params[24]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 25, sizeof (cl_uint), device_param->kernel_params[25]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 26, sizeof (cl_uint), device_param->kernel_params[26]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 27, sizeof (cl_uint), device_param->kernel_params[27]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 28, sizeof (cl_uint), device_param->kernel_params[28]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 29, sizeof (cl_uint), device_param->kernel_params[29]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 30, sizeof (cl_uint), device_param->kernel_params[30]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 31, sizeof (cl_uint), device_param->kernel_params[31]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 32, sizeof (cl_uint), device_param->kernel_params[32]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 33, sizeof (cl_uint), device_param->kernel_params[33]);
  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 34, sizeof (cl_uint), device_param->kernel_params[34]);

  if (CL_err == -1) return -1;

  cl_event event;

  if ((hashconfig->opts_type & OPTS_TYPE_PT_BITSLICE) && (user_options->attack_mode == ATTACK_MODE_BF))
  {
    const size_t global_work_size[3] = { num_elements,        32, 1 };
    const size_t local_work_size[3]  = { kernel_threads / 32, 32, 1 };

    CL_err = hc_clEnqueueNDRangeKernel (hashcat_ctx, device_param->command_queue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);

    if (CL_err == -1) return -1;
  }
  else
  {
    if (kern_run == KERN_RUN_2)
    {
      if (hashconfig->opti_type & OPTI_TYPE_SLOW_HASH_SIMD)
      {
        num_elements = CEIL (num_elements / device_param->vector_width);
      }
    }

    while (num_elements % kernel_threads) num_elements++;

    const size_t global_work_size[3] = { num_elements,   1, 1 };
    const size_t local_work_size[3]  = { kernel_threads, 1, 1 };

    CL_err = hc_clEnqueueNDRangeKernel (hashcat_ctx, device_param->command_queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, &event);

    if (CL_err == -1) return -1;
  }

  CL_err = hc_clFlush (hashcat_ctx, device_param->command_queue);

  if (CL_err == -1) return -1;

  if (device_param->nvidia_spin_damp > 0)
  {
    if (status_ctx->devices_status == STATUS_RUNNING)
    {
      if (iteration < EXPECTED_ITERATIONS)
      {
        switch (kern_run)
        {
          case KERN_RUN_1: if (device_param->exec_us_prev1[iteration] > 0) usleep ((useconds_t)(device_param->exec_us_prev1[iteration] * device_param->nvidia_spin_damp)); break;
          case KERN_RUN_2: if (device_param->exec_us_prev2[iteration] > 0) usleep ((useconds_t)(device_param->exec_us_prev2[iteration] * device_param->nvidia_spin_damp)); break;
          case KERN_RUN_3: if (device_param->exec_us_prev3[iteration] > 0) usleep ((useconds_t)(device_param->exec_us_prev3[iteration] * device_param->nvidia_spin_damp)); break;
        }
      }
    }
  }

  CL_err = hc_clWaitForEvents (hashcat_ctx, 1, &event);

  if (CL_err == -1) return -1;

  cl_ulong time_start;
  cl_ulong time_end;

  CL_err |= hc_clGetEventProfilingInfo (hashcat_ctx, event, CL_PROFILING_COMMAND_START, sizeof (time_start), &time_start, NULL);
  CL_err |= hc_clGetEventProfilingInfo (hashcat_ctx, event, CL_PROFILING_COMMAND_END,   sizeof (time_end),   &time_end,   NULL);

  if (CL_err == -1) return -1;

  const double exec_us = (double) (time_end - time_start) / 1000;

  if (status_ctx->devices_status == STATUS_RUNNING)
  {
    if (iteration < EXPECTED_ITERATIONS)
    {
      switch (kern_run)
      {
        case KERN_RUN_1: device_param->exec_us_prev1[iteration] = exec_us; break;
        case KERN_RUN_2: device_param->exec_us_prev2[iteration] = exec_us; break;
        case KERN_RUN_3: device_param->exec_us_prev3[iteration] = exec_us; break;
      }
    }
  }

  if (event_update)
  {
    u32 exec_pos = device_param->exec_pos;

    device_param->exec_ms[exec_pos] = exec_us / 1000;

    exec_pos++;

    if (exec_pos == EXEC_CACHE)
    {
      exec_pos = 0;
    }

    device_param->exec_pos = exec_pos;
  }

  CL_err = hc_clReleaseEvent (hashcat_ctx, event);

  if (CL_err == -1) return -1;

  CL_err = hc_clFinish (hashcat_ctx, device_param->command_queue);

  if (CL_err == -1) return -1;

  return 0;
}

int run_kernel_mp (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param, const u32 kern_run, const u32 num)
{
  cl_int CL_err = CL_SUCCESS;

  u32 num_elements = num;

  switch (kern_run)
  {
    case KERN_RUN_MP:   device_param->kernel_params_mp_buf32[8]   = num; break;
    case KERN_RUN_MP_R: device_param->kernel_params_mp_r_buf32[8] = num; break;
    case KERN_RUN_MP_L: device_param->kernel_params_mp_l_buf32[9] = num; break;
  }

  // causes problems with special threads like in bcrypt
  // const u32 kernel_threads = device_param->kernel_threads;

  u32 kernel_threads = device_param->kernel_threads;

  while (num_elements % kernel_threads) num_elements++;

  cl_kernel kernel = NULL;

  switch (kern_run)
  {
    case KERN_RUN_MP:   kernel = device_param->kernel_mp;   break;
    case KERN_RUN_MP_R: kernel = device_param->kernel_mp_r; break;
    case KERN_RUN_MP_L: kernel = device_param->kernel_mp_l; break;
  }

  switch (kern_run)
  {
    case KERN_RUN_MP:   CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 3, sizeof (cl_ulong), device_param->kernel_params_mp[3]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 4, sizeof (cl_uint),  device_param->kernel_params_mp[4]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 5, sizeof (cl_uint),  device_param->kernel_params_mp[5]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 6, sizeof (cl_uint),  device_param->kernel_params_mp[6]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 7, sizeof (cl_uint),  device_param->kernel_params_mp[7]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 8, sizeof (cl_uint),  device_param->kernel_params_mp[8]);
                        break;
    case KERN_RUN_MP_R: CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 3, sizeof (cl_ulong), device_param->kernel_params_mp_r[3]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 4, sizeof (cl_uint),  device_param->kernel_params_mp_r[4]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 5, sizeof (cl_uint),  device_param->kernel_params_mp_r[5]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 6, sizeof (cl_uint),  device_param->kernel_params_mp_r[6]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 7, sizeof (cl_uint),  device_param->kernel_params_mp_r[7]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 8, sizeof (cl_uint),  device_param->kernel_params_mp_r[8]);
                        break;
    case KERN_RUN_MP_L: CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 3, sizeof (cl_ulong), device_param->kernel_params_mp_l[3]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 4, sizeof (cl_uint),  device_param->kernel_params_mp_l[4]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 5, sizeof (cl_uint),  device_param->kernel_params_mp_l[5]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 6, sizeof (cl_uint),  device_param->kernel_params_mp_l[6]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 7, sizeof (cl_uint),  device_param->kernel_params_mp_l[7]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 8, sizeof (cl_uint),  device_param->kernel_params_mp_l[8]);
                        CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 9, sizeof (cl_uint),  device_param->kernel_params_mp_l[9]);
                        break;
  }

  if (CL_err == -1) return -1;

  const size_t global_work_size[3] = { num_elements,   1, 1 };
  const size_t local_work_size[3]  = { kernel_threads, 1, 1 };

  CL_err = hc_clEnqueueNDRangeKernel (hashcat_ctx, device_param->command_queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);

  if (CL_err == -1) return -1;

  CL_err = hc_clFlush (hashcat_ctx, device_param->command_queue);

  if (CL_err == -1) return -1;

  CL_err = hc_clFinish (hashcat_ctx, device_param->command_queue);

  if (CL_err == -1) return -1;

  return 0;
}

int run_kernel_tm (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param)
{
  cl_int CL_err = CL_SUCCESS;

  const u32 num_elements = 1024; // fixed

  u32 kernel_threads = 32;

  cl_kernel kernel = device_param->kernel_tm;

  const size_t global_work_size[3] = { num_elements,    1, 1 };
  const size_t local_work_size[3]  = { kernel_threads,  1, 1 };

  CL_err = hc_clEnqueueNDRangeKernel (hashcat_ctx, device_param->command_queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);

  if (CL_err == -1) return -1;

  CL_err = hc_clFlush (hashcat_ctx, device_param->command_queue);

  if (CL_err == -1) return -1;

  CL_err = hc_clFinish (hashcat_ctx, device_param->command_queue);

  if (CL_err == -1) return -1;

  return 0;
}

int run_kernel_amp (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param, const u32 num)
{
  cl_int CL_err = CL_SUCCESS;

  u32 num_elements = num;

  device_param->kernel_params_amp_buf32[6] = num_elements;

  // causes problems with special threads like in bcrypt
  // const u32 kernel_threads = device_param->kernel_threads;

  u32 kernel_threads = device_param->kernel_threads;

  while (num_elements % kernel_threads) num_elements++;

  cl_kernel kernel = device_param->kernel_amp;

  CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 6, sizeof (cl_uint), device_param->kernel_params_amp[6]);

  if (CL_err == -1) return -1;

  const size_t global_work_size[3] = { num_elements,    1, 1 };
  const size_t local_work_size[3]  = { kernel_threads,  1, 1 };

  CL_err = hc_clEnqueueNDRangeKernel (hashcat_ctx, device_param->command_queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);

  if (CL_err == -1) return -1;

  CL_err = hc_clFlush (hashcat_ctx, device_param->command_queue);

  if (CL_err == -1) return -1;

  CL_err = hc_clFinish (hashcat_ctx, device_param->command_queue);

  if (CL_err == -1) return -1;

  return 0;
}

int run_kernel_memset (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param, cl_mem buf, const u32 value, const u32 num)
{
  cl_int CL_err = CL_SUCCESS;

  const u32 num16d = num / 16;
  const u32 num16m = num % 16;

  if (num16d)
  {
    device_param->kernel_params_memset_buf32[1] = value;
    device_param->kernel_params_memset_buf32[2] = num16d;

    u32 kernel_threads = device_param->kernel_threads;

    u32 num_elements = num16d;

    while (num_elements % kernel_threads) num_elements++;

    cl_kernel kernel = device_param->kernel_memset;

    CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 0, sizeof (cl_mem),  (void *) &buf);
    CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 1, sizeof (cl_uint), device_param->kernel_params_memset[1]);
    CL_err |= hc_clSetKernelArg (hashcat_ctx, kernel, 2, sizeof (cl_uint), device_param->kernel_params_memset[2]);

    if (CL_err == -1) return -1;

    const size_t global_work_size[3] = { num_elements,   1, 1 };
    const size_t local_work_size[3]  = { kernel_threads, 1, 1 };

    CL_err = hc_clEnqueueNDRangeKernel (hashcat_ctx, device_param->command_queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);

    if (CL_err == -1) return -1;

    CL_err = hc_clFlush (hashcat_ctx, device_param->command_queue);

    if (CL_err == -1) return -1;

    CL_err = hc_clFinish (hashcat_ctx, device_param->command_queue);

    if (CL_err == -1) return -1;
  }

  if (num16m)
  {
    u32 tmp[4];

    tmp[0] = value;
    tmp[1] = value;
    tmp[2] = value;
    tmp[3] = value;

    CL_err = hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, buf, CL_TRUE, num16d * 16, num16m, tmp, 0, NULL, NULL);

    if (CL_err == -1) return -1;
  }

  return 0;
}

int run_kernel_bzero (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param, cl_mem buf, const size_t size)
{
  return run_kernel_memset (hashcat_ctx, device_param, buf, 0, size);
}

int run_copy (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param, const u32 pws_cnt)
{
  combinator_ctx_t     *combinator_ctx      = hashcat_ctx->combinator_ctx;
  hashconfig_t         *hashconfig          = hashcat_ctx->hashconfig;
  user_options_t       *user_options        = hashcat_ctx->user_options;
  user_options_extra_t *user_options_extra  = hashcat_ctx->user_options_extra;

  cl_int CL_err = CL_SUCCESS;

  if (user_options_extra->attack_kern == ATTACK_KERN_STRAIGHT)
  {
    CL_err = hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_pws_buf, CL_TRUE, 0, pws_cnt * sizeof (pw_t), device_param->pws_buf, 0, NULL, NULL);

    if (CL_err == -1) return -1;
  }
  else if (user_options_extra->attack_kern == ATTACK_KERN_COMBI)
  {
    if (user_options->attack_mode == ATTACK_MODE_COMBI)
    {
      if (combinator_ctx->combs_mode == COMBINATOR_MODE_BASE_RIGHT)
      {
        if (hashconfig->opts_type & OPTS_TYPE_PT_ADD01)
        {
          for (u32 i = 0; i < pws_cnt; i++)
          {
            const u32 pw_len = device_param->pws_buf[i].pw_len;

            u8 *ptr = (u8 *) device_param->pws_buf[i].i;

            ptr[pw_len] = 0x01;
          }
        }
        else if (hashconfig->opts_type & OPTS_TYPE_PT_ADD80)
        {
          for (u32 i = 0; i < pws_cnt; i++)
          {
            const u32 pw_len = device_param->pws_buf[i].pw_len;

            u8 *ptr = (u8 *) device_param->pws_buf[i].i;

            ptr[pw_len] = 0x80;
          }
        }
      }
    }
    else if (user_options->attack_mode == ATTACK_MODE_HYBRID2)
    {
      if (hashconfig->opts_type & OPTS_TYPE_PT_ADD01)
      {
        for (u32 i = 0; i < pws_cnt; i++)
        {
          const u32 pw_len = device_param->pws_buf[i].pw_len;

          u8 *ptr = (u8 *) device_param->pws_buf[i].i;

          ptr[pw_len] = 0x01;
        }
      }
      else if (hashconfig->opts_type & OPTS_TYPE_PT_ADD80)
      {
        for (u32 i = 0; i < pws_cnt; i++)
        {
          const u32 pw_len = device_param->pws_buf[i].pw_len;

          u8 *ptr = (u8 *) device_param->pws_buf[i].i;

          ptr[pw_len] = 0x80;
        }
      }
    }

    CL_err = hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_pws_buf, CL_TRUE, 0, pws_cnt * sizeof (pw_t), device_param->pws_buf, 0, NULL, NULL);

    if (CL_err == -1) return -1;
  }
  else if (user_options_extra->attack_kern == ATTACK_KERN_BF)
  {
    const u64 off = device_param->words_off;

    device_param->kernel_params_mp_l_buf64[3] = off;

    run_kernel_mp (hashcat_ctx, device_param, KERN_RUN_MP_L, pws_cnt);
  }

  return 0;
}

int run_cracker (hashcat_ctx_t *hashcat_ctx, hc_device_param_t *device_param, const u32 pws_cnt)
{
  combinator_ctx_t      *combinator_ctx     = hashcat_ctx->combinator_ctx;
  hashconfig_t          *hashconfig         = hashcat_ctx->hashconfig;
  hashes_t              *hashes             = hashcat_ctx->hashes;
  mask_ctx_t            *mask_ctx           = hashcat_ctx->mask_ctx;
  status_ctx_t          *status_ctx         = hashcat_ctx->status_ctx;
  straight_ctx_t        *straight_ctx       = hashcat_ctx->straight_ctx;
  user_options_t        *user_options       = hashcat_ctx->user_options;
  user_options_extra_t  *user_options_extra = hashcat_ctx->user_options_extra;

  // init speed timer

  u32 speed_pos = device_param->speed_pos;

  #if defined (_POSIX)
  if (device_param->timer_speed.tv_sec == 0)
  {
    hc_timer_set (&device_param->timer_speed);
  }
  #endif

  #if defined (_WIN)
  if (device_param->timer_speed.QuadPart == 0)
  {
    hc_timer_set (&device_param->timer_speed);
  }
  #endif

  // find higest password length, this is for optimization stuff

  u32 highest_pw_len = 0;

  if (user_options_extra->attack_kern == ATTACK_KERN_STRAIGHT)
  {
  }
  else if (user_options_extra->attack_kern == ATTACK_KERN_COMBI)
  {
  }
  else if (user_options_extra->attack_kern == ATTACK_KERN_BF)
  {
    highest_pw_len = device_param->kernel_params_mp_l_buf32[4]
                   + device_param->kernel_params_mp_l_buf32[5];
  }

  // we make use of this in status view

  device_param->outerloop_pos  = 0;
  device_param->outerloop_left = pws_cnt;

  // loop start: most outer loop = salt iteration, then innerloops (if multi)

  for (u32 salt_pos = 0; salt_pos < hashes->salts_cnt; salt_pos++)
  {
    while (status_ctx->devices_status == STATUS_PAUSED) hc_sleep (1);

    salt_t *salt_buf = &hashes->salts_buf[salt_pos];

    device_param->kernel_params_buf32[27] = salt_pos;
    device_param->kernel_params_buf32[31] = salt_buf->digests_cnt;
    device_param->kernel_params_buf32[32] = salt_buf->digests_offset;

    FILE *combs_fp = device_param->combs_fp;

    if (user_options->attack_mode == ATTACK_MODE_COMBI)
    {
      rewind (combs_fp);
    }

    // iteration type

    u32 innerloop_step = 0;
    u32 innerloop_cnt  = 0;

    if   (hashconfig->attack_exec == ATTACK_EXEC_INSIDE_KERNEL) innerloop_step = device_param->kernel_loops;
    else                                                        innerloop_step = 1;

    if      (user_options_extra->attack_kern == ATTACK_KERN_STRAIGHT)  innerloop_cnt  = straight_ctx->kernel_rules_cnt;
    else if (user_options_extra->attack_kern == ATTACK_KERN_COMBI)     innerloop_cnt  = combinator_ctx->combs_cnt;
    else if (user_options_extra->attack_kern == ATTACK_KERN_BF)        innerloop_cnt  = mask_ctx->bfs_cnt;

    // innerloops

    for (u32 innerloop_pos = 0; innerloop_pos < innerloop_cnt; innerloop_pos += innerloop_step)
    {
      while (status_ctx->devices_status == STATUS_PAUSED) hc_sleep (1);

      u32 fast_iteration = 0;

      u32 innerloop_left = innerloop_cnt - innerloop_pos;

      if (innerloop_left > innerloop_step)
      {
        innerloop_left = innerloop_step;

        fast_iteration = 1;
      }

      device_param->innerloop_pos  = innerloop_pos;
      device_param->innerloop_left = innerloop_left;

      device_param->kernel_params_buf32[30] = innerloop_left;

      // i think we can get rid of this
      if (innerloop_left == false)
      {
        puts ("bug, how should this happen????\n");

        continue;
      }

      if (hashes->salts_shown[salt_pos] == 1)
      {
        status_ctx->words_progress_done[salt_pos] += (u64) pws_cnt * (u64) innerloop_left;

        continue;
      }

      // initialize amplifiers

      if (user_options->attack_mode == ATTACK_MODE_COMBI)
      {
        char *line_buf = combinator_ctx->scratch_buf;

        u32 i = 0;

        while (i < innerloop_left)
        {
          if (feof (combs_fp)) break;

          int line_len = fgetl (combs_fp, line_buf);

          if (line_len >= PW_MAX1) continue;

          line_len = convert_from_hex (hashcat_ctx, line_buf, line_len);

          char *line_buf_new = line_buf;

          if (run_rule_engine (user_options_extra->rule_len_r, user_options->rule_buf_r))
          {
            char rule_buf_out[BLOCK_SIZE] = { 0 };

            int rule_len_out = _old_apply_rule (user_options->rule_buf_r, user_options_extra->rule_len_r, line_buf, line_len, rule_buf_out);

            if (rule_len_out < 0)
            {
              status_ctx->words_progress_rejected[salt_pos] += pws_cnt;

              continue;
            }

            line_len = rule_len_out;

            line_buf_new = rule_buf_out;
          }

          line_len = MIN (line_len, PW_DICTMAX);

          u8 *ptr = (u8 *) device_param->combs_buf[i].i;

          memcpy (ptr, line_buf_new, line_len);

          memset (ptr + line_len, 0, PW_DICTMAX1 - line_len);

          if (hashconfig->opts_type & OPTS_TYPE_PT_UPPER)
          {
            uppercase (ptr, line_len);
          }

          if (combinator_ctx->combs_mode == COMBINATOR_MODE_BASE_LEFT)
          {
            if (hashconfig->opts_type & OPTS_TYPE_PT_ADD80)
            {
              ptr[line_len] = 0x80;
            }

            if (hashconfig->opts_type & OPTS_TYPE_PT_ADD01)
            {
              ptr[line_len] = 0x01;
            }
          }

          device_param->combs_buf[i].pw_len = line_len;

          i++;
        }

        for (u32 j = i; j < innerloop_left; j++)
        {
          device_param->combs_buf[j].i[0] = 0;
          device_param->combs_buf[j].i[1] = 0;
          device_param->combs_buf[j].i[2] = 0;
          device_param->combs_buf[j].i[3] = 0;
          device_param->combs_buf[j].i[4] = 0;
          device_param->combs_buf[j].i[5] = 0;
          device_param->combs_buf[j].i[6] = 0;
          device_param->combs_buf[j].i[7] = 0;

          device_param->combs_buf[j].pw_len = 0;
        }

        innerloop_left = i;
      }
      else if (user_options->attack_mode == ATTACK_MODE_BF)
      {
        u64 off = innerloop_pos;

        device_param->kernel_params_mp_r_buf64[3] = off;

        run_kernel_mp (hashcat_ctx, device_param, KERN_RUN_MP_R, innerloop_left);
      }
      else if (user_options->attack_mode == ATTACK_MODE_HYBRID1)
      {
        u64 off = innerloop_pos;

        device_param->kernel_params_mp_buf64[3] = off;

        run_kernel_mp (hashcat_ctx, device_param, KERN_RUN_MP, innerloop_left);
      }
      else if (user_options->attack_mode == ATTACK_MODE_HYBRID2)
      {
        u64 off = innerloop_pos;

        device_param->kernel_params_mp_buf64[3] = off;

        run_kernel_mp (hashcat_ctx, device_param, KERN_RUN_MP, innerloop_left);
      }

      // copy amplifiers

      if (user_options->attack_mode == ATTACK_MODE_STRAIGHT)
      {
        cl_int CL_err = hc_clEnqueueCopyBuffer (hashcat_ctx, device_param->command_queue, device_param->d_rules, device_param->d_rules_c, innerloop_pos * sizeof (kernel_rule_t), 0, innerloop_left * sizeof (kernel_rule_t), 0, NULL, NULL);

        if (CL_err == -1) return -1;
      }
      else if (user_options->attack_mode == ATTACK_MODE_COMBI)
      {
        cl_int CL_err = hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_combs_c, CL_TRUE, 0, innerloop_left * sizeof (comb_t), device_param->combs_buf, 0, NULL, NULL);

        if (CL_err == -1) return -1;
      }
      else if (user_options->attack_mode == ATTACK_MODE_BF)
      {
        cl_int CL_err = hc_clEnqueueCopyBuffer (hashcat_ctx, device_param->command_queue, device_param->d_bfs, device_param->d_bfs_c, 0, 0, innerloop_left * sizeof (bf_t), 0, NULL, NULL);

        if (CL_err == -1) return -1;
      }
      else if (user_options->attack_mode == ATTACK_MODE_HYBRID1)
      {
        cl_int CL_err = hc_clEnqueueCopyBuffer (hashcat_ctx, device_param->command_queue, device_param->d_combs, device_param->d_combs_c, 0, 0, innerloop_left * sizeof (comb_t), 0, NULL, NULL);

        if (CL_err == -1) return -1;
      }
      else if (user_options->attack_mode == ATTACK_MODE_HYBRID2)
      {
        cl_int CL_err = hc_clEnqueueCopyBuffer (hashcat_ctx, device_param->command_queue, device_param->d_combs, device_param->d_combs_c, 0, 0, innerloop_left * sizeof (comb_t), 0, NULL, NULL);

        if (CL_err == -1) return -1;
      }

      if (user_options->speed_only == true)
      {
        hc_timer_set (&device_param->timer_speed);
      }

      int rc = choose_kernel (hashcat_ctx, device_param, highest_pw_len, pws_cnt, fast_iteration, salt_pos);

      if (rc == -1) return -1;

      /**
       * result
       */

      if (user_options->speed_only == false)
      {
        check_cracked (hashcat_ctx, device_param, salt_pos);
      }

      /**
       * progress
       */

      u64 perf_sum_all = (u64) pws_cnt * (u64) innerloop_left;

      hc_thread_mutex_lock (status_ctx->mux_counter);

      status_ctx->words_progress_done[salt_pos] += perf_sum_all;

      hc_thread_mutex_unlock (status_ctx->mux_counter);

      /**
       * speed
       */

      double speed_ms = hc_timer_get (device_param->timer_speed);

      hc_timer_set (&device_param->timer_speed);

      // current speed

      //hc_thread_mutex_lock (status_ctx->mux_display);

      device_param->speed_cnt[speed_pos] = perf_sum_all;

      device_param->speed_ms[speed_pos] = speed_ms;

      //hc_thread_mutex_unlock (status_ctx->mux_display);

      speed_pos++;

      if (speed_pos == SPEED_CACHE)
      {
        speed_pos = 0;
      }

      /**
       * benchmark
       */

      if (user_options->speed_only == true) break;

      if (status_ctx->run_thread_level2 == false) break;
    }

    //status screen makes use of this, can't reset here
    //device_param->innerloop_pos  = 0;
    //device_param->innerloop_left = 0;

    if (status_ctx->run_thread_level2 == false) break;
  }

  //status screen makes use of this, can't reset here
  //device_param->outerloop_pos  = 0;
  //device_param->outerloop_left = 0;

  device_param->speed_pos = speed_pos;

  return 0;
}

int opencl_ctx_init (hashcat_ctx_t *hashcat_ctx)
{
  opencl_ctx_t   *opencl_ctx   = hashcat_ctx->opencl_ctx;
  user_options_t *user_options = hashcat_ctx->user_options;

  opencl_ctx->enabled = false;

  if (user_options->keyspace    == true) return 0;
  if (user_options->left        == true) return 0;
  if (user_options->show        == true) return 0;
  if (user_options->usage       == true) return 0;
  if (user_options->version     == true) return 0;

  hc_device_param_t *devices_param = (hc_device_param_t *) hccalloc (hashcat_ctx, DEVICES_MAX, sizeof (hc_device_param_t)); VERIFY_PTR (devices_param);

  opencl_ctx->devices_param = devices_param;

  /**
   * Load and map OpenCL library calls
   */

  OCL_PTR *ocl = (OCL_PTR *) hcmalloc (hashcat_ctx, sizeof (OCL_PTR)); VERIFY_PTR (ocl);

  opencl_ctx->ocl = ocl;

  const int rc_ocl_init = ocl_init (hashcat_ctx);

  if (rc_ocl_init == -1) return -1;

  /**
   * OpenCL platform selection
   */

  u32 opencl_platforms_filter;

  const int rc_platforms_filter = setup_opencl_platforms_filter (hashcat_ctx, user_options->opencl_platforms, &opencl_platforms_filter);

  if (rc_platforms_filter == -1) return -1;

  opencl_ctx->opencl_platforms_filter = opencl_platforms_filter;

  /**
   * OpenCL device selection
   */

  u32 devices_filter;

  const int rc_devices_filter = setup_devices_filter (hashcat_ctx, user_options->opencl_devices, &devices_filter);

  if (rc_devices_filter == -1) return -1;

  opencl_ctx->devices_filter = devices_filter;

  /**
   * OpenCL device type selection
   */

  cl_device_type device_types_filter;

  const int rc_device_types_filter = setup_device_types_filter (hashcat_ctx, user_options->opencl_device_types, &device_types_filter);

  if (rc_device_types_filter == -1) return -1;

  opencl_ctx->device_types_filter = device_types_filter;

  /**
   * OpenCL platforms: detect
   */

  cl_uint         platforms_cnt         = 0;
  cl_platform_id *platforms             = (cl_platform_id *) hccalloc (hashcat_ctx, CL_PLATFORMS_MAX, sizeof (cl_platform_id)); VERIFY_PTR (platforms);
  cl_uint         platform_devices_cnt  = 0;
  cl_device_id   *platform_devices      = (cl_device_id *) hccalloc (hashcat_ctx, DEVICES_MAX, sizeof (cl_device_id)); VERIFY_PTR (platform_devices);

  cl_int CL_err = hc_clGetPlatformIDs (hashcat_ctx, CL_PLATFORMS_MAX, platforms, &platforms_cnt);

  if (CL_err == -1) return -1;

  if (platforms_cnt == 0)
  {
    event_log_info (hashcat_ctx, "ATTENTION! No OpenCL compatible platform found");
    event_log_info (hashcat_ctx, "");
    event_log_info (hashcat_ctx, "You're probably missing the OpenCL runtime installation");
    event_log_info (hashcat_ctx, "  AMD users require AMD drivers 14.9 or later (recommended 15.12 or later)");
    event_log_info (hashcat_ctx, "  Intel users require Intel OpenCL Runtime 14.2 or later (recommended 15.1 or later)");
    event_log_info (hashcat_ctx, "  NVidia users require NVidia drivers 346.59 or later (recommended 361.x or later)");
    event_log_info (hashcat_ctx, "");

    return -1;
  }

  if (opencl_platforms_filter != (u32) -1)
  {
    u32 platform_cnt_mask = ~(((u32) -1 >> platforms_cnt) << platforms_cnt);

    if (opencl_platforms_filter > platform_cnt_mask)
    {
      event_log_error (hashcat_ctx, "The platform selected by the --opencl-platforms parameter is larger than the number of available platforms (%d)", platforms_cnt);

      return -1;
    }
  }

  if (user_options->opencl_device_types == NULL)
  {
    /**
     * OpenCL device types:
     *   In case the user did not specify --opencl-device-types and the user runs hashcat in a system with only a CPU only he probably want to use that CPU.
     */

    cl_device_type device_types_all = 0;

    for (u32 platform_id = 0; platform_id < platforms_cnt; platform_id++)
    {
      if ((opencl_platforms_filter & (1u << platform_id)) == 0) continue;

      cl_platform_id platform = platforms[platform_id];

      cl_int CL_err = hc_clGetDeviceIDs (hashcat_ctx, platform, CL_DEVICE_TYPE_ALL, DEVICES_MAX, platform_devices, &platform_devices_cnt);

      if (CL_err != CL_SUCCESS) continue;

      for (u32 platform_devices_id = 0; platform_devices_id < platform_devices_cnt; platform_devices_id++)
      {
        cl_device_id device = platform_devices[platform_devices_id];

        cl_device_type device_type;

        cl_int CL_err = hc_clGetDeviceInfo (hashcat_ctx, device, CL_DEVICE_TYPE, sizeof (device_type), &device_type, NULL);

        if (CL_err == -1) return -1;

        device_types_all |= device_type;
      }
    }

    // In such a case, automatically enable cpu_md5CPU device type support, since it's disabled by default.

    if ((device_types_all & (CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR)) == 0)
    {
      device_types_filter |= CL_DEVICE_TYPE_CPU;
    }

    // In another case, when the user uses --stdout, using CPU devices is much faster to setup
    // If we have a CPU device, force it to be used

    if (user_options->stdout_flag == true)
    {
      if (device_types_all & CL_DEVICE_TYPE_CPU)
      {
        device_types_filter = CL_DEVICE_TYPE_CPU;
      }
    }

    opencl_ctx->device_types_filter = device_types_filter;
  }

  opencl_ctx->enabled = true;

  opencl_ctx->platforms_cnt         = platforms_cnt;
  opencl_ctx->platforms             = platforms;
  opencl_ctx->platform_devices_cnt  = platform_devices_cnt;
  opencl_ctx->platform_devices      = platform_devices;

  return 0;
}

void opencl_ctx_destroy (hashcat_ctx_t *hashcat_ctx)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  if (opencl_ctx->enabled == false) return;

  ocl_close (hashcat_ctx);

  hcfree (opencl_ctx->devices_param);

  hcfree (opencl_ctx->platforms);

  hcfree (opencl_ctx->platform_devices);

  memset (opencl_ctx, 0, sizeof (opencl_ctx_t));
}

int opencl_ctx_devices_init (hashcat_ctx_t *hashcat_ctx, const int comptime)
{
  opencl_ctx_t   *opencl_ctx   = hashcat_ctx->opencl_ctx;
  user_options_t *user_options = hashcat_ctx->user_options;

  if (opencl_ctx->enabled == false) return 0;

  /**
   * OpenCL devices: simply push all devices from all platforms into the same device array
   */

  cl_uint         platforms_cnt         = opencl_ctx->platforms_cnt;
  cl_platform_id *platforms             = opencl_ctx->platforms;
  cl_uint         platform_devices_cnt  = opencl_ctx->platform_devices_cnt;
  cl_device_id   *platform_devices      = opencl_ctx->platform_devices;

  bool need_adl     = false;
  bool need_nvml    = false;
  bool need_nvapi   = false;
  bool need_xnvctrl = false;

  u32 devices_cnt = 0;

  u32 devices_active = 0;

  if (user_options->opencl_info == true)
  {
    fprintf (stdout, "OpenCL Info:\n");
  }

  for (u32 platform_id = 0; platform_id < platforms_cnt; platform_id++)
  {
    cl_int CL_err = CL_SUCCESS;

    cl_platform_id platform = platforms[platform_id];

    char platform_vendor[HCBUFSIZ_TINY] = { 0 };

    CL_err = hc_clGetPlatformInfo (hashcat_ctx, platform, CL_PLATFORM_VENDOR, sizeof (platform_vendor), platform_vendor, NULL);

    if (CL_err == -1) return -1;

    // find our own platform vendor because pocl and mesa are pushing original vendor_id through opencl
    // this causes trouble with vendor id based macros
    // we'll assign generic to those without special optimization available

    cl_uint platform_vendor_id = 0;

    if (strcmp (platform_vendor, CL_VENDOR_AMD) == 0)
    {
      platform_vendor_id = VENDOR_ID_AMD;
    }
    else if (strcmp (platform_vendor, CL_VENDOR_AMD_USE_INTEL) == 0)
    {
      platform_vendor_id = VENDOR_ID_AMD_USE_INTEL;
    }
    else if (strcmp (platform_vendor, CL_VENDOR_APPLE) == 0)
    {
      platform_vendor_id = VENDOR_ID_APPLE;
    }
    else if (strcmp (platform_vendor, CL_VENDOR_INTEL_BEIGNET) == 0)
    {
      platform_vendor_id = VENDOR_ID_INTEL_BEIGNET;
    }
    else if (strcmp (platform_vendor, CL_VENDOR_INTEL_SDK) == 0)
    {
      platform_vendor_id = VENDOR_ID_INTEL_SDK;
    }
    else if (strcmp (platform_vendor, CL_VENDOR_MESA) == 0)
    {
      platform_vendor_id = VENDOR_ID_MESA;
    }
    else if (strcmp (platform_vendor, CL_VENDOR_NV) == 0)
    {
      platform_vendor_id = VENDOR_ID_NV;
    }
    else if (strcmp (platform_vendor, CL_VENDOR_POCL) == 0)
    {
      platform_vendor_id = VENDOR_ID_POCL;
    }
    else
    {
      platform_vendor_id = VENDOR_ID_GENERIC;
    }

    u32 platform_skipped = ((opencl_ctx->opencl_platforms_filter & (1u << platform_id)) == 0);

    CL_err = hc_clGetDeviceIDs (hashcat_ctx, platform, CL_DEVICE_TYPE_ALL, DEVICES_MAX, platform_devices, &platform_devices_cnt);

    if (CL_err != CL_SUCCESS)
    {
      //event_log_error (hashcat_ctx, "clGetDeviceIDs(): %s", val2cstr_cl (CL_err));

      //return -1;

      platform_skipped = 2;
    }

    if (user_options->opencl_info == true)
    {
      char platform_name[HCBUFSIZ_TINY] = { 0 };

      CL_err = hc_clGetPlatformInfo (hashcat_ctx, platform, CL_PLATFORM_NAME, HCBUFSIZ_TINY, platform_name, NULL);

      if (CL_err == -1) return -1;

      char platform_version[HCBUFSIZ_TINY] = { 0 };

      CL_err = hc_clGetPlatformInfo (hashcat_ctx, platform, CL_PLATFORM_VERSION, sizeof (platform_version), platform_version, NULL);

      if (CL_err == -1) return -1;

      fprintf (stdout, "\nPlatform ID #%u\n  Vendor   : %s\n  Name     : %s\n  Version  : %s\n\n", platform_id + 1, platform_vendor, platform_name, platform_version);
    }

    if ((user_options->benchmark == true || user_options->speed_only == true || user_options->quiet == false))
    {
      if (user_options->machine_readable == false)
      {
        if (platform_skipped == 0)
        {
          const int len = event_log_info (hashcat_ctx, "OpenCL Platform #%u: %s", platform_id + 1, platform_vendor);

          char line[256] = { 0 };

          for (int i = 0; i < len; i++) line[i] = '=';

          event_log_info (hashcat_ctx, line);
        }
        else if (platform_skipped == 1)
        {
          event_log_info (hashcat_ctx, "OpenCL Platform #%u: %s, skipped", platform_id + 1, platform_vendor);
          event_log_info (hashcat_ctx, "");
        }
        else if (platform_skipped == 2)
        {
          event_log_info (hashcat_ctx, "OpenCL Platform #%u: %s, skipped! No OpenCL compatible devices found", platform_id + 1, platform_vendor);
          event_log_info (hashcat_ctx, "");
        }
      }
    }

    if (platform_skipped == 1) continue;
    if (platform_skipped == 2) continue;

    hc_device_param_t *devices_param = opencl_ctx->devices_param;

    for (u32 platform_devices_id = 0; platform_devices_id < platform_devices_cnt; platform_devices_id++)
    {
      size_t param_value_size = 0;

      const u32 device_id = devices_cnt;

      hc_device_param_t *device_param = &devices_param[device_id];

      device_param->platform_vendor_id = platform_vendor_id;

      device_param->device = platform_devices[platform_devices_id];

      device_param->device_id = device_id;

      device_param->platform_devices_id = platform_devices_id;

      device_param->platform = platform;

      // device_type

      cl_device_type device_type;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_TYPE, sizeof (device_type), &device_type, NULL);

      if (CL_err == -1) return -1;

      device_type &= ~CL_DEVICE_TYPE_DEFAULT;

      device_param->device_type = device_type;

      // device_name

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_NAME, 0, NULL, &param_value_size);

      if (CL_err == -1) return -1;

      char *device_name = (char *) hcmalloc (hashcat_ctx, param_value_size); VERIFY_PTR (device_name);

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_NAME, param_value_size, device_name, NULL);

      if (CL_err == -1) return -1;

      device_param->device_name = device_name;

      // device_vendor

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_VENDOR, 0, NULL, &param_value_size);

      if (CL_err == -1) return -1;

      char *device_vendor = (char *) hcmalloc (hashcat_ctx, param_value_size); VERIFY_PTR (device_vendor);

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_VENDOR, param_value_size, device_vendor, NULL);

      if (CL_err == -1) return -1;

      device_param->device_vendor = device_vendor;

      cl_uint device_vendor_id = 0;

      if (strcmp (device_vendor, CL_VENDOR_AMD) == 0)
      {
        device_vendor_id = VENDOR_ID_AMD;
      }
      else if (strcmp (device_vendor, CL_VENDOR_AMD_USE_INTEL) == 0)
      {
        device_vendor_id = VENDOR_ID_AMD_USE_INTEL;
      }
      else if (strcmp (device_vendor, CL_VENDOR_APPLE) == 0)
      {
        device_vendor_id = VENDOR_ID_APPLE;
      }
      else if (strcmp (device_vendor, CL_VENDOR_INTEL_BEIGNET) == 0)
      {
        device_vendor_id = VENDOR_ID_INTEL_BEIGNET;
      }
      else if (strcmp (device_vendor, CL_VENDOR_INTEL_SDK) == 0)
      {
        device_vendor_id = VENDOR_ID_INTEL_SDK;
      }
      else if (strcmp (device_vendor, CL_VENDOR_MESA) == 0)
      {
        device_vendor_id = VENDOR_ID_MESA;
      }
      else if (strcmp (device_vendor, CL_VENDOR_NV) == 0)
      {
        device_vendor_id = VENDOR_ID_NV;
      }
      else if (strcmp (device_vendor, CL_VENDOR_POCL) == 0)
      {
        device_vendor_id = VENDOR_ID_POCL;
      }
      else
      {
        device_vendor_id = VENDOR_ID_GENERIC;
      }

      device_param->device_vendor_id = device_vendor_id;

      // device_version

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_VERSION, 0, NULL, &param_value_size);

      if (CL_err == -1) return -1;

      char *device_version = (char *) hcmalloc (hashcat_ctx, param_value_size); VERIFY_PTR (device_version);

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_VERSION, param_value_size, device_version, NULL);

      if (CL_err == -1) return -1;

      device_param->device_version = device_version;

      // device_opencl_version

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &param_value_size);

      if (CL_err == -1) return -1;

      char *device_opencl_version = (char *) hcmalloc (hashcat_ctx, param_value_size); VERIFY_PTR (device_opencl_version);

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_OPENCL_C_VERSION, param_value_size, device_opencl_version, NULL);

      if (CL_err == -1) return -1;

      device_param->opencl_v12 = device_opencl_version[9] > '1' || device_opencl_version[11] >= '2';

      // max_compute_units

      cl_uint device_processors;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof (device_processors), &device_processors, NULL);

      if (CL_err == -1) return -1;

      device_param->device_processors = device_processors;

      // device_maxmem_alloc
      // note we'll limit to 2gb, otherwise this causes all kinds of weird errors because of possible integer overflows in opencl runtimes

      cl_ulong device_maxmem_alloc;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof (device_maxmem_alloc), &device_maxmem_alloc, NULL);

      if (CL_err == -1) return -1;

      device_param->device_maxmem_alloc = MIN (device_maxmem_alloc, 0x7fffffff);

      // device_global_mem

      cl_ulong device_global_mem;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof (device_global_mem), &device_global_mem, NULL);

      if (CL_err == -1) return -1;

      device_param->device_global_mem = device_global_mem;

      // max_work_group_size

      size_t device_maxworkgroup_size;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof (device_maxworkgroup_size), &device_maxworkgroup_size, NULL);

      if (CL_err == -1) return -1;

      device_param->device_maxworkgroup_size = device_maxworkgroup_size;

      // max_clock_frequency

      cl_uint device_maxclock_frequency;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof (device_maxclock_frequency), &device_maxclock_frequency, NULL);

      if (CL_err == -1) return -1;

      device_param->device_maxclock_frequency = device_maxclock_frequency;

      // device_endian_little

      cl_bool device_endian_little;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_ENDIAN_LITTLE, sizeof (device_endian_little), &device_endian_little, NULL);

      if (CL_err == -1) return -1;

      if (device_endian_little == CL_FALSE)
      {
        event_log_info (hashcat_ctx, "- Device #%u: WARNING: Not a little endian device", device_id + 1);

        device_param->skipped = 1;
      }

      // device_available

      cl_bool device_available;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_AVAILABLE, sizeof (device_available), &device_available, NULL);

      if (CL_err == -1) return -1;

      if (device_available == CL_FALSE)
      {
        event_log_info (hashcat_ctx, "- Device #%u: WARNING: Device not available", device_id + 1);

        device_param->skipped = 1;
      }

      // device_compiler_available

      cl_bool device_compiler_available;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_COMPILER_AVAILABLE, sizeof (device_compiler_available), &device_compiler_available, NULL);

      if (CL_err == -1) return -1;

      if (device_compiler_available == CL_FALSE)
      {
        event_log_info (hashcat_ctx, "- Device #%u: WARNING: No compiler available for device", device_id + 1);

        device_param->skipped = 1;
      }

      // device_execution_capabilities

      cl_device_exec_capabilities device_execution_capabilities;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof (device_execution_capabilities), &device_execution_capabilities, NULL);

      if (CL_err == -1) return -1;

      if ((device_execution_capabilities & CL_EXEC_KERNEL) == 0)
      {
        event_log_info (hashcat_ctx, "- Device #%u: WARNING: Device does not support executing kernels", device_id + 1);

        device_param->skipped = 1;
      }

      // device_extensions

      size_t device_extensions_size;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_EXTENSIONS, 0, NULL, &device_extensions_size);

      if (CL_err == -1) return -1;

      char *device_extensions = hcmalloc (hashcat_ctx, device_extensions_size + 1); VERIFY_PTR (device_extensions);

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_EXTENSIONS, device_extensions_size, device_extensions, NULL);

      if (CL_err == -1) return -1;

      if (strstr (device_extensions, "base_atomics") == 0)
      {
        event_log_info (hashcat_ctx, "- Device #%u: WARNING: Device does not support base atomics", device_id + 1);

        device_param->skipped = 1;
      }

      if (strstr (device_extensions, "byte_addressable_store") == 0)
      {
        event_log_info (hashcat_ctx, "- Device #%u: WARNING: Device does not support byte addressable store", device_id + 1);

        device_param->skipped = 1;
      }

      hcfree (device_extensions);

      // device_local_mem_size

      cl_ulong device_local_mem_size;

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof (device_local_mem_size), &device_local_mem_size, NULL);

      if (CL_err == -1) return -1;

      if (device_local_mem_size < 32768)
      {
        event_log_info (hashcat_ctx, "- Device #%u: WARNING: Device local mem size is too small", device_id + 1);

        device_param->skipped = 1;
      }

      // If there's both an Intel CPU and an AMD OpenCL runtime it's a tricky situation
      // Both platforms support CPU device types and therefore both will try to use 100% of the physical resources
      // This results in both utilizing it for 50%
      // However, Intel has much better SIMD control over their own hardware
      // It makes sense to give them full control over their own hardware

      if (device_type & CL_DEVICE_TYPE_CPU)
      {
        if (device_param->device_vendor_id == VENDOR_ID_AMD_USE_INTEL)
        {
          if (user_options->force == 0)
          {
            event_log_info (hashcat_ctx, "- Device #%u: WARNING: Not a native Intel OpenCL runtime, expect massive speed loss", device_id + 1);
            event_log_info (hashcat_ctx, "             You can use --force to override this but do not post error reports if you do so");

            device_param->skipped = 1;
          }
        }
      }

      // skipped

      device_param->skipped |= ((opencl_ctx->devices_filter      & (1u << device_id)) == 0);
      device_param->skipped |= ((opencl_ctx->device_types_filter & (device_type))    == 0);

      // driver_version

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DRIVER_VERSION, 0, NULL, &param_value_size);

      if (CL_err == -1) return -1;

      char *driver_version = (char *) hcmalloc (hashcat_ctx, param_value_size); VERIFY_PTR (driver_version);

      CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DRIVER_VERSION, param_value_size, driver_version, NULL);

      if (CL_err == -1) return -1;

      device_param->driver_version = driver_version;

      // device_name_chksum

      char *device_name_chksum = (char *) hcmalloc (hashcat_ctx, HCBUFSIZ_TINY); VERIFY_PTR (device_name_chksum);

      #if defined (__x86_64__)
      snprintf (device_name_chksum, HCBUFSIZ_TINY - 1, "%u-%u-%u-%s-%s-%s-%u", 64, device_param->platform_vendor_id, device_param->vector_width, device_param->device_name, device_param->device_version, device_param->driver_version, comptime);
      #else
      snprintf (device_name_chksum, HCBUFSIZ_TINY - 1, "%u-%u-%u-%s-%s-%s-%u", 32, device_param->platform_vendor_id, device_param->vector_width, device_param->device_name, device_param->device_version, device_param->driver_version, comptime);
      #endif

      u32 device_name_digest[4] = { 0 };

      md5_64 ((u32 *) device_name_chksum, device_name_digest);

      snprintf (device_name_chksum, HCBUFSIZ_TINY - 1, "%08x", device_name_digest[0]);

      device_param->device_name_chksum = device_name_chksum;

      // vendor specific

      if (device_param->device_type & CL_DEVICE_TYPE_GPU)
      {
        if ((device_param->platform_vendor_id == VENDOR_ID_AMD) && (device_param->device_vendor_id == VENDOR_ID_AMD))
        {
          need_adl = true;
        }

        if ((device_param->platform_vendor_id == VENDOR_ID_NV) && (device_param->device_vendor_id == VENDOR_ID_NV))
        {
          need_nvml = true;

          #if defined (__linux__)
          need_xnvctrl = true;
          #endif

          #if defined (_WIN)
          need_nvapi = true;
          #endif
        }
      }

      if (device_type & CL_DEVICE_TYPE_GPU)
      {
        if (device_vendor_id == VENDOR_ID_NV)
        {
          cl_uint kernel_exec_timeout = 0;

          #define CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV            0x4005

          CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV, sizeof (kernel_exec_timeout), &kernel_exec_timeout, NULL);

          if (CL_err == -1) return -1;

          device_param->kernel_exec_timeout = kernel_exec_timeout;

          cl_uint sm_minor = 0;
          cl_uint sm_major = 0;

          #define CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV       0x4000
          #define CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV       0x4001

          CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV, sizeof (sm_minor), &sm_minor, NULL);

          if (CL_err == -1) return -1;

          CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV, sizeof (sm_major), &sm_major, NULL);

          if (CL_err == -1) return -1;

          device_param->sm_minor = sm_minor;
          device_param->sm_major = sm_major;

          // CPU burning loop damper
          // Value is given as number between 0-100
          // By default 100%

          device_param->nvidia_spin_damp = (double) user_options->nvidia_spin_damp;

          if (user_options->nvidia_spin_damp_chgd == false)
          {
            if (user_options->attack_mode == ATTACK_MODE_STRAIGHT)
            {
              /**
               * the workaround is not a friend of rule based attacks
               * the words from the wordlist combined with fast and slow rules cause
               * fluctuations which cause inaccurate wait time estimations
               * using a reduced damping percentage almost compensates this
               */

              device_param->nvidia_spin_damp = 64;
            }
          }

          device_param->nvidia_spin_damp /= 100;
        }
      }

      // display results

      if (user_options->opencl_info == true)
      {
        char *format = "  Device ID #%u\n    Type           : %s\n    Vendor ID      : %u\n    Vendor         : %s\n    Name           : %s\n    Processor(s)   : %u\n    Clock          : %u\n    Memory         : %lu/%lu MB allocatable\n    OpenCL Version : %s\n\n";

        fprintf (stdout, format, device_id + 1,
          ((device_type & CL_DEVICE_TYPE_CPU) ? "CPU" : ((device_type & CL_DEVICE_TYPE_GPU) ? "GPU" : "Accelerator")),
          device_vendor_id,
          device_vendor,
          device_name,
          device_processors,
          device_maxclock_frequency,
          device_maxmem_alloc / 1024 / 1024,
          device_global_mem / 1024 / 1024,
          device_opencl_version);
      }

      hcfree (device_opencl_version);

      if ((user_options->benchmark == true || user_options->speed_only == true || user_options->quiet == false))
      {
        if (user_options->machine_readable == false)
        {
          if (device_param->skipped == 0)
          {
            event_log_info (hashcat_ctx, "- Device #%u: %s, %lu/%lu MB allocatable, %uMCU",
                      device_id + 1,
                      device_name,
                      (unsigned int) (device_maxmem_alloc / 1024 / 1024),
                      (unsigned int) (device_global_mem   / 1024 / 1024),
                      (unsigned int)  device_processors);
          }
          else
          {
            event_log_info (hashcat_ctx, "- Device #%u: %s, skipped",
                      device_id + 1,
                      device_name);
          }
        }
      }

      // common driver check

      if (device_param->skipped == 0)
      {
        if (device_type & CL_DEVICE_TYPE_GPU)
        {
          if (platform_vendor_id == VENDOR_ID_AMD)
          {
            int catalyst_check = (user_options->force == 1) ? 0 : 1;

            int catalyst_warn = 0;

            int catalyst_broken = 0;

            if (catalyst_check == 1)
            {
              catalyst_warn = 1;

              // v14.9 and higher
              if (atoi (device_param->driver_version) >= 1573)
              {
                catalyst_warn = 0;
              }

              catalyst_check = 0;
            }

            if (catalyst_broken == 1)
            {
              event_log_info (hashcat_ctx, "ATTENTION! The Catalyst driver installed on your system is known to be broken!");
              event_log_info (hashcat_ctx, "It passes over cracked hashes and will not report them as cracked");
              event_log_info (hashcat_ctx, "You are STRONGLY encouraged not to use it");
              event_log_info (hashcat_ctx, "You can use --force to override this but do not post error reports if you do so");
              event_log_info (hashcat_ctx, "");

              return -1;
            }

            if (catalyst_warn == 1)
            {
              event_log_info (hashcat_ctx, "ATTENTION! Unsupported or incorrectly installed Catalyst driver detected!");
              event_log_info (hashcat_ctx, "You are STRONGLY encouraged to use the official supported catalyst driver");
              event_log_info (hashcat_ctx, "See hashcat's homepage for official supported catalyst drivers");
              #if defined (_WIN)
              event_log_info (hashcat_ctx, "Also see: http://hashcat.net/wiki/doku.php?id=upgrading_amd_drivers_how_to");
              #endif
              event_log_info (hashcat_ctx, "You can use --force to override this but do not post error reports if you do so");
              event_log_info (hashcat_ctx, "");

              return -1;
            }
          }
          else if (platform_vendor_id == VENDOR_ID_NV)
          {
            if (device_param->kernel_exec_timeout != 0)
            {
              event_log_info (hashcat_ctx, "- Device #%u: WARNING! Kernel exec timeout is not disabled, it might cause you errors of code 702", device_id + 1);
              event_log_info (hashcat_ctx, "             See the wiki on how to disable it: https://hashcat.net/wiki/doku.php?id=timeout_patch");
            }
          }
        }

        /* turns out pocl still creates segfaults (because of llvm)
        if (device_type & CL_DEVICE_TYPE_CPU)
        {
          if (platform_vendor_id == VENDOR_ID_AMD)
          {
            if (user_options->force == 0)
            {
              event_log_info (hashcat_ctx, "ATTENTION! OpenCL support for CPU of catalyst driver is not reliable.");
              event_log_info (hashcat_ctx, "You are STRONGLY encouraged not to use it");
              event_log_info (hashcat_ctx, "You can use --force to override this but do not post error reports if you do so");
              event_log_info (hashcat_ctx, "A good alternative is the free pocl >= v0.13, but make sure to use a LLVM >= v3.8");
              event_log_info (hashcat_ctx, "");

              return -1;
            }
          }
        }
        */

        /**
         * activate device
         */

        devices_active++;
      }

      // next please

      devices_cnt++;
    }

    if ((user_options->benchmark == true || user_options->speed_only == true || user_options->quiet == false))
    {
      if (user_options->machine_readable == false)
      {
        event_log_info (hashcat_ctx, "");
      }
    }
  }

  if (user_options->opencl_info == true)
  {
    return 0;
  }

  if (devices_active == 0)
  {
    event_log_error (hashcat_ctx, "No devices found/left");

    return -1;
  }

  // additional check to see if the user has chosen a device that is not within the range of available devices (i.e. larger than devices_cnt)

  if (opencl_ctx->devices_filter != (u32) -1)
  {
    const u32 devices_cnt_mask = ~(((u32) -1 >> devices_cnt) << devices_cnt);

    if (opencl_ctx->devices_filter > devices_cnt_mask)
    {
      event_log_error (hashcat_ctx, "The device specified by the --opencl-devices parameter is larger than the number of available devices (%d)", devices_cnt);

      return -1;
    }
  }

  opencl_ctx->target_ms           = TARGET_MS_PROFILE[user_options->workload_profile - 1];

  opencl_ctx->devices_cnt         = devices_cnt;
  opencl_ctx->devices_active      = devices_active;

  opencl_ctx->need_adl            = need_adl;
  opencl_ctx->need_nvml           = need_nvml;
  opencl_ctx->need_nvapi          = need_nvapi;
  opencl_ctx->need_xnvctrl        = need_xnvctrl;

  return 0;
}

void opencl_ctx_devices_destroy (hashcat_ctx_t *hashcat_ctx)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  if (opencl_ctx->enabled == false) return;

  for (u32 device_id = 0; device_id < opencl_ctx->devices_cnt; device_id++)
  {
    hc_device_param_t *device_param = &opencl_ctx->devices_param[device_id];

    if (device_param->skipped) continue;

    hcfree (device_param->device_name);
    hcfree (device_param->device_name_chksum);
    hcfree (device_param->device_version);
    hcfree (device_param->driver_version);
  }

  opencl_ctx->devices_cnt    = 0;
  opencl_ctx->devices_active = 0;

  opencl_ctx->need_adl       = false;
  opencl_ctx->need_nvml      = false;
  opencl_ctx->need_nvapi     = false;
  opencl_ctx->need_xnvctrl   = false;
}

void opencl_ctx_devices_update_power (hashcat_ctx_t *hashcat_ctx)
{
  opencl_ctx_t         *opencl_ctx          = hashcat_ctx->opencl_ctx;
  status_ctx_t         *status_ctx          = hashcat_ctx->status_ctx;
  user_options_extra_t *user_options_extra  = hashcat_ctx->user_options_extra;
  user_options_t       *user_options        = hashcat_ctx->user_options;

  if (opencl_ctx->enabled == false) return;

  u32 kernel_power_all = 0;

  for (u32 device_id = 0; device_id < opencl_ctx->devices_cnt; device_id++)
  {
    hc_device_param_t *device_param = &opencl_ctx->devices_param[device_id];

    kernel_power_all += device_param->kernel_power;
  }

  opencl_ctx->kernel_power_all = kernel_power_all;

  /*
   * Inform user about possible slow speeds
   */

  if ((user_options_extra->wordlist_mode == WL_MODE_FILE) || (user_options_extra->wordlist_mode == WL_MODE_MASK))
  {
    if (status_ctx->words_base < kernel_power_all)
    {
      if (user_options->quiet == false)
      {
        clear_prompt ();

        event_log_info (hashcat_ctx, "ATTENTION!");
        event_log_info (hashcat_ctx, "  The wordlist or mask you are using is too small.");
        event_log_info (hashcat_ctx, "  Therefore, hashcat is unable to utilize the full parallelization power of your device(s).");
        event_log_info (hashcat_ctx, "  The cracking speed will drop.");
        event_log_info (hashcat_ctx, "  Workaround: https://hashcat.net/wiki/doku.php?id=frequently_asked_questions#how_to_create_more_work_for_full_speed");
        event_log_info (hashcat_ctx, "");
      }
    }
  }
}

void opencl_ctx_devices_kernel_loops (hashcat_ctx_t *hashcat_ctx)
{
  combinator_ctx_t     *combinator_ctx      = hashcat_ctx->combinator_ctx;
  hashconfig_t         *hashconfig          = hashcat_ctx->hashconfig;
  hashes_t             *hashes              = hashcat_ctx->hashes;
  mask_ctx_t           *mask_ctx            = hashcat_ctx->mask_ctx;
  opencl_ctx_t         *opencl_ctx          = hashcat_ctx->opencl_ctx;
  straight_ctx_t       *straight_ctx        = hashcat_ctx->straight_ctx;
  user_options_extra_t *user_options_extra  = hashcat_ctx->user_options_extra;

  if (opencl_ctx->enabled == false) return;

  for (u32 device_id = 0; device_id < opencl_ctx->devices_cnt; device_id++)
  {
    hc_device_param_t *device_param = &opencl_ctx->devices_param[device_id];

    if (device_param->skipped) continue;

    if (device_param->kernel_loops_min < device_param->kernel_loops_max)
    {
      u32 innerloop_cnt = 0;

      if (hashconfig->attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
      {
        if      (user_options_extra->attack_kern == ATTACK_KERN_STRAIGHT)  innerloop_cnt = straight_ctx->kernel_rules_cnt;
        else if (user_options_extra->attack_kern == ATTACK_KERN_COMBI)     innerloop_cnt = combinator_ctx->combs_cnt;
        else if (user_options_extra->attack_kern == ATTACK_KERN_BF)        innerloop_cnt = mask_ctx->bfs_cnt;
      }
      else
      {
        innerloop_cnt = hashes->salts_buf[0].salt_iter;
      }

      if ((innerloop_cnt >= device_param->kernel_loops_min) &&
          (innerloop_cnt <= device_param->kernel_loops_max))
      {
        device_param->kernel_loops_max = innerloop_cnt;
      }
    }
  }
}

int opencl_session_begin (hashcat_ctx_t *hashcat_ctx)
{
  bitmap_ctx_t         *bitmap_ctx          = hashcat_ctx->bitmap_ctx;
  folder_config_t      *folder_config       = hashcat_ctx->folder_config;
  hashconfig_t         *hashconfig          = hashcat_ctx->hashconfig;
  hashes_t             *hashes              = hashcat_ctx->hashes;
  opencl_ctx_t         *opencl_ctx          = hashcat_ctx->opencl_ctx;
  straight_ctx_t       *straight_ctx        = hashcat_ctx->straight_ctx;
  user_options_extra_t *user_options_extra  = hashcat_ctx->user_options_extra;
  user_options_t       *user_options        = hashcat_ctx->user_options;

  if (opencl_ctx->enabled == false) return 0;

  /**
   * Some algorithm, like descrypt, can benefit from JIT compilation
   */

  opencl_ctx->force_jit_compilation = -1;

  if (hashconfig->hash_mode == 8900)
  {
    opencl_ctx->force_jit_compilation = 8900;
  }
  else if (hashconfig->hash_mode == 9300)
  {
    opencl_ctx->force_jit_compilation = 8900;
  }
  else if (hashconfig->hash_mode == 1500 && user_options->attack_mode == ATTACK_MODE_BF && hashes->salts_cnt == 1)
  {
    opencl_ctx->force_jit_compilation = 1500;
  }

  u32 hardware_power_all = 0;

  for (u32 device_id = 0; device_id < opencl_ctx->devices_cnt; device_id++)
  {
    cl_int CL_err = CL_SUCCESS;

    /**
     * host buffer
     */

    hc_device_param_t *device_param = &opencl_ctx->devices_param[device_id];

    if (device_param->skipped) continue;

    // vector_width

    cl_uint vector_width;

    if (user_options->opencl_vector_width_chgd == false)
    {
      // tuning db

      tuning_db_entry_t *tuningdb_entry = tuning_db_search (hashcat_ctx, device_param->device_name, device_param->device_type, user_options->attack_mode, hashconfig->hash_mode);

      if (tuningdb_entry == NULL || tuningdb_entry->vector_width == -1)
      {
        if (hashconfig->opti_type & OPTI_TYPE_USES_BITS_64)
        {
          CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, sizeof (vector_width), &vector_width, NULL);

          if (CL_err == -1) return -1;
        }
        else
        {
          CL_err = hc_clGetDeviceInfo (hashcat_ctx, device_param->device, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT,  sizeof (vector_width), &vector_width, NULL);

          if (CL_err == -1) return -1;
        }
      }
      else
      {
        vector_width = (cl_uint) tuningdb_entry->vector_width;
      }
    }
    else
    {
      vector_width = user_options->opencl_vector_width;
    }

    if (vector_width > 16) vector_width = 16;

    device_param->vector_width = vector_width;

    /**
     * kernel accel and loops tuning db adjustment
     */

    device_param->kernel_accel_min = 1;
    device_param->kernel_accel_max = 1024;

    device_param->kernel_loops_min = 1;
    device_param->kernel_loops_max = 1024;

    tuning_db_entry_t *tuningdb_entry = tuning_db_search (hashcat_ctx, device_param->device_name, device_param->device_type, user_options->attack_mode, hashconfig->hash_mode);

    if (tuningdb_entry != NULL)
    {
      u32 _kernel_accel = tuningdb_entry->kernel_accel;
      u32 _kernel_loops = tuningdb_entry->kernel_loops;

      if (_kernel_accel)
      {
        device_param->kernel_accel_min = _kernel_accel;
        device_param->kernel_accel_max = _kernel_accel;
      }

      if (_kernel_loops)
      {
        if (user_options->workload_profile == 1)
        {
          _kernel_loops = (_kernel_loops > 8) ? _kernel_loops / 8 : 1;
        }
        else if (user_options->workload_profile == 2)
        {
          _kernel_loops = (_kernel_loops > 4) ? _kernel_loops / 4 : 1;
        }

        device_param->kernel_loops_min = _kernel_loops;
        device_param->kernel_loops_max = _kernel_loops;
      }
    }

    // commandline parameters overwrite tuningdb entries

    if (user_options->kernel_accel_chgd == true)
    {
      device_param->kernel_accel_min = user_options->kernel_accel;
      device_param->kernel_accel_max = user_options->kernel_accel;
    }

    if (user_options->kernel_loops_chgd == true)
    {
      device_param->kernel_loops_min = user_options->kernel_loops;
      device_param->kernel_loops_max = user_options->kernel_loops;
    }


    /**
     * device properties
     */

    const char *device_name_chksum  = device_param->device_name_chksum;
    const u32   device_processors   = device_param->device_processors;

    /**
     * create context for each device
     */

    cl_context_properties properties[3];

    properties[0] = CL_CONTEXT_PLATFORM;
    properties[1] = (cl_context_properties) device_param->platform;
    properties[2] = 0;

    CL_err = hc_clCreateContext (hashcat_ctx, properties, 1, &device_param->device, NULL, NULL, &device_param->context);

    if (CL_err == -1) return -1;

    /**
     * create command-queue
     */

    // not supported with NV
    // device_param->command_queue = hc_clCreateCommandQueueWithProperties (hashcat_ctx, device_param->device, NULL);

    CL_err = hc_clCreateCommandQueue (hashcat_ctx, device_param->context, device_param->device, CL_QUEUE_PROFILING_ENABLE, &device_param->command_queue);

    if (CL_err == -1) return -1;

    /**
     * kernel threads: some algorithms need a fixed kernel-threads count
     *                 because of shared memory usage or bitslice
     *                 there needs to be some upper limit, otherwise there's too much overhead
     */

    u32 kernel_threads = hashconfig_enforce_kernel_threads (hashcat_ctx, device_param);

    device_param->kernel_threads = kernel_threads;

    device_param->hardware_power = device_processors * kernel_threads;

    hardware_power_all += device_param->hardware_power;

    /**
     * create input buffers on device : calculate size of fixed memory buffers
     */

    size_t size_root_css   = SP_PW_MAX *           sizeof (cs_t);
    size_t size_markov_css = SP_PW_MAX * CHARSIZ * sizeof (cs_t);

    device_param->size_root_css   = size_root_css;
    device_param->size_markov_css = size_markov_css;

    size_t size_results = sizeof (u32);

    device_param->size_results = size_results;

    size_t size_rules   = straight_ctx->kernel_rules_cnt * sizeof (kernel_rule_t);
    size_t size_rules_c = KERNEL_RULES                   * sizeof (kernel_rule_t);

    size_t size_plains  = hashes->digests_cnt * sizeof (plain_t);
    size_t size_salts   = hashes->salts_cnt   * sizeof (salt_t);
    size_t size_esalts  = hashes->salts_cnt   * hashconfig->esalt_size;
    size_t size_shown   = hashes->digests_cnt * sizeof (u32);
    size_t size_digests = hashes->digests_cnt * hashconfig->dgst_size;

    device_param->size_plains   = size_plains;
    device_param->size_digests  = size_digests;
    device_param->size_shown    = size_shown;
    device_param->size_salts    = size_salts;

    size_t size_combs = KERNEL_COMBS * sizeof (comb_t);
    size_t size_bfs   = KERNEL_BFS   * sizeof (bf_t);
    size_t size_tm    = 32           * sizeof (bs_word_t);

    // scryptV stuff

    u32 scrypt_tmp_size   = 0;
    u32 scrypt_tmto_final = 0;

    size_t size_scrypt = 4;

    if ((hashconfig->hash_mode == 8900) || (hashconfig->hash_mode == 9300))
    {
      // we need to check that all hashes have the same scrypt settings

      const u32 scrypt_N = hashes->salts_buf[0].scrypt_N;
      const u32 scrypt_r = hashes->salts_buf[0].scrypt_r;
      const u32 scrypt_p = hashes->salts_buf[0].scrypt_p;

      for (u32 i = 1; i < hashes->salts_cnt; i++)
      {
        if ((hashes->salts_buf[i].scrypt_N != scrypt_N)
         || (hashes->salts_buf[i].scrypt_r != scrypt_r)
         || (hashes->salts_buf[i].scrypt_p != scrypt_p))
        {
          event_log_error (hashcat_ctx, "Mixed scrypt settings not supported");

          return -1;
        }
      }

      scrypt_tmp_size = (128 * scrypt_r * scrypt_p);

      hashconfig->tmp_size = scrypt_tmp_size;

      u32 tmto_start = 0;
      u32 tmto_stop  = 10;

      if (user_options->scrypt_tmto)
      {
        tmto_start = user_options->scrypt_tmto;
      }
      else
      {
        // in case the user did not specify the tmto manually
        // use some values known to run best (tested on 290x for AMD and GTX1080 for NV)

        if (hashconfig->hash_mode == 8900)
        {
          if (device_param->device_vendor_id == VENDOR_ID_AMD)
          {
            tmto_start = 3;
          }
          else if (device_param->device_vendor_id == VENDOR_ID_NV)
          {
            tmto_start = 2;
          }
        }
        else if (hashconfig->hash_mode == 9300)
        {
          if (device_param->device_vendor_id == VENDOR_ID_AMD)
          {
            tmto_start = 2;
          }
          else if (device_param->device_vendor_id == VENDOR_ID_NV)
          {
            tmto_start = 4;
          }
        }
      }

      device_param->kernel_accel_min = 1;
      device_param->kernel_accel_max = 8;

      u32 tmto;

      for (tmto = tmto_start; tmto < tmto_stop; tmto++)
      {
        size_scrypt = (128 * scrypt_r) * scrypt_N;

        size_scrypt /= 1u << tmto;

        size_scrypt *= device_param->device_processors * device_param->kernel_threads * device_param->kernel_accel_max;

        if ((size_scrypt / 4) > device_param->device_maxmem_alloc)
        {
          event_log_info (hashcat_ctx, "Not enough single-block device memory allocatable to use --scrypt-tmto %d, increasing...", tmto);

          continue;
        }

        if (size_scrypt > device_param->device_global_mem)
        {
          event_log_info (hashcat_ctx, "Not enough total device memory allocatable to use --scrypt-tmto %d, increasing...", tmto);

          continue;
        }

        for (u32 salts_pos = 0; salts_pos < hashes->salts_cnt; salts_pos++)
        {
          scrypt_tmto_final = tmto;
        }

        break;
      }

      if (tmto == tmto_stop)
      {
        event_log_error (hashcat_ctx, "Can't allocate enough device memory");

        return -1;
      }

      if (user_options->quiet == false) event_log_info (hashcat_ctx, "SCRYPT tmto optimizer value set to: %u, mem: %" PRIu64, scrypt_tmto_final, size_scrypt);
      if (user_options->quiet == false) event_log_info (hashcat_ctx, "");
    }

    size_t size_scrypt4 = size_scrypt / 4;

    /**
     * some algorithms need a fixed kernel-loops count
     */

    const u32 kernel_loops_fixed = hashconfig_enforce_kernel_loops (hashcat_ctx);

    if (kernel_loops_fixed != 0)
    {
      device_param->kernel_loops_min = kernel_loops_fixed;
      device_param->kernel_loops_max = kernel_loops_fixed;
    }

    u32 kernel_accel_min = device_param->kernel_accel_min;
    u32 kernel_accel_max = device_param->kernel_accel_max;

    // find out if we would request too much memory on memory blocks which are based on kernel_accel

    size_t size_pws   = 4;
    size_t size_tmps  = 4;
    size_t size_hooks = 4;

    while (kernel_accel_max >= kernel_accel_min)
    {
      const u32 kernel_power_max = device_processors * kernel_threads * kernel_accel_max;

      // size_pws

      size_pws = kernel_power_max * sizeof (pw_t);

      // size_tmps

      size_tmps = kernel_power_max * hashconfig->tmp_size;

      // size_hooks

      size_hooks = kernel_power_max * hashconfig->hook_size;

      // now check if all device-memory sizes which depend on the kernel_accel_max amplifier are within its boundaries
      // if not, decrease amplifier and try again

      int memory_limit_hit = 0;

      if (size_pws   > device_param->device_maxmem_alloc) memory_limit_hit = 1;
      if (size_tmps  > device_param->device_maxmem_alloc) memory_limit_hit = 1;
      if (size_hooks > device_param->device_maxmem_alloc) memory_limit_hit = 1;

      const u64 size_total
        = bitmap_ctx->bitmap_size
        + bitmap_ctx->bitmap_size
        + bitmap_ctx->bitmap_size
        + bitmap_ctx->bitmap_size
        + bitmap_ctx->bitmap_size
        + bitmap_ctx->bitmap_size
        + bitmap_ctx->bitmap_size
        + bitmap_ctx->bitmap_size
        + size_bfs
        + size_combs
        + size_digests
        + size_esalts
        + size_hooks
        + size_markov_css
        + size_plains
        + size_pws
        + size_pws // not a bug
        + size_results
        + size_root_css
        + size_rules
        + size_rules_c
        + size_salts
        + size_scrypt4
        + size_scrypt4
        + size_scrypt4
        + size_scrypt4
        + size_shown
        + size_tm
        + size_tmps;

      if (size_total > device_param->device_global_mem) memory_limit_hit = 1;

      if (memory_limit_hit == 1)
      {
        kernel_accel_max--;

        continue;
      }

      break;
    }

    if (kernel_accel_max < kernel_accel_min)
    {
      event_log_error (hashcat_ctx, "- Device #%u: Device does not provide enough allocatable device-memory to handle this attack", device_id + 1);

      return -1;
    }

    device_param->kernel_accel_min = kernel_accel_min;
    device_param->kernel_accel_max = kernel_accel_max;

    /*
    if (kernel_accel_max < kernel_accel)
    {
      if (user_options->quiet == false) event_log_info (hashcat_ctx, "- Device #%u: Reduced maximum kernel-accel to %u", device_id + 1, kernel_accel_max);

      device_param->kernel_accel = kernel_accel_max;
    }
    */

    device_param->size_bfs     = size_bfs;
    device_param->size_combs   = size_combs;
    device_param->size_rules   = size_rules;
    device_param->size_rules_c = size_rules_c;
    device_param->size_pws     = size_pws;
    device_param->size_tmps    = size_tmps;
    device_param->size_hooks   = size_hooks;

    /**
     * default building options
     */

    if (chdir (folder_config->cpath_real) == -1)
    {
      event_log_error (hashcat_ctx, "%s: %s", folder_config->cpath_real, strerror (errno));

      return -1;
    }

    char build_opts[1024] = { 0 };

    #if defined (_WIN)
    snprintf (build_opts, sizeof (build_opts) - 1, "-I \"%s\"", folder_config->cpath_real);
    #else
    snprintf (build_opts, sizeof (build_opts) - 1, "-I %s", folder_config->cpath_real);
    #endif

    // include check
    // this test needs to be done manually because of osx opencl runtime
    // if there's a problem with permission, its not reporting back and erroring out silently

    #define files_cnt 15

    const char *files_names[files_cnt] =
    {
      "inc_cipher_aes256.cl",
      "inc_cipher_serpent256.cl",
      "inc_cipher_twofish256.cl",
      "inc_common.cl",
      "inc_comp_multi_bs.cl",
      "inc_comp_multi.cl",
      "inc_comp_single_bs.cl",
      "inc_comp_single.cl",
      "inc_hash_constants.h",
      "inc_hash_functions.cl",
      "inc_rp.cl",
      "inc_rp.h",
      "inc_simd.cl",
      "inc_types.cl",
      "inc_vendor.cl",
    };

    for (int i = 0; i < files_cnt; i++)
    {
      FILE *fd = fopen (files_names[i], "r");

      if (fd == NULL)
      {
        event_log_error (hashcat_ctx, "%s: fopen(): %s", files_names[i], strerror (errno));

        return -1;
      }

      char buf[1];

      size_t n = fread (buf, 1, 1, fd);

      if (n != 1)
      {
        event_log_error (hashcat_ctx, "%s: fread(): %s", files_names[i], strerror (errno));

        return -1;
      }

      fclose (fd);
    }

    // we don't have sm_* on vendors not NV but it doesn't matter

    char build_opts_new[1024] = { 0 };

    #if defined (DEBUG)
    snprintf (build_opts_new, sizeof (build_opts_new) - 1, "%s -D VENDOR_ID=%u -D CUDA_ARCH=%d -D VECT_SIZE=%u -D DEVICE_TYPE=%u -D DGST_R0=%u -D DGST_R1=%u -D DGST_R2=%u -D DGST_R3=%u -D DGST_ELEM=%u -D KERN_TYPE=%u -D _unroll -cl-std=CL1.1", build_opts, device_param->device_vendor_id, (device_param->sm_major * 100) + device_param->sm_minor, device_param->vector_width, (u32) device_param->device_type, hashconfig->dgst_pos0, hashconfig->dgst_pos1, hashconfig->dgst_pos2, hashconfig->dgst_pos3, hashconfig->dgst_size / 4, hashconfig->kern_type);
    #else
    snprintf (build_opts_new, sizeof (build_opts_new) - 1, "%s -D VENDOR_ID=%u -D CUDA_ARCH=%d -D VECT_SIZE=%u -D DEVICE_TYPE=%u -D DGST_R0=%u -D DGST_R1=%u -D DGST_R2=%u -D DGST_R3=%u -D DGST_ELEM=%u -D KERN_TYPE=%u -D _unroll -cl-std=CL1.1 -w", build_opts, device_param->device_vendor_id, (device_param->sm_major * 100) + device_param->sm_minor, device_param->vector_width, (u32) device_param->device_type, hashconfig->dgst_pos0, hashconfig->dgst_pos1, hashconfig->dgst_pos2, hashconfig->dgst_pos3, hashconfig->dgst_size / 4, hashconfig->kern_type);
    #endif

    strncpy (build_opts, build_opts_new, sizeof (build_opts));

    #if defined (DEBUG)
    event_log_info (hashcat_ctx, "- Device #%u: build_opts '%s'", device_id + 1, build_opts);
    #endif

    /**
     * main kernel
     */

    {
      /**
       * kernel source filename
       */

      char source_file[256] = { 0 };

      generate_source_kernel_filename (hashconfig->attack_exec, user_options_extra->attack_kern, hashconfig->kern_type, folder_config->shared_dir, source_file);

      struct stat sst;

      if (stat (source_file, &sst) == -1)
      {
        event_log_error (hashcat_ctx, "%s: %s", source_file, strerror (errno));

        return -1;
      }

      /**
       * kernel cached filename
       */

      char cached_file[256] = { 0 };

      generate_cached_kernel_filename (hashconfig->attack_exec, user_options_extra->attack_kern, hashconfig->kern_type, folder_config->profile_dir, device_name_chksum, cached_file);

      int cached = 1;

      struct stat cst;

      if ((stat (cached_file, &cst) == -1) || cst.st_size == 0)
      {
        cached = 0;
      }

      /**
       * kernel compile or load
       */

      size_t *kernel_lengths = (size_t *) hcmalloc (hashcat_ctx, sizeof (size_t)); VERIFY_PTR (kernel_lengths);

      char **kernel_sources = (char **) hcmalloc (hashcat_ctx, sizeof (char *)); VERIFY_PTR (kernel_sources);

      if (opencl_ctx->force_jit_compilation == -1)
      {
        if (cached == 0)
        {
          if (user_options->quiet == false) event_log_info_nn (hashcat_ctx, "- Device #%u: Kernel %s not found in cache! Building may take a while...", device_id + 1, filename_from_filepath (cached_file));

          const int rc_read_kernel = read_kernel_binary (hashcat_ctx, source_file, 1, kernel_lengths, kernel_sources);

          if (rc_read_kernel == -1) return -1;

          CL_err = hc_clCreateProgramWithSource (hashcat_ctx, device_param->context, 1, (const char **) kernel_sources, NULL, &device_param->program);

          if (CL_err == -1) return -1;

          CL_err = hc_clBuildProgram (hashcat_ctx, device_param->program, 1, &device_param->device, build_opts, NULL, NULL);

          //if (CL_err == -1) return -1;

          size_t build_log_size = 0;

          hc_clGetProgramBuildInfo (hashcat_ctx, device_param->program, device_param->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_size);

          //if (CL_err == -1) return -1;

          #if defined (DEBUG)
          if ((build_log_size != 0) || (CL_err != CL_SUCCESS))
          #else
          if (CL_err != CL_SUCCESS)
          #endif
          {
            char *build_log = (char *) hcmalloc (hashcat_ctx, build_log_size + 1); VERIFY_PTR (build_log);

            CL_err = hc_clGetProgramBuildInfo (hashcat_ctx, device_param->program, device_param->device, CL_PROGRAM_BUILD_LOG, build_log_size, build_log, NULL);

            if (CL_err == -1) return -1;

            puts (build_log);

            hcfree (build_log);
          }

          if (CL_err != CL_SUCCESS)
          {
            device_param->skipped = true;

            event_log_info (hashcat_ctx, "- Device #%u: Kernel %s build failure. Proceeding without this device.", device_id + 1, source_file);

            continue;
          }

          size_t binary_size;

          CL_err = hc_clGetProgramInfo (hashcat_ctx, device_param->program, CL_PROGRAM_BINARY_SIZES, sizeof (size_t), &binary_size, NULL);

          if (CL_err == -1) return -1;

          char *binary = (char *) hcmalloc (hashcat_ctx, binary_size); VERIFY_PTR (binary);

          CL_err = hc_clGetProgramInfo (hashcat_ctx, device_param->program, CL_PROGRAM_BINARIES, sizeof (binary), &binary, NULL);

          if (CL_err == -1) return -1;

          const int rc_write = write_kernel_binary (hashcat_ctx, cached_file, binary, binary_size);

          if (rc_write == -1) return -1;

          hcfree (binary);
        }
        else
        {
          #if defined (DEBUG)
          event_log_info (hashcat_ctx, "- Device #%u: Kernel %s (%ld bytes)", device_id + 1, cached_file, cst.st_size);
          #endif

          const int rc_read_kernel = read_kernel_binary (hashcat_ctx, cached_file, 1, kernel_lengths, kernel_sources);

          if (rc_read_kernel == -1) return -1;

          CL_err = hc_clCreateProgramWithBinary (hashcat_ctx, device_param->context, 1, &device_param->device, kernel_lengths, (const unsigned char **) kernel_sources, NULL, &device_param->program);

          if (CL_err == -1) return -1;

          CL_err = hc_clBuildProgram (hashcat_ctx, device_param->program, 1, &device_param->device, build_opts, NULL, NULL);

          if (CL_err == -1) return -1;
        }
      }
      else
      {
        #if defined (DEBUG)
        event_log_info (hashcat_ctx, "- Device #%u: Kernel %s (%ld bytes)", device_id + 1, source_file, sst.st_size);
        #endif

        const int rc_read_kernel = read_kernel_binary (hashcat_ctx, source_file, 1, kernel_lengths, kernel_sources);

        if (rc_read_kernel == -1) return -1;

        CL_err = hc_clCreateProgramWithSource (hashcat_ctx, device_param->context, 1, (const char **) kernel_sources, NULL, &device_param->program);

        if (CL_err == -1) return -1;

        char build_opts_update[1024] = { 0 };

        if (opencl_ctx->force_jit_compilation == 1500)
        {
          snprintf (build_opts_update, sizeof (build_opts_update) - 1, "%s -DDESCRYPT_SALT=%u", build_opts, hashes->salts_buf[0].salt_buf[0]);
        }
        else if (opencl_ctx->force_jit_compilation == 8900)
        {
          snprintf (build_opts_update, sizeof (build_opts_update) - 1, "%s -DSCRYPT_N=%u -DSCRYPT_R=%u -DSCRYPT_P=%u -DSCRYPT_TMTO=%u -DSCRYPT_TMP_ELEM=%u", build_opts, hashes->salts_buf[0].scrypt_N, hashes->salts_buf[0].scrypt_r, hashes->salts_buf[0].scrypt_p, 1 << scrypt_tmto_final, scrypt_tmp_size / 16);
        }
        else
        {
          snprintf (build_opts_update, sizeof (build_opts_update) - 1, "%s", build_opts);
        }

        CL_err = hc_clBuildProgram (hashcat_ctx, device_param->program, 1, &device_param->device, build_opts_update, NULL, NULL);

        //if (CL_err == -1) return -1;

        size_t build_log_size = 0;

        hc_clGetProgramBuildInfo (hashcat_ctx, device_param->program, device_param->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_size);

        //if (CL_err == -1) return -1;

        #if defined (DEBUG)
        if ((build_log_size != 0) || (CL_err != CL_SUCCESS))
        #else
        if (CL_err != CL_SUCCESS)
        #endif
        {
          char *build_log = (char *) hcmalloc (hashcat_ctx, build_log_size + 1); VERIFY_PTR (build_log);

          CL_err = hc_clGetProgramBuildInfo (hashcat_ctx, device_param->program, device_param->device, CL_PROGRAM_BUILD_LOG, build_log_size, build_log, NULL);

          if (CL_err == -1) return -1;

          puts (build_log);

          hcfree (build_log);
        }

        if (CL_err != CL_SUCCESS)
        {
          device_param->skipped = true;

          event_log_info (hashcat_ctx, "- Device #%u: Kernel %s build failure. Proceeding without this device.", device_id + 1, source_file);
        }
      }

      hcfree (kernel_lengths);
      hcfree (kernel_sources[0]);
      hcfree (kernel_sources);
    }

    /**
     * word generator kernel
     */

    if (user_options->attack_mode != ATTACK_MODE_STRAIGHT)
    {
      /**
       * kernel mp source filename
       */

      char source_file[256] = { 0 };

      generate_source_kernel_mp_filename (hashconfig->opti_type, hashconfig->opts_type, folder_config->shared_dir, source_file);

      struct stat sst;

      if (stat (source_file, &sst) == -1)
      {
        event_log_error (hashcat_ctx, "%s: %s", source_file, strerror (errno));

        return -1;
      }

      /**
       * kernel mp cached filename
       */

      char cached_file[256] = { 0 };

      generate_cached_kernel_mp_filename (hashconfig->opti_type, hashconfig->opts_type, folder_config->profile_dir, device_name_chksum, cached_file);

      int cached = 1;

      struct stat cst;

      if (stat (cached_file, &cst) == -1)
      {
        cached = 0;
      }

      /**
       * kernel compile or load
       */

      size_t *kernel_lengths = (size_t *) hcmalloc (hashcat_ctx, sizeof (size_t)); VERIFY_PTR (kernel_lengths);

      char **kernel_sources = (char **) hcmalloc (hashcat_ctx, sizeof (char *)); VERIFY_PTR (kernel_sources);

      if (cached == 0)
      {
        if (user_options->quiet == false) event_log_info_nn (hashcat_ctx, "- Device #%u: Kernel %s not found in cache! Building may take a while...", device_id + 1, filename_from_filepath (cached_file));

        const int rc_read_kernel = read_kernel_binary (hashcat_ctx, source_file, 1, kernel_lengths, kernel_sources);

        if (rc_read_kernel == -1) return -1;

        CL_err = hc_clCreateProgramWithSource (hashcat_ctx, device_param->context, 1, (const char **) kernel_sources, NULL, &device_param->program_mp);

        if (CL_err == -1) return -1;

        CL_err = hc_clBuildProgram (hashcat_ctx, device_param->program_mp, 1, &device_param->device, build_opts, NULL, NULL);

        //if (CL_err == -1) return -1;

        size_t build_log_size = 0;

        hc_clGetProgramBuildInfo (hashcat_ctx, device_param->program_mp, device_param->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_size);

        //if (CL_err == -1) return -1;

        #if defined (DEBUG)
        if ((build_log_size != 0) || (CL_err != CL_SUCCESS))
        #else
        if (CL_err != CL_SUCCESS)
        #endif
        {
          char *build_log = (char *) hcmalloc (hashcat_ctx, build_log_size + 1); VERIFY_PTR (build_log);

          CL_err = hc_clGetProgramBuildInfo (hashcat_ctx, device_param->program_mp, device_param->device, CL_PROGRAM_BUILD_LOG, build_log_size, build_log, NULL);

          if (CL_err == -1) return -1;

          puts (build_log);

          hcfree (build_log);
        }

        if (CL_err != CL_SUCCESS)
        {
          device_param->skipped = true;

          event_log_info (hashcat_ctx, "- Device #%u: Kernel %s build failure. Proceeding without this device.", device_id + 1, source_file);

          continue;
        }

        size_t binary_size;

        CL_err = hc_clGetProgramInfo (hashcat_ctx, device_param->program_mp, CL_PROGRAM_BINARY_SIZES, sizeof (size_t), &binary_size, NULL);

        if (CL_err == -1) return -1;

        char *binary = (char *) hcmalloc (hashcat_ctx, binary_size); VERIFY_PTR (binary);

        CL_err = hc_clGetProgramInfo (hashcat_ctx, device_param->program_mp, CL_PROGRAM_BINARIES, sizeof (binary), &binary, NULL);

        if (CL_err == -1) return -1;

        write_kernel_binary (hashcat_ctx, cached_file, binary, binary_size);

        hcfree (binary);
      }
      else
      {
        #if defined (DEBUG)
        event_log_info (hashcat_ctx, "- Device #%u: Kernel %s (%ld bytes)", device_id + 1, cached_file, cst.st_size);
        #endif

        const int rc_read_kernel = read_kernel_binary (hashcat_ctx, cached_file, 1, kernel_lengths, kernel_sources);

        if (rc_read_kernel == -1) return -1;

        CL_err = hc_clCreateProgramWithBinary (hashcat_ctx, device_param->context, 1, &device_param->device, kernel_lengths, (const unsigned char **) kernel_sources, NULL, &device_param->program_mp);

        if (CL_err == -1) return -1;

        CL_err = hc_clBuildProgram (hashcat_ctx, device_param->program_mp, 1, &device_param->device, build_opts, NULL, NULL);

        if (CL_err == -1) return -1;
      }

      hcfree (kernel_lengths);
      hcfree (kernel_sources[0]);
      hcfree (kernel_sources);
    }

    /**
     * amplifier kernel
     */

    if (hashconfig->attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
    {

    }
    else
    {
      /**
       * kernel amp source filename
       */

      char source_file[256] = { 0 };

      generate_source_kernel_amp_filename (user_options_extra->attack_kern, folder_config->shared_dir, source_file);

      struct stat sst;

      if (stat (source_file, &sst) == -1)
      {
        event_log_error (hashcat_ctx, "%s: %s", source_file, strerror (errno));

        return -1;
      }

      /**
       * kernel amp cached filename
       */

      char cached_file[256] = { 0 };

      generate_cached_kernel_amp_filename (user_options_extra->attack_kern, folder_config->profile_dir, device_name_chksum, cached_file);

      int cached = 1;

      struct stat cst;

      if (stat (cached_file, &cst) == -1)
      {
        cached = 0;
      }

      /**
       * kernel compile or load
       */

      size_t *kernel_lengths = (size_t *) hcmalloc (hashcat_ctx, sizeof (size_t)); VERIFY_PTR (kernel_lengths);

      char **kernel_sources = (char **) hcmalloc (hashcat_ctx, sizeof (char *)); VERIFY_PTR (kernel_sources);

      if (cached == 0)
      {
        if (user_options->quiet == false) event_log_info_nn (hashcat_ctx, "- Device #%u: Kernel %s not found in cache! Building may take a while...", device_id + 1, filename_from_filepath (cached_file));

        const int rc_read_kernel = read_kernel_binary (hashcat_ctx, source_file, 1, kernel_lengths, kernel_sources);

        if (rc_read_kernel == -1) return -1;

        CL_err = hc_clCreateProgramWithSource (hashcat_ctx, device_param->context, 1, (const char **) kernel_sources, NULL, &device_param->program_amp);

        if (CL_err == -1) return -1;

        CL_err = hc_clBuildProgram (hashcat_ctx, device_param->program_amp, 1, &device_param->device, build_opts, NULL, NULL);

        //if (CL_err == -1) return -1;

        size_t build_log_size = 0;

        hc_clGetProgramBuildInfo (hashcat_ctx, device_param->program_amp, device_param->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_size);

        //if (CL_err == -1) return -1;

        #if defined (DEBUG)
        if ((build_log_size != 0) || (CL_err != CL_SUCCESS))
        #else
        if (CL_err != CL_SUCCESS)
        #endif
        {
          char *build_log = (char *) hcmalloc (hashcat_ctx, build_log_size + 1); VERIFY_PTR (build_log);

          CL_err = hc_clGetProgramBuildInfo (hashcat_ctx, device_param->program_amp, device_param->device, CL_PROGRAM_BUILD_LOG, build_log_size, build_log, NULL);

          if (CL_err == -1) return -1;

          puts (build_log);

          hcfree (build_log);
        }

        if (CL_err != CL_SUCCESS)
        {
          device_param->skipped = true;

          event_log_info (hashcat_ctx, "- Device #%u: Kernel %s build failure. Proceed without this device.", device_id + 1, source_file);

          continue;
        }

        size_t binary_size;

        CL_err = hc_clGetProgramInfo (hashcat_ctx, device_param->program_amp, CL_PROGRAM_BINARY_SIZES, sizeof (size_t), &binary_size, NULL);

        if (CL_err == -1) return -1;

        char *binary = (char *) hcmalloc (hashcat_ctx, binary_size); VERIFY_PTR (binary);

        CL_err = hc_clGetProgramInfo (hashcat_ctx, device_param->program_amp, CL_PROGRAM_BINARIES, sizeof (binary), &binary, NULL);

        if (CL_err == -1) return -1;

        write_kernel_binary (hashcat_ctx, cached_file, binary, binary_size);

        hcfree (binary);
      }
      else
      {
        #if defined (DEBUG)
        if (user_options->quiet == false) event_log_info (hashcat_ctx, "- Device #%u: Kernel %s (%ld bytes)", device_id + 1, cached_file, cst.st_size);
        #endif

        const int rc_read_kernel = read_kernel_binary (hashcat_ctx, cached_file, 1, kernel_lengths, kernel_sources);

        if (rc_read_kernel == -1) return -1;

        CL_err = hc_clCreateProgramWithBinary (hashcat_ctx, device_param->context, 1, &device_param->device, kernel_lengths, (const unsigned char **) kernel_sources, NULL, &device_param->program_amp);

        if (CL_err == -1) return -1;

        CL_err = hc_clBuildProgram (hashcat_ctx, device_param->program_amp, 1, &device_param->device, build_opts, NULL, NULL);

        if (CL_err == -1) return -1;
      }

      hcfree (kernel_lengths);
      hcfree (kernel_sources[0]);
      hcfree (kernel_sources);
    }

    // return back to the folder we came from initially (workaround)

    if (chdir (folder_config->cwd) == -1)
    {
      event_log_error (hashcat_ctx, "%s: %s", folder_config->cwd, strerror (errno));

      return -1;
    }

    // some algorithm collide too fast, make that impossible

    if (user_options->benchmark == true)
    {
      ((u32 *) hashes->digests_buf)[0] = -1u;
      ((u32 *) hashes->digests_buf)[1] = -1u;
      ((u32 *) hashes->digests_buf)[2] = -1u;
      ((u32 *) hashes->digests_buf)[3] = -1u;
    }

    /**
     * global buffers
     */

    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   size_pws,     NULL, &device_param->d_pws_buf);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   size_pws,     NULL, &device_param->d_pws_amp_buf);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_WRITE,  size_tmps,    NULL, &device_param->d_tmps);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_WRITE,  size_hooks,   NULL, &device_param->d_hooks);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   bitmap_ctx->bitmap_size,  NULL, &device_param->d_bitmap_s1_a);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   bitmap_ctx->bitmap_size,  NULL, &device_param->d_bitmap_s1_b);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   bitmap_ctx->bitmap_size,  NULL, &device_param->d_bitmap_s1_c);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   bitmap_ctx->bitmap_size,  NULL, &device_param->d_bitmap_s1_d);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   bitmap_ctx->bitmap_size,  NULL, &device_param->d_bitmap_s2_a);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   bitmap_ctx->bitmap_size,  NULL, &device_param->d_bitmap_s2_b);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   bitmap_ctx->bitmap_size,  NULL, &device_param->d_bitmap_s2_c);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   bitmap_ctx->bitmap_size,  NULL, &device_param->d_bitmap_s2_d);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_WRITE,  size_plains,  NULL, &device_param->d_plain_bufs);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   size_digests, NULL, &device_param->d_digests_buf);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_WRITE,  size_shown,   NULL, &device_param->d_digests_shown);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY,   size_salts,   NULL, &device_param->d_salt_bufs);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_WRITE,  size_results, NULL, &device_param->d_result);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_WRITE,  size_scrypt4, NULL, &device_param->d_scryptV0_buf);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_WRITE,  size_scrypt4, NULL, &device_param->d_scryptV1_buf);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_WRITE,  size_scrypt4, NULL, &device_param->d_scryptV2_buf);
    CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_WRITE,  size_scrypt4, NULL, &device_param->d_scryptV3_buf);

    if (CL_err == -1) return -1;

    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_bitmap_s1_a,    CL_TRUE, 0, bitmap_ctx->bitmap_size,  bitmap_ctx->bitmap_s1_a,        0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_bitmap_s1_b,    CL_TRUE, 0, bitmap_ctx->bitmap_size,  bitmap_ctx->bitmap_s1_b,        0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_bitmap_s1_c,    CL_TRUE, 0, bitmap_ctx->bitmap_size,  bitmap_ctx->bitmap_s1_c,        0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_bitmap_s1_d,    CL_TRUE, 0, bitmap_ctx->bitmap_size,  bitmap_ctx->bitmap_s1_d,        0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_bitmap_s2_a,    CL_TRUE, 0, bitmap_ctx->bitmap_size,  bitmap_ctx->bitmap_s2_a,        0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_bitmap_s2_b,    CL_TRUE, 0, bitmap_ctx->bitmap_size,  bitmap_ctx->bitmap_s2_b,        0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_bitmap_s2_c,    CL_TRUE, 0, bitmap_ctx->bitmap_size,  bitmap_ctx->bitmap_s2_c,        0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_bitmap_s2_d,    CL_TRUE, 0, bitmap_ctx->bitmap_size,  bitmap_ctx->bitmap_s2_d,        0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_digests_buf,    CL_TRUE, 0, size_digests, hashes->digests_buf,   0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_digests_shown,  CL_TRUE, 0, size_shown,   hashes->digests_shown, 0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_salt_bufs,      CL_TRUE, 0, size_salts,   hashes->salts_buf,     0, NULL, NULL);

    if (CL_err == -1) return -1;

    /**
     * special buffers
     */

    if (user_options_extra->attack_kern == ATTACK_KERN_STRAIGHT)
    {
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_rules,   NULL, &device_param->d_rules);
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_rules_c, NULL, &device_param->d_rules_c);

      if (CL_err == -1) return -1;

      CL_err = hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_rules, CL_TRUE, 0, size_rules, straight_ctx->kernel_rules_buf, 0, NULL, NULL);

      if (CL_err == -1) return -1;
    }
    else if (user_options_extra->attack_kern == ATTACK_KERN_COMBI)
    {
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_combs,      NULL, &device_param->d_combs);
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_combs,      NULL, &device_param->d_combs_c);
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_root_css,   NULL, &device_param->d_root_css_buf);
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_markov_css, NULL, &device_param->d_markov_css_buf);

      if (CL_err == -1) return -1;
    }
    else if (user_options_extra->attack_kern == ATTACK_KERN_BF)
    {
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_bfs,        NULL, &device_param->d_bfs);
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_bfs,        NULL, &device_param->d_bfs_c);
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_tm,         NULL, &device_param->d_tm_c);
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_root_css,   NULL, &device_param->d_root_css_buf);
      CL_err |= hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_markov_css, NULL, &device_param->d_markov_css_buf);

      if (CL_err == -1) return -1;
    }

    if (size_esalts)
    {
      CL_err = hc_clCreateBuffer (hashcat_ctx, device_param->context, CL_MEM_READ_ONLY, size_esalts, NULL, &device_param->d_esalt_bufs);

      if (CL_err == -1) return -1;

      CL_err = hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_esalt_bufs, CL_TRUE, 0, size_esalts, hashes->esalts_buf, 0, NULL, NULL);

      if (CL_err == -1) return -1;
    }

    /**
     * main host data
     */

    pw_t *pws_buf = (pw_t *) hcmalloc (hashcat_ctx, size_pws); VERIFY_PTR (pws_buf);

    device_param->pws_buf = pws_buf;

    comb_t *combs_buf = (comb_t *) hccalloc (hashcat_ctx, KERNEL_COMBS, sizeof (comb_t)); VERIFY_PTR (combs_buf);

    device_param->combs_buf = combs_buf;

    void *hooks_buf = hcmalloc (hashcat_ctx, size_hooks); VERIFY_PTR (hooks_buf);

    device_param->hooks_buf = hooks_buf;

    /**
     * kernel args
     */

    device_param->kernel_params_buf32[24] = bitmap_ctx->bitmap_mask;
    device_param->kernel_params_buf32[25] = bitmap_ctx->bitmap_shift1;
    device_param->kernel_params_buf32[26] = bitmap_ctx->bitmap_shift2;
    device_param->kernel_params_buf32[27] = 0; // salt_pos
    device_param->kernel_params_buf32[28] = 0; // loop_pos
    device_param->kernel_params_buf32[29] = 0; // loop_cnt
    device_param->kernel_params_buf32[30] = 0; // kernel_rules_cnt
    device_param->kernel_params_buf32[31] = 0; // digests_cnt
    device_param->kernel_params_buf32[32] = 0; // digests_offset
    device_param->kernel_params_buf32[33] = 0; // combs_mode
    device_param->kernel_params_buf32[34] = 0; // gid_max

    device_param->kernel_params[ 0] = (hashconfig->attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
                                    ? &device_param->d_pws_buf
                                    : &device_param->d_pws_amp_buf;
    device_param->kernel_params[ 1] = &device_param->d_rules_c;
    device_param->kernel_params[ 2] = &device_param->d_combs_c;
    device_param->kernel_params[ 3] = &device_param->d_bfs_c;
    device_param->kernel_params[ 4] = &device_param->d_tmps;
    device_param->kernel_params[ 5] = &device_param->d_hooks;
    device_param->kernel_params[ 6] = &device_param->d_bitmap_s1_a;
    device_param->kernel_params[ 7] = &device_param->d_bitmap_s1_b;
    device_param->kernel_params[ 8] = &device_param->d_bitmap_s1_c;
    device_param->kernel_params[ 9] = &device_param->d_bitmap_s1_d;
    device_param->kernel_params[10] = &device_param->d_bitmap_s2_a;
    device_param->kernel_params[11] = &device_param->d_bitmap_s2_b;
    device_param->kernel_params[12] = &device_param->d_bitmap_s2_c;
    device_param->kernel_params[13] = &device_param->d_bitmap_s2_d;
    device_param->kernel_params[14] = &device_param->d_plain_bufs;
    device_param->kernel_params[15] = &device_param->d_digests_buf;
    device_param->kernel_params[16] = &device_param->d_digests_shown;
    device_param->kernel_params[17] = &device_param->d_salt_bufs;
    device_param->kernel_params[18] = &device_param->d_esalt_bufs;
    device_param->kernel_params[19] = &device_param->d_result;
    device_param->kernel_params[20] = &device_param->d_scryptV0_buf;
    device_param->kernel_params[21] = &device_param->d_scryptV1_buf;
    device_param->kernel_params[22] = &device_param->d_scryptV2_buf;
    device_param->kernel_params[23] = &device_param->d_scryptV3_buf;
    device_param->kernel_params[24] = &device_param->kernel_params_buf32[24];
    device_param->kernel_params[25] = &device_param->kernel_params_buf32[25];
    device_param->kernel_params[26] = &device_param->kernel_params_buf32[26];
    device_param->kernel_params[27] = &device_param->kernel_params_buf32[27];
    device_param->kernel_params[28] = &device_param->kernel_params_buf32[28];
    device_param->kernel_params[29] = &device_param->kernel_params_buf32[29];
    device_param->kernel_params[30] = &device_param->kernel_params_buf32[30];
    device_param->kernel_params[31] = &device_param->kernel_params_buf32[31];
    device_param->kernel_params[32] = &device_param->kernel_params_buf32[32];
    device_param->kernel_params[33] = &device_param->kernel_params_buf32[33];
    device_param->kernel_params[34] = &device_param->kernel_params_buf32[34];

    device_param->kernel_params_mp_buf64[3] = 0;
    device_param->kernel_params_mp_buf32[4] = 0;
    device_param->kernel_params_mp_buf32[5] = 0;
    device_param->kernel_params_mp_buf32[6] = 0;
    device_param->kernel_params_mp_buf32[7] = 0;
    device_param->kernel_params_mp_buf32[8] = 0;

    device_param->kernel_params_mp[0] = &device_param->d_combs;
    device_param->kernel_params_mp[1] = &device_param->d_root_css_buf;
    device_param->kernel_params_mp[2] = &device_param->d_markov_css_buf;
    device_param->kernel_params_mp[3] = &device_param->kernel_params_mp_buf64[3];
    device_param->kernel_params_mp[4] = &device_param->kernel_params_mp_buf32[4];
    device_param->kernel_params_mp[5] = &device_param->kernel_params_mp_buf32[5];
    device_param->kernel_params_mp[6] = &device_param->kernel_params_mp_buf32[6];
    device_param->kernel_params_mp[7] = &device_param->kernel_params_mp_buf32[7];
    device_param->kernel_params_mp[8] = &device_param->kernel_params_mp_buf32[8];

    device_param->kernel_params_mp_l_buf64[3] = 0;
    device_param->kernel_params_mp_l_buf32[4] = 0;
    device_param->kernel_params_mp_l_buf32[5] = 0;
    device_param->kernel_params_mp_l_buf32[6] = 0;
    device_param->kernel_params_mp_l_buf32[7] = 0;
    device_param->kernel_params_mp_l_buf32[8] = 0;
    device_param->kernel_params_mp_l_buf32[9] = 0;

    device_param->kernel_params_mp_l[0] = &device_param->d_pws_buf;
    device_param->kernel_params_mp_l[1] = &device_param->d_root_css_buf;
    device_param->kernel_params_mp_l[2] = &device_param->d_markov_css_buf;
    device_param->kernel_params_mp_l[3] = &device_param->kernel_params_mp_l_buf64[3];
    device_param->kernel_params_mp_l[4] = &device_param->kernel_params_mp_l_buf32[4];
    device_param->kernel_params_mp_l[5] = &device_param->kernel_params_mp_l_buf32[5];
    device_param->kernel_params_mp_l[6] = &device_param->kernel_params_mp_l_buf32[6];
    device_param->kernel_params_mp_l[7] = &device_param->kernel_params_mp_l_buf32[7];
    device_param->kernel_params_mp_l[8] = &device_param->kernel_params_mp_l_buf32[8];
    device_param->kernel_params_mp_l[9] = &device_param->kernel_params_mp_l_buf32[9];

    device_param->kernel_params_mp_r_buf64[3] = 0;
    device_param->kernel_params_mp_r_buf32[4] = 0;
    device_param->kernel_params_mp_r_buf32[5] = 0;
    device_param->kernel_params_mp_r_buf32[6] = 0;
    device_param->kernel_params_mp_r_buf32[7] = 0;
    device_param->kernel_params_mp_r_buf32[8] = 0;

    device_param->kernel_params_mp_r[0] = &device_param->d_bfs;
    device_param->kernel_params_mp_r[1] = &device_param->d_root_css_buf;
    device_param->kernel_params_mp_r[2] = &device_param->d_markov_css_buf;
    device_param->kernel_params_mp_r[3] = &device_param->kernel_params_mp_r_buf64[3];
    device_param->kernel_params_mp_r[4] = &device_param->kernel_params_mp_r_buf32[4];
    device_param->kernel_params_mp_r[5] = &device_param->kernel_params_mp_r_buf32[5];
    device_param->kernel_params_mp_r[6] = &device_param->kernel_params_mp_r_buf32[6];
    device_param->kernel_params_mp_r[7] = &device_param->kernel_params_mp_r_buf32[7];
    device_param->kernel_params_mp_r[8] = &device_param->kernel_params_mp_r_buf32[8];

    device_param->kernel_params_amp_buf32[5] = 0; // combs_mode
    device_param->kernel_params_amp_buf32[6] = 0; // gid_max

    device_param->kernel_params_amp[0] = &device_param->d_pws_buf;
    device_param->kernel_params_amp[1] = &device_param->d_pws_amp_buf;
    device_param->kernel_params_amp[2] = &device_param->d_rules_c;
    device_param->kernel_params_amp[3] = &device_param->d_combs_c;
    device_param->kernel_params_amp[4] = &device_param->d_bfs_c;
    device_param->kernel_params_amp[5] = &device_param->kernel_params_amp_buf32[5];
    device_param->kernel_params_amp[6] = &device_param->kernel_params_amp_buf32[6];

    device_param->kernel_params_tm[0] = &device_param->d_bfs_c;
    device_param->kernel_params_tm[1] = &device_param->d_tm_c;

    device_param->kernel_params_memset_buf32[1] = 0; // value
    device_param->kernel_params_memset_buf32[2] = 0; // gid_max

    device_param->kernel_params_memset[0] = NULL;
    device_param->kernel_params_memset[1] = &device_param->kernel_params_memset_buf32[1];
    device_param->kernel_params_memset[2] = &device_param->kernel_params_memset_buf32[2];

    /**
     * kernel name
     */

    size_t kernel_wgs_tmp;

    char kernel_name[64] = { 0 };

    if (hashconfig->attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
    {
      if (hashconfig->opti_type & OPTI_TYPE_SINGLE_HASH)
      {
        snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_s%02d", hashconfig->kern_type, 4);

        CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel1);

        if (CL_err == -1) return -1;

        snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_s%02d", hashconfig->kern_type, 8);

        CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel2);

        if (CL_err == -1) return -1;

        snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_s%02d", hashconfig->kern_type, 16);

        CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel3);

        if (CL_err == -1) return -1;
      }
      else
      {
        snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_m%02d", hashconfig->kern_type, 4);

        CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel1);

        if (CL_err == -1) return -1;

        snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_m%02d", hashconfig->kern_type, 8);

        CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel2);

        if (CL_err == -1) return -1;

        snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_m%02d", hashconfig->kern_type, 16);

        CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel3);

        if (CL_err == -1) return -1;
      }

      if (user_options->attack_mode == ATTACK_MODE_BF)
      {
        if (hashconfig->opts_type & OPTS_TYPE_PT_BITSLICE)
        {
          snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_tm", hashconfig->kern_type);

          CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel_tm);

          if (CL_err == -1) return -1;

          CL_err = hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel_tm, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);

          if (CL_err == -1) return -1;
        }
      }
    }
    else
    {
      snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_init", hashconfig->kern_type);

      CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel1);

      if (CL_err == -1) return -1;

      snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_loop", hashconfig->kern_type);

      CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel2);

      if (CL_err == -1) return -1;

      snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_comp", hashconfig->kern_type);

      CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel3);

      if (CL_err == -1) return -1;

      if (hashconfig->opts_type & OPTS_TYPE_HOOK12)
      {
        snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_hook12", hashconfig->kern_type);

        CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel12);

        if (CL_err == -1) return -1;

        CL_err = hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel12, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);

        if (CL_err == -1) return -1;
      }

      if (hashconfig->opts_type & OPTS_TYPE_HOOK23)
      {
        snprintf (kernel_name, sizeof (kernel_name) - 1, "m%05d_hook23", hashconfig->kern_type);

        CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, kernel_name, &device_param->kernel23);

        if (CL_err == -1) return -1;

        CL_err = hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel23, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);

        if (CL_err == -1) return -1;
      }
    }

    CL_err |= hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel1, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);
    CL_err |= hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel2, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);
    CL_err |= hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel3, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);

    if (CL_err == -1) return -1;

    for (u32 i = 0; i <= 23; i++)
    {
      CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel1, i, sizeof (cl_mem), device_param->kernel_params[i]);
      CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel2, i, sizeof (cl_mem), device_param->kernel_params[i]);
      CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel3, i, sizeof (cl_mem), device_param->kernel_params[i]);

      if (hashconfig->opts_type & OPTS_TYPE_HOOK12) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel12, i, sizeof (cl_mem), device_param->kernel_params[i]);
      if (hashconfig->opts_type & OPTS_TYPE_HOOK23) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel23, i, sizeof (cl_mem), device_param->kernel_params[i]);

      if (CL_err == -1) return -1;
    }

    for (u32 i = 24; i <= 34; i++)
    {
      CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel1, i, sizeof (cl_uint), device_param->kernel_params[i]);
      CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel2, i, sizeof (cl_uint), device_param->kernel_params[i]);
      CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel3, i, sizeof (cl_uint), device_param->kernel_params[i]);

      if (hashconfig->opts_type & OPTS_TYPE_HOOK12) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel12, i, sizeof (cl_uint), device_param->kernel_params[i]);
      if (hashconfig->opts_type & OPTS_TYPE_HOOK23) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel23, i, sizeof (cl_uint), device_param->kernel_params[i]);

      if (CL_err == -1) return -1;
    }

    // GPU memset

    CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program, "gpu_memset", &device_param->kernel_memset);

    if (CL_err == -1) return -1;

    CL_err = hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel_memset, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);

    if (CL_err == -1) return -1;

    CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_memset, 0, sizeof (cl_mem),  device_param->kernel_params_memset[0]);
    CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_memset, 1, sizeof (cl_uint), device_param->kernel_params_memset[1]);
    CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_memset, 2, sizeof (cl_uint), device_param->kernel_params_memset[2]);

    if (CL_err == -1) return -1;

    // MP start

    if (user_options->attack_mode == ATTACK_MODE_BF)
    {
      CL_err |= hc_clCreateKernel (hashcat_ctx, device_param->program_mp, "l_markov", &device_param->kernel_mp_l);
      CL_err |= hc_clCreateKernel (hashcat_ctx, device_param->program_mp, "r_markov", &device_param->kernel_mp_r);

      if (CL_err == -1) return -1;

      CL_err |= hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel_mp_l, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);
      CL_err |= hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel_mp_r, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);

      if (CL_err == -1) return -1;

      if (hashconfig->opts_type & OPTS_TYPE_PT_BITSLICE)
      {
        CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_tm, 0, sizeof (cl_mem), device_param->kernel_params_tm[0]);
        CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_tm, 1, sizeof (cl_mem), device_param->kernel_params_tm[1]);

        if (CL_err == -1) return -1;
      }
    }
    else if (user_options->attack_mode == ATTACK_MODE_HYBRID1)
    {
      CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program_mp, "C_markov", &device_param->kernel_mp);

      if (CL_err == -1) return -1;

      CL_err = hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel_mp, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);

      if (CL_err == -1) return -1;
    }
    else if (user_options->attack_mode == ATTACK_MODE_HYBRID2)
    {
      CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program_mp, "C_markov", &device_param->kernel_mp);

      if (CL_err == -1) return -1;

      CL_err = hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel_mp, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);

      if (CL_err == -1) return -1;
    }

    if (hashconfig->attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
    {
      // nothing to do
    }
    else
    {
      CL_err = hc_clCreateKernel (hashcat_ctx, device_param->program_amp, "amp", &device_param->kernel_amp);

      if (CL_err == -1) return -1;

      CL_err = hc_clGetKernelWorkGroupInfo (hashcat_ctx, device_param->kernel_amp, device_param->device, CL_KERNEL_WORK_GROUP_SIZE, sizeof (size_t), &kernel_wgs_tmp, NULL); kernel_threads = MIN (kernel_threads, kernel_wgs_tmp);

      if (CL_err == -1) return -1;
    }

    if (hashconfig->attack_exec == ATTACK_EXEC_INSIDE_KERNEL)
    {
      // nothing to do
    }
    else
    {
      for (u32 i = 0; i < 5; i++)
      {
        CL_err = hc_clSetKernelArg (hashcat_ctx, device_param->kernel_amp, i, sizeof (cl_mem), device_param->kernel_params_amp[i]);

        if (CL_err == -1) return -1;
      }

      for (u32 i = 5; i < 7; i++)
      {
        CL_err = hc_clSetKernelArg (hashcat_ctx, device_param->kernel_amp, i, sizeof (cl_uint), device_param->kernel_params_amp[i]);

        if (CL_err == -1) return -1;
      }
    }

    // maybe this has been updated by clGetKernelWorkGroupInfo()
    // value can only be decreased, so we don't need to reallocate buffers

    device_param->kernel_threads = kernel_threads;

    // zero some data buffers

    run_kernel_bzero (hashcat_ctx, device_param, device_param->d_pws_buf,        size_pws);
    run_kernel_bzero (hashcat_ctx, device_param, device_param->d_pws_amp_buf,    size_pws);
    run_kernel_bzero (hashcat_ctx, device_param, device_param->d_tmps,           size_tmps);
    run_kernel_bzero (hashcat_ctx, device_param, device_param->d_hooks,          size_hooks);
    run_kernel_bzero (hashcat_ctx, device_param, device_param->d_plain_bufs,     size_plains);
    run_kernel_bzero (hashcat_ctx, device_param, device_param->d_result,         size_results);

    /**
     * special buffers
     */

    if (user_options_extra->attack_kern == ATTACK_KERN_STRAIGHT)
    {
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_rules_c, size_rules_c);
    }
    else if (user_options_extra->attack_kern == ATTACK_KERN_COMBI)
    {
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_combs,          size_combs);
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_combs_c,        size_combs);
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_root_css_buf,   size_root_css);
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_markov_css_buf, size_markov_css);
    }
    else if (user_options_extra->attack_kern == ATTACK_KERN_BF)
    {
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_bfs,            size_bfs);
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_bfs_c,          size_bfs);
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_tm_c,           size_tm);
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_root_css_buf,   size_root_css);
      run_kernel_bzero (hashcat_ctx, device_param, device_param->d_markov_css_buf, size_markov_css);
    }

    if ((user_options->attack_mode == ATTACK_MODE_HYBRID1) || (user_options->attack_mode == ATTACK_MODE_HYBRID2))
    {
      /**
       * prepare mp
       */

      if (user_options->attack_mode == ATTACK_MODE_HYBRID1)
      {
        device_param->kernel_params_mp_buf32[5] = 0;
        device_param->kernel_params_mp_buf32[6] = 0;
        device_param->kernel_params_mp_buf32[7] = 0;

        if (hashconfig->opts_type & OPTS_TYPE_PT_ADD01)     device_param->kernel_params_mp_buf32[5] = full01;
        if (hashconfig->opts_type & OPTS_TYPE_PT_ADD80)     device_param->kernel_params_mp_buf32[5] = full80;
        if (hashconfig->opts_type & OPTS_TYPE_PT_ADDBITS14) device_param->kernel_params_mp_buf32[6] = 1;
        if (hashconfig->opts_type & OPTS_TYPE_PT_ADDBITS15) device_param->kernel_params_mp_buf32[7] = 1;
      }
      else if (user_options->attack_mode == ATTACK_MODE_HYBRID2)
      {
        device_param->kernel_params_mp_buf32[5] = 0;
        device_param->kernel_params_mp_buf32[6] = 0;
        device_param->kernel_params_mp_buf32[7] = 0;
      }

      for (u32 i = 0; i < 3; i++) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_mp, i, sizeof (cl_mem), (void *) device_param->kernel_params_mp[i]);

      if (CL_err == -1) return -1;
    }
    else if (user_options->attack_mode == ATTACK_MODE_BF)
    {
      /**
       * prepare mp_r and mp_l
       */

      device_param->kernel_params_mp_l_buf32[6] = 0;
      device_param->kernel_params_mp_l_buf32[7] = 0;
      device_param->kernel_params_mp_l_buf32[8] = 0;

      if (hashconfig->opts_type & OPTS_TYPE_PT_ADD01)     device_param->kernel_params_mp_l_buf32[6] = full01;
      if (hashconfig->opts_type & OPTS_TYPE_PT_ADD80)     device_param->kernel_params_mp_l_buf32[6] = full80;
      if (hashconfig->opts_type & OPTS_TYPE_PT_ADDBITS14) device_param->kernel_params_mp_l_buf32[7] = 1;
      if (hashconfig->opts_type & OPTS_TYPE_PT_ADDBITS15) device_param->kernel_params_mp_l_buf32[8] = 1;

      for (u32 i = 0; i < 3; i++) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_mp_l, i, sizeof (cl_mem), (void *) device_param->kernel_params_mp_l[i]);
      for (u32 i = 0; i < 3; i++) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_mp_r, i, sizeof (cl_mem), (void *) device_param->kernel_params_mp_r[i]);

      if (CL_err == -1) return -1;
    }
  }

  opencl_ctx->hardware_power_all = hardware_power_all;

  return 0;
}

void opencl_session_destroy (hashcat_ctx_t *hashcat_ctx)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  if (opencl_ctx->enabled == false) return;

  for (u32 device_id = 0; device_id < opencl_ctx->devices_cnt; device_id++)
  {
    hc_device_param_t *device_param = &opencl_ctx->devices_param[device_id];

    if (device_param->skipped) continue;

    cl_int CL_err = CL_SUCCESS;

    hcfree (device_param->pws_buf);
    hcfree (device_param->combs_buf);
    hcfree (device_param->hooks_buf);

    if (device_param->d_pws_buf)          CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_pws_buf);
    if (device_param->d_pws_amp_buf)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_pws_amp_buf);
    if (device_param->d_rules)            CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_rules);
    if (device_param->d_rules_c)          CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_rules_c);
    if (device_param->d_combs)            CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_combs);
    if (device_param->d_combs_c)          CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_combs_c);
    if (device_param->d_bfs)              CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bfs);
    if (device_param->d_bfs_c)            CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bfs_c);
    if (device_param->d_bitmap_s1_a)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bitmap_s1_a);
    if (device_param->d_bitmap_s1_b)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bitmap_s1_b);
    if (device_param->d_bitmap_s1_c)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bitmap_s1_c);
    if (device_param->d_bitmap_s1_d)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bitmap_s1_d);
    if (device_param->d_bitmap_s2_a)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bitmap_s2_a);
    if (device_param->d_bitmap_s2_b)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bitmap_s2_b);
    if (device_param->d_bitmap_s2_c)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bitmap_s2_c);
    if (device_param->d_bitmap_s2_d)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_bitmap_s2_d);
    if (device_param->d_plain_bufs)       CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_plain_bufs);
    if (device_param->d_digests_buf)      CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_digests_buf);
    if (device_param->d_digests_shown)    CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_digests_shown);
    if (device_param->d_salt_bufs)        CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_salt_bufs);
    if (device_param->d_esalt_bufs)       CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_esalt_bufs);
    if (device_param->d_tmps)             CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_tmps);
    if (device_param->d_hooks)            CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_hooks);
    if (device_param->d_result)           CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_result);
    if (device_param->d_scryptV0_buf)     CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_scryptV0_buf);
    if (device_param->d_scryptV1_buf)     CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_scryptV1_buf);
    if (device_param->d_scryptV2_buf)     CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_scryptV2_buf);
    if (device_param->d_scryptV3_buf)     CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_scryptV3_buf);
    if (device_param->d_root_css_buf)     CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_root_css_buf);
    if (device_param->d_markov_css_buf)   CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_markov_css_buf);
    if (device_param->d_tm_c)             CL_err |= hc_clReleaseMemObject (hashcat_ctx, device_param->d_tm_c);

    if (device_param->kernel1)            CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel1);
    if (device_param->kernel12)           CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel12);
    if (device_param->kernel2)            CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel2);
    if (device_param->kernel23)           CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel23);
    if (device_param->kernel3)            CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel3);
    if (device_param->kernel_mp)          CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel_mp);
    if (device_param->kernel_mp_l)        CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel_mp_l);
    if (device_param->kernel_mp_r)        CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel_mp_r);
    if (device_param->kernel_tm)          CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel_tm);
    if (device_param->kernel_amp)         CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel_amp);
    if (device_param->kernel_memset)      CL_err |= hc_clReleaseKernel (hashcat_ctx, device_param->kernel_memset);

    if (device_param->program)            CL_err |= hc_clReleaseProgram (hashcat_ctx, device_param->program);
    if (device_param->program_mp)         CL_err |= hc_clReleaseProgram (hashcat_ctx, device_param->program_mp);
    if (device_param->program_amp)        CL_err |= hc_clReleaseProgram (hashcat_ctx, device_param->program_amp);

    if (device_param->command_queue)      CL_err |= hc_clReleaseCommandQueue (hashcat_ctx, device_param->command_queue);

    if (device_param->context)            CL_err |= hc_clReleaseContext (hashcat_ctx, device_param->context);

    //if (CL_err == -1) return -1;

    device_param->pws_buf           = NULL;
    device_param->combs_buf         = NULL;
    device_param->hooks_buf         = NULL;

    device_param->d_pws_buf         = NULL;
    device_param->d_pws_amp_buf     = NULL;
    device_param->d_rules           = NULL;
    device_param->d_rules_c         = NULL;
    device_param->d_combs           = NULL;
    device_param->d_combs_c         = NULL;
    device_param->d_bfs             = NULL;
    device_param->d_bfs_c           = NULL;
    device_param->d_bitmap_s1_a     = NULL;
    device_param->d_bitmap_s1_b     = NULL;
    device_param->d_bitmap_s1_c     = NULL;
    device_param->d_bitmap_s1_d     = NULL;
    device_param->d_bitmap_s2_a     = NULL;
    device_param->d_bitmap_s2_b     = NULL;
    device_param->d_bitmap_s2_c     = NULL;
    device_param->d_bitmap_s2_d     = NULL;
    device_param->d_plain_bufs      = NULL;
    device_param->d_digests_buf     = NULL;
    device_param->d_digests_shown   = NULL;
    device_param->d_salt_bufs       = NULL;
    device_param->d_esalt_bufs      = NULL;
    device_param->d_tmps            = NULL;
    device_param->d_hooks           = NULL;
    device_param->d_result          = NULL;
    device_param->d_scryptV0_buf    = NULL;
    device_param->d_scryptV1_buf    = NULL;
    device_param->d_scryptV2_buf    = NULL;
    device_param->d_scryptV3_buf    = NULL;
    device_param->d_root_css_buf    = NULL;
    device_param->d_markov_css_buf  = NULL;
    device_param->d_tm_c            = NULL;
    device_param->kernel1           = NULL;
    device_param->kernel12          = NULL;
    device_param->kernel2           = NULL;
    device_param->kernel23          = NULL;
    device_param->kernel3           = NULL;
    device_param->kernel_mp         = NULL;
    device_param->kernel_mp_l       = NULL;
    device_param->kernel_mp_r       = NULL;
    device_param->kernel_tm         = NULL;
    device_param->kernel_amp        = NULL;
    device_param->kernel_memset     = NULL;
    device_param->program           = NULL;
    device_param->program_mp        = NULL;
    device_param->program_amp       = NULL;
    device_param->command_queue     = NULL;
    device_param->context           = NULL;
  }
}

void opencl_session_reset (hashcat_ctx_t *hashcat_ctx)
{
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  if (opencl_ctx->enabled == false) return;

  for (u32 device_id = 0; device_id < opencl_ctx->devices_cnt; device_id++)
  {
    hc_device_param_t *device_param = &opencl_ctx->devices_param[device_id];

    if (device_param->skipped) continue;

    device_param->speed_pos = 0;

    memset (device_param->speed_cnt, 0, SPEED_CACHE * sizeof (u64));
    memset (device_param->speed_ms,  0, SPEED_CACHE * sizeof (double));

    device_param->exec_pos = 0;

    memset (device_param->exec_ms, 0, EXEC_CACHE * sizeof (double));

    device_param->outerloop_pos  = 0;
    device_param->outerloop_left = 0;
    device_param->innerloop_pos  = 0;
    device_param->innerloop_left = 0;

    // some more resets:

    if (device_param->pws_buf) memset (device_param->pws_buf, 0, device_param->size_pws);

    device_param->pws_cnt = 0;

    device_param->words_off  = 0;
    device_param->words_done = 0;
  }

  opencl_ctx->kernel_power_all   = 0;
  opencl_ctx->kernel_power_final = 0;
}

int opencl_session_update_combinator (hashcat_ctx_t *hashcat_ctx)
{
  combinator_ctx_t *combinator_ctx = hashcat_ctx->combinator_ctx;
  hashconfig_t     *hashconfig     = hashcat_ctx->hashconfig;
  opencl_ctx_t     *opencl_ctx     = hashcat_ctx->opencl_ctx;

  if (opencl_ctx->enabled == false) return 0;

  for (u32 device_id = 0; device_id < opencl_ctx->devices_cnt; device_id++)
  {
    hc_device_param_t *device_param = &opencl_ctx->devices_param[device_id];

    if (device_param->skipped) continue;

    // kernel_params

    device_param->kernel_params_buf32[33] = combinator_ctx->combs_mode;

    cl_int CL_err = CL_SUCCESS;

    CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel1, 33, sizeof (cl_uint), device_param->kernel_params[33]);
    CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel2, 33, sizeof (cl_uint), device_param->kernel_params[33]);
    CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel3, 33, sizeof (cl_uint), device_param->kernel_params[33]);

    if (hashconfig->opts_type & OPTS_TYPE_HOOK12) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel12, 33, sizeof (cl_uint), device_param->kernel_params[33]);
    if (hashconfig->opts_type & OPTS_TYPE_HOOK23) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel23, 33, sizeof (cl_uint), device_param->kernel_params[33]);

    if (CL_err == -1) return -1;

    // kernel_params_amp

    device_param->kernel_params_amp_buf32[5] = combinator_ctx->combs_mode;

    if (hashconfig->attack_exec == ATTACK_EXEC_OUTSIDE_KERNEL)
    {
      CL_err = hc_clSetKernelArg (hashcat_ctx, device_param->kernel_amp, 5, sizeof (cl_uint), device_param->kernel_params_amp[5]);

      if (CL_err == -1) return -1;
    }
  }

  return 0;
}

int opencl_session_update_mp (hashcat_ctx_t *hashcat_ctx)
{
  mask_ctx_t   *mask_ctx   = hashcat_ctx->mask_ctx;
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  if (opencl_ctx->enabled == false) return 0;

  for (u32 device_id = 0; device_id < opencl_ctx->devices_cnt; device_id++)
  {
    hc_device_param_t *device_param = &opencl_ctx->devices_param[device_id];

    if (device_param->skipped) continue;

    device_param->kernel_params_mp_buf64[3] = 0;
    device_param->kernel_params_mp_buf32[4] = mask_ctx->css_cnt;

    cl_int CL_err = CL_SUCCESS;

    for (u32 i = 3; i < 4; i++) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_mp, i, sizeof (cl_ulong), (void *) device_param->kernel_params_mp[i]);
    for (u32 i = 4; i < 8; i++) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_mp, i, sizeof (cl_uint),  (void *) device_param->kernel_params_mp[i]);

    if (CL_err == -1) return -1;

    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_root_css_buf,   CL_TRUE, 0, device_param->size_root_css,   mask_ctx->root_css_buf,   0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_markov_css_buf, CL_TRUE, 0, device_param->size_markov_css, mask_ctx->markov_css_buf, 0, NULL, NULL);

    if (CL_err == -1) return -1;
  }

  return 0;
}

int opencl_session_update_mp_rl (hashcat_ctx_t *hashcat_ctx, const u32 css_cnt_l, const u32 css_cnt_r)
{
  mask_ctx_t   *mask_ctx   = hashcat_ctx->mask_ctx;
  opencl_ctx_t *opencl_ctx = hashcat_ctx->opencl_ctx;

  if (opencl_ctx->enabled == false) return 0;

  for (u32 device_id = 0; device_id < opencl_ctx->devices_cnt; device_id++)
  {
    hc_device_param_t *device_param = &opencl_ctx->devices_param[device_id];

    if (device_param->skipped) continue;

    device_param->kernel_params_mp_l_buf64[3] = 0;
    device_param->kernel_params_mp_l_buf32[4] = css_cnt_l;
    device_param->kernel_params_mp_l_buf32[5] = css_cnt_r;

    device_param->kernel_params_mp_r_buf64[3] = 0;
    device_param->kernel_params_mp_r_buf32[4] = css_cnt_r;

    cl_int CL_err = CL_SUCCESS;

    for (u32 i = 3; i < 4; i++) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_mp_l, i, sizeof (cl_ulong), (void *) device_param->kernel_params_mp_l[i]);
    for (u32 i = 4; i < 9; i++) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_mp_l, i, sizeof (cl_uint),  (void *) device_param->kernel_params_mp_l[i]);

    for (u32 i = 3; i < 4; i++) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_mp_r, i, sizeof (cl_ulong), (void *) device_param->kernel_params_mp_r[i]);
    for (u32 i = 4; i < 8; i++) CL_err |= hc_clSetKernelArg (hashcat_ctx, device_param->kernel_mp_r, i, sizeof (cl_uint),  (void *) device_param->kernel_params_mp_r[i]);

    if (CL_err == -1) return -1;

    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_root_css_buf,   CL_TRUE, 0, device_param->size_root_css,   mask_ctx->root_css_buf,   0, NULL, NULL);
    CL_err |= hc_clEnqueueWriteBuffer (hashcat_ctx, device_param->command_queue, device_param->d_markov_css_buf, CL_TRUE, 0, device_param->size_markov_css, mask_ctx->markov_css_buf, 0, NULL, NULL);

    if (CL_err == -1) return -1;
  }

  return 0;
}
