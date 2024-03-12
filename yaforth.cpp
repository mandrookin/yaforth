// yaforth.cpp : This yet another Forth implementation
// 
// With hope that this is useful stuff but without any warrany
//
// This file distributed under Xameleon Greens License
//
//
#ifdef _WIN32
#define  _CRT_SECURE_NO_WARNINGS

#include <direct.h>
#include <io.h>
#include <Windows.h>

#define isatty _isatty

#else

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#define _getcwd getcwd

unsigned char getch()
{
    return getc(stdin);
}


#endif

#include "yaforth.h"
#include "memory.h"
#include "generators.h"

//#define TRACE true

constexpr unsigned int hash(const char* s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
}

static bool                         interactive;
static std::stack<word_t>           integer_stack;
static std::stack<word_t>           return_stack;
static std::stack<word_t>           condition_stack;
static std::vector<word_t>          leave_counts;
static registry_t                   words;
static std::string                  func_name;
static ram_memory                   memory;
static int                          line_no;

state_t     forth(const char* str);
uint32_t    register_word(std::string &word, word_t address);
state_t     register_variable(std::string &word);
state_t     register_constant(std::string& word);
state_t     execute();

bool is_compile_mode()
{
    return !func_name.empty();
}

record_t* find_record(uint32_t h)
{
    record_t* rec_ptr = nullptr;
    if (words.count(h) > 0)
        rec_ptr = &words[h];
    return rec_ptr;
}


#pragma region Snippets



#ifdef _WIN32
word_t getch()
{
    DWORD mode, cc;
    word_t c = 0;
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);

    if (h == NULL) {
        return 0; // console not found
    }

    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    ReadConsole(h, &c, 1, &cc, NULL);
    SetConsoleMode(h, mode);
    return c;
}
#endif

state_t parse_number(std::string& number_str)
{
    word_t n = std::stol(number_str);
    memory.put(hash("^push"));
    memory.put(n);
    number_str.clear();
    return neutral;
}

state_t register_function()
{
    word_t  w = memory.get();
    return neutral;
}

state_t push_value()
{
    word_t  w = memory.get();
    integer_stack.push(w);
#ifdef TRACE
        printf("%d ", w);
#endif
    return neutral;
}

state_t do_here()
{
    word_t  w = memory.get_current_address();
    integer_stack.push(w);
#ifdef TRACE
    printf("%d ", w);
#endif
    return neutral;
}

state_t do_jump()
{
    word_t  w = memory.get();
    memory.jump(w);
    return neutral;
}

state_t to_R()
{
    word_t n = integer_stack.top();
    integer_stack.pop();
    return_stack.push(n);
    return neutral;
}

state_t from_R()
{
    word_t n = return_stack.top();
    return_stack.pop();
    integer_stack.push(n);
    return neutral;
}

state_t fetch_R()
{
    word_t n = return_stack.top();
    integer_stack.push(n);
    return neutral;
}

state_t do_add()
{
    word_t n1, n2;
    n1 = integer_stack.top();
    integer_stack.pop();
    n2 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 + n2);
    return neutral;
}

state_t do_sub()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 - n2);
    return neutral;
}

state_t do_store()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    memory.set(n2, n1);
    return neutral;
}

state_t do_fetch()
{
    word_t n1;
    n1 = integer_stack.top();
    integer_stack.pop();
    n1 = memory.get(n1);
    integer_stack.push(n1);
    return neutral;
}

state_t do_mul()
{
    word_t n1, n2;
    n1 = integer_stack.top();
    integer_stack.pop();
    n2 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 * n2);
    return neutral;
}

state_t do_div()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 / n2);
    return neutral;
}

state_t check_eq()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 == n2 ? -1 : 0);
    return neutral;
}

state_t check_not_eq()
{
    word_t  n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 != n2 ? -1 : 0);
    return neutral;
}

state_t check_less()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 < n2 ? -1 : 0);
    return neutral;
}

state_t check_more()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(signed(n1) > signed(n2) ? -1 : 0);
    return neutral;
}

