import numpy as np

def make_stencil_3d(NX, NY, NZ, seed=12345):
    """Build a 27-point variable-coefficient 3D operator (CSR, int64 indices, ascending within row).
    Off-diagonals negative, diagonal = sum of |off-diagonals| + small mass. Coefficients 'jump'
    between regions to mimic the jumping-coefficient operator."""
    rng = np.random.default_rng(seed)
    n = NX*NY*NZ
    # per-node coefficient (jumping: a few distinct values per random region)
    region = np.random.default_rng(seed+1).integers(0, 6, size=n)
    level = np.array([1.0, 2.5, 4.0, 10.0, 16.0, 30.0])[region]  # 6 distinct levels -> jumps
    def flat(x,y,z): return (x*NY + y)*NZ + z
    # neighbor offsets (dx,dy,dz), 27-point
    offs = [(dx,dy,dz) for dx in (-1,0,1) for dy in (-1,0,1) for dz in (-1,0,1) if not (dx==0 and dy==0 and dz==0)]
    entries = []
    for x in range(NX):
        for y in range(NY):
            for z in range(NZ):
                i = flat(x,y,z)
                ii = level[i]
                row = [i]
                vals = []
                diag = 0.0
                for (dx,dy,dz) in offs:
                    nx,ny,nz = x+dx,y+dy,z+dz
                    if nx<0 or ny<0 or nz<0 or nx>=NX or ny>=NY or nz>=NZ: continue
                    j = flat(nx,ny,nz)
                    jj = level[j]
                    # geometric-mean interface coefficient (jumping)
                    k = np.sqrt(ii*jj)
                    # weight scaled by the stencil "distance" so that corner/edge weights differ
                    w = k
                    row.append(j); vals.append(-w); diag += w
                # diagonal = sum of |off-diag| + mass
                mass = 0.1*ii
                row_vals = list(vals)
                # place diagonal
                idx = row.index(i)
                row_vals.insert(idx, diag + mass)
                entries.append((i, row, row_vals))
    # sort each row by column
    indptr = [0]
    indices = []
    data = []
    for (i, row, row_vals) in entries:
        pairs = sorted(zip(row, row_vals))
        for (j,v) in pairs:
            indices.append(j); data.append(v)
        indptr.append(len(indices))
    assert indptr[-1] == (3*NX-2)*(3*NY-2)*(3*NZ-2), (indptr[-1], (3*NX-2)*(3*NY-2)*(3*NZ-2))
    return (np.array(data, dtype=np.float64),
            np.array(indices, dtype=np.int64),
            np.array(indptr, dtype=np.int64))
