#!/usr/bin/env python3
"""Writes the DaCe follow-up variants into variants/. Each changes ONE thing; every edit is asserted to
apply exactly once.

DaCe side (``<k>_<name>.cpp``, placed over the generated src/cpu/canon_cpu.cpp by dace_gap.py
--override-cpp): edits of the canon/new code in ../emitted/<k>/canon_new/src/cpu/canon_cpu.cpp.
C++ side (``<k>_<name>_fp64.cpp``, placed over the translated cpp_backend/<k>_fp64.cpp by --override-src):
edits of the translated C++ at 5cfcb4f2 (base_cpp/, as emitted by the benchmark's translator).
"""
import pathlib

HERE = pathlib.Path(__file__).resolve().parent
EMIT = HERE.parent / "emitted"
BASE = HERE / "base_cpp"
V = HERE / "variants"
V.mkdir(exist_ok=True)


def once(s, a, b):
    assert s.count(a) == 1, (a[:90], s.count(a))
    return s.replace(a, b)


def dace_src(k):
    return (EMIT / k / "canon_new" / "src" / "cpu" / "canon_cpu.cpp").read_text()


def write_dace(k, name, note, s):
    first, rest = s.split("\n", 1)
    assert "DaCe AUTO-GENERATED" in first
    (V / f"{k}_{name}.cpp").write_text(f"/* extractedMatrix-style follow-up variant `{name}` of DaCe's canon/new code for {k}\n"
                                       f"   (llr40Matrix/v2/dace/emitted/{k}/canon_new): {note} */\n" + rest)


def cpp_src(k):
    return (BASE / f"{k}_fp64.cpp").read_text()


def write_cpp(k, name, note, s):
    (V / f"{k}_{name}_fp64.cpp").write_text(f"// DaCe follow-up variant `{name}` of the translated {k}_fp64.cpp at 5cfcb4f2: {note}\n" + s)


# ------------------------------------------------------------------ DaCe side
# compact_threshold_pack: the parallel compaction (mask pass, exclusive scan, sum, scatter pass)
k = "compact_threshold_pack"; s = dace_src(k)
start = s.index("        #pragma omp parallel for\n        for (int64_t compaction_it_for_17_compaction_mask")
end = s.index("    n = (0 + (1 * compaction_total_for_17));")
region = s[start:end]
assert region.count("#pragma omp parallel for") == 2
seq = s[:start] + """        /* follow-up `dseq`: the four passes as one sequential compaction loop */
        int64_t n_ = 0;
        for (int64_t i_ = 0; i_ < LEN_1D; ++i_) {
            if (src[i_] > 0.0) {
                packed[n_] = (src[i_] * weight[i_]);
                n_ += 1;
            }
        }
        compaction_total_for_17 = n_;
    }
""" + s[end:]
write_dace(k, "dseq", "the mask pass, the exclusive scan, the sum and the scatter pass replaced by one sequential\n"
           "   compaction loop, as the translated C++ does it. Allocation of the (now unused) mask/rank arrays unchanged.", seq)
nolib = once(s, "            ::dace::scan::exclusive_sum((__state->__0_compaction_mask_for_17), (__state->__0_compaction_mask_for_17) + (LEN_1D), (__state->__0_compaction_rank_for_17), static_cast<int64_t>(0));",
             "            { int64_t acc_ = 0; for (int64_t k_ = 0; k_ < LEN_1D; ++k_) { __state->__0_compaction_rank_for_17[k_] = acc_; acc_ += __state->__0_compaction_mask_for_17[k_]; } }  /* follow-up `dnolib` */")
nolib = once(nolib, "            (&compaction_total_for_17)[0] = ::dace::reduce::sum((__state->__0_compaction_mask_for_17), (long)(LEN_1D), (long)(1), (&compaction_total_for_17)[0]);",
             "            { int64_t acc_ = (&compaction_total_for_17)[0]; for (int64_t k_ = 0; k_ < LEN_1D; ++k_) acc_ += __state->__0_compaction_mask_for_17[k_]; (&compaction_total_for_17)[0] = acc_; }  /* follow-up `dnolib` */")
write_dace(k, "dnolib", "the four passes kept, but dace::scan::exclusive_sum and dace::reduce::sum written as plain\n"
           "   sequential loops (same values).", nolib)