state_t op_and()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 & n2);
    return neutral;
}

state_t op_or()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 | n2);
    return neutral;
}

state_t op_xor()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 ^ n2);
    return neutral;
}

state_t op_lshift()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 << n2);
    return neutral;
}

state_t op_rshift()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 >> n2);
    return neutral;
}

state_t invert()
{
    word_t n = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(~n);
    return neutral;
}

state_t mod()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 % n2);
    return neutral;
}

state_t divmod()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1 % n2);
    integer_stack.push(n1 / n2);
    return neutral;
}

state_t dup()
{
    word_t n;
    n = integer_stack.top();
    integer_stack.push(n);
    return neutral;
}

state_t swap()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n2);
    integer_stack.push(n1);
    return neutral;
}

state_t over()
{
    word_t n1, n2;
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n1);
    integer_stack.push(n2);
    integer_stack.push(n1);
    return neutral;
}

state_t rot()
{
    word_t n1, n2, n3;
    n3 = integer_stack.top();
    integer_stack.pop();
    n2 = integer_stack.top();
    integer_stack.pop();
    n1 = integer_stack.top();
    integer_stack.pop();
    integer_stack.push(n2);
    integer_stack.push(n3);
    integer_stack.push(n1);
    return neutral;
}

state_t drop()
{
    integer_stack.pop();
    return neutral;
}

state_t do_dot()
{
    if (integer_stack.empty())
    {
        printf("Stack underflow\n");
        return error;;
    }
    printf("%d ", integer_stack.top());
    integer_stack.pop();
    return neutral;
}

state_t get_key()
{
    word_t ch = getch();
    integer_stack.push(ch);
    return neutral;
}

state_t emit()
{
    word_t  ch = integer_stack.top();
    integer_stack.pop();
    putc(ch, stdout);
    return neutral;
}

state_t do_print_string()
{
    word_t a = memory.get_current_address();
    a += 2;
    word_t len = memory.get(a);
    word_t word_count = (len + 3) / sizeof(word_t);

    char* buffer = (char*)alloca((word_count + 1) << 2);
    word_t* slots = (word_t*)buffer;
    int i;
    for (i = 0; i < word_count; i++)
        slots[i] = memory.get(++a);
    slots[i] = 0;
    buffer[len] = 0;
    printf("%s ", buffer);
    fflush(stdout);

    return neutral;
}


#pragma endregion

state_t do_begin()
{
    word_t  a = memory.get_current_address();
    return_stack.push(a);
    return neutral;
}

state_t do_until()
{
    word_t  i, a;

    a = return_stack.top();

    i = integer_stack.top();
    integer_stack.pop();


    if (i == -1 ) {
        return_stack.pop();
        return neutral;
    }
    memory.jump(a);
    return neutral;

}


state_t do_loop()
{
    word_t max, i;

//    a = memory.get_current_address();

    i = integer_stack.top();
    integer_stack.pop();
    max = integer_stack.top();
    integer_stack.pop();

    return_stack.push(max);
    return_stack.push(i);

    return neutral;
}

state_t prepare_enter_loop(word_t h)
{
    word_t a;
    memory.put(h);
    a = memory.get_current_address();
    return_stack.push(a);     // loop start address
    leave_counts.push_back(0);
    return neutral;
}

state_t prepare_loop_exit(word_t h)
{
    word_t ja = memory.get_current_address() + 2;

    // update leave addresses
    int idx = leave_counts.size();
    if (idx > 0) {
        word_t a;
        --idx;
        int count = leave_counts[idx];
        while(count) {
            a = return_stack.top();
            return_stack.pop();
            memory.set(a, ja);
            --count;
        }
        leave_counts.pop_back();
    }

    word_t end;
    word_t enter_loop = return_stack.top();
    return_stack.pop();
    memory.put(h);
    memory.put(enter_loop);

    end = memory.get_current_address();
    if (end != ja) {
        printf("something going wrong in prepare leave\n");
    }
    return neutral;
}

state_t do_leave()
{
    return_stack.pop();
    return_stack.pop();

    word_t ra = memory.get();
    memory.jump(ra);
    return neutral;
}

