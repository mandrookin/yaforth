#ifndef _EAFORTH_H_
#define _EAFORTH_H_

#ifdef _WIN32
#define  _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string>
#include <stack>
#include <map>
#include <list>
#include <vector>
#include <stdint.h>
#include <malloc.h>


#include "memory.h"

typedef enum {
    builtin,
    user,
    constant,
    variable
} word_type_t;

#endif

typedef enum {
    neutral,
    function_name,
    variable_state,
    constant_value,
    print_string,
    done,
    error,
    finish,
    eol_comment,
    forth_comment,
} state_t;

typedef struct word_record record_t;

typedef state_t(*code_ptr_t)();
typedef void (*genetator_t)(ram_memory*, record_t*); // struct word_record*);

struct word_record {
    word_t          INDEX;
    word_t          LOCAL_INDEX;
    std::string     NAME;
    word_type_t     TYPE;
    union {
        uint32_t        ADDR;
        uint32_t        CONSTANT;
        code_ptr_t      BLTN;
    };
    genetator_t         GENERATE;
};


typedef struct {
    record_t                                function;
    std::vector<record_t>                   code;
    std::map<std::string, uint32_t>         strings;
} function_t;

typedef std::vector<function_t>             function_list_t;


void define_constant(record_t* co);
void define_global_variable(record_t* co);
void define_function(record_t* co);
void register_code(record_t* co);
bool is_compile_mode();
int get_stack_size();
bool try_register_local_string(std::string& name, uint32_t address);
const char* find_variable_by_address(word_t counter);
function_t* lookup_current_function(void);
bool reset_current_function();
function_list_t& get_program();

record_t* find_record(uint32_t h);

typedef std::map<uint32_t, record_t>    registry_t;

state_t forth(const char* str);

extern bool                                ansi_colors;


