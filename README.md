# LoongArchAssembler v2.1
+ Support Pseudo instruction like EQU in data section
+ Support Pseudo instruction like li.w(load immediate word) and la(load address)
+ Add constrain that $r0 or $zero can't be chosen as rd in some instructions

A Assembler to translate ASM code to Machine code, based on ARCH of LoongSon(LoongArch), using Qt to construct UI.
