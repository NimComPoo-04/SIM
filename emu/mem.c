#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mem.h"

void mem_create(mem_t *m, int size)
{
	m->data = calloc(sizeof(char), size);
	m->size = size;
}

void mem_load_img(mem_t *m, char *img, int size)
{
	if(size > m->size / 2)
		printf("Size of Image exceded ROM size!");
	else
		memcpy(m->data, img, size);
}

char mem_get(mem_t *m, uint16_t index)
{
	if(index == 0xFFFF)
		return 0x69;
	else
	{
		if(index >= m->size)
		{
			printf("Out of memory write\n");
			return 0xaa;				// represents random noise
		}
		else
			return m->data[index];
	}
}

void mem_set(mem_t *m, uint16_t index, char data)
{
//	printf("%d %d\n", index, data);
	if(index == 0xFFFF)
		putc(data, stderr);
	else
	{
		if(index >= m->size)
			printf("Out of memory write\n");
		else if(index >= m->size / 2)
			m->data[index] = data;
		else
			printf("ROM memory write\n");
	}
}

void mem_destroy(mem_t *m)
{
	free(m->data);
}

void mem_dump(mem_t *m)
{
	for(int i = 0; i < m->size; i+=16)
	{
		for(int j = 0; j < 16; j++)
			printf("%02hhx ", m->data[i + j]);
		puts("");
	}
}
