#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#include <stdio.h>

typedef enum
{
	OPCODE,
	DATA,
	ORIGIN
} AsTypes;

typedef struct
{
	AsTypes type;
	char *data;
	uint32_t size;
} block_t;

typedef struct patch_table_t
{
	token_t id;
	uint16_t *pos;
	struct patch_table_t *next;
} patch_table_t;

typedef struct
{
	token_t id;
	uint16_t value;
} label_t;

typedef struct
{
	block_t *blocks;
	uint32_t num_blocks;

	patch_table_t *patches;
	label_t *labels;
	int label_count;

	uint32_t size;
	uint32_t cursor;
} assembler_t;

extern int AssemblerError;

assembler_t assemble_tokens(token_t *tokens, int size);
void print_assembled(assembler_t *as);
void print_lables(assembler_t *as);
void print_patches(assembler_t *as);

uint8_t *assembler_to_bin(assembler_t *a);

#endif