# s323: the recurrence split into a map, a library scan and a second map
k = "tsvc_2_s323"; s = dace_src(k)
start = s.index("        double _scan_seed_b;")
end = s.index("}\n\nDACE_EXPORTED void __program_canon_cpu(")
seq = s[:start] + """        /* follow-up `dseq`: the recurrence as one sequential loop, the translated code's order */
        for (int64_t i_ = 1; i_ < LEN_1D; ++i_) {
            a[i_] = (b[(i_ - 1)] + (c[i_] * d[i_]));
            b[i_] = (a[i_] + (c[i_] * e[i_]));
        }
    }
""" + s[end:]
write_dace(k, "dseq", "the map, the library scan and the second map replaced by one sequential loop, as the\n"
           "   translated C++ computes it (a, then b, per element).", seq)
nolib = once(s, "            ::dace::scan::inclusive_sum((__state->__0__scan_in_b), (__state->__0__scan_in_b) + ((LEN_1D - 1)), (b + 1), _scan_seed_b);",
             "            { double acc_ = _scan_seed_b; for (int64_t k_ = 0; k_ < (LEN_1D - 1); ++k_) { acc_ = acc_ + __state->__0__scan_in_b[k_]; b[1 + k_] = acc_; } }  /* follow-up `dnolib` */")
write_dace(k, "dnolib", "the three passes kept, but dace::scan::inclusive_sum written as a plain sequential loop.", nolib)

# s3112: a running sum as the library scan
k = "tsvc_2_s3112"; s = dace_src(k)
nolib = once(s, "            ::dace::scan::inclusive_sum(a, a + (LEN_1D), b, _scan_seed_b);",
             "            { double acc_ = _scan_seed_b; for (int64_t k_ = 0; k_ < LEN_1D; ++k_) { acc_ = acc_ + a[k_]; b[k_] = acc_; } }  /* follow-up `dnolib` */")
write_dace(k, "dnolib", "dace::scan::inclusive_sum written as a plain sequential loop (same additions, same order).", nolib)

# scatter_accum_dup: one atomic update per element
k = "scatter_accum_dup"; s = dace_src(k)
na = once(s, "        dace::wcr_fixed<dace::ReductionType::Sum, double>::reduce_atomic(bins + ip_index, *(&_wcr_priv__Add____out));",
          "        bins[ip_index] += _wcr_priv__Add____out;  /* follow-up `dnoatomic`: was reduce_atomic */")
write_dace(k, "dnoatomic", "the atomic update (wcr_fixed<Sum>::reduce_atomic) replaced by a plain += (one thread).", na)

# small losses: OpenMP worksharing on one thread
for k in ("tsvc_2_s115", "wf_diff_skew", "fuse_move_ifs"):
    s = dace_src(k)
    lines = s.split("\n")
    n = sum(1 for l in lines if l.strip().startswith("#pragma omp"))
    assert n >= 1, k
    s2 = "\n".join(l for l in lines if not l.strip().startswith("#pragma omp"))
    write_dace(k, "dnoomp", f"its {n} `#pragma omp` line(s) removed (the loops and blocks they annotate stay), so nothing\n"
               "   is outlined into an OpenMP region and no worksharing loop has a barrier.", s2)

# s311: the sum through dace::reduce::sum (omp parallel for simd reduction)
k = "tsvc_2_s311"; s = dace_src(k)
io = once(s, "            sum_out[0] = ::dace::reduce::sum(a, (long)(LEN_1D), (long)(1), sum_out[0]);",
          "            { double acc_ = sum_out[0]; for (long k_ = 0; k_ < (long)(LEN_1D); ++k_) acc_ = acc_ + a[k_]; sum_out[0] = acc_; }  /* follow-up `dinorder` */")
write_dace(k, "dinorder", "dace::reduce::sum (an `omp parallel for simd reduction`, which may reorder the additions)\n"
           "   written as a plain in-order loop.", io)

# ------------------------------------------------------------------ C++ side
k = "tsvc_2_s231"; s = cpp_src(k)
s = once(s, """        for (int64_t i = 0; i < LEN_2D; ++i) {
          for (int64_t j = 1; j < LEN_2D; ++j) {
            aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + bb[(j)*(LEN_2D) + (i)]);
          }
        }""", """        for (int64_t j = 1; j < LEN_2D; ++j) {
          for (int64_t i = 0; i < LEN_2D; ++i) {
            aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + bb[(j)*(LEN_2D) + (i)]);
          }
        }""")
write_cpp(k, "cppinter", "the i and j loops interchanged (bit-identical result).", s)