state_t prepare_leave()
{
    int idx = leave_counts.size();
    if (idx < 1) {
        printf("Misplaced operator 'leave'\n");
        return error;
    }
    idx--;

    word_t n = hash("^leave");
    memory.put(n);
    word_t a = memory.get_current_address();
    memory.put(0);
    return_stack.push(a);
    leave_counts[idx]++;
    return neutral;
}

state_t end_loop()
{
    word_t a, i, max;

    i = return_stack.top();
    return_stack.pop();
    max = return_stack.top();

    i++;

    if (i >= max) {
        return_stack.pop();
        a = memory.get();
        return neutral;
    }
    return_stack.push(i);
    a = memory.get();
    memory.jump(a);
    return neutral;
}

state_t end_plus_loop()
{
    word_t a, i, max, step;

    step = integer_stack.top();
    integer_stack.pop();

    i = return_stack.top();
    return_stack.pop();
    max = return_stack.top();

    i+= step;

    if (i >= max) {
        return_stack.pop();
        a = memory.get();
        return neutral;
    }
    return_stack.push(i);
    a = memory.get();
    memory.jump(a);
    return neutral;
}

state_t index_i()
{
    word_t  i = return_stack.top();
    integer_stack.push(i);
    return neutral;
}

state_t index_j()
{
    word_t  i = return_stack.top();
    integer_stack.push(i);
    return neutral;
}

state_t op_return()
{
    word_t r = return_stack.top();
    return_stack.pop();
    memory.jump(r);
    return neutral;
}

state_t do_condition()
{
    word_t condition = integer_stack.top();
    integer_stack.pop();
    word_t a = memory.get();
    if (condition == 0)
    {
        memory.jump(a);
    }
    return neutral;
}

state_t do_allot()
{
    state_t state;
    // debug
    word_t ca = memory.get_current_address();
    word_t ea = memory.get_execution_address();
    ca = memory.get(ca);
    ea = memory.get(ea);
    record_t* ea_record = &words[ea];
    record_t* ca_record = &words[ca];
    if(memory.may_run())
        state = execute();

    word_t count = integer_stack.top();
    integer_stack.pop();
    if (count & 0x3) {
        printf("TODO: fix variable alignment");
        return error;
    }
    count >>= 2;
    ca = memory.get_current_address() + count;
    memory.jump(ca);
    memory.update_execution_address();
    return neutral;
}

#include <algorithm>
#include <cctype>

state_t check_item(std::string& item, state_t state)
{
    uint32_t n, a, b;
    uint32_t h = hash(item.c_str());

    switch (h)
    {
        case hash(":"):
            state = function_name;
            break;
        case hash(";"):
            memory.put(h);
            func_name.clear();
            memory.update_execution_address();
            state = neutral;
            break;
        case hash("if"):
            memory.put(h);
            n = memory.get_current_address();
            condition_stack.push(n);
            memory.put(0);
            break;
        case hash("else"):
            n = hash("^jump");
            memory.put(n);
            b = memory.get_current_address();
            memory.put(0);
            a = memory.get_current_address();
            n = condition_stack.top();
            condition_stack.pop();
            condition_stack.push(b);
            memory.set(n, a);
            break;
        case hash("then"):
            a = memory.get_current_address();
            n = condition_stack.top();
            condition_stack.pop();
            memory.set(n, a);
            break;
        case hash("do"):
        case hash("begin"):
            prepare_enter_loop(h);
            break;
        case hash ("leave"):
            prepare_leave();
            break;
        case hash("until"):
        case hash("loop"):
        case hash("+loop"):
            prepare_loop_exit(h);
            break;
        case hash("variable"):
            state = variable_state;
            break;
        case hash("constant"):
            state = constant_value;;
            break;
        case hash("allot"):
            state = do_allot();
            break;
        case hash(".\""):
            state = print_string;
            break;
        case hash("\\"):
            state = eol_comment;
            break;
        case hash("("):
            state = forth_comment;
            break;
        case hash("bye"):
            state = finish;
            break;
        default:
        {

            if (words.count(h) == 0)
            {

                std::string lowered = item;
                std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                    [](unsigned char c) { return std::tolower(c); });                
                
                uint32_t lower_h = hash(lowered.c_str());
                if (words.count(lower_h) > 0) {
                    record_t* r = &words[lower_h];
                    if (r->TYPE == builtin)
                    {
                        h = lower_h;
                    }
                    else {
                        printf("Warn: case conversion case word %s is %s\n", lowered.c_str(), item.c_str());
                    }
                }

            }

            if (words.count(h) > 0)
            {
                record_t* r = &words[h];
                switch (r->TYPE) {
                case builtin:
                case user:
                    memory.put(h);
                    break;
                case constant:
                    memory.put(hash("^push"));
                    memory.put(r->CONSTANT);
                    break;
                case variable:
                    memory.put(hash("^push"));
                    memory.put(r->ADDR);
                    break;
                default:
                    return error;
                }
            }
            else
            {
                if (isdigit(item[0])) {
                    state = parse_number(item);
                }
                else {
                    printf("Line: %d Undefined name: %s\n", line_no, item.c_str());
                    state = error;
                }
            }
        }
        break;
    }
    item.clear();
    return state;
}

