#include "log.h"
#include "executor.h"
#include "predeffunc.h"

#define BLOCK_SIZE 32

uint32_t token_hash(token_t tok);
uint8_t execute_expr(executor_t *exe, expr_t *e)
{
	if(e == NULL)
		return 0;

	switch(e->type)
	{
		case EXPR_NUM:
			return e->num;

		case EXPR_OP:
			{
				uint8_t t0 = execute_expr(exe, e->op.lhs);
				uint8_t t1 = execute_expr(exe, e->op.rhs);

				switch(e->toktype)
				{
					case LEX_PLUS:
						return t0 + t1;

					case LEX_MINUS:
						return t0 - t1;

					case LEX_LROT:
						return (t0 << t1) | (t0 >> (8 - t1));

					case LEX_RROT:
						return (t0 >> t1) | (t0 << (8 - t1));

					case LEX_NAND:
						return ~(t0 & t1);

					case LEX_LT:
						return t0 < t1;

					case LEX_GT:
						return t0 > t1;

					case LEX_EQ:
						return t0 == t1;

					case LEX_ASSIGN:
						symbol_t *s = NULL;
						int indx = 0;
						switch(e->op.lhs->type)
						{
							case EXPR_IDENT:
								s = table_get(&(exe->symbols), e->op.lhs->ident);
								if(s == NULL)
									ERROR("Variable is not yet defined. %d", e->pos);
								if(s->isconst == 1)
									ERROR("Editing a constant variable. %d", e->pos);
								if(s->isarray)
									ERROR("Variable is not of scaler type. %d", e->pos);
								return s->data[0] = t1;

							case EXPR_ARRVAL:
								s = table_get(&(exe->symbols), e->op.lhs->arrval.ident);
								indx = execute_expr(exe, e->arrval.value);

								if(s == NULL)
									ERROR("Variable is not yet defined. %d", e->pos);
								if(s->isconst == 1)
									ERROR("Editing a constant variable. %d", e->pos);
								if(s->isarray)
									ERROR("Variable is not of array type. %d", e->pos);
								if(indx >= s->size || indx < 0)
									ERROR("Array Index out of Bounds. %d", e->pos);

								return s->data[indx] = t1;

							default:
								ERROR("Unassignable Value of expression %d", e->pos);
								return 0;
						}
						break;

					default:
						return 0;
				}
			}

		case EXPR_IDENT:
			{
				symbol_t *s = table_get(&(exe->symbols), e->ident);
				if(s == NULL)
					ERROR("Variable is not yet defined %d.", e->pos);
				if(s->size > 1)
					ERROR("Variable is not of scaler type %d.", e->pos);
				return s->data[0];
			}

		case EXPR_ARRVAL:
			{
				symbol_t *s = table_get(&(exe->symbols), e->arrval.ident);
				int indx = execute_expr(exe, e->arrval.value);

				if(s == NULL)
					ERROR("Variable is not yet defined %d", e->pos);
				if(indx >= s->size || indx < 0)
					ERROR("Array Index out of Bounds. %d", e->pos);

				return s->data[indx];
			}

		case EXPR_FNCALL:
			for(int i = 0; i < FUNC_COUNT; i++)
			{
				uint32_t uid = token_hash((token_t){.str = FUNC[i].str, .len = FUNC[i].len});
				if(uid == e->fncall.ident)
				{
					char *v = FUNC[i].func(exe, e->fncall.args);

					return v ? *v : 0;
				}
			}
			ERROR("Function is not defined.");
	}

	return 0;
}

void execute_block(executor_t *e, block_t *blk)
{
	switch(blk->type)
	{
		case STET_LET:
		case STET_CONST:
			symbol_t sym = {0};

			sym.uid = blk->let.ident;

			sym.isconst = blk->type == STET_CONST;

			sym.isarray = blk->let.type == DECL_ARRAY ||
				blk->let.type == DECL_UINNIT || blk->let.type == DECL_STRING;

			if(sym.isarray)
			{
				if(blk->let.type != DECL_STRING)
				{
					expr_list_t *t = blk->let.expr;

					while(e)
					{
						if(sym.size % BLOCK_SIZE == 0)
							sym.data = realloc(sym.data, sym.size);
						sym.data[sym.size++] = execute_expr(e, t->value);

						t = t->next;
					}
				}
				else
					ERROR("Strings not implemented Yet. %d\n", blk->pos);
			}
			else
			{
				sym.data = calloc(1, sizeof(char));
				sym.size = 1;
				sym.data[0] = execute_expr(e, blk->let.expr->value);
			}

			table_set(&(e->symbols), blk->let.ident, sym);

			break;

		case STET_IF:
			{
				char c = execute_expr(e, blk->_if.cond);

				if(c)
					execute_block(e, blk->_if.true_blk);
				else
					execute_block(e, blk->_if.false_blk);
			}
			break;

		case STET_WHILE:
			{
				while(execute_expr(e, blk->_while.cond))
				{
					execute_block(e, blk->_while.blk);
				}
			}
			break;

		case STET_EXPR:
			execute_expr(e, blk->expr);
			break;

		case STET_MANY:
			for(int i = 0; i < blk->block_list.len; i++)
			{
				execute_block(e, blk->block_list.blks[i]);
			}
			break;
	}
}

void execute_program(executor_t *e)
{
	for(int i = 0; i < e->program->len; i++)
		execute_block(e, e->program->blks[i]);
}

symbol_t *table_get(table_t *t, uint32_t uid)
{
	for(int i = 0; i < t->size; i++)
	{
		if(t->symbols[i].uid == uid)
			return t->symbols + i;
	}

	return NULL;
}

void table_set(table_t *t, uint32_t uid, symbol_t sym)
{
	for(int i = 0; i < t->size; i++)
	{
		if(t->symbols[i].uid == uid)
		{
			t->symbols[i] = sym;
			return;
		}
	}

	if(t->size % BLOCK_SIZE == 0)
		t->symbols = realloc(t->symbols, sizeof(symbol_t) * (BLOCK_SIZE + t->size));

	t->symbols[t->size++] = sym;
}

void table_print(table_t *t)
{
	for(int i = 0; i < t->size; i++)
	{
		printf("%d : ", t->symbols[i].uid);
		for(int j = 0; j < t->symbols[i].size; j++)
			printf("%d ", t->symbols[i].data[j]);
		puts("");
	}
}
