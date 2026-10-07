#include <dace/dace.h>
typedef void * canon_gpuHandle_t;
extern "C" canon_gpuHandle_t __dace_init_canon_gpu(int64_t NSEG);
extern "C" int __dace_exit_canon_gpu(canon_gpuHandle_t handle);
extern "C" void __program_canon_gpu(canon_gpuHandle_t handle, double * __restrict__ out, int64_t * __restrict__ row_ptr, double * __restrict__ val, double * __restrict__ w, int64_t NSEG);
