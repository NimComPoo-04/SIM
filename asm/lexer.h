#ifndef _LEXER_H_
#define _LExER_H_

typedef enum
{
	END,
	DOT,
	COMMA,
	COLON,
	LSQUARE,
	RSQUARE,
	IDENTIFIER,
	STRING,
	NUMBER
} LexTypes;

typedef struct
{
	LexTypes type;
	
	int pos;
	const char *str;
	int len;
} token_t;

extern int TokenizerError;

token_t *tokenize_string(const char *str, int len, int *size);
char *token_to_str(token_t tok);

#endif
