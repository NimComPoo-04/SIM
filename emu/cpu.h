#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>

#include "mem.h"

typedef struct cpu_t
{
	uint16_t MAR;
	uint8_t MBR;

	uint16_t A;
	uint16_t T;

	uint16_t PC;
	uint8_t IR;
	uint16_t ARG;

	struct
	{
		uint8_t CARRY: 1;
		uint8_t ZERO : 1;
	} F;
} cpu_t;

void cpu_reset(cpu_t *cpu);
void cpu_fetch(cpu_t *cpu, mem_t *mem);
void cpu_execute(cpu_t *cpu, mem_t *mem);

void cpu_step(cpu_t *cpu, mem_t *mem);


// Instructions
void cpu_CLR(cpu_t *cpu);
void cpu_ADD(cpu_t *cpu, mem_t *mem);
void cpu_SUB(cpu_t *cpu, mem_t *mem);
void cpu_NAND(cpu_t *cpu, mem_t *mem);
void cpu_RTL(cpu_t *cpu, mem_t *mem);
void cpu_RTR(cpu_t *cpu, mem_t *mem);
void cpu_JMP(cpu_t *cpu);
void cpu_MOV(cpu_t *cpu, mem_t *mem);

void cpu_dump(cpu_t *cpu);

enum Opcode_Type
{
	CLR  = 0, 
	ADD  = 1,
	SUB  = 2,
	NAND = 3,
	RTR  = 4,
	RTL  = 5,
	JMP  = 6,
	MOV  = 7
};

// These become a little different fro MOV and CLR
// instructions
enum Addressing_Mode
{
	IMM8     = 0,
	IMM16    = 1,
	REG      = 2,
	MEM_REG  = 3,
};

enum Register
{
	REG_A = 0,
	REG_T
};
#endif
