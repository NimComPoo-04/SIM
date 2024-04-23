#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "cpu.h"

char *read_file(char *file, int *SIZE)
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

	*SIZE = size;
	return data;
}

int main(int argc, char **argv)
{
	mem_t mem;
	mem_create(&mem, 512);

	if(argc < 2)
	{
		printf("INPUT FILE NAME!\n");
		return 1;
	}

	int size = 0;
	char *data = read_file(argv[1], &size);

	mem_load_img(&mem, data, size);

	cpu_t cpu;

	cpu_reset(&cpu);

	// There is not halting
	while(1)
	{
		cpu_step(&cpu, &mem);
	}

	mem_dump(&mem);

	mem_destroy(&mem);

	return 0;
}
