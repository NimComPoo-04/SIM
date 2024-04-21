#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "assembler.h"

char *read_file(char *file)
{
	FILE *s = fopen(file, "rb");

	if(!s)
	{
		printf("Source %s not found!\n", file);
		exit(1);
	}

	int size = ftell(s);
	fseek(s, 0, SEEK_END);
	size = ftell(s) - size;
	fseek(s, 0, SEEK_SET);

	char *data = calloc(1, size + 1);
	fread(data, size, 1, s);

	data[size] = 0;

	fclose(s);

	return data;
}

void write_file(char *dest, uint8_t *data, int len)
{
	FILE *f = fopen(dest, "wb+");
	fwrite(data, len, 1, f);
	fclose(f);
}

int main(int argc, char **argv)
{
	char *src = argc < 2 ? "in.asm" : argv[1];
	char *dest = argc < 3 ? "out.bin" : argv[2];

	char *source_code = read_file(src);

	printf("Source Code: \n");
	printf("%s\n", source_code);

	int toks;
	token_t *tokens = tokenize_string(source_code, strlen(source_code), &toks);

	if(TokenizerError)
		return 1;

	printf("\nTokens: \n");
	for(int i = 0; i < toks; i++)
		printf("%s\n", token_to_str(tokens[i]));

	assembler_t as = assemble_tokens(tokens, toks);

	if(AssemblerError)
		return 2;

	printf("\nAssembled Program: \n");
	print_assembled(&as);

	printf("\nLables: \n");
	print_lables(&as);

	uint8_t *data = assembler_to_bin(&as);

	if(AssemblerError)
		return 3;

	write_file(dest, data, as.size);

	return 0;
}
