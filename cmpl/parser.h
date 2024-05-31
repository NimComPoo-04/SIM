#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdint.h>
#include "lexer.h"

typedef enum
{
	EXPR_NUM,
	EXPR_IDENT,
	EXPR_ARRVAL,
	EXPR_FNCALL,
	EXPR_OP,
} TypeExpr;

typedef struct expr_list_t
{
	struct expr_t *value;
	struct expr_list_t *next;
} expr_list_t;

typedef struct expr_t
{
	TypeExpr type;
	TokenType toktype;

	int pos;
	int isparen;

	union
	{
		int16_t num;

		uint32_t ident;

		struct
		{
			uint32_t ident;
			struct expr_t *value;
		} arrval;

		struct
		{
			uint32_t ident;
			struct expr_list_t *args;
		} fncall;

		struct
		{
			struct expr_t *lhs;
			struct expr_t *rhs;
		} op;
	};

} expr_t;

typedef enum
{
	STET_LET,
	STET_CONST,
	STET_IF,
	STET_WHILE,
	STET_EXPR,
	STET_MANY
} BlockExpr;

typedef struct args_list_t
{
	uint32_t ident;
	uint8_t isptr;
	struct args_list_t *next;
} args_list_t;

typedef enum
{
	DECL_NONE,
	DECL_UINNIT, // uninitialized array
	DECL_SCALER,
	DECL_ARRAY,
	DECL_STRING,
} DeclType;

typedef struct block_t
{
	BlockExpr type;

	union
	{
		struct
		{
			uint32_t ident;
			DeclType type;
			expr_list_t *expr;
			token_t str;
		} let, _const;
		
		struct
		{
			expr_t *cond;
			struct block_t *true_blk;
			struct block_t *false_blk;
		} _if;

		struct
		{
			expr_t *cond;
			struct block_t *blk;
		} _while;

		struct
		{
			uint32_t ident;
			args_list_t *args;
			size_t count;
			struct block_t *blk;
		} func;

		struct
		{
			int len;
			struct block_t **blks;
		} block_list;

		expr_t *expr;
	};
} block_t;

expr_t *parse_expr(lexer_t *l);
expr_t *parse_expr_int(expr_t *head, lexer_t *l, int *lparen, int *sqparen);

void expr_destory(expr_t *e);
void expr_print(expr_t *e, int depth);

block_t *parse_block(lexer_t *l);
void block_print(block_t *b, int depth);

// Commonly needed
int16_t to_number(token_t tok);

typedef struct
{
	block_t **blks;
	int len;
} program_t;

program_t parse_program(lexer_t *lex);
void program_print(program_t *prg);

#endif

