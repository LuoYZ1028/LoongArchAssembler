## LoongArchAssembler v2.6
+ Add User's help document, it's a hyperlink to one of my personal blog site.

## LoongArchAssembler v2.5
+ Support Macro function now! Although the error detection function of it is still weak...
+ Add Highlight of EQU and any word that might be a EQU name(like ABC, _A2, ...)

## LoongArchAssembler v2.3
+ Support multiple line of annotation like (/* ... */) now, but notice that if used outside data section of text section, it must be isolated(no meaningful content should be write in the same line of the symbol of the mark)
+ Simplified some functions, processing speed becomes faster now

## LoongArchAssembler v2.2
+ changed lots of basic structures, the Software is more robust now

## LoongArchAssembler v2.1
+ Support Pseudo instruction like EQU in data section
+ Support Pseudo instruction like li.w(load immediate word) and la(load address)
+ Add constrain that $r0 or $zero can't be chosen as rd in some instructions

# Basic Information
A Assembler to translate ASM code to Machine code, based on ARCH of LoongSon(LoongArch), using Qt to construct UI.
