#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "lexer.h"
#include "assembler.h"

#define BLOCKSIZE 32

int AssemblerError = 0;

int ustrnncmp(const char *start, int len, const char *second, int t)
{
	if(t != len)
		return len - t;

	while(len)
	{
		if(tolower(*start) != tolower(*second))
			return *start - *second;
		start++;
		second++;
		len--;
	}

	return 0;
}

int ustrncmp(const char *start, int len, const char *second)
{
	return ustrnncmp(start, len, second, strlen(second));
}

uint32_t read_number(const char *start, int len, uint16_t bits, int indx)
{
	int is_neg = 1;
	int pos = 0;

	if(start[pos] == '-')
	{
		is_neg = -1;
		pos++;
	}

	int base = 10;

	if(start[pos] == '0')
	{
		switch(tolower(start[pos+1]))
		{
			case 'b':
				base = 2;
				pos += 2;
				break;

			case 'o':
				base = 8;
				pos += 2;
				break;

			case 'x':
				base = 16;
				pos += 2;
				break;
		}
	}

	uint32_t value = 0;

	while(pos < len && !AssemblerError)
	{
		int val = 0;
		if(isdigit(start[pos]))
			val = start[pos] - '0';
		else if(isalpha(start[pos]))
			val = tolower(start[pos]) - 'a' + 10;
		else
		{
			printf("Assembler Error %d : Malformed Number %d %.*s\n",
					indx,  val, len, start);
			AssemblerError = 1;
		}

		if(val >= base)
		{
			printf("Assembler Error %d : Digits Exceeded Base %d %.*s\n",
					indx, base, len, start);
			AssemblerError = 1;
		}

		value = value * base + val;
		pos++;
	}

	value *= is_neg;

	if(value >= (2ul << bits))
	{
		printf("Assembler Error %d : Number Exceeded Size %.*s\n",
				indx, len, start);
		AssemblerError = 1;
	}

	return value;
}

void *allocate_block(assembler_t *as, AsTypes type, uint32_t size)
{
	block_t blk = {0};
	blk.type = type;
	blk.size = size;

	if(blk.type != ORIGIN)
	{
		blk.data = calloc(sizeof(uint8_t), size);
		as->cursor += size;
	}
	else
	{
		as->cursor = size;
	}

	if(as->num_blocks % BLOCKSIZE == 0)
		as->blocks = realloc(as->blocks, sizeof(block_t) * (as->num_blocks + BLOCKSIZE));
	as->blocks[as->num_blocks] = blk;
	as->num_blocks++;


	return blk.data;
}

void read_string(uint8_t *dig, const char *src, uint32_t len)
{
	while(len)
	{
		if(*src == '\\')
		{
			src++;
			switch(*src)
			{
				case 'n':
					*dig = '\n';
					break;
				default:
					*dig = *src;
			}
		}
		else
		{
			*dig = *src;
		}

		src++;
		dig++;

		len--;
	}
}

int check_if_register(token_t tok)
{
	if(tok.type != IDENTIFIER)
		return -1;

	// XXX: DO NOT CHANGE THE ORDER OF THIS ARRAY BOIS
	static const char *regs[] = {
		"ZR", "CR", "A", "T",
		"AL", "AH", "TL", "TH",
		"ARG", "PC", "IR", "MAR",
		"MBR" };

	for(uint32_t i = 0; i < sizeof regs / sizeof regs[0]; i++)
	{
		if(ustrncmp(tok.str, tok.len, regs[i]) == 0)
			return i;
	}

	return -1;
}

void register_patch(assembler_t *as, token_t id, uint16_t *pos)
{
	if(as->patches)
	{
		void *next = as->patches->next;
		as->patches->next = calloc(sizeof(patch_table_t), 1);
		as->patches->next->id = id;
		as->patches->next->pos = pos;
		as->patches->next->next = next;
	}
	else
	{
		as->patches = calloc(sizeof(patch_table_t), 1);
		as->patches->id = id;
		as->patches->pos = pos;
		as->patches->next = NULL;
	}
}

int register_label(assembler_t *as, token_t id, uint32_t value)
{
	label_t l = {0};
	l.id = id;
	l.value = value;

	if(as->label_count % BLOCKSIZE == 0)
		as->labels = realloc(as->labels, sizeof(label_t) * (as->label_count + BLOCKSIZE));

	as->labels[as->label_count] = l;
	return as->label_count++;
}