k = "tsvc_2_s2233"; s = cpp_src(k)
s = once(s, """        for (int64_t i = 8; i < LEN_2D; ++i) {
          for (int64_t j = 8; j < LEN_2D; ++j) {
            aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + cc[(j)*(LEN_2D) + (i)]);
          }
          for (int64_t j = 8; j < LEN_2D; ++j) {
            bb[(i)*(LEN_2D) + (j)] = (bb[((i - 1))*(LEN_2D) + (j)] + cc[(i)*(LEN_2D) + (j)]);
          }
        }""", """        for (int64_t j = 8; j < LEN_2D; ++j) {
          for (int64_t i = 8; i < LEN_2D; ++i) {
            aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + cc[(j)*(LEN_2D) + (i)]);
          }
        }
        for (int64_t i = 8; i < LEN_2D; ++i) {
          for (int64_t j = 8; j < LEN_2D; ++j) {
            bb[(i)*(LEN_2D) + (j)] = (bb[((i - 1))*(LEN_2D) + (j)] + cc[(i)*(LEN_2D) + (j)]);
          }
        }""")
write_cpp(k, "cppinter", "the two inner loops distributed, the first nest interchanged (bit-identical result).", s)

k = "tsvc_2_s235"; s = cpp_src(k)
s = once(s, """        for (int64_t i = 0; i < LEN_2D; ++i) {
          a[i] = (a[i] + (b[i] * c[i]));
          for (int64_t j = 1; j < LEN_2D; ++j) {
            aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + (bb[(j)*(LEN_2D) + (i)] * a[i]));
          }
        }""", """        for (int64_t i = 0; i < LEN_2D; ++i) {
          a[i] = (a[i] + (b[i] * c[i]));
        }
        for (int64_t j = 1; j < LEN_2D; ++j) {
          for (int64_t i = 0; i < LEN_2D; ++i) {
            aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + (bb[(j)*(LEN_2D) + (i)] * a[i]));
          }
        }""")
write_cpp(k, "cppinter", "the a[i] update distributed, the aa nest interchanged (bit-identical result).", s)

k = "tsvc_2_s2275"; s = cpp_src(k)
s = once(s, """        for (int64_t i = 0; i < LEN_2D; ++i) {
          for (int64_t j = 0; j < LEN_2D; ++j) {
            aa[(j)*(LEN_2D) + (i)] = (aa[(j)*(LEN_2D) + (i)] + (bb[(j)*(LEN_2D) + (i)] * cc[(j)*(LEN_2D) + (i)]));
          }
          a[i] = (b[i] + (c[i] * d[i]));
        }""", """        for (int64_t j = 0; j < LEN_2D; ++j) {
          for (int64_t i = 0; i < LEN_2D; ++i) {
            aa[(j)*(LEN_2D) + (i)] = (aa[(j)*(LEN_2D) + (i)] + (bb[(j)*(LEN_2D) + (i)] * cc[(j)*(LEN_2D) + (i)]));
          }
        }
        for (int64_t i = 0; i < LEN_2D; ++i) {
          a[i] = (b[i] + (c[i] * d[i]));
        }""")
write_cpp(k, "cppinter", "the a[i] update distributed, the aa nest interchanged (bit-identical result).", s)

k = "tsvc_2_s1232"; s = cpp_src(k)
s = once(s, """        for (int64_t j = 0; j < LEN_2D; ++j) {
          for (int64_t i = (j * VLEN); i < LEN_2D; ++i) {
            aa[(i)*(LEN_2D) + (j)] = (bb[(i)*(LEN_2D) + (j)] + cc[(i)*(LEN_2D) + (j)]);
          }
        }""", """        for (int64_t i = 0; i < LEN_2D; ++i) {
          for (int64_t j = 0; j < LEN_2D && (j * VLEN) <= i; ++j) {
            aa[(i)*(LEN_2D) + (j)] = (bb[(i)*(LEN_2D) + (j)] + cc[(i)*(LEN_2D) + (j)]);
          }
        }""")
write_cpp(k, "cppinter", "the triangular nest interchanged, same (i, j) pairs (bit-identical result).", s)

k = "tsvc_2_s275"; s = cpp_src(k)
s = once(s, """        for (int64_t i = 0; i < LEN_2D; ++i) {
          if ((aa[(0)*(LEN_2D) + (i)] > 0.0)) {
            for (int64_t j = 1; j < LEN_2D; ++j) {
              aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + (bb[(j)*(LEN_2D) + (i)] * cc[(j)*(LEN_2D) + (i)]));
            }
          }
        }""", """        for (int64_t j = 1; j < LEN_2D; ++j) {
          for (int64_t i = 0; i < LEN_2D; ++i) {
            if ((aa[(0)*(LEN_2D) + (i)] > 0.0)) {
              aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + (bb[(j)*(LEN_2D) + (i)] * cc[(j)*(LEN_2D) + (i)]));
            }
          }
        }""")