#define CLEAN_AFTER_RUN

state_t execute()
{
    word_t  r;
#ifdef CLEAN_AFTER_RUN
    word_t start = memory.get_current_address();
#endif
    while (memory.running())
    {
#ifdef TRACE
        a = memory.get_current_address();
#endif
        word_t cmd = memory.get();

        if (words.count(cmd) == 0) {
            printf("\nCommand not implemented\n");
            return error;
        }
        record_t* rec = &words[cmd];
#ifdef TRACE
            printf("\n%08x: %s ", a, rec->NAME.c_str());
#endif
        switch (rec->TYPE)
        {
        case builtin:
            rec->BLTN();
            break;
        case user:
            r = memory.get_current_address();
            return_stack.push(r);
            memory.jump(rec->ADDR);
            break;
        case constant:
            throw "Not implemented";
            break;
        case variable:
            throw "Not implemented";
            break;

        default:
            throw "Non implemented command type";
        }

    }
#ifdef CLEAN_AFTER_RUN
    word_t stop = memory.get_finish();
    for (int i = start; i < stop; i++)
    {
        memory.set(i, 0);
    }
#endif
    return neutral;
}

state_t forth(const char* str)
{
    int ch;
    std::string     buffer;
    int status = 0;
    state_t state = neutral;

    for (; state != error && state != done && state != finish;)
    {
        ch = *str++;
        switch (state)
        {
        case neutral:
            switch (ch)
            {
            case 0:
            case '\n':
            case '\t':
            case ' ':
                if (!buffer.empty())
                    state = check_item(buffer, state);
                if (state == error || state == done)
                    continue;
                if (func_name.empty() && (ch == '\n' || ch == 0)) {
                    if (memory.may_run())
                        state = execute();
                }
                if (ch == 0)
                    state = done;
                continue;

            default:
                buffer += ch;
            }
            continue;

        case eol_comment:
            switch (ch)
            {
            case '\n':
                state = neutral;
                break;
            case 0:
                if (memory.may_run())
                    state = execute();
                if(state != error)
                    state = done;
            }
            continue;

        case forth_comment:
            if (ch == ')')
                state = neutral;
            continue;

        case function_name:
            if (ch != ' ' && ch != '\n') {
                buffer += ch;
            }
            else {
                 func_name = buffer;
                buffer.clear();
                word_t  position = memory.get_current_address();
                uint32_t h = register_word(func_name, position);
                memory.put(hash(":"));
                memory.put(h);
                state = neutral;
            }
            continue;

        case variable_state:
            switch (ch)
            {
            case 0:
            case '\n':
            case ' ':
                if (!buffer.empty())
                    state = register_variable(buffer);
                if (ch == 0)
                    state = done;
                buffer.clear();
                continue;
            default:
                buffer += ch;
            }
            continue;

        case constant_value:
            switch(ch)
            {
            case 0:
            case '\n':
            case ' ':
                if (!buffer.empty())
                    state = register_constant(buffer);
                if (ch == 0)
                    state = done;
                buffer.clear();
                continue;
            default:
                buffer += ch;
            }
            continue;

        case print_string:
            if (ch == '\"') {
                memory.put(hash(".\""));
                memory.put(hash("^jump"));
                word_t a = memory.get_current_address();
                memory.put(0);
                memory.put(buffer.length());
                int wc = (buffer.length() + 3) / 4; // Only for 32-bit system
                word_t* ptr = (word_t*) buffer.c_str();
                for (int i = 0; i < wc; i++)
                    memory.put(*ptr++);
                memory.set(a, memory.get_current_address());
                buffer.clear();
                state = neutral;
                continue;
            }
            buffer += ch;
            continue;
            
//        case finish:
            

        default:
            puts("Unknown state");
            state = error;
            status = -1;
        }
    }
    return state;
}

