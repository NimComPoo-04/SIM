#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cpu.h"

void cpu_reset(cpu_t *cpu)
{
	memset(cpu, 0, sizeof(*cpu));
}

void cpu_fetch(cpu_t *cpu, mem_t *mem)
{
	cpu->MAR = cpu->PC++;
	cpu->MBR = mem_get(mem, cpu->MAR);

	cpu->IR = cpu->MBR;

	int opcode = cpu->IR & 7;

	int mode = (cpu->IR >> 3) & 3;

	// TODO: possible optimization, in the case when the condition flag fail we can completely
	// skip loading the MAR, ARG and MBR registers instead we could just increment PC and move
	// on.
	switch(opcode)
	{
		case CLR:
			break;

		case MOV:
			switch(mode)
			{
				case 0:
					break;

				case 1:
					cpu->MAR = cpu->PC++;
					cpu->MBR = mem_get(mem, cpu->MAR);
					break;

				case 2:
					cpu->MAR = cpu->PC++;
					cpu->MBR = mem_get(mem, cpu->MAR);
					cpu->ARG = cpu->MBR;

					cpu->MAR = cpu->PC++;
					cpu->MBR = mem_get(mem, cpu->MAR);
					cpu->ARG = (cpu->MBR << 8) | cpu->ARG;

					cpu->MAR = cpu->ARG;

					break;

				case 3:
					cpu->MAR = cpu->PC++;
					cpu->MBR = mem_get(mem, cpu->MAR);
					cpu->ARG = cpu->MBR;

					cpu->MAR = cpu->PC++;
					cpu->MBR = mem_get(mem, cpu->MAR);
					cpu->ARG = (cpu->MBR << 8) | cpu->ARG;

					cpu->MAR = cpu->PC++;
					cpu->MBR = mem_get(mem, cpu->MAR);

					cpu->MAR = cpu->ARG;

					break;
			}
			break;

		default:
			switch(mode)
			{
				case IMM8:
					cpu->MAR = cpu->PC++;
					cpu->MBR = mem_get(mem, cpu->MAR);
					cpu->ARG = cpu->MBR;
					break;

				case IMM16:
					cpu->MAR = cpu->PC++;
					cpu->MBR = mem_get(mem, cpu->MAR);
					cpu->ARG = cpu->MBR;

					cpu->MAR = cpu->PC++;
					cpu->MBR = mem_get(mem, cpu->MAR);
					cpu->ARG = (cpu->MBR << 8) | cpu->ARG;
					break;

				case REG:
					break;

				case MEM_REG:
					break;
			}
			break;
	}
}

void cpu_CLR(cpu_t *cpu)
{
	int reg = (cpu->IR >> 3) & 15;

	switch(reg)
	{
		case 0: cpu->F.ZERO = 0; break;
		case 1: cpu->F.CARRY = 0; break;

		case 2: cpu->A = 0; break;
		case 3: cpu->T = 0; break;

		case 4: cpu->A &= 0xFF00; break;
		case 5: cpu->A &= 0x00FF; break;

		case 6: cpu->T &= 0xFF00; break;
		case 7: cpu->T &= 0x00FF; break;

		case 8: cpu->ARG = 0; break;
		case 9: cpu->PC = 0; break;
		case 10: cpu->IR = 0; break;

		case 11: cpu->MAR = 0; break;
		case 12: cpu->MBR = 0; break;

		case 14:
			 cpu->F.CARRY = !(cpu->F.ZERO);
			 cpu->F.ZERO = 0;
			 break;

		case 15:
			 cpu->F.ZERO = !(cpu->F.CARRY);
			 cpu->F.CARRY = 0;
			 break;
	}
}

