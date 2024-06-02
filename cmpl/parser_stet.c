#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "parser.h"

#define BLOCK_SIZE 32

block_t *parse_block(lexer_t *l)
{
	block_t *b = NULL;

	token_t tok = lex_next(l);

	int prevpos = l->pos;

	switch(tok.type)
	{
		case LEX_LET:
		case LEX_CONST:

			b = calloc(1, sizeof *b);

			b->type = (tok.type == LEX_LET ? STET_LET : STET_CONST);

			tok = lex_next(l);
			if(tok.type == LEX_IDENT)
				b->let.ident = tok.uid;
			else
				ERROR("Identifier Not Found. %d\n", tok.pos);

			tok = lex_next(l);
			if(tok.type == LEX_SEMICOLON)
				break;
			else if(tok.type != LEX_ASSIGN)
				ERROR("Unexpected Token. %d\n", tok.pos);

			if(l->toks[l->pos].type == LEX_LCURLY)
			{
				b->let.expr = calloc(1, sizeof(expr_list_t));
				expr_list_t *et = b->let.expr;

				b->let.type = DECL_ARRAY;

				lex_next(l);
				while(1)
				{
					et->value = parse_expr(l);

					token_t tok = lex_next(l);
					if(tok.type == LEX_RCURLY)
						break;
					else if(tok.type == LEX_COMMA)
					{
						et->next = calloc(1, sizeof(expr_list_t));
						et = et->next;
					}
					else
						ERROR("Unexpected Token. %d\n", tok.pos);
				}
			}
			else if(l->toks[l->pos].type == LEX_LSQUARE)
			{
				lex_next(l);

				b->let.expr = calloc(1, sizeof(expr_list_t));
				b->let.type = DECL_UINNIT;

				int paren = 0;
				int square = 1;

				while((l->toks[l->pos].type >= LEX_LPAREN && l->toks[l->pos].type < LEX_PTR)
						|| l->toks[l->pos].type == LEX_NUMBER
						|| l->toks[l->pos].type == LEX_IDENT)
				{
					b->let.expr->value = parse_expr_int(b->let.expr->value, l, &paren, &square);
					if(square == 0)
						break;
				}
				
				if(square != 0)
					ERROR("Unmatched Square Paren. %d", tok.pos);
			}
			else if(l->toks[l->pos].type == LEX_STRING)
			{
				b->let.type = DECL_STRING;
				b->let.str = lex_next(l);
			}
			else
			{
				b->let.type = DECL_SCALER;
				b->let.expr = calloc(1, sizeof(expr_list_t));
				b->let.expr->value = parse_expr(l);
			}

			tok = lex_next(l);
			if(tok.type != LEX_SEMICOLON)
				ERROR("Expected Semicolon. %d\n", tok.pos);

			break;

		case LEX_IF:
			b = calloc(1, sizeof *b);
			b->type = STET_IF;
			if(!(b->_if.cond = parse_expr(l)))
				ERROR("Missing Condition. %d\n", tok.pos);

			if((tok = lex_next(l)).type != LEX_THEN)
				ERROR("Missing 'then' keyword. %d\n", tok.pos);

			b->_if.true_blk = parse_block(l);
			if(l->toks[l->pos].type == LEX_ELSE)
			{
				l->pos++;
				b->_if.false_blk = parse_block(l);
			}
			break;

		case LEX_ELSE:
			ERROR("Else without an If. %d\n", tok.pos);
			break;

		case LEX_WHILE:
			b = calloc(1, sizeof *b);
			b->type = STET_WHILE;
			if(!(b->_while.cond = parse_expr(l)))
				ERROR("Missing Condition. %d\n", tok.pos);
			if((tok = lex_next(l)).type != LEX_DO)
				ERROR("Missing 'do' keyword. %d\n", tok.pos);
			b->_while.blk = parse_block(l);
			break;

		case LEX_LCURLY:
			b = calloc(1, sizeof *b);
			b->type = STET_MANY;

			while(l->toks[l->pos].type != LEX_RCURLY)
			{
				if(b->block_list.len % BLOCK_SIZE == 0)
					b->block_list.blks = realloc(b->block_list.blks, sizeof(block_t *) * (BLOCK_SIZE + b->block_list.len));

				b->block_list.blks[b->block_list.len++] = parse_block(l);

				if(l->toks[l->pos].type == LEX_END)
					ERROR("Unexpected end of file. %d\n", tok.pos);
			}

			lex_next(l);

			break;

		case LEX_FN:
			ERROR("Functions Not Implemented Yet.");
			break;

		case LEX_RCURLY:
			return NULL;

		case LEX_END:
			return NULL;

		default:
			l->pos--;	// unread the thing
			b = calloc(1, sizeof *b);
			b->type = STET_EXPR;
			b->expr = parse_expr(l);		
			if((tok = lex_next(l)).type != LEX_SEMICOLON)
				ERROR("Missing Semicolon. %d\n", tok.pos);

			if(b->expr == NULL)
			{
				// Empty statement
				free(b);
				b = NULL;
			}
			break;
	}

	b->pos = prevpos;

	return b;
}

