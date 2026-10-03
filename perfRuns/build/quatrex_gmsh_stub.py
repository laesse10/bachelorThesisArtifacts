"""Loud stub for `gmsh`, which has no aarch64 wheel.

QuaTrEx imports gmsh unconditionally via
quatrex.electrostatics.meshing, but the NEGF/GW path only needs the pure
numpy geometry helpers from that module (`inside_shape`), never the mesh
generation that actually drives gmsh.

Any real attribute access raises, so a code path that genuinely needs
gmsh fails loudly instead of silently producing wrong results.
"""


class _GmshStubError(RuntimeError):
    pass


def __getattr__(name: str):
    raise _GmshStubError(
        f"gmsh.{name} was accessed, but gmsh is stubbed out in this "
        "environment (no aarch64 wheel; QuaTrEx's own Dockerfile builds it "
        "from source). This code path genuinely needs gmsh -- build it or "
        "avoid the meshing/electrostatics path."
    )
