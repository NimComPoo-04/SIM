#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

#define BLOCKSIZE 32

int TokenizerError = 0;

token_t *tokenize_string(const char *str, int len, int *size)
{
	TokenizerError = 0;

	token_t *tokens = NULL;
	*size = 0;

	for(int i = 0; i <= len && !TokenizerError; i++)
	{
		if(isspace(str[i]))
			continue;

		token_t tok = { 0, i, str + i, 0 };

		switch(str[i])
		{
			case 0:
				break;

			case ';':
				while(str[i] != '\n' && i < len)
					i++;
				continue;

			case '.':
				tok.type = DOT;
				tok.len = 1;
				break;

			case ',':
				tok.type = COMMA;
				tok.len = 1;
				break;

			case ':':
				tok.type = COLON;
				tok.len = 1;
				break;

			case '[':
				tok.type = LSQUARE;
				tok.len = 1;
				break;

			case ']':
				tok.type = RSQUARE;
				tok.len = 1;
				break;

			case '"':
				tok.type = STRING;
				i++;
				tok.str = str + i;
				tok.len = i;
				while(str[i] != '"' && i < len)
					i++;
				tok.len = i - tok.len;
				break;

			default:
				if(isdigit(str[i]) || str[i] == '-')
				{
					tok.type = NUMBER;
					tok.len = i;
					if(str[i] == '-') i++;
					if(str[i] == '0' &&
							(tolower(str[i+1]) == 'x' ||
							 tolower(str[i+1]) == 'b' ||
							 tolower(str[i+1]) == 'o'))
						i += 2;
					while(isdigit(str[i]) ||
						(tolower(str[i]) >= 'a' && tolower(str[i]) <= 'f'))
					{
						i++;
					}
					tok.len = i - tok.len;
					i--;
				}
				else if(isalpha(str[i]))
				{
					tok.type = IDENTIFIER;
					tok.len = i;
					while(isalnum(str[i]))
						i++;
					tok.len = i - tok.len;
					i--;
				}
				else
				{
					printf("Lexer Error %d : Unrecognized Token %c\n", i, str[i]);
					TokenizerError = 1;
					continue;
				}
		}

		// printf("%s\n", token_to_str(tok));

		if(*size % BLOCKSIZE == 0)
		{
			tokens = realloc(tokens, sizeof(token_t) * (*size + BLOCKSIZE));
		}
		tokens[*size] = tok;
		*size = *size + 1;
	}

	return tokens;
}

const char *LexTypes_Str[] = {"END", "DOT", "COMMA", "COLON",
	"LSQUARE", "RSQUARE", "IDENTIFIER", "STRING",
	"NUMBER", "CHAR"
};

char *token_to_str(token_t tok)
{
	static char string[256] = {0};

	sprintf(string, "[ %s : %.*s ]",
			LexTypes_Str[tok.type],
			tok.len, tok.str);

	return string;
}