write_cpp(k, "cppinter", "the nest interchanged, the test of aa[0][i] (row 0, never written) moved inside\n"
          "// (bit-identical result), as DaCe's pipeline does it.", s)

k = "fuse_diamond"; s = cpp_src(k)
a = s.index("        double *t = (double *)malloc"); b = s.index("        free(v);\n") + len("        free(v);\n")
s = s[:a] + """        for (int64_t i = 0; i < LEN_1D; ++i) {
          const double t = (a[i] * a[i]);
          const double u = (t + 1.0);
          const double v = (t - 1.0);
          out[i] = (u * v);
        }
""" + s[b:]
write_cpp(k, "cppfused", "the four loops fused, t, u, v as scalars (same operations per element).", s)

k = "fuse_stencil_through_transient"; s = cpp_src(k)
a = s.index("        double *tmp = (double *)malloc"); b = s.index("        free(tmp);\n") + len("        free(tmp);\n")
s = s[:a] + """        for (int64_t i = 1; i < (LEN_1D - 2); ++i) {
          out[i] = (((a[(i - 1)] + a[i]) + a[(i + 1)]) * ((a[i] + a[(i + 1)]) + a[(i + 2)]));
        }
""" + s[b:]
write_cpp(k, "cppfused", "the two loops fused, tmp computed in place with the same association.", s)

k = "tsvc_2_s453"; s = cpp_src(k)
s = once(s, """        s = 0.0;
        for (int64_t i = 0; i < LEN_1D; ++i) {
          s = (s + 2.0);
          a[i] = (s * b[i]);
        }""", """        s = 0.0;
        for (int64_t i = 0; i < LEN_1D; ++i) {
          a[i] = ((s + (2.0 * (i + 1))) * b[i]);
        }""")
write_cpp(k, "cppclosed", "the induction variable s replaced by its closed form s0 + 2(i+1), as DaCe's pipeline\n"
          "// writes it (exact: integers far below 2^53).", s)

print("\n".join(sorted(p.name for p in V.iterdir())))

# ------------------------------------------------------------------ the six smaller gains
# s119: DaCe's OpenMP structure (parallel region, `omp for` on the inner loop)
k = "tsvc_2_s119"; s = dace_src(k)
lines = s.split("\n"); n = sum(1 for l in lines if l.strip().startswith("#pragma omp"))
write_dace(k, "dnoomp", f"its {n} `#pragma omp` line(s) removed (the loops stay).", "\n".join(l for l in lines if not l.strip().startswith("#pragma omp")))
s = cpp_src(k)
s = once(s, """        for (int64_t i = 1; i < LEN_2D; ++i) {
          for (int64_t j = 1; j < LEN_2D; ++j) {
            aa[(i)*(LEN_2D) + (j)] = (aa[((i - 1))*(LEN_2D) + ((j - 1))] + bb[(i)*(LEN_2D) + (j)]);
          }
        }""", """        #pragma omp parallel
        {
        for (int64_t i = 1; i < LEN_2D; ++i) {
          #pragma omp for
          for (int64_t j = 1; j < LEN_2D; ++j) {
            aa[(i)*(LEN_2D) + (j)] = (aa[((i - 1))*(LEN_2D) + ((j - 1))] + bb[(i)*(LEN_2D) + (j)]);
          }
        }
        }""")
write_cpp(k, "cppomp", "the nest wrapped in DaCe's OpenMP structure: a parallel region around the outer loop and\n"
          "// `omp for` on the inner loop. Same loops, same arithmetic.", s)

# s152: DaCe fuses the two loops
k = "tsvc_2_s152"; s = cpp_src(k)
s = once(s, """        for (int64_t i = 0; i < LEN_1D; ++i) {
          b[i] = (d[i] * e[i]);
        }
        for (int64_t i = 0; i < LEN_1D; ++i) {
          a[i] = (a[i] + (b[i] * c[i]));
        }""", """        for (int64_t i = 0; i < LEN_1D; ++i) {
          b[i] = (d[i] * e[i]);
          a[i] = (a[i] + (b[i] * c[i]));
        }""")
write_cpp(k, "cppfused", "the two loops fused, as DaCe's pipeline does (same operations per element).", s)