uint16_t get_label_value(assembler_t *as, token_t id)
{
	for(int i = 0; i < as->label_count; i++)
	{
		if(ustrnncmp(id.str, id.len, as->labels[i].id.str, as->labels[i].id.len) == 0)
		{
			return as->labels[i].value;
		}
	}

	printf("Assembler Error %d : Lable %s Does not Exist\n",
			id.pos, token_to_str(id));
	AssemblerError = 1;

	return 0;
}

void opcode_with_arg(assembler_t *as, uint8_t opcode, token_t *tokens, int *i, int sxi)
{
	int r = check_if_register(tokens[*i]);
	if(r != 2  && r != 3)
	{
		printf("Assembler Error %d : Register Expected%s\n",
				tokens[*i].pos, token_to_str(tokens[*i]));
		AssemblerError = 1;
	}

	opcode |= (r & 1) << 5;
	++*i;

	if(tokens[*i].type != COMMA)
	{
		printf("Assembler Error %d : Multiple Arguments Expected%s\n",
				tokens[*i].pos, token_to_str(tokens[*i]));
		AssemblerError = 1;
	}
	++*i;

	r = check_if_register(tokens[*i]);
	if(r == 2 || r == 3)
	{
		opcode |= 2 << 3;
		opcode |= (r & 1) << 6;

		uint8_t *opq = allocate_block(as, OPCODE, 1);
		*opq = opcode;
	}
	else if(tokens[*i].type == LSQUARE)
	{
		opcode |= (3 << 3);

		++*i;
		r = check_if_register(tokens[*i]);
		if(r == 2 || r == 3)
		{
			opcode |= (r & 1) << 6;
		}
		else
		{
			printf("Assembler Error %d : General Perpose Register Expected%s\n",
					tokens[*i].pos, token_to_str(tokens[*i]));
			AssemblerError = 1;
		}

		++*i;
		if(tokens[*i].type != RSQUARE)
		{
			printf("Assembler Error %d : Right Square Bracket Expected%s\n",
					tokens[*i].pos, token_to_str(tokens[*i]));
			AssemblerError = 1;
		}

		uint8_t *opq = allocate_block(as, OPCODE, 1);
		*opq = opcode;
	}
	else if(tokens[*i].type == NUMBER)
	{
		uint16_t v = read_number(tokens[*i].str, tokens[*i].len, 16, *i);

		if(v > 0xFF)
		{
			if(!sxi)
			{
				printf("Assembler Error %d : Opcode does not support 16-bit imm%s\n",
						tokens[*i].pos, token_to_str(tokens[*i]));
				AssemblerError = 1;
			}

			opcode |= (1 << 3);
			uint8_t *opq = allocate_block(as, OPCODE, 3);
			opq[0] = opcode;
			opq[1] = v & 0xFF;
			opq[2] = (v >> 8) & 0xFF;
		}
		else
		{
			uint8_t *opq = allocate_block(as, OPCODE, 2);
			opcode |= (0 << 3);
			opq[0] = opcode;
			opq[1] = v & 0xFF;
		}
	}
	else if(tokens[*i].type == IDENTIFIER && sxi)
	{
		opcode |= (1 << 3);
		uint8_t *opq = allocate_block(as, OPCODE, 3);
		opq[0] = opcode;

		register_patch(as, tokens[*i], (uint16_t *)(opq + 1));
	}
	else
	{
		printf("Assembler Error %d : Unexpected Argument%s\n",
				tokens[*i].pos, token_to_str(tokens[*i]));
		AssemblerError = 1;
	}
}

void opcode_jmp(assembler_t *as, uint8_t opcode, token_t *tokens, int *i)
{
	if(tokens[*i].type == IDENTIFIER)
	{
		int r = check_if_register(tokens[*i]);

		if(r == -1)
		{
			opcode |= (1 << 3);
			uint8_t *opq = allocate_block(as, OPCODE, 3);
			opq[0] = opcode;

			register_patch(as, tokens[*i], (uint16_t *)(opq + 1));
		}
		else if(r == 2 || r == 3)
		{
			opcode |= (2 << 3);
			opcode |= (r & 1) << 5;

			uint8_t *dat = allocate_block(as, OPCODE, 1);
			dat[0] = opcode;
		}
		else
		{
			printf("Assembler Error %d : Unexpected Arguement%s\n",
					tokens[*i].pos, token_to_str(tokens[*i]));
			AssemblerError = 1;
		}
	}
	else if(tokens[*i].type == NUMBER)
	{
		opcode |= (1 << 3);
		uint16_t value = read_number(tokens[*i].str, tokens[*i].len, 16, *i);
		uint8_t *dat = allocate_block(as, OPCODE, 3);

		dat[0] = opcode;
		dat[1] = value & 0xFF;
		dat[2] = (value >> 8) & 0xFF;
	}
	else
	{
		printf("Assembler Error %d : Invalid Operands%s\n",
				tokens[*i].pos, token_to_str(tokens[*i]));
		AssemblerError = 1;
	}
}

