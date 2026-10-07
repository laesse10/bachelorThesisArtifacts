// Thin adapter for the `native` cell of warpx_boris_push (extractedMatrix, Part A).
// Placed by the sweep as cpp_backend/warpx_boris_push_fp64.cpp and built by the harness's `cpp`
// framework (g++ 13.3.1, the cpp column's flags). It includes the hand-written upstream
// transcription UNCHANGED (warpx_boris_push_reference.cpp in the kernel folder at HPCAgent-Bench
// 26a4f0cf) and exports the harness ABI symbol. It ONLY maps arguments: reorders them, casts
// int64_t -> int / long, and passes `dt` as the manifest's pinned value 1.0e-13 (config
// dt: {value: 1.0e-13}). The harness ABI carries no dt because every translated column compiles
// that value in (`constexpr double dt = 1e-13;`). No arithmetic, copies or loops.
#include "../warpx_boris_push_reference.cpp"

extern "C" void warpx_boris_push_fp64(const double *Bx, const double *By, const double *Bz, const double *Ex,
                                      const double *Ey, const double *Ez, double *ux, double *uy, double *uz,
                                      const double m, const int64_t momentum_push_type, const int64_t np_particles,
                                      const double q) {
  warpx_boris_push_original(Bx, By, Bz, Ex, Ey, Ez, ux, uy, uz, /*dt=*/1.0e-13, m, (int)momentum_push_type, q,
                            (long)np_particles);
}
