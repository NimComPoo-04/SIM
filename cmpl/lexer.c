#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"
#include "log.h"

#define BLOCK_SIZE 32

uint32_t token_hash(token_t tok);
lexer_t lex_tokenize(const char *source, int length)
{
	lexer_t lex = {
		.toks = 0,
		.len = 0,
		.pos = 0
	};

	for(int i = 0; i < length; i++)
	{
		// Ignore Spaces
		if(isspace(source[i])) continue;

		// Ignore Comments
		if(source[i] == '/')
		{
			i++;

			// Single line comment
			if(source[i] == '/')
			{
				while(i < length && source[i] != '\n')
					i++;
			}

			// Multiline comment
			else if(source[i] == '*')
			{
				i++;
				while(i + 1 < length && !(source[i] == '*' && source[i + 1] == '/'))
					i++;
				i++;
			}

			// REPORT!!! ...
			else
				ERROR("Unexpected Token. Comment? %d", i);

			continue;
		}

		token_t tok = {
			.type = 0,
			.str = source + i,
			.len = 1,
			.scaler_value = 0,
			.pos = i
		};

		switch(source[i])
		{
			case ',': tok.type = LEX_COMMA; break;
			case ';': tok.type = LEX_SEMICOLON; break;

			case '+': tok.type = LEX_PLUS; break;
			case '-': tok.type = LEX_MINUS; break;
			case '*': tok.type = LEX_PTR; break;
			case '>': tok.type = LEX_GT; break;
			case '<': tok.type = LEX_LT; break;

			case '(': tok.type = LEX_LPAREN; break;
			case ')': tok.type = LEX_RPAREN; break;
			case '{': tok.type = LEX_LCURLY; break;
			case '}': tok.type = LEX_RCURLY; break;
			case '[': tok.type = LEX_LSQUARE; break;
			case ']': tok.type = LEX_RSQUARE; break;

			case '=':
				  if(i + 1 < length && source[i+1] == '=')
					  tok.type = LEX_EQ;
				  else
					  tok.type = LEX_ASSIGN;
				  break;

			case '"':
				  tok.type = LEX_STRING;
				  tok.len = i;
				  i++;
				  while(i < length && source[i] != '"')
				  {
					  if(source[i] == '\\')
						  i++;
					  i++;
				  }
				  tok.len = i - tok.len;
				  tok.len++;
				  // i++;
				  break;

			default:
				  if(isalpha(source[i]))
					  tok.type = LEX_IDENT;

				  else if(isdigit(source[i]))
					  tok.type = LEX_NUMBER;

				  else
					  ERROR("Unexpected Token. %c %d", source[i], i);

				  tok.len = i;
				  while(isalnum(source[i]))
					  i++;
				  tok.len = i - tok.len;
				  i--;

				  static struct
				  {
					  const char *name;
					  int len;
					  TokenType type;
				  } keyword[] = {
					  {"lrot", 4, LEX_LROT},
					  {"rrot", 4, LEX_RROT},
					  {"nand", 4, LEX_NAND},
					  {"let", 3, LEX_LET},
					  {"const", 5, LEX_CONST},
					  {"if", 2, LEX_IF},
					  {"then", 4, LEX_THEN},
					  {"else", 4, LEX_ELSE},
					  {"while", 5, LEX_WHILE},
					  {"do", 2, LEX_DO},
					  {"fn", 2, LEX_FN}
				  };

				  if(tok.type == LEX_IDENT)
				  {
					  for(size_t i = 0; i < sizeof keyword / sizeof keyword[0]; i++)
					  {
						  if(tok.len == keyword[i].len && strncmp(keyword[i].name, tok.str, tok.len) == 0)
						  {
							  tok.type = keyword[i].type;
							  break;
						  }
					  }

					  if(tok.type == LEX_IDENT)
						  tok.uid = token_hash(tok);
				  }
		}

		if(lex.len % BLOCK_SIZE == 0)
			lex.toks = realloc(lex.toks, (lex.len + BLOCK_SIZE) * sizeof(token_t));
		lex.toks[lex.len++] = tok;
	}

	if(lex.len % BLOCK_SIZE == 0)
		lex.toks = realloc(lex.toks, (lex.len + BLOCK_SIZE) * sizeof(token_t));
	lex.toks[lex.len++] = (token_t){ .type = LEX_END, .str = "\nEOF", .len = 4};

	return lex;
}

uint32_t token_hash(token_t tok)
{
	uint32_t size = 0;

	// hash = sum(ai + 2 ^ i)
	for(int i = 0; i < tok.len; i++)
	{
		size += tok.str[i] << i;
	}

	return size;
}

token_t lex_next(lexer_t *l)
{
	if(l->pos > (int)l->len)
		return (token_t){
			.type = LEX_END
		};
	return l->toks[l->pos++];
}

void lex_reset(lexer_t *l)
{
	l->pos = 0;
}

void lex_print(lexer_t *l)
{
	for(size_t i = 0; i < l->len; i++)
	{
		printf("%.*s ", l->toks[i].len, l->toks[i].str);
	}
}

void lex_destroy(lexer_t *l)
{
	free(l->toks);
}
