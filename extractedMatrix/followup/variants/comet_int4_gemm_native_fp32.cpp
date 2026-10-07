// extractedMatrix/followup variant: the fp32 half of the `native` cell of comet_int4_gemm.
// The harness binds lib<k>_cpp.so's symbol by the ARGUMENT dtypes (cpp_runtime.wrap_kernel: fp64 only if
// some argument is a float64/complex128 array). CoMet's arguments are int8/int32, so the harness calls
// comet_int4_gemm_fp32, and in the matrix (job 4996043) that was still the translator's fp32 C++ code: the
// adapter (../adapters/comet_int4_gemm_native.cpp) replaced only comet_int4_gemm_fp64.cpp. Placed over
// cpp_backend/comet_int4_gemm_fp32.cpp, next to that adapter over the fp64 file, this exports the fp32 symbol
// as the same thin mapping. comet_int4_gemm_ref has external linkage and is defined once, by the fp64
// adapter's #include of the upstream file, so it is only declared here.
#include <cstdint>

extern "C" int comet_int4_gemm_ref(const int8_t *__restrict__ codes_left, const int8_t *__restrict__ codes_right,
                                   int32_t *__restrict__ out, int num_left, int num_right, int num_field);

extern "C" void comet_int4_gemm_fp32(const int8_t *codes_left, const int8_t *codes_right, int32_t *out,
                                     const int64_t num_field, const int64_t num_vector) {
  (void)comet_int4_gemm_ref(codes_left, codes_right, out, (int)num_vector, (int)num_vector, (int)num_field);
}