void block_print(block_t *b, int depth)
{
	if(b == NULL)
		return;

	switch(b->type)
	{
		case STET_IF:
			printf("%*cIF\n", depth * 4, ' ');	
			expr_print(b->_if.cond, depth + 1);
			printf("%*cTHEN\n", depth * 4, ' ');
			block_print(b->_if.true_blk, depth + 1);
			if(b->_if.false_blk)
			{
				printf("%*cELSE\n", depth * 4, ' ');
				block_print(b->_if.false_blk, depth + 1);
			}
			break;

		case STET_WHILE:
			printf("%*cWHILE \n", depth * 4, ' ');	
			expr_print(b->_while.cond, depth + 1);
			printf("%*cDO \n", depth * 4, ' ');
			block_print(b->_while.blk, depth + 1);
			break;

		case STET_LET:
		case STET_CONST:
			const char *lecon = (b->type == STET_LET ? "LET" : "CONST");

			switch(b->let.type)
			{
				case DECL_NONE: // do nothin'
					printf("%*c%s %d NOINIT\n", depth * 4, ' ', lecon, b->let.ident);
					break;

				case DECL_UINNIT: // do nothin'
					printf("%*c%s %d BLOCK\n", depth * 4, ' ', lecon, b->let.ident);
					expr_print(b->let.expr->value, depth + 1);
					break;

				case DECL_SCALER:
					printf("%*c%s %d SCALER\n", depth * 4, ' ', lecon, b->let.ident);
					expr_print(b->let.expr->value, depth + 1);
					break;

				case DECL_ARRAY:
					printf("%*c%s %d ARRAY\n", depth * 4, ' ', lecon, b->let.ident);
					expr_list_t *e = b->let.expr;
					while(e)
					{
						expr_print(e->value, depth + 1);
						e = e->next;
					}
					break;

				case DECL_STRING:
					printf("%*c%s %d STRING\n", depth * 4, ' ', lecon, b->let.ident);
					printf("%*c%.*s\n", (depth + 1) * 4, ' ', b->let.str.len, b->let.str.str);
					break;
			}

			break;

		case STET_EXPR:
			expr_print(b->expr, depth);
			break;

		case STET_MANY:
			for(int i = 0; i < b->block_list.len; i++)
			{
				block_print(b->block_list.blks[i], depth);
			}
			break;

		default:
			break;
	}
}

program_t parse_program(lexer_t *l)
{
	program_t prg = {0};

	while(l->toks[l->pos].type != LEX_END)
	{
		if(prg.len % BLOCK_SIZE == 0)
			prg.blks = realloc(prg.blks, sizeof(block_t *) * (BLOCK_SIZE + prg.len));

		prg.blks[prg.len++] = parse_block(l);;
	}
	
	return prg;
}

void program_print(program_t *prg)
{
	for(int i = 0; i < prg->len; i++)
		block_print(prg->blks[i], 0);
}
