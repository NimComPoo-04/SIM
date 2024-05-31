#include <stdio.h>

#include "lexer.h"
#include "parser.h"

const char values[] = 
"let a = 0;"
"let b = 1;"
"let c = 0;"
"let n = 10;"
"let buff = [100];"
"while n do"
"{"
"	c = a + b;"
"	a = b;"
"	b = c;"
"	n = n - 1;"
"	print(tonumber(buff, a));"
"}"
;

int main(void)
{
	lexer_t l = lex_tokenize(values, sizeof values - 1);
	lex_print(&l);

	puts("");

	program_t prg = parse_program(&l);

	program_print(&prg);

	//expr_destory(e);
	lex_destroy(&l);

	return 0;
}
