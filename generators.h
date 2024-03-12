#ifndef _GENERATORS_H_
#define _GENERATORS_H_

#ifdef _WIN32
#define  _CRT_SECURE_NO_WARNINGS
#endif

//#include "yaforth.h"
#include "memory.h"

//void generate_code(ram_memory& memory, registry_t& words);

void asm_function(ram_memory* mem, struct word_record* rec);

void asm_push_value(ram_memory* mem, struct word_record* rec);
void asm_jump(ram_memory* mem, struct word_record* rec);
void asm_here(ram_memory* mem, struct word_record* rec);
void asm_store(ram_memory* mem, struct word_record* rec);
void asm_fetch(ram_memory* mem, struct word_record* rec);

void asm_add(ram_memory* mem, struct word_record* rec);
void asm_sub(ram_memory* mem, struct word_record* rec);
void asm_mul(ram_memory* mem, struct word_record* rec);
void asm_div(ram_memory* mem, struct word_record* rec);

void asm_check_equ(ram_memory* mem, struct word_record* rec);
void asm_check_notequ(ram_memory* mem, struct word_record* rec);
void asm_check_less(ram_memory* mem, struct word_record* rec);
void asm_check_more(ram_memory* mem, struct word_record* rec);
void asm_and(ram_memory* mem, struct word_record* rec);
void asm_or(ram_memory* mem, struct word_record* rec);
void asm_xor(ram_memory* mem, struct word_record* rec);
void asm_left_shift(ram_memory* mem, struct word_record* rec);
void asm_right_shift(ram_memory* mem, struct word_record* rec);
void asm_not(ram_memory* mem, struct word_record* rec);
void asm_mod(ram_memory* mem, struct word_record* rec);
void asm_divmod(ram_memory* mem, struct word_record* rec);

void asm_dup(ram_memory* mem, struct word_record* rec);

void asm_swap(ram_memory* mem, struct word_record* rec);
void asm_over(ram_memory* mem, struct word_record* rec);
void asm_rot(ram_memory* mem, struct word_record* rec);
void asm_drop(ram_memory* mem, struct word_record* rec);
void asm_dot(ram_memory* mem, struct word_record* rec);
void asm_key(ram_memory* mem, struct word_record* rec);
void asm_emit(ram_memory* mem, struct word_record* rec);
void asm_print_string(ram_memory* mem, struct word_record* rec);
void asm_return(ram_memory* mem, struct word_record* rec);
void asm_if(ram_memory* mem, struct word_record* rec);


void asm_loop(ram_memory* mem, struct word_record* rec);
void asm_endloop(ram_memory* mem, struct word_record* rec);
void asm_endplusloop(ram_memory* mem, struct word_record* rec);
void asm_index_i(ram_memory* mem, struct word_record* rec);
void asm_index_j(ram_memory* mem, struct word_record* rec);
void asm_leave(ram_memory* mem, struct word_record* rec);

void asm_beginloop(ram_memory* mem, struct word_record* rec);
void asm_untilloop(ram_memory* mem, struct word_record* rec);

void asm_to_R(ram_memory* mem, struct word_record* rec);
void asm_from_R(ram_memory* mem, struct word_record* rec);
void asm_fetch_R(ram_memory* mem, struct word_record* rec);

#endif