#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <memory.h>

typedef unsigned int word_t;

bool is_compile_mode();

class ram_memory
{
    word_t          mem[1024 * 10];
    int             counter;
    int             execution_address;
    int             finish;
public:

    word_t  get_current_address()
    {
        return counter;
    }

    word_t get()
    {
        word_t w;
        w = mem[counter++];
        return w;
    }

    word_t get(word_t a)
    {
        // Добавить проверку границ массива
        return mem[a];
    }

    void set(word_t addr, word_t val)
    {
        mem[addr] = val;
    }

    void put(word_t w)
    {
        mem[counter++] = w;
    }

    void jump(word_t addr)
    {
        counter = addr;
    }

    void update_execution_address()
    {
        execution_address = counter;
    }

    word_t get_execution_address()
    {
        return execution_address;
    }

    word_t get_finish()
    {
        return finish;
    }

    bool may_run()
    {
        bool run = (execution_address != counter) && !is_compile_mode();
        finish = counter;
        if (run)
            counter = execution_address;
        return run;
    }

    bool running()
    {
        bool run = counter != finish;
        if (!run)
            counter = execution_address;
        return run;
    }

    ram_memory()
    {
        memset(mem, 0, sizeof(mem));
        counter = 0;
        execution_address = 0;
    }
};



#endif
