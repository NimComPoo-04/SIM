#ifndef _LEXER_H_
#define _LEXER_H_

#include <stdint.h>
#include <stdlib.h>

typedef enum
{
	LEX_END,

	LEX_LCURLY, LEX_RCURLY,
	LEX_LPAREN, LEX_RPAREN,
	LEX_LSQUARE, LEX_RSQUARE,

	LEX_PLUS, LEX_MINUS, LEX_LROT, LEX_RROT, LEX_NAND,
	LEX_LT, LEX_GT, LEX_EQ, LEX_ASSIGN, LEX_PTR,

	LEX_LET, LEX_CONST, LEX_IF, LEX_THEN,
	LEX_ELSE, LEX_WHILE, LEX_DO, LEX_FN,

	LEX_IDENT, LEX_NUMBER, LEX_STRING,

	LEX_COMMA, LEX_SEMICOLON
} TokenType;

typedef struct
{
	TokenType type;

	const char *str;
	int len;

	int scaler_value;
	uint32_t uid;
	int pos;
} token_t;

typedef struct
{
	token_t *toks;
	size_t len;
	int pos;
} lexer_t;

lexer_t lex_tokenize(const char *source, int length);
token_t lex_next(lexer_t *l);
void lex_reset(lexer_t *l);
void lex_print(lexer_t *l);
void lex_destroy(lexer_t *l);

#endif