void opcode_mov(assembler_t *as, uint8_t opcode, token_t *tokens, int *i)
{
	int r0 = -1;
	int r1 = -1;
	uint16_t addr;
	uint16_t imm;

	int is_indef = 0;
	token_t id = {0};

	if(tokens[*i].type == LSQUARE)
		++*i;
	else
	{
		printf("Assembler Error %d : Expecting [ %s\n",
				tokens[*i].pos, token_to_str(tokens[*i]));
		AssemblerError = 1;
	}

	r0 = check_if_register(tokens[*i]);
	if(r0 == 2 || r0 == 3)
	{
	}
	else if(tokens[*i].type == NUMBER)
	{
		addr = read_number(tokens[*i].str, tokens[*i].len, 16, *i);
	}
	else if(tokens[*i].type == IDENTIFIER)
	{
		is_indef = 1;
		id = tokens[*i];
	}
	else
	{
		printf("Assembler Error %d : Unexpected Argument%s\n",
				tokens[*i].pos, token_to_str(tokens[*i]));
		AssemblerError = 1;
	}

	++*i;

	if(tokens[*i].type == RSQUARE)
		++*i;
	else
	{
		printf("Assembler Error %d : Expected ] %s\n",
				tokens[*i].pos, token_to_str(tokens[*i]));
		AssemblerError = 1;
	}

	if(tokens[*i].type == COMMA)
		++*i;
	else
	{
		printf("Assembler Error %d : Expected , %s\n",
				tokens[*i].pos, token_to_str(tokens[*i]));
		AssemblerError = 1;
	}

	r1 = check_if_register(tokens[*i]);
	if(r1 == 2 || r1 == 3)
	{
	}
	else if(tokens[*i].type == NUMBER)
	{
		imm = read_number(tokens[*i].str, tokens[*i].len, 8, *i);
	}
	else
	{
		printf("Assembler Error %d : Unexpected Argument%s\n",
				tokens[*i].pos, token_to_str(tokens[*i]));
		AssemblerError = 1;
	}

	if(r0 != -1 && r1 != -1)
	{
		opcode |= (0 << 3);

		opcode |= (r0 & 1) << 5;
		opcode |= (r1 & 1) << 6;

		uint8_t *dat = allocate_block(as, OPCODE, 1);
		dat[0] = opcode;
	}
	else if(r0 != -1 && r1 == -1)
	{
		opcode |= (1 << 3);

		opcode |= (r0 & 1) << 5;

		uint8_t *dat = allocate_block(as, OPCODE, 2);
		dat[0] = opcode;
		dat[1] = imm;
	}
	else if(r0 == -1 && r1 != -1)
	{
		opcode |= (2 << 3);

		opcode |= (r1 & 1) << 5;

		uint8_t *dat = allocate_block(as, OPCODE, 3);
		dat[0] = opcode;

		if(is_indef)
		{
			register_patch(as, id, (uint16_t *)(dat + 1));
		}
		else
		{
			dat[1] = addr & 0xFF;
			dat[2] = (addr >> 8) & 0xFF;
		}

	}
	else if(r0 == -1 && r1 == -1)
	{
		opcode |= (3 << 3);
		uint8_t *dat = allocate_block(as, OPCODE, 4);
		dat[0] = opcode;
		if(is_indef)
		{
			register_patch(as, id, (uint16_t *)(dat + 1));
		}
		else
		{
			dat[1] = addr & 0xFF;
			dat[2] = (addr >> 8) & 0xFF;
		}
		dat[3] = imm;
	}
}

