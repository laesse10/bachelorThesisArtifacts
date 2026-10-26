"""Instruction sequence of one function from objdump text: mnemonic and operands, without addresses,
encodings, branch targets or comments, so that two builds can be compared line by line."""
import re, sys
out = []
for line in open(sys.argv[1]):
    m = re.match(r"\s*[0-9a-f]+:\s+(?:[0-9a-f]{8}\s+)?(\S+)\s*(.*)", line)
    if not m:
        continue
    op, args = m.group(1), m.group(2)
    args = re.sub(r"//.*", "", args)
    args = re.sub(r"<[^>]*>", "", args).strip()
    if op == "b" or op.startswith("b.") or op in ("cbz", "cbnz", "tbz", "tbnz", "bl"):
        args = re.sub(r"(^|,\s*)(0x)?[0-9a-f]+$", "", args)
    out.append(f"{op} {args}".strip())
print("\n".join(out))
