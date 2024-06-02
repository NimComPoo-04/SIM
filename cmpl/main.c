#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "executor.h"

static char *source;
static int size;

int read_file(const char *fname)
{
	FILE *f = fopen(fname, "rb+");

	if(!f)
		return 0;

	size = ftell(f);
	fseek(f, 0, SEEK_END);
	size = ftell(f) - size;
	fseek(f, 0, SEEK_SET);

	source = calloc(sizeof(char), size + 1);
	fread(source, 1, size, f);

	fclose(f);

	return 1;
}

int main(int argn, char **argv)
{
	if(argn != 2)
	{
		fprintf(stderr, "./cmpl filename\n");
		return -1;
	}

	if(!read_file(argv[1]))
	{
		fprintf(stderr, "File does not exist!\n");
		return -2;
	}

	lexer_t l = lex_tokenize(source, size);

	puts("=== Tokens ===");
	lex_print(&l);
	puts("");

	program_t prg = parse_program(&l);
	puts("");

	puts("=== Program ===");
	program_print(&prg);
	puts("");

	puts("=== Output ===");
	executor_t e = {.program = &prg};
	execute_program(&e);
	puts("");

	puts("=== Symbol Table ===");
	table_print(&e.symbols);
	puts("");

	return 0;
}
