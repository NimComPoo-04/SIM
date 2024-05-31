#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

#define ERROR(str, ...) do { printf("ERROR %d %s ", __LINE__, __FILE__); printf(str ,##__VA_ARGS__); puts(""); exit(1); } while(0)
#define WARN(str, ...)  do { printf("WARN %d %s ", __LINE__, __FILE__); printf(str ,##__VA_ARGS__); puts(""); } while(0)
#define INFO(str, ...)  do { printf("INFO %d %s ", __LINE__, __FILE__); printf(str ,##__VA_ARGS__); puts(""); } while(0)

#endif
