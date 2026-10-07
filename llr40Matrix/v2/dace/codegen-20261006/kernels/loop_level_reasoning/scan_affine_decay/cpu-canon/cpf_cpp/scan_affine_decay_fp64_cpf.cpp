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
static constexpr inline int64_t y_idx(int64_t __d0) { return __d0; }
extern "C" void scan_affine_decay_fp64(const double * __restrict__ c, const double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {
        double _scan_seed_y;

        {  // _assign_y_to__scan_seed_y
            double _out;
            _out = y[y_idx(0)];
            _scan_seed_y = _out;
        }
        // scan: running (prefix) fold along an axis
        // parallel scan; canonicalization takes the parallel form.
        // Alternative: a sequential loop over parallel maps.
        // CPU: the loop is worth trying -- the scan does more work, and the loop may already saturate the memory system.
        // GPU: the scan is usually the better of the two.
        // Both are correct. Measure before choosing.
        {  // for_16_affine_scan_op
            { const long cpf_n = (long)(static_cast<long>((LEN_1D - 1))); const long cpf_s = (long)(1);
                if (cpf_s <= 0) std::abort();
                for (long cpf_r = 0; cpf_r < cpf_s; ++cpf_r) {
                    if (cpf_r >= cpf_n) continue;
                    (y + 1)[cpf_r] = (c + 1)[cpf_r] * (_scan_seed_y) + (x + 1)[cpf_r];
                    for (long cpf_j = cpf_r + cpf_s; cpf_j < cpf_n; cpf_j += cpf_s) (y + 1)[cpf_j] = (c + 1)[cpf_j] * (y + 1)[cpf_j - cpf_s] + (x + 1)[cpf_j];
                }
            };
        }

    }
}