assembler_t assemble_tokens(token_t *tokens, int size)
{
	AssemblerError = 0;
	assembler_t as = {0};

	for(int i = 0; i < size && !AssemblerError; i++)
	{
		switch(tokens[i].type)
		{
			case DOT:
				i++;
				if(tokens[i].type != IDENTIFIER)
				{
					printf("Assembler Error %d : Not a Directive %s\n",
							tokens[i].pos, token_to_str(tokens[i]));
					AssemblerError = 1;
				}

				if(ustrncmp(tokens[i].str, tokens[i].len, "STRING") == 0)
				{
					if(tokens[++i].type != STRING)
					{
						printf("Assembler Error %d : String Expected%s\n",
								tokens[i].pos, token_to_str(tokens[i]));
						AssemblerError = 1;
					}

					uint8_t *dig = allocate_block(&as, DATA, tokens[i].len + 1);
					read_string(dig, tokens[i].str, tokens[i].len);
				}
				else if(ustrncmp(tokens[i].str, tokens[i].len, "WORD") == 0)
				{
					if(tokens[++i].type != NUMBER)
					{
						printf("Assembler Error %d : Number Expected%s\n",
								tokens[i].pos, token_to_str(tokens[i]));
						AssemblerError = 1;
					}

					uint8_t *dig = allocate_block(&as, DATA, 1);
					*dig =  read_number(tokens[i].str, tokens[i].len, 8, tokens[i].pos);
				}
				else if(ustrncmp(tokens[i].str, tokens[i].len, "DWORD") == 0)
				{
					if(tokens[++i].type != NUMBER)
					{
						printf("Assembler Error %d : Number Expected%s\n",
								tokens[i].pos, token_to_str(tokens[i]));
						AssemblerError = 1;
					}

					uint16_t *dig = allocate_block(&as, DATA, 2);
					*dig =  read_number(tokens[i].str, tokens[i].len, 16, tokens[i].pos);
				}
				else if(ustrncmp(tokens[i].str, tokens[i].len, "SIZE") == 0)
				{
					if(tokens[++i].type != NUMBER)
					{
						printf("Assembler Error %d : Number Expected%s\n",
								tokens[i].pos, token_to_str(tokens[i]));
						AssemblerError = 1;
					}

					if(as.size != 0)
					{
						printf("Assembler Error %d : Size already Specified %s\n",
								tokens[i].pos, token_to_str(tokens[i]));
						AssemblerError = 1;
					}

					as.size = read_number(tokens[i].str, tokens[i].len, 16, tokens[i].pos);
				}
				else if(ustrncmp(tokens[i].str, tokens[i].len, "ORG") == 0)
				{
					if(tokens[++i].type != NUMBER)
					{
						printf("Assembler Error %d : Number Expected%s\n",
								tokens[i].pos, token_to_str(tokens[i]));
						AssemblerError = 1;
					}

					uint16_t siz = read_number(tokens[i].str, tokens[i].len, 16, tokens[i].pos);
					allocate_block(&as, ORIGIN, siz);
				}
				else
				{
					printf("Assembler Error %d : Invalid Token %s\n",
							tokens[i].pos, token_to_str(tokens[i]));
					AssemblerError = 1;
				}

				break;

			case IDENTIFIER:
				{
					uint8_t opcode = 0;
					uint8_t cond = 0;

#define CONDUP if(cond) i += 2; else i += 1;

					if(tokens[i+1].type == IDENTIFIER &&
							ustrncmp(tokens[i+1].str, tokens[i+1].len, "COND") == 0)
					{
						cond = 1;
					}

					opcode |= cond << 7;

					if(ustrncmp(tokens[i].str, tokens[i].len, "CLR") == 0)
					{
						CONDUP;

						int r = 0;
						if((r = check_if_register(tokens[i])) == -1)
						{
							printf("Assembler Error %d : Register Expected%s\n",
									tokens[i].pos, token_to_str(tokens[i]));
							AssemblerError = 1;
						}

						if(tokens[i+1].type == COMMA)
						{
							i+=2;

							int r2 = check_if_register(tokens[i]);
							if(r2 >= 2 || r2 == -1)
							{
								//FIXME: Get better error message
								printf("Assembler Error %d : Unexpected Clear Register, Register%s\n",
										tokens[i].pos, token_to_str(tokens[i]));
								AssemblerError = 1;
							}

							if(r + r2 == 1)
								r = 0xF - r2;
						}

						opcode |= r << 3;

						uint8_t *opq = allocate_block(&as, OPCODE, 1);
						*opq = opcode;
					}
					else if(ustrncmp(tokens[i].str, tokens[i].len, "ADD") == 0)
					{
						CONDUP;
						opcode |= 1 << 0;
						opcode_with_arg(&as, opcode, tokens, &i, 1);
					}
					else if(ustrncmp(tokens[i].str, tokens[i].len, "SUB") == 0)
					{
						CONDUP;
						opcode |= 2 << 0;
						opcode_with_arg(&as, opcode, tokens, &i, 1);
					}
					else if(ustrncmp(tokens[i].str, tokens[i].len, "NAND") == 0)
					{
						CONDUP;
						opcode |= 3 << 0;
						opcode_with_arg(&as, opcode, tokens, &i, 1);
					}
					else if(ustrncmp(tokens[i].str, tokens[i].len, "RTR") == 0)
					{
						CONDUP;
						opcode |= 4 << 0;
						opcode_with_arg(&as, opcode, tokens, &i, 0);
					}
					else if(ustrncmp(tokens[i].str, tokens[i].len, "RTL") == 0)
					{
						CONDUP;
						opcode |= 5 << 0;
						opcode_with_arg(&as, opcode, tokens, &i, 0);
					}
					else if(ustrncmp(tokens[i].str, tokens[i].len, "JMP") == 0)
					{
						CONDUP;
						opcode |= 6 << 0;
						opcode_jmp(&as, opcode, tokens, &i);
					}
					else if(ustrncmp(tokens[i].str, tokens[i].len, "MOV") == 0)
					{
						CONDUP;
						opcode |= 7 << 0;
						opcode_mov(&as, opcode, tokens, &i);
					}
					else
					{
						if(tokens[i + 1].type != COLON)
						{
							printf("Assembler Error %d : Unrecognized Token%s\n",
									tokens[i].pos, token_to_str(tokens[i]));
							AssemblerError = 1;
						}
						else
						{
							register_label(&as, tokens[i], as.cursor);
							i++;
						}
					}
				}
				break;

			case END:
				// we done bois
				break;

			default:
				printf("Assembler Error %d : Invalid Token %s\n",
						tokens[i].pos, token_to_str(tokens[i]));
				AssemblerError = 1;
				break;
		}
	}
	
	patch_table_t *p = as.patches;

	while(p)
	{
		*(p->pos) = get_label_value(&as, p->id);
		p = p->next;
	}

	as.cursor = 0;

	return as;
}

