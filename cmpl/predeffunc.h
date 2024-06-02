#ifndef _PREDEFFUNC_H_
#define _PREDEFFUNC_H_

typedef struct
{
	char *str;
	int len;
	void *(*func)(executor_t *, expr_list_t *);
} predef_func_t;

extern predef_func_t *FUNC;
extern int FUNC_COUNT;

#endif
