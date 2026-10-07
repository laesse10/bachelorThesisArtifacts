// Thin adapter for the `native` cell of comet_int4_gemm (extractedMatrix, Part A).
// Placed by the sweep as cpp_backend/comet_int4_gemm_fp64.cpp and built by the harness's `cpp`
// framework (g++ 13.3.1, the cpp column's flags). It includes the hand-written upstream extraction
// UNCHANGED (tests/ports/comet_int4_gemm/comet_int4_gemm_ref.cpp at HPCAgent-Bench 26a4f0cf) and
// exports the harness ABI symbol. It ONLY maps arguments: num_left = num_right = num_vector,
// num_field passed through, int64_t -> int casts. No arithmetic, copies or loops. The original's
// int status (0 ok; 1 null pointer; 2 non-positive size) is discarded.
#include "../../../../../../tests/ports/comet_int4_gemm/comet_int4_gemm_ref.cpp"

extern "C" void comet_int4_gemm_fp64(const int8_t *codes_left, const int8_t *codes_right, int32_t *out,
                                     const int64_t num_field, const int64_t num_vector) {
  (void)comet_int4_gemm_ref(codes_left, codes_right, out, (int)num_vector, (int)num_vector, (int)num_field);
}
