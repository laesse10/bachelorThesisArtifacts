// Rendered by DaCe CPF (canonical parallel form): self-contained, no DaCe runtime.
// Already parallelized, with basic heuristics applied. Every loop names its class first:
//   parallel, sequential -- settled; their parallelism needs no further reasoning.
//   unsure               -- open; the only loops whose parallelism is worth reasoning about.
// Spend the effort on heuristic optimizations and restructuring.
#include <cstdint>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <complex>
#include <numeric>
#include <new>
#include <type_traits>
#include <hip/hip_runtime.h>
#include <hipcub/hipcub.hpp>
#include <utility>

// What dace/dace.h would define for the device side.
using gpuStream_t = hipStream_t;
using gpuEvent_t = hipEvent_t;
using gpuError_t = hipError_t;
static constexpr gpuError_t gpuSuccess = hipSuccess;
static constexpr gpuError_t gpuErrorMemoryAllocation = hipErrorOutOfMemory;

//: A failed GPU call ends the program, naming the call site.
static inline void cpf_gpu_check(gpuError_t status, const char *file = __builtin_FILE(), int line = __builtin_LINE()) {
    if (status != gpuSuccess) {
        fprintf(stderr, "%s:%d: GPU error %d (%s)\n", file, line, (int)status, hipGetErrorString(status));
        abort();
    }
}

//: The one stream canon offloads onto, in the shape the generated frame indexes.
struct cpf_gpu_context {
    gpuStream_t streams[1];
    gpuEvent_t events[1];
    gpuError_t lasterror;
};

//: A failed kernel launch ends the program, naming the kernel and its launch geometry.
static inline void cpf_kernel_launch_check(gpuError_t err, const char *name, long long gx, long long gy, long long gz,
                                           long long bx, long long by, long long bz) {
    if (err != gpuSuccess) {
        fprintf(stderr, "%s launch failed (grid %lld,%lld,%lld block %lld,%lld,%lld): %s\n", name, gx, gy, gz, bx, by,
                bz, hipGetErrorString(err));
        abort();
    }
}

//: The backend cub, under the name the code generator writes -- what gpucub.cuh aliases. hipCUB
//: keeps the functor structs CCCL 3 dropped, and a unit built by hipcc is never built against CCCL.
namespace gpucub = hipcub;