void print_assembled(assembler_t *as)
{
	printf("SIZE: %d\n", as->size);

	for(uint32_t i = 0; i < as->num_blocks; i++)
	{
		switch(as->blocks[i].type)
		{
			case ORIGIN:
				printf("ORIGIN: %hx\n", as->blocks[i].size);
				break;

			case DATA:
				printf("DATA: ");
				if(as->num_blocks > 16)
				{
					puts("");
				}

				for(uint32_t j = 0; j < as->blocks[i].size; j += 16)
				{
					for(uint32_t k = 0; k < 16; k++)
					{
						if(j + k >= as->blocks[i].size)
							break;
						printf("%02hhx ", as->blocks[i].data[j + k]);
					}	
					puts("");
				}	

				break;

			case OPCODE:
				printf("OPCODE: ");
				for(uint32_t j = 0; j < as->blocks[i].size; j++)
					printf("%02hhx ", as->blocks[i].data[j]);
				puts("");
				break;
		}
	}
}

void print_patches(assembler_t *as)
{
	patch_table_t *p = as->patches;
	while(p)
	{
		printf("%s %p\n", token_to_str(p->id), p->pos);
		p = p->next;
	}
}

void print_lables(assembler_t *as)
{
	for(int i = 0; i < as->label_count; i++)
	{
		printf("%s %x\n", token_to_str(as->labels[i].id), as->labels[i].value);
	}
}

uint8_t *assembler_to_bin(assembler_t *a)
{
	if(a->size == 0)
	{
		printf("Binary Size Error : check SIZE directive!!\n");
		AssemblerError = 1;
		return NULL;
	}

	uint8_t *data = calloc(sizeof(uint8_t), a->size);
	uint8_t *checker = calloc(sizeof(uint8_t), a->size);

	a->cursor = 0;
	for(uint32_t i = 0; i < a->num_blocks; i++)
	{
		if(checker[a->cursor] == 1)
		{
			printf("Binary Override Error : check ORG directive!!\n");
			AssemblerError = 1;
			break;
		}

		if(a->cursor + a->blocks[i].size >= a->size)
		{
			printf("Binary OutOfMemory Error : check SIZE directive!!\n");
			AssemblerError = 1;
			break;
		}

		if(a->blocks[i].type == ORIGIN)
		{
			a->cursor = a->blocks[i].size;
		}
		else
		{
			memcpy(data + a->cursor, a->blocks[i].data, a->blocks[i].size);
			memset(checker + a->cursor, 1, a->blocks[i].size);

			a->cursor += a->blocks[i].size;
		}
	}

	free(checker);

	return data;
}