void cpu_ADD(cpu_t *cpu, mem_t *mem)
{
	int mode = (cpu->IR >> 3) & 3;
	int reg_1 = (cpu->IR >> 5) & 1;
	int reg_2 = (cpu->IR >> 6) & 1;

	uint16_t *dest = reg_1 ? &(cpu->T) : &(cpu->A);
	uint16_t *src  = reg_2 ? &(cpu->T) : &(cpu->A);

	uint32_t k = *dest;

	switch(mode)
	{
		case IMM8:
		case IMM16:
			k += cpu->ARG + cpu->F.CARRY;
			break;

		case REG:
			k += *src + cpu->F.CARRY;
			break;

		case MEM_REG:
			cpu->MAR = *src;
			cpu->MBR = mem_get(mem, cpu->MAR);
			k += cpu->MBR + cpu->F.CARRY;
			break;
	}

	if(k >> 16)
		cpu->F.CARRY = 1;
	else
		cpu->F.CARRY = 0;

	if(k == 0)
		cpu->F.ZERO = 1;
	else
		cpu->F.ZERO = 0;

	*dest = (uint16_t)k;
}

void cpu_SUB(cpu_t *cpu, mem_t *mem)
{
	int mode = (cpu->IR >> 3) & 3;
	int reg_1 = (cpu->IR >> 5) & 1;
	int reg_2 = (cpu->IR >> 6) & 1;

	uint16_t *dest = reg_1 ? &(cpu->T) : &(cpu->A);
	uint16_t *src  = reg_2 ? &(cpu->T) : &(cpu->A);

	uint32_t k = *dest;

	switch(mode)
	{
		case IMM8:
		case IMM16:
			k = k - cpu->ARG - cpu->F.CARRY;
			break;

		case REG:
			k = k - *src - cpu->F.CARRY;
			break;

		case MEM_REG:
			cpu->MAR = *src;
			cpu->MBR = mem_get(mem, cpu->MAR);
			k = k - cpu->MBR - cpu->F.CARRY;
			break;
	}

	if(k >> 16)
		cpu->F.CARRY = 1;
	else
		cpu->F.CARRY = 0;

	if(k == 0)
		cpu->F.ZERO = 1;
	else
		cpu->F.ZERO = 0;

	*dest = (uint16_t)k;
}

void cpu_NAND(cpu_t *cpu, mem_t *mem)
{
	int mode = (cpu->IR >> 3) & 3;
	int reg_1 = (cpu->IR >> 5) & 1;
	int reg_2 = (cpu->IR >> 6) & 1;

	uint16_t *dest = reg_1 ? &(cpu->T) : &(cpu->A);
	uint16_t *src  = reg_2 ? &(cpu->T) : &(cpu->A);

	uint32_t k = *dest;

	switch(mode)
	{
		case IMM8:
		case IMM16:
			k = ~(k & cpu->ARG);
			break;

		case REG:
			k = ~(k & *src);
			break;

		case MEM_REG:
			cpu->MAR = *src;
			cpu->MBR = mem_get(mem, cpu->MAR);
			k = ~(k & cpu->MBR);
			break;
	}

	*dest = (uint16_t)k;
}

void cpu_RTL(cpu_t *cpu, mem_t *mem)
{
	int mode = (cpu->IR >> 3) & 3;
	int reg_1 = (cpu->IR >> 5) & 1;
	int reg_2 = (cpu->IR >> 6) & 1;

	uint16_t *dest = reg_1 ? &(cpu->T) : &(cpu->A);
	uint16_t *src  = reg_2 ? &(cpu->T) : &(cpu->A);

	uint32_t k = *dest;

	switch(mode)
	{
		case IMM8:
			k <<= cpu->ARG & 0xF;
			break;

		case REG:
			k <<= *src & 0xF;
			break;

		case MEM_REG:
			cpu->MAR = *src;
			cpu->MBR = mem_get(mem, cpu->MAR);
			k <<= cpu->MBR;
			break;
	}

	k = (k & 0xFFFF) | ((k & 0xFFFF0000) >> 16);
	

	*dest = (uint16_t)k;
}

