#include "yaforth.h"

static bool                         interactive;

#ifdef _WIN32
#define  _CRT_SECURE_NO_WARNINGS

#include <direct.h>
#include <io.h>
#include <Windows.h>
#include <conio.h>

#define isatty _isatty
#define getch _getch

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


state_t init();
void generate_asm_code(const char*);

int intro()
{
    const int max_line_size = 1024;
    char* line = (char*)alloca(max_line_size);
    line[max_line_size - 1] = 0;

    printf("Welcome to Yet Another Forth!\n");
    char* cur_dir = _getcwd(line, max_line_size);
    printf("Current direcory is %s\n", line);
    printf("This is interactive mode.\nType bye and press enter to exit or\nuse Forth syntax>\n");
    return 0;
}

// Do not uncomment
#if VIRTUAL_MACHINE_STUB_AND_TESTS
int * return_next_instruction_pointer()
{
    _asm {
        mov     eax, [ebp+4]
    }
}

void test_write_to_code_segment() {
    int* ptr = return_next_instruction_pointer();
    DWORD old;
    result = VirtualProtect( ptr, 4096, PAGE_EXECUTE_READWRITE, &old);
    *ptr = 0xfeedface;
}
#endif

#define SKIP_PREFIX \
if (wait_prefix) { \
    if (*ptr == -17) \
        ptr += 3; \
    wait_prefix = false; \
}


int main(int argc, char* argv[])
{
    const int max_line_size = 1024;
    FILE* fp = nullptr;
    int result = 0;
    char* line = (char*)alloca(max_line_size);
    line[max_line_size - 1] = 0;

#if _WIN32
    SetConsoleOutputCP(65001);
#endif

    interactive = isatty(0);

    init();

    if (argc == 1) {
        intro();
        fp = stdin;
    }

    if (argc > 1) {
        fp = fopen(argv[1], "rt");
        if (!fp) {
            printf("Unable open file: %s\n", argv[1]);
            exit(-1);
        }
        interactive = false;
    }
    bool wait_prefix = true;
    state_t state = neutral;
    
    while (!feof(fp) && state != finish)
    {
        if (state == error && !interactive)
            break;

        char* ptr = fgets(line, max_line_size - 1, fp);
        if (ptr)
        {
            SKIP_PREFIX
            //if (wait_prefix) {
            //    if (*ptr == -17)
            //        ptr += 3;
            //    wait_prefix = false;
            //}

            state = forth(ptr);
        }
        if (fp == stdin) {
            void show_status(state_t state);
            show_status(state);
        }
    }

    if (!interactive)
        generate_asm_code((const char*)argv[1]);
    return result;
}
