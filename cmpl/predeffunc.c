#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "lexer.h"
#include "parser.h"
#include "executor.h"
#include "predeffunc.h"


void *FUNC_print(executor_t *exe, expr_list_t *v)
{
	if(!v || v->next != NULL)
		ERROR("Print only accepts 1 value.");

	expr_t *e = v->value;
	printf("%d", execute_expr(exe, e));
	return NULL;
}

void *FUNC_newline(executor_t *exe, expr_list_t *v)
{
	(void)exe;
	if(v) ERROR("Newline only accepts 1 value.");

	puts("");
	return NULL;
}

void *FUNC_getc(executor_t *exe, expr_list_t *v)
{
	(void)exe;
	if(v) ERROR("Getc only accepts 1 value.");

	static char val;
	val = getc(stdin);
	return &val;
}

void *FUNC_gets(executor_t *exe, expr_list_t *v)
{
	if(!v || v->next != NULL)
		ERROR("Gets only accepts 1 value.");

	expr_t *e = v->value;
	if(e->type != EXPR_IDENT)
		ERROR("Expression is not an identifier.");

	symbol_t *sym = table_get(&(exe->symbols), e->ident);
	if(sym->isconst)
		ERROR("Unable to edit constant symbol.");

	if(!sym->isarray)
		ERROR("Symbol is not of array type.");

	fgets(sym->data, sym->size, stdin);
	return NULL;
}

// For now this is what we have to do 'tis fine

static predef_func_t FUNC_int[] = {
	{"print", 5, FUNC_print},
	{"newline", 7, FUNC_newline},
	{"getc", 4, FUNC_getc},
	{"gets", 4, FUNC_gets},
};

int FUNC_COUNT = sizeof FUNC_int / sizeof(predef_func_t);

// Workaround
predef_func_t  *FUNC = FUNC_int;
