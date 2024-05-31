#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "parser.h"
#include "log.h"

int16_t to_number(token_t tok)
{
	int base = 10;
	int32_t value = 0;

	int indx = 0;
	if(tok.len > 2 && tok.str[indx] == '0')
	{
		switch(tok.str[indx + 1])
		{
			case 'b':
				indx += 2;
				base = 2;
				break;

			case 'o':
				indx += 2;
				base = 8;
				break;

			case 'h':
				indx += 2;
				base = 16;
				break;

			default:
		}
	}

	for(int i = indx; i < tok.len; i++)
	{
		int dig = -1;

		if(isdigit(tok.str[i]))
			dig = tok.str[i] - '0';

		else if(isalpha(tok.str[i]))
			dig = toupper(tok.str[i]) - 'A' + 10;
			
		if(dig >= base || dig < 0)
			ERROR("Unrecognized number format. Possibly Malformed. %d\n", tok.pos);

		value = value * base + dig;
	}

	if((uint32_t)value > 0xFFFF)
		WARN("Larger than possible number, dropping top bits. %d %d\n", value, tok.pos);

	return value;
}

static int precedence_lookup(TokenType type)
{
	switch(type)
	{
		case LEX_PLUS:
		case LEX_MINUS:
			return 4;
		
		case LEX_LROT:
		case LEX_RROT:
			return 3;
		
		case LEX_NAND:
			return 2;

		case LEX_LT:
		case LEX_GT:
		case LEX_EQ:
			return 1;	

		case LEX_ASSIGN:
			return 0;

		default:
			return 100;
	}
}

expr_t *parse_expr_int(expr_t *head, lexer_t *l, int *lparen, int *sqparen)
{
	token_t tok = lex_next(l);	

	expr_t *e = NULL;

	switch(tok.type)
	{
		case LEX_NUMBER:
			e = calloc(1, sizeof(*e));
			e->pos = tok.pos;
			e->type = EXPR_NUM;
			e->toktype = tok.type;
			e->num = to_number(tok);
			break;

		case LEX_IDENT:
			if(l->toks[l->pos].type == LEX_LSQUARE)
			{
				e = calloc(1, sizeof(*e));
				e->pos = tok.pos;
				e->type = EXPR_ARRVAL;
				e->toktype = tok.type;
				e->arrval.ident = tok.uid;

				int t = *sqparen;
				++*sqparen;

				while((l->toks[l->pos].type >= LEX_LPAREN && l->toks[l->pos].type < LEX_PTR)
						|| l->toks[l->pos].type == LEX_NUMBER
						|| l->toks[l->pos].type == LEX_IDENT)
				{
					e->arrval.value = parse_expr_int(e->arrval.value, l, lparen, sqparen);
					if(*sqparen == t)
						break;
				}

				if(*sqparen != t)
					ERROR("Unmatched Parenthesis. %d", e ? e->pos : -1);
			}
			else if(l->toks[l->pos].type == LEX_LPAREN)
			{

				e = calloc(1, sizeof(*e));
				e->pos = tok.pos;
				e->type = EXPR_FNCALL;
				e->toktype = tok.type;

				e->fncall.ident = tok.uid;

				// No arg thing
				if(l->toks[l->pos + 1].type == LEX_RPAREN)
				{
					l->pos += 2;
					break;
				}

				e->fncall.args = calloc(1, sizeof(expr_list_t));
				expr_list_t **et = &(e->fncall.args);

				int t = *lparen;

				lex_next(l);
				++*lparen;

				while(1)
				{
					while((l->toks[l->pos].type >= LEX_LPAREN && l->toks[l->pos].type < LEX_PTR)
							|| l->toks[l->pos].type == LEX_NUMBER
							|| l->toks[l->pos].type == LEX_IDENT)
					{
						(*et)->value = parse_expr_int((*et)->value, l, lparen, sqparen);

						if(*lparen == t)
							goto OUTSIDE;
					}

					token_t tok = lex_next(l);
					if(tok.type == LEX_COMMA)
					{
						(*et)->next = calloc(1, sizeof(expr_list_t));
						et = &((*et)->next);
					}
					else
						ERROR("Unexpected token %d", tok.pos);
				}
OUTSIDE:
			}
			else
			{
				e = calloc(1, sizeof(*e));
				e->pos = tok.pos;
				e->type = EXPR_IDENT;
				e->toktype = tok.type;
				e->ident = tok.uid;
			}
			break;

		case LEX_PLUS: case LEX_MINUS:
		case LEX_LROT: case LEX_RROT:	
		case LEX_NAND: case LEX_LT:
		case LEX_GT: case LEX_EQ:
		case LEX_ASSIGN:
			if(head == NULL)
			{
				ERROR("Illegal use of Operators. %d\n", tok.pos);
			}
			else
			{
				if(precedence_lookup(tok.type) < precedence_lookup(head->toktype))
				{
					e = calloc(1, sizeof(*e));
					e->pos = tok.pos;
					e->type = EXPR_OP;

					e->toktype = tok.type;	
					e->op.lhs = head;
					e->op.rhs = parse_expr_int(NULL, l, lparen, sqparen);
				}
				else
				{
					e = calloc(1, sizeof(*e));
					e->pos = tok.pos;
					e->type = EXPR_OP;

					e->toktype = tok.type;

					expr_t *t = head;
					while(!t->op.rhs->isparen && precedence_lookup(tok.type) > precedence_lookup(t->op.rhs->toktype))
						t = head->op.rhs;

					e->op.lhs = t->op.rhs;
					t->op.rhs = e;

					e->op.rhs = parse_expr_int(NULL, l, lparen, sqparen);

					e = head;
				}
			}
			break;

		case LEX_LPAREN:
			{
				int t = *lparen;
				++*lparen;
				while((l->toks[l->pos].type >= LEX_LPAREN && l->toks[l->pos].type < LEX_PTR)
						|| l->toks[l->pos].type == LEX_NUMBER
						|| l->toks[l->pos].type == LEX_IDENT)
				{
					e = parse_expr_int(e, l, lparen, sqparen);

					if(*lparen == t)
						break;
				}

				if(*lparen != t)
					ERROR("Unmatched Parenthesis. %d", e ? e->pos : -1);

				e->isparen = 1;
			}
			break;

		case LEX_RPAREN:
			--*lparen;
			return head;

		case LEX_RSQUARE:
			--*sqparen;
			return head;

		default:
			// l->pos--;
			return head;
	}

	return e;
}

