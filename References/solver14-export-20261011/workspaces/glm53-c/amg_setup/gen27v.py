import numpy as np

def make_stencil_3d_vec(NX, NY, NZ, seed=0):
    """Vectorized 27-point stencil with jittered values, CSR, columns ascending."""
    rng = np.random.default_rng(seed)
    n = NX*NY*NZ
    idx = np.arange(n).reshape(NZ, NY, NX)  # i = ix + NX*(iy + NY*iz)
    # neighbor offsets sorted so that column index ascending: i = ix + NX*iy + NX*NY*iz
    # column order: sort offsets by (dz, dy, dx) with dz major? column = jx + NX*(jy + NY*jz)
    # ascending column order == sort by (jz, jy, jx) lexicographic
    offs = []
    for dz in (-1,0,1):
        for dy in (-1,0,1):
            for dx in (-1,0,1):
                offs.append((dx,dy,dz))
    # counts
    cnt = np.zeros((NZ,NY,NX), dtype=np.int64)
    masks = []
    for (dx,dy,dz) in offs:
        m = np.zeros((NZ,NY,NX), dtype=bool)
        x0,x1 = max(0,-dx), min(NX, NX-dx)
        y0,y1 = max(0,-dy), min(NY, NY-dy)
        z0,z1 = max(0,-dz), min(NZ, NZ-dz)
        m[z0:z1, y0:y1, x0:x1] = True
        masks.append(m)
        cnt += m
    indptr = np.zeros(n+1, dtype=np.int64)
    indptr[1:] = np.cumsum(cnt.ravel())
    nnz = int(indptr[-1])
    indices = np.zeros(nnz, dtype=np.int64)
    data = np.zeros(nnz, dtype=np.float64)
    # values: distance-based magnitude with jitter, diagonal = -sum(offdiag)*(1+j)+shift
    # first build full dense-ish per-offset value arrays
    vals = {}
    offsum = np.zeros((NZ,NY,NX))
    for (dx,dy,dz) in offs:
        if dx==0 and dy==0 and dz==0: continue
        m = masks[offs.index((dx,dy,dz))]
        w = 1.0/(abs(dx)+abs(dy)+abs(dz))
        v = np.full((NZ,NY,NX), -w) * rng.uniform(0.6, 1.4, size=(NZ,NY,NX))
        v[~m] = 0.0
        vals[(dx,dy,dz)] = v
        offsum += -v  # v negative, offdiag contribution positive magnitude
    diagv = offsum * rng.uniform(1.02,1.25,size=(NZ,NY,NX)) + rng.uniform(0.01,0.1,size=(NZ,NY,NX))
    # emit in ascending column order per row
    # for row (iz,iy,ix), neighbors sorted by (jz,jy,jx)
    pos = 0
    # precompute per-row neighbor lists as flat arrays: build once
    # build index array: for each offset in sorted order, the (row, col) pairs
    pairs = []
    for dz in (-1,0,1):
        for dy in (-1,0,1):
            for dx in (-1,0,1):
                pairs.append((dx,dy,dz))
    # for each row we need its entries sorted by column; gather via per-offset slices:
    # iterate offsets in ascending column order determined per row... column of offset (dx,dy,dz) relative
    # order by (dz,dy,dx) lexicographic where key = dz*NY*NX + dy*NX + dx sign corrected:
    # column = jz*NX*NY + jy*NX + jx; ascending column order = sort offsets by (dz, dy, dx) with
    # key(dz,dy,dx) = dz*(NX*NY) + dy*NX + dx. dz,dy,dx in {-1,0,1}: key order matches lexicographic (dz,dy,dx).
    offs_sorted = sorted(pairs, key=lambda o: o[2]*(NX*NY) + o[1]*NX + o[0])
    for row in range(n):
        pass
    # vectorized emit: for each offset in offs_sorted, the set of rows where valid, appended in order.
    # rows valid for offset o: rows where mask true. For each row, entries appear in offs_sorted order == ascending col.
    # but we must write them into the correct positions in indices/data: position = indptr[row] + (count of earlier valid offsets for this row)
    # do sequential scan over offsets, maintaining cur[row] pointer
    curptr = indptr[:-1].copy()
    for (dx,dy,dz) in offs_sorted:
        m = masks[offs.index((dx,dy,dz))]
        rows = np.nonzero(m.ravel())[0]
        cols = rows + dx + NX*dy + NX*NY*dz
        if dx==0 and dy==0 and dz==0:
            v = diagv.ravel()[rows]
        else:
            v = vals[(dx,dy,dz)].ravel()[rows]
        indices[curptr[rows]] = cols
        data[curptr[rows]] = v
        curptr[rows] += 1
    assert np.all(curptr == indptr[1:])
    return data, indices, indptr

def rescale_diag_vec(indptr, indices, data, seed=0):
    rng = np.random.default_rng(seed+999)
    n = len(indptr)-1
    d = np.exp(rng.uniform(-1.0,1.0,size=n))
    rows = np.repeat(np.arange(n), np.diff(indptr))
    out = data * d[rows] * d[indices]
    return out, indices, indptr

if __name__ == "__main__":
    import sys
    d,i,p = make_stencil_3d_vec(8,8,8,seed=1)
    print(len(d), p[-1], i[:5], d[:3])
