# LLR-40 with LLVM 22.1 for C, C++ and Fortran

Median of the 90 timed runs per cell (three interleaved rounds of 30, one exclusive node per kernel).
A ratio above 1: the named build is faster than its baseline.

| comparison | n | geomean | 95% CI | faster / within 3% / slower |
|---|---:|---:|---|---|
| clangxx_over_clang | 39 | 0.996 | 0.986 - 1.006 | 1 / 36 / 2 |
| flang_over_clang | 39 | 1.001 | 0.982 - 1.021 | 4 / 29 / 6 |
| clang_over_gcc | 39 | 0.887 | 0.788 - 0.998 | 7 / 22 / 10 |

| kernel | C++ vs C (LLVM) | Fortran vs C (LLVM) | clang vs GCC (C) |
|---|---:|---:|---:|
| `argmax_with_index` | 0.9999 | 0.9996 | 0.2908 |
| `compact_threshold_pack` | 1.0010 | 1.0136 | 1.0328 |
| `ext_break_capture` | 0.9998 | 0.9993 | 0.9972 |
| `ext_war_unit` | 1.0006 | 1.0002 | 0.9996 |
| `fuse_diamond` | 1.0001 | 1.1681 | 0.9938 |
| `fuse_move_ifs` | 1.0031 | 1.0039 | 0.9997 |
| `fuse_stencil_through_transient` | 1.0326 | 1.2796 | 0.9472 |
| `quasi_affine_reduce_odd` | 1.0008 | 1.0673 | 0.9443 |
| `scan_affine_decay` | 1.0001 | 1.0001 | 0.9998 |
| `scatter_accum_dup` | 1.0002 | 0.9211 | 0.9732 |
| `segment_reduce_ragged` | 1.0010 | 1.0054 | 0.8967 |
| `tsvc_2_s115` |  |  |  |
| `tsvc_2_s119` | 0.9994 | 1.0015 | 0.9971 |
| `tsvc_2_s1232` | 1.0235 | 0.9086 | 1.0424 |
| `tsvc_2_s1244` | 1.0012 | 1.0028 | 0.9871 |
| `tsvc_2_s152` | 1.0082 | 1.0080 | 0.9859 |
| `tsvc_2_s2233` | 0.9978 | 0.9979 | 1.0041 |
| `tsvc_2_s2275` | 0.8568 | 0.8578 | 0.8815 |
| `tsvc_2_s231` | 0.9997 | 1.0021 | 1.0987 |
| `tsvc_2_s233` | 1.0036 | 1.0034 | 0.9989 |
| `tsvc_2_s235` | 0.9030 | 1.0009 | 1.1095 |
| `tsvc_2_s252` | 1.0021 | 0.9996 | 1.0546 |
| `tsvc_2_s255` | 0.9994 | 0.9982 | 1.1945 |
| `tsvc_2_s2710` | 0.9936 | 1.0019 | 0.9972 |
| `tsvc_2_s275` | 1.0223 | 1.0347 | 0.9779 |
| `tsvc_2_s311` | 0.9996 | 1.0000 | 1.0004 |
| `tsvc_2_s3110` | 0.9999 | 0.9998 | 0.2871 |
| `tsvc_2_s3111` | 1.0003 | 1.0001 | 0.4006 |
| `tsvc_2_s3112` | 0.9998 | 1.0004 | 0.9999 |
| `tsvc_2_s316` | 1.0002 | 0.9999 | 0.9519 |
| `tsvc_2_s318` | 1.0004 | 0.9999 | 0.2838 |
| `tsvc_2_s319` | 1.0058 | 1.0033 | 0.9866 |
| `tsvc_2_s323` | 0.9998 | 1.0005 | 0.9389 |
| `tsvc_2_s4112` | 1.0002 | 0.9576 | 1.0351 |
| `tsvc_2_vag` | 0.9994 | 0.9561 | 1.0010 |
| `tsvc_2_vpvts` | 0.9997 | 0.9994 | 0.9817 |
| `tsvc_2_vtvtv` | 0.9989 | 0.9999 | 0.9996 |
| `versioned_distance_update` | 0.9993 | 1.0000 | 1.0033 |
| `wf_diff_skew` | 0.9997 | 0.9349 | 1.0021 |
| `wf_triangular` | 0.9982 | 0.9960 | 1.0062 |

Cells not ok: `tsvc_2_s115`/c (incorrect), `tsvc_2_s115`/clang (incorrect), `tsvc_2_s115`/clangxx (incorrect), `tsvc_2_s115`/flang (incorrect)

LLVM cells whose library carries no LLVM mark: none