expr_t *parse_expr(lexer_t *l)
{
	expr_t *e = NULL;
	int paren = 0;
	int square = 0;

	while((l->toks[l->pos].type >= LEX_LPAREN && l->toks[l->pos].type < LEX_PTR)
			|| l->toks[l->pos].type == LEX_NUMBER
			|| l->toks[l->pos].type == LEX_IDENT)
	{
		e = parse_expr_int(e, l, &paren, &square);
	}

	if(paren != 0 || square != 0)
		ERROR("Unmatched Parenthesis. %d", e ? e->pos : -1);

	return e;
}

void expr_destory(expr_t *e)
{
	if(e == NULL) return;

	switch(e->type)
	{
		case EXPR_NUM:
			free(e);
			break;

			// No idents for now
		case EXPR_IDENT:
		case EXPR_FNCALL:
		case EXPR_ARRVAL:
			break;

		case EXPR_OP:
			expr_destory(e->op.lhs);
			expr_destory(e->op.rhs);
			free(e);
			break;
	}
}

void expr_print(expr_t *e, int depth)
{
	if(e == NULL) return;

	if(e->type == EXPR_OP)
	{
		char *c = 0;
		switch(e->toktype)
		{
			case LEX_PLUS: c = "PLUS"; break;
			case LEX_MINUS: c = "MINUS"; break;
			case LEX_LROT: c = "LROT"; break;
			case LEX_RROT:	 c = "RROT"; break;
			case LEX_NAND: c = "NAND"; break;
			case LEX_LT: c = "LT"; break;
			case LEX_GT: c = "GT"; break;
			case LEX_EQ: c = "EQ"; break;
			case LEX_ASSIGN: c = "ASSIGN"; break;

			default:
				 break;
		}

		printf("%*c%s\n", depth * 4, ' ', c);
		expr_print(e->op.lhs, depth + 1);
		expr_print(e->op.rhs, depth + 1);
	}
	else if(e->type == EXPR_NUM)
		printf("%*c%d\n", depth * 4, ' ', e->num);
	else if(e->type == EXPR_IDENT)
		printf("%*c%d\n", depth * 4, ' ', e->ident);
	else if(e->type == EXPR_ARRVAL)
	{
		printf("%*cARRAY %d\n", depth * 4, ' ', e->arrval.ident);
		expr_print(e->arrval.value, depth + 1);
	}
	else if(e->type == EXPR_FNCALL)
	{
		printf("%*cCALL %d\n", depth * 4, ' ', e->fncall.ident);

		expr_list_t *tmp = e->fncall.args;

		while(tmp)
		{
			expr_print(tmp->value, depth + 1);
			tmp = tmp->next;
		}
	}
}
