all: asm emu cmpl

asm:
	make -C asm
	cp asm/asm asm_exe

emu:
	make -C emu
	cp emu/emu emu_exe

cmpl:
	make -C cmpl
	cp cmpl/cmpl cmpl_exe

.PHONY: asm emu cmpl

