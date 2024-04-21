# SIMP

A very simple computer built for fun :D

Little endian computer. so heigher order bits are on the higher memory addresses

Devices:

16-bit internal bus.

ALU operations:
addition, substraction, shift left, shift right, arithmatic shift right.

Memory map:
flat memory model, with the first half of memory as ROM, the next half being RAM

the last 8 addresses are basically ports, reading and writing from them
is done for device communication.

Registers:

MAR -- 16 bits
MBR -- 8 bits

ARG -- 16 bits  the argument for the IR

A -- Accumulator        16 bits
AL -- lower 8 bits
AH -- higher 8 bits

T -- GP register 16 bits
TL -- lower 8 bits
TH -- higher 8 bits

PC -- program counter
IR -- instruction register 8 bits long

FLAG -- 2 bits [ CARRY | ZERO ]

Opcode Layout:

[ opcode (3bits) | address mode (2 bits) | register (2 bits) | condition (1 bit)]
[ opcode == CLR  | register (4 bits) | condition (1 bit) ]

instruction lowerbits higherbits

Instructions:

0 : CLR Reg                             # clears any register
1 : ADD^ Reg , imm8/imm16/[Reg]/Reg       # add
2 : SUB^ Reg , imm8/imm16/[Reg]/Reg       # sub 
3 : NAND Reg , imm8/imm16/[Reg]/Reg       # bitwise nand

4 : RTR Reg , imm8/[Reg]/Reg     # rotate left 
5 : RTL Reg , imm8/[Reg]/Reg     # rotate right
6 : JMP imm16/Reg                  # jump to a specific location or indirect it idk

7 : MOV [imm16]/[Reg], imm8/Reg         # moves immidiate value or register to memory

^ instructions set the flag register

Example.
Set the value of register to a number.

Load Instruction
SUB Reg, Reg    --> Reg = 0
ADD Reg, imm8

Shift instruction

CLR AH
ROTR A, 6
CLR AH

CLR AH
ROTL A, 6
CLR AH
