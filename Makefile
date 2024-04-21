all: asm emu

asm:
	make -C asm
	cp asm/asm asm_exe

emu:
	make -C emu
	cp emu/emu emu_exe

.PHONY: asm emu