// Functions the DaCe runtime headers would otherwise provide.
template <typename T>
static constexpr inline T cpf_min(const T& value) {
    return value;
}
template <typename T, typename... Ts>
static constexpr inline typename std::common_type<T, Ts...>::type cpf_min(const T& a, const Ts&... rest) {
    return (cpf_min(rest...) < a) ? cpf_min(rest...) : a;
}
//: The CUB scratch pool, as much of it as one entry point can hold: one buffer per tag,
//: allocated on first use and grown, never shrunk, so a repeated call pays no allocation.
//: The runtime pre-allocates and releases it from the entry points a repeated invocation
//: shares; one self-contained call has neither, so the buffer owns itself and frees at
//: static destruction. It is also not keyed by stream, as the runtime pool is: a CPF unit
//: issues every launch on the one stream (see cpf_gpu_context), so its calls are ordered.
struct ScanTag {};
struct DetectFlagTag {};
struct cpf_scratch_block {
    void *ptr = nullptr;
    size_t capacity = 0;
    ~cpf_scratch_block() { if (ptr != nullptr) { (void)hipFree(ptr); } }
};
template <typename Tag>
static inline void *get_scratch(size_t bytes, gpuStream_t stream, gpuError_t *status) {
    static cpf_scratch_block block;
    *status = gpuSuccess;
    if (bytes <= block.capacity) return block.ptr;
    if (block.ptr != nullptr) {
        // In flight work may still be reading the old buffer.
        *status = hipStreamSynchronize(stream);
        if (*status != gpuSuccess) return nullptr;
        *status = hipFree(block.ptr);
        block.ptr = nullptr;
        block.capacity = 0;
        if (*status != gpuSuccess) return nullptr;
    }
    *status = hipMalloc(&block.ptr, bytes);
    if (*status != gpuSuccess) { block.ptr = nullptr; return nullptr; }
    block.capacity = bytes;
    return block.ptr;
}
template <typename T, typename U, typename R = decltype(std::declval<T>() / std::declval<U>())>
static constexpr inline R int_ceil(const T& numerator, const U& denominator) {
    return (numerator + denominator - 1) / denominator;
}
//: The first-order linear recurrence out[k] = c[k]*out[k-1] + d[k], entered at out[-1] = seed.
//: The carry is the affine MAP x -> a*x + b rather than a value, and map composition is
//: associative, so a plain prefix scan over the maps computes the recurrence.
//:
//: The seed is folded into element 0 rather than handed to cub as an init value, and that is
//: numerically load-bearing: element 0 comes out as the CONSTANT map {0, c[0]*seed + d[0]},
//: so every prefix including it carries a == 0 and the coefficient product never spans more
//: than one composed segment -- which is what keeps it off the overflow the closed form hits.
template <typename E>
struct cpf_affine_map { E a; E b; };
template <typename E>
struct cpf_affine_compose {
    __device__ __forceinline__ cpf_affine_map<E> operator()(const cpf_affine_map<E> &x,
                                                            const cpf_affine_map<E> &y) const {
        return cpf_affine_map<E>{y.a * x.a, y.a * x.b + y.b};
    }
};
template <typename E, typename C, typename D, typename S>
__global__ void cpf_affine_pack_kernel(const C *__restrict__ c, const D *__restrict__ d,
                                       cpf_affine_map<E> *__restrict__ m, const S *__restrict__ seed_ptr,
                                       E seed_val, long long n) {
    const long long k = (long long)blockIdx.x * (long long)blockDim.x + (long long)threadIdx.x;
    if (k >= n) return;
    const E ck = (E)c[k];
    const E dk = (E)d[k];
    // A device-resident seed arrives as a pointer; a host-readable one by value.
    if (k == 0) {
        const E s = (seed_ptr != nullptr) ? (E)(*seed_ptr) : seed_val;
        m[0] = cpf_affine_map<E>{(E)0, ck * s + dk};
    } else {
        m[k] = cpf_affine_map<E>{ck, dk};
    }
}
template <typename E>
__global__ void cpf_affine_unpack_kernel(const cpf_affine_map<E> *__restrict__ m, E *__restrict__ out,
                                         long long n) {
    const long long k = (long long)blockIdx.x * (long long)blockDim.x + (long long)threadIdx.x;
    if (k < n) out[k] = m[k].b;  // every composed prefix is constant, so b IS the value
}
template <typename E, typename C, typename D, typename S>
static inline gpuError_t inclusive_affine(const C *coef, const D *delta, const S *seed_ptr, E seed_val,
                                          E *out, long long n, gpuStream_t stream) {
    using M = cpf_affine_map<E>;
    constexpr int block_threads = 256;
    if (n <= 0) return gpuSuccess;
    cpf_affine_compose<E> op;
    size_t cub_bytes = 0;
    gpuError_t status = gpucub::DeviceScan::InclusiveScan(nullptr, cub_bytes, (M *)nullptr, (M *)nullptr,
                                                         op, n, stream);
    if (status != gpuSuccess) return status;
    // 256-byte alignment for the workspace that follows: cub assumes an allocation at least as
    // aligned as hipMalloc gives, and the maps sit in front of it in the one block.
    const size_t map_bytes = (((size_t)n * sizeof(M)) + 255u) & ~(size_t)255u;
    void *scratch = get_scratch<ScanTag>(map_bytes + cub_bytes, stream, &status);
    if (scratch == nullptr) return status != gpuSuccess ? status : gpuErrorMemoryAllocation;
    M *maps = (M *)scratch;
    void *workspace = (char *)scratch + map_bytes;
    const unsigned blocks = (unsigned)((n + block_threads - 1) / block_threads);
    cpf_affine_pack_kernel<E, C, D, S><<<blocks, block_threads, 0, stream>>>(coef, delta, maps, seed_ptr,
                                                                            seed_val, n);
    status = hipGetLastError();
    if (status != gpuSuccess) return status;
    status = gpucub::DeviceScan::InclusiveScan(workspace, cub_bytes, maps, maps, op, n, stream);
    if (status != gpuSuccess) return status;
    cpf_affine_unpack_kernel<E><<<blocks, block_threads, 0, stream>>>(maps, out, n);
    return hipGetLastError();
}

struct versioned_distance_update_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