state_t   register_constant(std::string& word)
{
    state_t state = error;
    uint32_t h = hash(word.c_str());
    record_t    rec;

    if (words.count(h) > 0) {
        printf("Constant %s already defined\n", word.c_str());
        exit(-1);
    }

    if (memory.may_run())
        state = execute();

    rec.INDEX = words.size();
    rec.NAME = word;
    rec.TYPE = constant;
    rec.CONSTANT = integer_stack.top();
    rec.GENERATE = nullptr;
    integer_stack.pop();
    words[h] = rec;
    define_constant(&rec);
    return state;
}

state_t   register_variable(std::string &word)
{
    uint32_t h = hash(word.c_str());
    record_t    rec;

    if (words.count(h) > 0) {
        printf("Variable %s already defined\n", word.c_str());
        exit(-1);
    }

    uint32_t ea = memory.get_execution_address();
    uint32_t a = memory.get_current_address();
    if (ea != a)
    {
        throw "Insert jmp";
        // Это не сработает для allot
        memory.put(hash("^jump"));
        word_t a = memory.get_current_address();
        memory.put(a + 3);
    }
    rec.INDEX = words.size();
    rec.NAME = word;
    rec.TYPE = variable;
    rec.ADDR = memory.get_current_address();
    words[h] = rec;
    memory.put(0);
    memory.update_execution_address();
    define_global_variable(&rec);
    return neutral;
}

uint32_t    register_word(std::string &word, word_t address)
{
    uint32_t h = hash(word.c_str());
    record_t    rec;

    if (words.count(h) > 0) {
        printf("Word %s already defined\n", word.c_str());
        exit(-1);
    }

    rec.INDEX = words.size();
    rec.NAME = word;
    rec.TYPE = user;
    rec.ADDR = address;
    rec.GENERATE = nullptr;
    words[h] = rec;
    define_function(&rec);
    return h;
}

state_t   register_builtin(std::string name, code_ptr_t code, genetator_t generator)
{
    uint32_t h = hash(name.c_str());
    record_t    rec;

    if (words.count(h) > 0) {
        printf("Word %s already defined\n", name.c_str());
        exit(-1);
    }

    rec.NAME = name;
    rec.TYPE = builtin;
    rec.BLTN = code;
    rec.GENERATE = generator;
    words[h] = rec;
    register_code(&rec);
    return neutral;
}

