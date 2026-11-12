import sys
sys.path.insert(0,'work')
from harness import build, make_loader, correctness
so = sys.argv[1] if len(sys.argv)>1 else "rk4_ensemble.c"
lib = so.replace(".c",".so")
build(so, lib)
f = make_loader(lib)
print(f"Loaded {lib}")
allok=True
for (a,b,ep,t_end) in [(1.0,3.0,0.01,5.0),(1.0,3.0,1.0,5.0),(2.0,4.0,0.5,10.0)]:
    ok = correctness(f, a, b, ep, t_end)
    print(f"params a={a} b={b} ep={ep} t_end={t_end}: {'OK' if ok else 'FAIL'}")
    allok = allok and ok
print("ALL OK" if allok else "SOME FAILED")