# s233: DaCe makes the aa statement run along rows; bb keeps its strided inner loop
k = "tsvc_2_s233"; s = cpp_src(k)
s = once(s, """        for (int64_t i = 8; i < LEN_2D; ++i) {
          for (int64_t j = 8; j < LEN_2D; ++j) {
            aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + cc[(j)*(LEN_2D) + (i)]);
          }
          for (int64_t j = 8; j < LEN_2D; ++j) {
            bb[(j)*(LEN_2D) + (i)] = (bb[(j)*(LEN_2D) + ((i - 1))] + cc[(j)*(LEN_2D) + (i)]);
          }
        }""", """        for (int64_t j = 8; j < LEN_2D; ++j) {
          for (int64_t i = 8; i < LEN_2D; ++i) {
            aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + cc[(j)*(LEN_2D) + (i)]);
          }
        }
        for (int64_t i = 8; i < LEN_2D; ++i) {
          for (int64_t j = 8; j < LEN_2D; ++j) {
            bb[(j)*(LEN_2D) + (i)] = (bb[(j)*(LEN_2D) + ((i - 1))] + cc[(j)*(LEN_2D) + (i)]);
          }
        }""")
write_cpp(k, "cppinter", "the aa statement distributed into its own nest and interchanged so its inner loop runs\n"
          "// along a row; the bb nest unchanged (still strided), as in DaCe's code. Bit-identical result.", s)

# s255: DaCe rematerialises the carried scalars x, y from b[i-1], b[i-2]
k = "tsvc_2_s255"; s = cpp_src(k)
s = once(s, """        x = b[(LEN_1D - 1)];
        y = b[(LEN_1D - 2)];
        for (int64_t i = 0; i < LEN_1D; ++i) {
          a[i] = (((b[i] + x) + y) * 0.333);
          y = x;
          x = b[i];
        }""", """        x = b[(LEN_1D - 1)];
        y = b[(LEN_1D - 2)];
        a[0] = (((b[0] + x) + y) * 0.333);
        a[1] = (((b[1] + b[0]) + x) * 0.333);
        for (int64_t i = 2; i < LEN_1D; ++i) {
          a[i] = (((b[i] + b[(i - 1)]) + b[(i - 2)]) * 0.333);
        }""")
write_cpp(k, "cppremat", "the carried scalars x and y replaced by b[i-1] and b[i-2] (the first two iterations\n"
          "// peeled), as DaCe's pipeline does. Same values and association.", s)

# versioned_distance_update: DaCe's strided affine scan keeps each recurrence in a register
k = "versioned_distance_update"; s = dace_src(k)
s = once(s, "                ::dace::scan::inclusive_affine_strided(_scan_coef_a, _scan_in_a, (a + K), static_cast<long>(((- K) + LEN_1D)), static_cast<long>(K), (__state->__0__scan_seed_a));",
         "                { double* o_ = a + K; for (long k_ = 0; k_ < ((- K) + LEN_1D); ++k_) { const double prev_ = (k_ < K) ? __state->__0__scan_seed_a[k_] : o_[k_ - K]; o_[k_] = _scan_coef_a[k_] * prev_ + _scan_in_a[k_]; } }  /* follow-up `dnolib` */")
write_dace(k, "dnolib", "dace::scan::inclusive_affine_strided (one recurrence per residue class, its value kept in a\n"
           "   register) written as the plain recurrence that reads a[i-K] back from memory, as the translated code does.", s)

# wf_triangular: DaCe's skewed 64x64 tiling
k = "wf_triangular"; s = dace_src(k)
a_ = s.index("inline void nested_sdfg_0_1_0(")
b_ = s.index("\n}\n", a_) + 3
s = s[:a_] + """inline void nested_sdfg_0_1_0(canon_cpu_state_t *__state, double* __restrict__ a, int64_t LEN_2D) {
    /* follow-up `duntiled`: the skewed 64x64 tiles undone, the translated code's row-by-row order */
    for (int64_t i_ = 1; i_ < LEN_2D; ++i_) {
        for (int64_t j_ = i_; j_ < LEN_2D; ++j_) {
            a[(i_ * LEN_2D) + j_] = (((a[(i_ * LEN_2D) + j_] + a[((i_ - 1) * LEN_2D) + j_]) + a[(i_ * LEN_2D) + (j_ - 1)]) / 3.0);
        }
    }
}
""" + s[b_:]
write_dace(k, "duntiled", "nested_sdfg_0_1_0, the skewed and 64x64-tiled wavefront, replaced by the plain row-by-row\n"
           "   loop of the translated code (same operations, same association). The parallel region around it stays.", s)