state_t init()
{
#if _WIN32
    SetConsoleOutputCP(65001);
#endif

    register_builtin(":", register_function, asm_function);
    register_builtin("^push", push_value, asm_push_value);
    register_builtin("^jump", do_jump, asm_jump);
    register_builtin("here", do_here, asm_here);
    register_builtin("!", do_store, asm_store);
    register_builtin("@", do_fetch, asm_fetch);
    register_builtin("+", do_add, asm_add);
    register_builtin("-", do_sub, asm_sub);
    register_builtin("*", do_mul, asm_mul);
    register_builtin("/", do_div, asm_div);
    register_builtin("=", check_eq, asm_check_equ);
    register_builtin("<>", check_not_eq, asm_check_notequ);
    register_builtin("<", check_less, asm_check_less);
    register_builtin(">", check_more, asm_check_more);
    register_builtin("and", op_and, asm_and);
    register_builtin("or", op_or, asm_or);
    register_builtin("xor", op_xor, asm_xor);
    register_builtin("invert", invert, asm_not);
    register_builtin("mod", mod, asm_mod);
    register_builtin("/mod", divmod, asm_divmod);
    register_builtin("rshift", op_rshift, asm_right_shift);
    register_builtin("lshift", op_lshift, asm_left_shift);

    register_builtin(">r", to_R, asm_to_R);
    register_builtin("r>", from_R, asm_from_R);
    register_builtin("r@", fetch_R, asm_fetch_R);

    register_builtin("dup", dup, asm_dup);
    register_builtin("drop", drop, asm_drop);
    register_builtin("swap", swap, asm_swap);
    register_builtin("over", over, asm_over);
    register_builtin("rot", rot, asm_rot);
    register_builtin(";", op_return, asm_return);

    register_builtin("if", do_condition, asm_if);

    register_builtin("do", do_loop, asm_loop);
    register_builtin("loop", end_loop, asm_endloop);
    register_builtin("+loop", end_plus_loop, asm_endplusloop);
    register_builtin("^leave", do_leave, asm_leave);
    register_builtin("i", index_i, asm_index_i);
    register_builtin("j", index_j, asm_index_j);

    register_builtin("begin", do_begin, asm_beginloop);
    register_builtin("until", do_until, asm_untilloop);

    register_builtin(".", do_dot, asm_dot);
    register_builtin(".\"", do_print_string, asm_print_string);
    register_builtin("emit", emit, asm_emit);
    register_builtin("key", get_key, asm_key);

    forth(": cr 10 emit ;");
    forth(": cells 4 * ;");
    forth(": ? @ . ;");
    return neutral;
}

//int * return_next_instruction_pointer()
//{
//    _asm {
//        mov     eax, [ebp+4]
//    }
//}

void generate_code(const char *, ram_memory& memory, registry_t& words);

int intro()
{
    const int max_line_size = 1024;
    char* line = (char*) alloca(max_line_size);
    line[max_line_size - 1] = 0;

        printf("Welcome to Yet Another Forth!\n");
        char * cur_dir = _getcwd(line, max_line_size);
        printf("Current direcory is %s\n", line);
        printf("This is interactive mode.\nType bye and press enter to exit or\nuse Forth syntax>\n");
        return 0;
}

int main(int argc, char * argv[])
{
    const int max_line_size = 1024;
    FILE* fp = nullptr;
    int result = 0;
    char* line = (char*) alloca(max_line_size);
    line[max_line_size - 1] = 0;

    interactive =  isatty(0);

    //int* ptr = return_next_instruction_pointer();
    //DWORD old;
    //result = VirtualProtect( ptr, 4096, PAGE_EXECUTE_READWRITE, &old);
    //*ptr = 0xfeedface;

    init();
    if (argc > 1) {
        fp = fopen(argv[1], "rt");
        if (!fp) {
            printf("Unable open file: %s\n", argv[1]);
            exit(-1);
        }
        interactive = false;
    }
    else {
        intro();
        fp = stdin;
    }

    bool wait_prefix = true;
    state_t state = neutral;
    while (!feof(fp) && state != finish )
    {
        if(state == error && !interactive)
        {
            break;
        }
        char * ptr = fgets(line, max_line_size-1, fp);
        if (ptr)
        {
            if (wait_prefix)
            {
                if (*ptr == -17)
                {
                    ptr += 3;
                }
                wait_prefix = false;
            }
            ++line_no;
            state = forth(ptr);
        }
        if (fp == stdin) {
            if(state == error)
                printf("Error\n");
            else if (!integer_stack.empty())
                printf("Ok %ld\n", integer_stack.size());
            else
                printf("Ok\n");
        }
    }

    if(!interactive)
       generate_code((const char *) argv[1], memory, words);
    return result;
}
