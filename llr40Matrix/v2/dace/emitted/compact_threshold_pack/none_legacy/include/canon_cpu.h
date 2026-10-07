#include <dace/dace.h>
typedef void * canon_cpuHandle_t;
extern "C" canon_cpuHandle_t __dace_init_canon_cpu(int64_t LEN_1D);
extern "C" int __dace_exit_canon_cpu(canon_cpuHandle_t handle);
extern "C" void __program_canon_cpu(canon_cpuHandle_t handle, int64_t * __restrict__ out_count, double * __restrict__ packed, double * __restrict__ src, double * __restrict__ weight, int64_t LEN_1D);