gpuError_t __cpf_scan_affine_versioned_distance_update_fp64_1r0r0_8(const double* __sc_c, const double* __sc_d, const double* __sc_seed_ptr, double __sc_seed_val, double* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
void __cpf_runkernel_versioned_distance_update_fp64_single_state_body_map_2_0_17(versioned_distance_update_fp64_state_t *__state, double * __restrict__ _scan_coef_a, double * __restrict__ _scan_in_a, const double * __restrict__ b, const double * __restrict__ c, int K, int64_t LEN_1D, gpuStream_t gpu_stream);
static constexpr inline int64_t _scan_coef_a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t _scan_in_a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_host_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t b_host_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t c_host_idx(int64_t __d0) { return __d0; }
extern "C" void versioned_distance_update_fp64(double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, int64_t K, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    versioned_distance_update_fp64_state_t __cpf_state{&__cpf_context};
    versioned_distance_update_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    double * _scan_seed_a;
    cpf_gpu_check(hipMalloc((void**)&_scan_seed_a, K * sizeof(double)));
    double a_index_0;
    double a_index_1;
    double __tmp0_0;
    double __tmp0_1;
    double b_slice_times_c_slice_0;
    double b_slice_times_c_slice_1;
    double __tmp1_0;
    double __tmp1_1;
    double* __restrict__ a_host = new (std::align_val_t(64)) double[LEN_1D];
    double* __restrict__ c_host = new (std::align_val_t(64)) double[LEN_1D];
    double* __restrict__ b_host = new (std::align_val_t(64)) double[LEN_1D];
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {

        {  // check_assumption_0
            if ((K < 0)) {
                std::abort();
            }
        }
        {  // check_assumption_1
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    if ((K >= 1)) {
        {
            double * _scan_in_a;
            cpf_gpu_check(hipMalloc((void**)&_scan_in_a, ((- K) + LEN_1D) * sizeof(double)));
            double * _scan_coef_a;
            cpf_gpu_check(hipMalloc((void**)&_scan_coef_a, ((- K) + LEN_1D) * sizeof(double)));

            {
                gpuStream_t __cpf_current_stream = gpu_streams[0];
                __cpf_runkernel_versioned_distance_update_fp64_single_state_body_map_2_0_17(__state, _scan_coef_a, _scan_in_a, b, c, K, LEN_1D, __cpf_current_stream);
            }
            {  // copy_a_to__scan_seed_a
                hipMemcpyAsync(_scan_seed_a, a, K * sizeof(double), hipMemcpyDeviceToDevice, gpu_streams[gpu_streams_idx(0)]);
            }
            // scan: running (prefix) fold along an axis
            // parallel scan; canonicalization takes the parallel form.
            // Alternative: a sequential loop over parallel maps.
            // CPU: the loop is worth trying -- the scan does more work, and the loop may already saturate the memory system.
            // GPU: the scan is usually the better of the two.
            // Both are correct. Measure before choosing.
            {  // for_17_affine_scan_op
                cpf_gpu_check(__cpf_scan_affine_versioned_distance_update_fp64_1r0r0_8(_scan_coef_a, _scan_in_a, _scan_seed_a, static_cast<double>(0), (a + K), (((- K) + LEN_1D)), gpu_streams[gpu_streams_idx(0)]));
            }
            cpf_gpu_check(hipFree(_scan_in_a));
            cpf_gpu_check(hipFree(_scan_coef_a));

        }
    } else if (((! (K >= 1)) && (K == 0))) {
        {

            {  // copy_a_to_a_host
                hipMemcpyAsync(a_host, a, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }
            {  // copy_c_to_c_host
                hipMemcpyAsync(c_host, c, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }
            {  // copy_b_to_b_host
                hipMemcpyAsync(b_host, b, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }

        }

        // sequential -- pinned by an earlier pass; the dependence test finds the iterations independent
        // settled: the pass keeps the order on purpose (such as the fallback arm of a specialization)
        for (int64_t _loop_it_2 = K; (_loop_it_2 < LEN_1D); _loop_it_2 = (_loop_it_2 + 1)) {
            {

                a_index_0 = a_host[a_host_idx(((- K) + _loop_it_2))];  // _assign_a_to_a_index_0
                __tmp0_0 = (0.75 * a_index_0);  // _Mult_
                b_slice_times_c_slice_0 = (b_host[b_host_idx(_loop_it_2)] * c_host[c_host_idx(_loop_it_2)]);  // _Mult_
                __tmp1_0 = (__tmp0_0 + b_slice_times_c_slice_0);  // _Add_
                a_host[a_host_idx(_loop_it_2)] = __tmp1_0;  // _assign___tmp1_0_to_a

            }

        }

        {

            {  // copy_a_host_to_a
                hipMemcpyAsync(a, a_host, LEN_1D * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
            }

        }
    } else {
        {

            {  // copy_a_to_a_host
                hipMemcpyAsync(a_host, a, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }
            {  // copy_c_to_c_host
                hipMemcpyAsync(c_host, c, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }
            {  // copy_b_to_b_host
                hipMemcpyAsync(b_host, b, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }

        }

        // unsure -- never examined for dependences
        // open: not proven either way; kept sequential to be safe, and may well be parallel -- a fact about the indices (a bound, a permutation) settles it
        for (int64_t _loop_it_3 = K; (_loop_it_3 < LEN_1D); _loop_it_3 = (_loop_it_3 + 1)) {
            {

                a_index_1 = a_host[a_host_idx(((- K) + _loop_it_3))];  // _assign_a_to_a_index_1
                __tmp0_1 = (0.75 * a_index_1);  // _Mult_
                b_slice_times_c_slice_1 = (b_host[b_host_idx(_loop_it_3)] * c_host[c_host_idx(_loop_it_3)]);  // _Mult_
                __tmp1_1 = (__tmp0_1 + b_slice_times_c_slice_1);  // _Add_
                a_host[a_host_idx(_loop_it_3)] = __tmp1_1;  // _assign___tmp1_1_to_a

            }

        }

        {

            {  // copy_a_host_to_a
                hipMemcpyAsync(a, a_host, LEN_1D * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
            }

        }
    }


    cpf_gpu_check(hipFree(_scan_seed_a));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](a_host, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](c_host, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](b_host, std::align_val_t(64));
    gpu_streams = nullptr;
}

gpuError_t __cpf_scan_affine_versioned_distance_update_fp64_1r0r0_8(const double* __sc_c, const double* __sc_d, const double* __sc_seed_ptr, double __sc_seed_val, double* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
gpuError_t __cpf_scan_affine_versioned_distance_update_fp64_1r0r0_8(const double* __sc_c, const double* __sc_d, const double* __sc_seed_ptr, double __sc_seed_val, double* __sc_out, long long __sc_n, gpuStream_t __sc_stream) {
    return inclusive_affine<double, double, double, double>(
    __sc_c, __sc_d, __sc_seed_ptr, __sc_seed_val, __sc_out, __sc_n, __sc_stream);
}


// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) versioned_distance_update_fp64_single_state_body_map_2_0_17(double * __restrict__ _scan_coef_a, double * __restrict__ _scan_in_a, const double * __restrict__ b, const double * __restrict__ c, int K, int64_t LEN_1D) {
    int64_t b__loop_it_1 = (K + (256 * static_cast<int64_t>(blockIdx.x)));
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t _loop_it_1 = (threadIdx.x + b__loop_it_1);
        if (_loop_it_1 >= b__loop_it_1 && _loop_it_1 < (cpf_min((LEN_1D - 1), (b__loop_it_1 + 255)) + 1)) {
            _scan_coef_a[_scan_coef_a_idx(((- K) + _loop_it_1))] = 0.75;  // affine_coef
            _scan_in_a[_scan_in_a_idx(((- K) + _loop_it_1))] = (b[b_idx(_loop_it_1)] * c[c_idx(_loop_it_1)]);  // affine_delta
        }
    }
}


void __cpf_runkernel_versioned_distance_update_fp64_single_state_body_map_2_0_17(versioned_distance_update_fp64_state_t *__state, double * __restrict__ _scan_coef_a, double * __restrict__ _scan_in_a, const double * __restrict__ b, const double * __restrict__ c, int K, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_versioned_distance_update_fp64_single_state_body_map_2_0_17(versioned_distance_update_fp64_state_t *__state, double * __restrict__ _scan_coef_a, double * __restrict__ _scan_in_a, const double * __restrict__ b, const double * __restrict__ c, int K, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil(((- K) + LEN_1D), 256)) <= 0)) {

        return;
    }

    void  *versioned_distance_update_fp64_single_state_body_map_2_0_17_args[] = { (void *)&_scan_coef_a, (void *)&_scan_in_a, (void *)&b, (void *)&c, (void *)&K, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)versioned_distance_update_fp64_single_state_body_map_2_0_17, dim3(int_ceil(((- K) + LEN_1D), 256), 1, 1), dim3(256, 1, 1), versioned_distance_update_fp64_single_state_body_map_2_0_17_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "versioned_distance_update_fp64_single_state_body_map_2_0_17", int_ceil(((- K) + LEN_1D), 256), 1, 1, 256, 1, 1);
}