void cpu_RTR(cpu_t *cpu, mem_t *mem)
{
	int mode = (cpu->IR >> 3) & 3;
	int reg_1 = (cpu->IR >> 5) & 1;
	int reg_2 = (cpu->IR >> 6) & 1;

	uint16_t *dest = reg_1 ? &(cpu->T) : &(cpu->A);
	uint16_t *src  = reg_2 ? &(cpu->T) : &(cpu->A);

	uint32_t k = (uint32_t)*dest << 16;

	switch(mode)
	{
		case IMM8:
			k >>= cpu->ARG & 0xF;
			break;

		case REG:
			k >>= *src & 0xF;
			break;

		case MEM_REG:
			cpu->MAR = *src;
			cpu->MBR = mem_get(mem, cpu->MAR);
			k >>= cpu->MBR;
			break;
	}

	k = (((k & 0xFFFF) << 16) | (k & 0xFFFF0000)) >> 16;

	*dest = (uint16_t)k;
}

void cpu_JMP(cpu_t *cpu)
{
	int mode = (cpu->IR >> 3) & 3;
	int reg = (cpu->IR >> 5) & 1;

	uint16_t *dest = reg ? &(cpu->T) : &(cpu->A);

	switch(mode)
	{
		case IMM16:
			cpu->PC = cpu->ARG;
			break;

		case REG:
			cpu->PC = *dest;
			break;
	}
}

void cpu_MOV(cpu_t *cpu, mem_t *mem)
{
	int mode = (cpu->IR >> 3) & 3;
	int reg_1 = (cpu->IR >> 5) & 1;
	int reg_2 = (cpu->IR >> 6) & 1;

	uint16_t *dest = reg_1 ? &(cpu->T) : &(cpu->A);
	uint16_t *src  = reg_2 ? &(cpu->T) : &(cpu->A);

	switch(mode)
	{
		case 0:
			cpu->MAR = *dest;
			cpu->MBR = (uint8_t)*src;
			mem_set(mem, cpu->MAR, cpu->MBR);
			break;

		case 1:
			cpu->MAR = *dest;
			mem_set(mem, cpu->MAR, cpu->MBR);
			break;

		case 2:
			cpu->MBR = (uint8_t)*dest;
			mem_set(mem, cpu->MAR, cpu->MBR);
			break;

		case 3:
			mem_set(mem, cpu->MAR, cpu->MBR);
			break;
	}
}

void cpu_execute(cpu_t *cpu, mem_t *mem)
{
	int cond = cpu->IR >> 7;

	// don't do anything if cond check is on and the flag is zero
	if(cond && cpu->F.ZERO)
		return;

	int opcode = cpu->IR & 7;

	switch(opcode)
	{
		case CLR: cpu_CLR(cpu); break;
		case ADD: cpu_ADD(cpu, mem); break;
		case SUB: cpu_SUB(cpu, mem); break;
		case NAND: cpu_NAND(cpu, mem); break;
		case RTR: cpu_RTR(cpu, mem); break;
		case RTL: cpu_RTL(cpu, mem); break;
		case JMP: cpu_JMP(cpu); break;
		case MOV: cpu_MOV(cpu, mem); break;
	}
}

void cpu_dump(cpu_t *cpu)
{
	printf("Register State: \n");

	printf("A : %04x\n", cpu->A);
	printf("T : %04x\n", cpu->T);

	printf("PC : %04x\n", cpu->PC);
	printf("IR : %02x\n", cpu->IR);
	printf("ARG : %04x\n", cpu->ARG);

	printf("CARRY %d\n", cpu->F.CARRY);
	printf("ZERO  %d\n", cpu->F.CARRY);

	printf("MAR : %04x\n", cpu->MAR);
	printf("MBR : %02x\n", cpu->MBR);
}

void cpu_step(cpu_t *cpu, mem_t *mem)
{
	cpu_fetch(cpu, mem);
	cpu_execute(cpu, mem);
}
