/* aarch64 stand-in for libs/MNRL/C++/src/mnrl_schema.asm (nasm, x86-only):
   the same three symbols around the embedded mnrl-schema.json. */
    .section .rodata
    .global _binary_mnrl_schema_json_start
    .global _binary_mnrl_schema_json_end
    .global _binary_mnrl_schema_json_size
_binary_mnrl_schema_json_start:
    .incbin "/capstor/scratch/cscs/lhulsbergen/vasim/libs/MNRL/mnrl-schema.json"
_binary_mnrl_schema_json_end:
    .balign 4
_binary_mnrl_schema_json_size:
    .word _binary_mnrl_schema_json_end - _binary_mnrl_schema_json_start
    .section .note.GNU-stack,"",%progbits
