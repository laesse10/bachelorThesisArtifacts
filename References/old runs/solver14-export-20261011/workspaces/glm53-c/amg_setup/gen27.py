import numpy as np

def make_stencil_3d(NX, NY, NZ, seed=0, rescale=True):
    """27-point variable-coefficient operator, CSR, columns ascending within a row."""
    rng = np.random.default_rng(seed)
    n = NX*NY*NZ
    # neighbor offsets in column-major (x fastest) numbering: idx = ix + NX*(iy + NY*iz)
    offs = []
    for dz in (-1,0,1):
        for dy in (-1,0,1):
            for dx in (-1,0,1):
                offs.append((dx,dy,dz))
    indptr = np.zeros(n+1, dtype=np.int64)
    # count entries per row
    cnt = np.zeros(n, dtype=np.int64)
    for iz in range(NZ):
        for iy in range(NY):
            for ix in range(NX):
                c = 0
                for (dx,dy,dz) in offs:
                    jx, jy, jz = ix+dx, iy+dy, iz+dz
                    if 0 <= jx < NX and 0 <= jy < NY and 0 <= jz < NZ:
                        c += 1
                cnt[ix + NX*(iy + NY*iz)] = c
    indptr[1:] = np.cumsum(cnt)
    nnz = int(indptr[n])
    indices = np.zeros(nnz, dtype=np.int64)
    data = np.zeros(nnz, dtype=np.float64)
    pos = 0
    # distance-based base weights
    def wbase(dx,dy,dz):
        if dx==0 and dy==0 and dz==0: return 0.0
        m = abs(dx)+abs(dy)+abs(dz)
        return 1.0/(1.0*m) if m>0 else 0.0
    for iz in range(NZ):
        for iy in range(NY):
            for ix in range(NX):
                i = ix + NX*(iy + NY*iz)
                # diagonal = sum |offdiag| * (1 + jitter) + shift
                offd = []
                for (dx,dy,dz) in offs:
                    jx, jy, jz = ix+dx, iy+dy, iz+dz
                    if 0 <= jx < NX and 0 <= jy < NY and 0 <= jz < NZ:
                        if dx==0 and dy==0 and dz==0: continue
                        j = jx + NX*(jy + NY*jz)
                        v = wbase(dx,dy,dz) * rng.uniform(0.6, 1.4)
                        offd.append((j, -v))
                s = sum(-v for _, v in offd)
                diag = s * rng.uniform(1.02, 1.25) + rng.uniform(0.01, 0.1)
                # emit ascending columns
                for (dx,dy,dz) in offs:
                    jx, jy, jz = ix+dx, iy+dy, iz+dz
                    if not (0 <= jx < NX and 0 <= jy < NY and 0 <= jz < NZ): continue
                    j = jx + NX*(jy + NY*jz)
                    if j == i:
                        indices[pos] = i; data[pos] = diag
                    else:
                        # find value we assigned
                        for (jj, v) in offd:
                            if jj == j:
                                indices[pos] = j; data[pos] = v; break
                    pos += 1
    assert pos == nnz
    if rescale:
        A = rescale_diag(indptr, indices, data, rng)
        return A
    return (data, indices, indptr)

def rescale_diag(indptr, indices, data, rng):
    """Symmetric diagonal rescale D A D (like revalue)."""
    n = len(indptr)-1
    d = np.exp(rng.uniform(-1.0, 1.0, size=n))
    out = data.copy()
    for i in range(n):
        for k in range(indptr[i], indptr[i+1]):
            out[k] = data[k]*d[i]*d[indices[k]]
    return (out, indices, indptr)

if __name__ == "__main__":
    print("ok")
