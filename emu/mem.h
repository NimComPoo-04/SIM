#ifndef _MEM_H_
#define _MEM_H_

#include <stdint.h>

typedef struct
{
	char *data;
	int size;
} mem_t;

void mem_create(mem_t *m, int size);
void mem_load_img(mem_t *m, char *img, int size);
char mem_get(mem_t *m, uint16_t index);
void mem_set(mem_t *m, uint16_t index, char data);
void mem_destroy(mem_t *m);

void mem_dump(mem_t *m);

#endif
