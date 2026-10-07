#include <dace/dace.h>
typedef void * canon_gpuHandle_t;
extern "C" canon_gpuHandle_t __dace_init_canon_gpu(int64_t LEN_1D);
extern "C" int __dace_exit_canon_gpu(canon_gpuHandle_t handle);
extern "C" void __program_canon_gpu(canon_gpuHandle_t handle, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t LEN_1D);
