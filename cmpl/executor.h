#ifndef _EXECUTOR_H_
#define _EXECUTOR_H_

#include "lexer.h"
#include "parser.h"

typedef struct
{
	uint32_t uid;
	char isconst;
	char isarray;
	char *data;
	int size;
} symbol_t;

typedef struct
{
	symbol_t *symbols;
	int size;
} table_t;

symbol_t *table_get(table_t *t, uint32_t uid);
void table_set(table_t *t, uint32_t uid, symbol_t sym);
void table_print(table_t *t);

typedef struct
{
	program_t *program;
	table_t symbols;
} executor_t;

uint8_t execute_expr(executor_t *exe, expr_t *e);
void execute_block(executor_t *e, block_t *blk);

void execute_program(executor_t *e);

#endif
