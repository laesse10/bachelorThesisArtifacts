// Convert a SNAP edge list (text, '#' comments, one "u v" pair per line) into
// GraphAIBench's binary CSR: <out>.meta.txt, <out>.vertex.bin, <out>.edge.bin.
//
// The graph is symmetrized, self-loops and duplicate edges are dropped, vertex IDs are
// compacted to 0..nv-1 in increasing original-ID order, and every neighbor list is
// sorted (the TC kernels assume sorted lists). Labels are not produced.
//
// meta.txt: nv / ne / vid_size eid_size vlabel_size elabel_size / max_degree /
//           feat_len / #vclasses / #eclasses, with ne = number of symmetric entries.
//
// Build: g++ -O3 -fopenmp -std=c++17 snap2csr.cc -o snap2csr
// Use:   zcat com-orkut.ungraph.txt.gz | ./snap2csr - out/graph

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

using vid_t = uint32_t;
using eid_t = uint64_t;

static void parse(FILE *in, std::vector<uint64_t> &src, std::vector<uint64_t> &dst) {
  std::vector<char> buf(1 << 24);
  std::string carry;
  size_t n;
  auto line = [&](const char *p, const char *end) {
    while (p < end && (*p == ' ' || *p == '\t')) p++;
    if (p == end || *p == '#' || *p == '%') return;
    uint64_t v[2];
    for (int k = 0; k < 2; k++) {
      while (p < end && (*p < '0' || *p > '9')) p++;
      if (p == end) return;
      uint64_t x = 0;
      while (p < end && *p >= '0' && *p <= '9') x = x * 10 + uint64_t(*p++ - '0');
      v[k] = x;
    }
    src.push_back(v[0]);
    dst.push_back(v[1]);
  };
  while ((n = fread(buf.data(), 1, buf.size(), in)) > 0) {
    const char *p = buf.data(), *end = p + n;
    const char *nl;
    while ((nl = static_cast<const char *>(memchr(p, '\n', end - p)))) {
      if (!carry.empty()) {
        carry.append(p, nl);
        line(carry.data(), carry.data() + carry.size());
        carry.clear();
      } else {
        line(p, nl);
      }
      p = nl + 1;
    }
    carry.append(p, end);
  }
  if (!carry.empty()) line(carry.data(), carry.data() + carry.size());
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s <edgelist.txt|-> <out_prefix>\n", argv[0]);
    return 1;
  }
  FILE *in = std::string(argv[1]) == "-" ? stdin : fopen(argv[1], "r");
  if (!in) { perror(argv[1]); return 1; }
  std::vector<uint64_t> src, dst;
  parse(in, src, dst);
  if (in != stdin) fclose(in);
  fprintf(stderr, "read %zu edge lines\n", src.size());

  // compact IDs
  std::vector<uint64_t> ids(src);
  ids.insert(ids.end(), dst.begin(), dst.end());
  std::sort(ids.begin(), ids.end());
  ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
  const uint64_t nv = ids.size();
  if (nv >= UINT32_MAX) { fprintf(stderr, "too many vertices for 32-bit IDs\n"); return 1; }
  auto remap = [&](uint64_t x) { return vid_t(std::lower_bound(ids.begin(), ids.end(), x) - ids.begin()); };

  // symmetrize into CSR by counting sort, dropping self-loops
  std::vector<eid_t> rowptr(nv + 1, 0);
  std::vector<vid_t> s(src.size()), d(src.size());
#pragma omp parallel for
  for (size_t i = 0; i < src.size(); i++) { s[i] = remap(src[i]); d[i] = remap(dst[i]); }
  std::vector<uint64_t>().swap(src);
  std::vector<uint64_t>().swap(dst);
  for (size_t i = 0; i < s.size(); i++)
    if (s[i] != d[i]) { rowptr[s[i] + 1]++; rowptr[d[i] + 1]++; }
  for (uint64_t v = 0; v < nv; v++) rowptr[v + 1] += rowptr[v];
  std::vector<vid_t> col(rowptr[nv]);
  std::vector<eid_t> fill(rowptr.begin(), rowptr.end() - 1);
  for (size_t i = 0; i < s.size(); i++)
    if (s[i] != d[i]) { col[fill[s[i]]++] = d[i]; col[fill[d[i]]++] = s[i]; }

  // sort and de-duplicate each row, then compact
  std::vector<eid_t> deg(nv);
#pragma omp parallel for schedule(dynamic, 1024)
  for (uint64_t v = 0; v < nv; v++) {
    auto b = col.begin() + rowptr[v], e = col.begin() + rowptr[v + 1];
    std::sort(b, e);
    deg[v] = std::unique(b, e) - b;
  }
  std::vector<eid_t> out_ptr(nv + 1, 0);
  for (uint64_t v = 0; v < nv; v++) out_ptr[v + 1] = out_ptr[v] + deg[v];
  for (uint64_t v = 0; v < nv; v++)
    std::copy(col.begin() + rowptr[v], col.begin() + rowptr[v] + deg[v], col.begin() + out_ptr[v]);
  const eid_t ne = out_ptr[nv];
  col.resize(ne);
  const eid_t max_degree = *std::max_element(deg.begin(), deg.end());

  std::string pre = argv[2];
  std::ofstream(pre + ".vertex.bin", std::ios::binary)
      .write(reinterpret_cast<const char *>(out_ptr.data()), out_ptr.size() * sizeof(eid_t));
  std::ofstream(pre + ".edge.bin", std::ios::binary)
      .write(reinterpret_cast<const char *>(col.data()), col.size() * sizeof(vid_t));
  std::ofstream(pre + ".meta.txt") << nv << "\n" << ne << "\n"
                                   << sizeof(vid_t) << " " << sizeof(eid_t) << " 1 2\n"
                                   << max_degree << "\n0\n0\n0\n";
  fprintf(stderr, "nv=%llu ne=%llu (symmetric entries) max_degree=%llu\n",
          (unsigned long long)nv, (unsigned long long)ne, (unsigned long long)max_degree);
  return 0;
}