print("ok round 2")

# ------------------------------------------------------------------ round 3: three second steps
# s2233: DaCe interchanges the aa nest AND fuses it with the bb nest (cc[j][i] read once for both)
k = "tsvc_2_s2233"; s = cpp_src(k)
s = once(s, """        for (int64_t i = 8; i < LEN_2D; ++i) {
          for (int64_t j = 8; j < LEN_2D; ++j) {
            aa[(j)*(LEN_2D) + (i)] = (aa[((j - 1))*(LEN_2D) + (i)] + cc[(j)*(LEN_2D) + (i)]);
          }
          for (int64_t j = 8; j < LEN_2D; ++j) {
            bb[(i)*(LEN_2D) + (j)] = (bb[((i - 1))*(LEN_2D) + (j)] + cc[(i)*(LEN_2D) + (j)]);
          }
        }""", """        for (int64_t r = 8; r < LEN_2D; ++r) {
          for (int64_t c = 8; c < LEN_2D; ++c) {
            aa[(r)*(LEN_2D) + (c)] = (aa[((r - 1))*(LEN_2D) + (c)] + cc[(r)*(LEN_2D) + (c)]);
            bb[(r)*(LEN_2D) + (c)] = (bb[((r - 1))*(LEN_2D) + (c)] + cc[(r)*(LEN_2D) + (c)]);
          }
        }""")
write_cpp(k, "cppinterfused", "the aa nest interchanged and fused with the bb nest (both now run along rows and read\n"
          "// cc[r][c] in the same iteration), as DaCe's pipeline does. Bit-identical result.", s)

# fuse_move_ifs: DaCe fuses the two passes over src into one, with the condition chain per element
k = "fuse_move_ifs"; s = cpp_src(k)
a_ = s.index("        for (int64_t i = 0; i < LEN_2D; ++i) {\n          if ((cond[i] > 0.0)) {")
b_ = s.index("}\n} // extern \"C\"")
s = s[:a_] + """        for (int64_t i = 0; i < LEN_2D; ++i) {
          for (int64_t j = 0; j < LEN_2D; ++j) {
            const double c_ = cond[i];
            if (((c_ > 0.0) && (K > 0))) {
              a[(i)*(LEN_2D) + (j)] = (src[(i)*(LEN_2D) + (j)] * 2.0);
              b[(i)*(LEN_2D) + (j)] = (src[(i)*(LEN_2D) + (j)] + 1.0);
            } else if (((! (c_ > 0.0)) && (K > 0))) {
              b[(i)*(LEN_2D) + (j)] = (src[(i)*(LEN_2D) + (j)] + 1.0);
            } else if (((c_ > 0.0) && (! (K > 0)))) {
              a[(i)*(LEN_2D) + (j)] = (src[(i)*(LEN_2D) + (j)] * 2.0);
            }
          }
        }
""" + s[b_:]
write_cpp(k, "cppfused", "the two nests fused into one pass over src, with DaCe's condition chain per element\n"
          "// (same writes, same values). No OpenMP.", s)

# s275: DaCe's band loop: py_floor bounds in the inner loop's condition, inside an OpenMP region
k = "tsvc_2_s275"; s = dace_src(k)
old = "        for (int64_t _loop_it_0 = py_floor((LEN_2D * __dace_band), __dace_num_threads); _loop_it_0 < py_floor((LEN_2D * (__dace_band + 1)), __dace_num_threads); _loop_it_0 += 1) {"
h = once(s, old, "        const int64_t lo_ = py_floor((LEN_2D * __dace_band), __dace_num_threads);  /* follow-up `dhoist` */\n"
         "        const int64_t hi_ = py_floor((LEN_2D * (__dace_band + 1)), __dace_num_threads);\n"
         "        for (int64_t _loop_it_0 = lo_; _loop_it_0 < hi_; _loop_it_0 += 1) {")
write_dace(k, "dhoist", "the band bounds py_floor(...) computed once before the inner loop instead of in its condition.", h)
lines = s.split("\n"); n = sum(1 for l in lines if l.strip().startswith("#pragma omp"))
write_dace(k, "dnoomp", f"its {n} `#pragma omp` line(s) removed (the band loop runs once, band 0 of 1).",
           "\n".join(l for l in lines if not l.strip().startswith("#pragma omp")))
print("ok round 3")
